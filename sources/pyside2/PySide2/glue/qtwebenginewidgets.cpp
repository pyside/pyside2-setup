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

// @snippet qwebenginepage-findtext
auto callable = %PYARG_3;
auto callback = [callable](bool found)
{
    if (!PyCallable_Check(callable)) {
        qWarning("Argument 3 of %FUNCTION_NAME must be a callable.");
        return;
    }
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(1));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[bool](found));
    Shiboken::AutoDecRef ret(PyObject_CallObject(callable, arglist));
    Py_DECREF(callable);

};
Py_INCREF(callable);
%CPPSELF.%FUNCTION_NAME(%1, %2, callback);
// @snippet qwebenginepage-findtext

// @snippet qwebenginepage-print
auto printer = %PYARG_1;
auto callable = %PYARG_2;
auto callback = [printer, callable](bool succeeded)
{
    if (!PyCallable_Check(callable)) {
        qWarning("Argument 2 of %FUNCTION_NAME must be a callable.");
        return;
    }
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(1));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[bool](succeeded));
    Shiboken::AutoDecRef ret(PyObject_CallObject(callable, arglist));
    Py_DECREF(callable);
    Py_DECREF(printer);

};
Py_INCREF(printer); // Add a reference to the printer until asynchronous printing has finished
Py_INCREF(callable);
%CPPSELF.%FUNCTION_NAME(%1, callback);
// @snippet qwebenginepage-print

// @snippet qwebenginepage-convertto
auto callable = %PYARG_1;
auto callback = [callable](const QString &text)
{
    if (!PyCallable_Check(callable)) {
        qWarning("Argument 1 of %FUNCTION_NAME must be a callable.");
        return;
    }
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(1));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QString](text));
    Shiboken::AutoDecRef ret(PyObject_CallObject(callable, arglist));
    Py_DECREF(callable);

};
Py_INCREF(callable);
%CPPSELF.%FUNCTION_NAME(callback);
// @snippet qwebenginepage-convertto

// @snippet qwebenginepage-runjavascript
auto callable = %PYARG_3;
auto callback = [callable](const QVariant &result)
{
    if (!PyCallable_Check(callable)) {
        qWarning("Argument 3 of %FUNCTION_NAME must be a callable.");
        return;
    }
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(1));
    switch (result.type()) {
    case QVariant::Bool: {
        const bool value = result.toBool();
        PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QString](value));
    }
        break;
     case QVariant::Int:
     case QVariant::UInt:
     case QVariant::LongLong:
     case QVariant::ULongLong:
     case QVariant::Double: {
        const double number = result.toDouble();
        PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[double](number));
    }
        break;
    default: {
        const QString value = result.toString();
        PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QString](value));
    }
        break;
    }
   // PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[bool](found));
    Shiboken::AutoDecRef ret(PyObject_CallObject(callable, arglist));
    Py_DECREF(callable);

};
Py_INCREF(callable);
%CPPSELF.%FUNCTION_NAME(%1, %2, callback);
// @snippet qwebenginepage-runjavascript

// @snippet qwebenginepage-printtopdf
auto callable = %PYARG_1;
auto callback = [callable](const QByteArray &pdf)
{
    if (!PyCallable_Check(callable)) {
        qWarning("Argument 1 of %FUNCTION_NAME must be a callable.");
        return;
    }
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(1));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QByteArray](pdf));
    Shiboken::AutoDecRef ret(PyObject_CallObject(callable, arglist));
    Py_DECREF(callable);

};
Py_INCREF(callable);
%CPPSELF.%FUNCTION_NAME(callback, %2);
// @snippet qwebenginepage-printtopdf
