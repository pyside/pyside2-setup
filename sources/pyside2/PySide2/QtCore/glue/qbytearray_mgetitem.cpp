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
    Py_ssize_t _i;
    _i = PyNumber_AsSsize_t(_key, PyExc_IndexError);
    if (_i < 0 || _i >= %CPPSELF.size()) {
        PyErr_SetString(PyExc_IndexError, "index out of bounds");
        return 0;
    } else {
        char res[2];
        res[0] = %CPPSELF.at(_i);
        res[1] = 0;
        return PyBytes_FromStringAndSize(res, 1);
    }
} else if (PySlice_Check(_key)) {
    Py_ssize_t start, stop, step, slicelength, cur;

#ifdef IS_PY3K
    PyObject *key = _key;
#else
    PySliceObject *key = reinterpret_cast<PySliceObject *>(_key);
#endif
    if (PySlice_GetIndicesEx(key, %CPPSELF.count(), &start, &stop, &step, &slicelength) < 0) {
        return NULL;
    }

    QByteArray ba;
    if (slicelength <= 0) {
        return %CONVERTTOPYTHON[QByteArray](ba);
    } else if (step == 1) {
        Py_ssize_t max = %CPPSELF.count();
        start = qBound(Py_ssize_t(0), start, max);
        stop = qBound(Py_ssize_t(0), stop, max);
        QByteArray ba;
        if (start < stop)
            ba = %CPPSELF.mid(start, stop - start);
        return %CONVERTTOPYTHON[QByteArray](ba);
    } else {
        QByteArray ba;
        for (cur = start; slicelength > 0; cur += static_cast<size_t>(step), slicelength--) {
            ba.append(%CPPSELF.at(cur));
        }
        return %CONVERTTOPYTHON[QByteArray](ba);
    }
} else {
    PyErr_Format(PyExc_TypeError,
                 "list indices must be integers or slices, not %.200s",
                 PepType_tp_name(_key->ob_type));
    return NULL;
}
