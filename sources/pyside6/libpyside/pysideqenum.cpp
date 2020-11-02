/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <shiboken.h>

#include "pysideqenum.h"
#include "dynamicqmetaobject.h"
#include "pyside_p.h"


///////////////////////////////////////////////////////////////
//
// PYSIDE-957: Create QEnum dynamically from Python Enum
//
//
extern "C" {

using namespace Shiboken;

static PyObject *analyzePyEnum(PyObject *pyenum, PyObject *container = nullptr)
{
    /*
     * This is the straight-forward implementation of QEnum/QFlag. It does no
     * longer create an equivalent Qt enum but takes the Python enum as-is.
     *
     * It parses an Enum/Flag derived Python enum completely so that
     * registering can be done without error checks. This would be impossible
     * in MetaObjectBuilderPrivate::parsePythonType.
     */
    AutoDecRef members(PyObject_GetAttr(pyenum, Shiboken::PyMagicName::members()));
    if (members.isNull())
        return nullptr;
    AutoDecRef items(PyMapping_Items(members));
    if (items.isNull())
        return nullptr;
    int iflag = PySide::QEnum::isFlag(pyenum);
    if (iflag < 0)
        return nullptr;
    Py_ssize_t nr_items = PySequence_Length(items);
    if (nr_items < 0)
        return nullptr;

    for (Py_ssize_t idx = 0; idx < nr_items; ++idx) {
        AutoDecRef item(PySequence_GetItem(items, idx));
        if (item.isNull())
            return nullptr;

        // The item should be a 2-element sequence of the key name and an
        // object containing the value.
        AutoDecRef key(PySequence_GetItem(item, 0));
        AutoDecRef member(PySequence_GetItem(item, 1));
        if (key.isNull() || member.isNull())
            return nullptr;
        if (!Shiboken::String::check(key)) {
            // '%.200s' is the safety stringbuffer size of most CPython functions.
            PyErr_Format(PyExc_TypeError,
                "QEnum expected a string mapping as __members__, got '%.200s'",
                Py_TYPE(key)->tp_name);
            return nullptr;
        }

        // Get the value.
        AutoDecRef value(PyObject_GetAttr(member, Shiboken::PyName::value()));
        if (value.isNull())
            return nullptr;
        if (!PyInt_Check(value)) {  // int/long cheating
            PyErr_Format(PyExc_TypeError,
                "QEnum expected an int value as '%.200s', got '%.200s'",
                Shiboken::String::toCString(key), Py_TYPE(value)->tp_name);
            return nullptr;
        }
    }
    Py_RETURN_NONE;
}

static Py_ssize_t get_lineno()
{
    PyObject *frame = reinterpret_cast<PyObject *>(PyEval_GetFrame());  // borrowed ref
    AutoDecRef ob_lineno(PyObject_GetAttr(frame, Shiboken::PyName::f_lineno()));
    if (ob_lineno.isNull() || !PyInt_Check(ob_lineno))  // int/long cheating
        return -1;
    return PyInt_AsSsize_t(ob_lineno);  // int/long cheating
}

static bool is_module_code()
{
    PyObject *frame = reinterpret_cast<PyObject *>(PyEval_GetFrame());  // borrowed ref
    AutoDecRef ob_code(PyObject_GetAttr(frame, Shiboken::PyName::f_code()));
    if (ob_code.isNull())
        return false;
    AutoDecRef ob_name(PyObject_GetAttr(ob_code, Shiboken::PyName::co_name()));
    if (ob_name.isNull())
        return false;
    const char *codename = Shiboken::String::toCString(ob_name);
    return strcmp(codename, "<module>") == 0;
}

} // extern "C"

namespace PySide { namespace QEnum {

static std::map<int, PyObject *> enumCollector;

int isFlag(PyObject *obType)
{
    /*
     * Find out if this is an Enum or a Flag derived class.
     * It checks also if things come from the enum module and if it is
     * an Enum or Flag class at all.
     *
     * The function is called in MetaObjectBuilderPrivate::parsePythonType
     * again to obtain the flag value.
     */
    if (!PyType_Check(obType)) {
        PyErr_Format(PyExc_TypeError, "a class argument was expected, not a '%.200s' instance",
                                      Py_TYPE(obType)->tp_name);
        return -1;
    };
    auto *type = reinterpret_cast<PyTypeObject *>(obType);
    PyObject *mro = type->tp_mro;
    Py_ssize_t i, n = PyTuple_GET_SIZE(mro);
    bool right_module = false;
    bool have_enum = false;
    bool have_flag = false;
    bool have_members = PyObject_HasAttr(obType, PyMagicName::members());
    for (i = 0; i < n; i++) {
        obType = PyTuple_GET_ITEM(mro, i);
        type = reinterpret_cast<PyTypeObject *>(obType);
        AutoDecRef mod(PyObject_GetAttr(obType, PyMagicName::module()));
        QByteArray cmod = String::toCString(mod);
        QByteArray cname = type->tp_name;
        if (cmod == "enum") {
            right_module = true;
            if (cname == "Enum")
                have_enum = true;
            else if (cname == "Flag")
                have_flag = true;
        }
    }
    if (!right_module || !(have_enum || have_flag) || !have_members) {
        PyErr_Format(PyExc_TypeError, "type %.200s does not inherit from 'Enum' or 'Flag'",
                                      type->tp_name);
        return -1;
    }
    return bool(have_flag);
}

PyObject *QEnumMacro(PyObject *pyenum, bool flag)
{
    /*
     * This is the official interface of 'QEnum'. It first calls 'analyzePyEnum'.
     * When called as toplevel enum, it simply returns after some checks.
     * Otherwise, 'pyenum' is stored for later use by the meta class registation.
     */
    int computedFlag = isFlag(pyenum);
    if (computedFlag < 0)
        return nullptr;
    if (bool(computedFlag) != flag) {
        AutoDecRef name(PyObject_GetAttr(pyenum, PyMagicName::qualname()));
        auto cname = String::toCString(name);
        const char *e = "Enum";
        const char *f = "Flag";
        PyErr_Format(PyExc_TypeError, "expected '%s' but got '%s' (%.200s)",
                                      flag ? f : e, flag ? e : f, cname);
        return nullptr;
    }
    auto ok = analyzePyEnum(pyenum);
    if (ok == nullptr)
        return nullptr;
    if (is_module_code()) {
        // This is a toplevel enum which we resolve immediately.
        Py_INCREF(pyenum);
        return pyenum;
    }

    Py_ssize_t lineno = get_lineno();
    if (lineno < 0)
        return nullptr;
    // Handle the rest via line number and the meta class.
    Py_INCREF(pyenum);
    Py_XDECREF(enumCollector[lineno]);
    enumCollector[lineno] = pyenum;
    Py_RETURN_NONE;
}

std::vector<PyObject *> resolveDelayedQEnums(PyTypeObject *containerType)
{
    /*
     * This is the internal interface of 'QEnum'.
     * It is called at the end of the meta class call 'SbkObjectTypeTpNew' via
     * MetaObjectBuilderPrivate::parsePythonType and resolves the collected
     * Python Enum arguments. The result is then registered.
     */
    if (enumCollector.empty())
        return {};
    PyObject *obContainerType = reinterpret_cast<PyObject *>(containerType);
    Py_ssize_t lineno = get_lineno();

    std::vector<PyObject *> result;

    auto it = enumCollector.begin();
    while (it != enumCollector.end()) {
        int nr = it->first;
        PyObject *pyenum = it->second;
        if (nr >= lineno) {
            AutoDecRef name(PyObject_GetAttr(pyenum, PyMagicName::name()));
            if (name.isNull() || PyObject_SetAttr(obContainerType, name, pyenum) < 0)
                return {};
            result.push_back(pyenum);
            it = enumCollector.erase(it);
        } else {
            ++it;
        }
    }
    return result;
}

} // namespace Enum
} // namespace Shiboken

//
///////////////////////////////////////////////////////////////
