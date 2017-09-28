/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

// Borrowed reference to QtWidgets module
extern PyObject* moduleQtWidgets;

static int QApplicationArgCount;
static char** QApplicationArgValues;
static const char QAPP_MACRO[] = "qApp";

bool QApplicationConstructorStart(PyObject* argv)
{
    if (QApplication::instance()) {
        PyErr_SetString(PyExc_RuntimeError, "A QApplication instance already exists.");
        return false;
    }

    return Shiboken::listToArgcArgv(argv, &QApplicationArgCount, &QApplicationArgValues, "PySideApp");
}

void QApplicationConstructorEnd(PyObject* self)
{
    // Verify if qApp is in main module
    PyObject* globalsDict = PyEval_GetGlobals();
    if (globalsDict) {
        PyObject* qAppObj = PyDict_GetItemString(globalsDict, QAPP_MACRO);
        if (qAppObj)
            PyDict_SetItemString(globalsDict, QAPP_MACRO, self);
    }

    PyObject_SetAttrString(moduleQtWidgets, QAPP_MACRO, self);
    PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    Py_INCREF(self);
}

static void QApplicationConstructor(PyObject* self, PyObject* argv, QApplicationWrapper** cptr)
{
    PyObject *stringlist = PyTuple_GET_ITEM(argv, 0);
    if (QApplicationConstructorStart(stringlist)) {
        *cptr = new QApplicationWrapper(QApplicationArgCount, QApplicationArgValues, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject*>(self));
        QApplicationConstructorEnd(self);
    }
}
