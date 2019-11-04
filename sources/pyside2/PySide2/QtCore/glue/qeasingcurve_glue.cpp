/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <sbkpython.h>
#include <shiboken.h>
#include <pysideweakref.h>
#include <QEasingCurve>

#include "glue/qeasingcurve_glue.h"

#define MAX_CUSTOM_FUNCTIONS    10

static void deleteData(void *data);

struct CustomFunctionsData
{
    static CustomFunctionsData m_list[MAX_CUSTOM_FUNCTIONS];

    PySideEasingCurveFunctor *m_obj;
    QEasingCurve::EasingFunction m_func;
};

CustomFunctionsData CustomFunctionsData::m_list[MAX_CUSTOM_FUNCTIONS];

template<int N>
struct CustomFunctions
{
    static void init()
    {
        CustomFunctionsData data;
        data.m_obj = 0;
        data.m_func = &CustomFunctions<N>::callback;
        CustomFunctionsData::m_list[N] = data;

        CustomFunctions<N-1>::init();
    }

    static qreal callback(qreal v)
    {
        return (*CustomFunctionsData::m_list[N].m_obj)(v);
    }
};

template<>
struct CustomFunctions<0>
{
    static void init()
    {
        CustomFunctionsData data;
        data.m_obj = 0;
        data.m_func = &CustomFunctions<0>::callback;
        CustomFunctionsData::m_list[0] = data;
    }

    static qreal callback(qreal v)
    {
        return (*CustomFunctionsData::m_list[0].m_obj)(v);
    }
};

void deleteData(void *data)
{
    delete (PySideEasingCurveFunctor *)(data);
}

void PySideEasingCurveFunctor::init()
{
    CustomFunctions<MAX_CUSTOM_FUNCTIONS-1>::init();
}

QEasingCurve::EasingFunction PySideEasingCurveFunctor::createCustomFuntion(PyObject *parent, PyObject *pyFunc)
{
    for(int i=0; i < MAX_CUSTOM_FUNCTIONS; i++) {
        CustomFunctionsData &data = CustomFunctionsData::m_list[i];
        if (data.m_obj == 0) {
            data.m_obj = new PySideEasingCurveFunctor(i, parent, pyFunc);
            return data.m_func;
        }
    }
    //PyErr_Format(PyExc_RuntimeError, "PySide only supports %d custom functions simultaneously.", MAX_CUSTOM_FUNCTIONS);
    return 0;
}

PySideEasingCurveFunctor::~PySideEasingCurveFunctor()
{

    CustomFunctionsData::m_list[m_index].m_obj = 0;
    PyObject_SetAttr(m_parent, Shiboken::PyMagicName::ecf(), Py_None);
}

qreal PySideEasingCurveFunctor::operator()(qreal progress)
{
    Shiboken::GilState state;
    PyObject *args = Py_BuildValue("(f)", progress);
    PyObject *result = PyObject_CallObject(m_func, args);
    qreal cppResult = 0.0;
    if (result) {
        Shiboken::Conversions::pythonToCppCopy(Shiboken::Conversions::PrimitiveTypeConverter<qreal>(), result, &cppResult);
        Py_DECREF(result);
    }
    Py_DECREF(args);
    return cppResult;
}

PyObject *PySideEasingCurveFunctor::callable()
{
    Py_INCREF(m_func);
    return m_func;
}

PyObject *PySideEasingCurveFunctor::callable(PyObject *parent)
{
    return PyObject_GetAttr(parent, Shiboken::PyMagicName::ecf());
}

PySideEasingCurveFunctor::PySideEasingCurveFunctor(int index, PyObject *parent, PyObject *pyFunc)
    : m_parent(parent), m_func(pyFunc), m_index(index)
{
    PyObject_SetAttr(m_parent, Shiboken::PyMagicName::ecf(), m_func);
    PySide::WeakRef::create(m_parent, deleteData, this);
}

