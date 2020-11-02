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

// @snippet qglbuffer-allocate
int size = (%2 < 0) ? %1.size() : %2;
%CPPSELF.allocate(static_cast<const void *>(%1.data()), size);
// @snippet qglbuffer-allocate

// @snippet qglbuffer-read
char *data = new char[%3];
bool result = %CPPSELF.read(%1, data, %3);
QByteArray ret;
if (result)
    ret.append(data, %3);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[bool](result));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QByteArray](ret));
delete[] data;
// @snippet qglbuffer-read

// @snippet qglbuffer-write
int size = (%3 < 0) ? %2.size() : %3;
%CPPSELF.write(%1, static_cast<const void *>(%2.data()), size);
// @snippet qglbuffer-write

// @snippet qglbuffer-map
Py_ssize_t dataSize = %CPPSELF.size();
void *data = %CPPSELF.map(%1);

if (!data) {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
} else if (%1 == QGLBuffer::ReadOnly) {
    %PYARG_0 = Shiboken::Buffer::newObject(data, dataSize, Shiboken::Buffer::ReadOnly);
} else {
    %PYARG_0 = Shiboken::Buffer::newObject(data, dataSize, Shiboken::Buffer::ReadWrite);
}
// @snippet qglbuffer-map
