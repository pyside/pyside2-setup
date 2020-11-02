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

// @snippet qudpsocket-readdatagram
Shiboken::AutoArrayPointer<char> data(%ARGUMENT_NAMES);
QHostAddress ha;
quint16 port;
%BEGIN_ALLOW_THREADS
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(data, %ARGUMENT_NAMES, &ha, &port);
%END_ALLOW_THREADS
QByteArray ba(data, retval);
%PYARG_0 = PyTuple_New(3);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[QByteArray](ba));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QHostAddress](ha));
PyTuple_SET_ITEM(%PYARG_0, 2, %CONVERTTOPYTHON[quint16](port));
// @snippet qudpsocket-readdatagram

// @snippet qipv6address-len
return 16;
// @snippet qipv6address-len

// @snippet qipv6address-getitem
if (_i >= 16) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
}
if (_i < 0)
    _i = 16 - qAbs(_i);

uint item = %CPPSELF.c[_i];
return %CONVERTTOPYTHON[uint](item);
// @snippet qipv6address-getitem

// @snippet qipv6address-setitem
if (_i >= 16) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return -1;
}
if (_i < 0)
    _i = 16 - qAbs(_i);
quint8 item = %CONVERTTOCPP[quint8](_value);
%CPPSELF.c[_i] = item;
return 0;
// @snippet qipv6address-setitem
