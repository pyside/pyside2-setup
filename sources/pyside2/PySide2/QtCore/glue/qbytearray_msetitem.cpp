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

if (PyIndex_Check(_key)) {
    Py_ssize_t _i = PyNumber_AsSsize_t(_key, PyExc_IndexError);
    if (_i == -1 && PyErr_Occurred())
        return -1;

    if (_i < 0)
        _i += %CPPSELF.count();

    if (_i < 0 || _i >= %CPPSELF.size()) {
        PyErr_SetString(PyExc_IndexError, "QByteArray index out of range");
        return -1;
    }

    // Provide more specific error message for bytes/str, bytearray, QByteArray respectively
#ifdef IS_PY3K
    if (PyBytes_Check(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "bytes must be of size 1");
#else
    if (PyString_CheckExact(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "str must be of size 1");
#endif
            return -1;
        }
    } else if (PyByteArray_Check(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "bytearray must be of size 1");
            return -1;
        }
    } else if (PepType(Py_TYPE(_value)) == PepType(SbkPySide2_QtCoreTypes[SBK_QBYTEARRAY_IDX])) {
        if (PyObject_Length(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "QByteArray must be of size 1");
            return -1;
        }
    } else {
#ifdef IS_PY3K
        PyErr_SetString(PyExc_ValueError, "a bytes, bytearray, QByteArray of size 1 is required");
#else
        PyErr_SetString(PyExc_ValueError, "a str, bytearray, QByteArray of size 1 is required");
#endif
        return -1;
    }

    // Not support int or long.
    %CPPSELF.remove(_i, 1);
    PyObject *args = Py_BuildValue("(nO)", _i, _value);
    PyObject *result = Sbk_QByteArrayFunc_insert(self, args);
    Py_DECREF(args);
    Py_XDECREF(result);
    return !result ? -1 : 0;
} else if (PySlice_Check(_key)) {
    Py_ssize_t start, stop, step, slicelength, value_length;

#ifdef IS_PY3K
    PyObject *key = _key;
#else
    PySliceObject *key = reinterpret_cast<PySliceObject *>(_key);
#endif
    if (PySlice_GetIndicesEx(key, %CPPSELF.count(), &start, &stop, &step, &slicelength) < 0) {
        return -1;
    }
    // The parameter candidates are: bytes/str, bytearray, QByteArray itself.
    // Not support iterable which contains ints between 0~255

    // case 1: value is NULL, means delete the items within the range
    // case 2: step is 1, means shrink or expanse
    // case 3: step is not 1, then the number of slots have to equal the number of items in _value
    QByteArray ba;
    if (_value == NULL || _value == Py_None) {
        ba = QByteArray();
        value_length = 0;
    } else if (!(PyBytes_Check(_value) || PyByteArray_Check(_value) || PepType(Py_TYPE(_value)) == PepType(SbkPySide2_QtCoreTypes[SBK_QBYTEARRAY_IDX]))) {
        PyErr_Format(PyExc_TypeError, "bytes, bytearray or QByteArray is required, not %.200s", PepType(Py_TYPE(_value))->tp_name);
        return -1;
    } else {
        value_length = PyObject_Length(_value);
    }

    if (step != 1 && value_length != slicelength) {
        PyErr_Format(PyExc_ValueError, "attempt to assign %s of size %d to extended slice of size %d",PepType(Py_TYPE(_value))->tp_name, value_length, slicelength);
        return -1;
    }

    if (step != 1) {
        int i = start;
        for (int j = 0; j < slicelength; j++) {
            PyObject *item = PyObject_GetItem(_value, PyLong_FromLong(j));
            QByteArray temp;
#ifdef IS_PY3K
            if (PyLong_Check(item)) {
#else
            if (PyLong_Check(item) || PyInt_Check(item)) {
#endif
                int overflow;
                long ival = PyLong_AsLongAndOverflow(item, &overflow);
                // Not suppose to bigger than 255 because only bytes, bytearray, QByteArray were accept
                const char *el = reinterpret_cast<const char*>(&ival);
                temp = QByteArray(el);
            } else {
                temp = %CONVERTTOCPP[QByteArray](item);
            }

            %CPPSELF.replace(i, 1, temp);
            i += step;
        }
        return 0;
    } else {
        ba = %CONVERTTOCPP[QByteArray](_value);
        %CPPSELF.replace(start, slicelength, ba);
        return 0;
    }
} else {
    PyErr_Format(PyExc_TypeError, "QBytearray indices must be integers or slices, not %.200s",
                  PepType(Py_TYPE(_key))->tp_name);
    return -1;
}


