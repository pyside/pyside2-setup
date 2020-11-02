/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

// @snippet qscriptvalue-repr
if (%CPPSELF.isVariant() || %CPPSELF.isString()) {
    QString format = QString::asprintf("%s(\"%s\")",
            Py_TYPE(%PYSELF)->tp_name,
            qPrintable(%CPPSELF.toString()));
    %PYARG_0 = Shiboken::String::fromCString(qPrintable(format));
 } else {
    %PYARG_0 = Shiboken::String::fromCString(Py_TYPE(%PYSELF)->tp_name);
}
// @snippet qscriptvalue-repr

// @snippet qscriptvalue-mgetitem
Shiboken::AutoDecRef key(PyObject_Str(_key));
QVariant res = %CPPSELF.property(Shiboken::String::toCString(key.object())).toVariant();
if (res.isValid()) {
    return %CONVERTTOPYTHON[QVariant](res);
} else {
    PyObject *errorType = PyInt_Check(_key) ? PyExc_IndexError : PyExc_KeyError;
    PyErr_SetString(errorType, "Key not found.");
    return 0;
}
// @snippet qscriptvalue-mgetitem

// @snippet qscriptvalueiterator-next
if (%CPPSELF.hasNext()) {
    %CPPSELF.next();
    QString name = %CPPSELF.name();
    QVariant value = %CPPSELF.value().toVariant();
    %PYARG_0 = PyTuple_New(2);
    PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[QString](name));
    PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QVariant](value));
} else {
    PyErr_SetNone(PyExc_StopIteration);
}
// @snippet qscriptvalueiterator-next
