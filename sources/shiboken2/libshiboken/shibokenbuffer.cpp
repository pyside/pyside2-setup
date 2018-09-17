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

#include "shibokenbuffer.h"
#include <cstdlib>
#include <cstring>

bool Shiboken::Buffer::checkType(PyObject* pyObj)
{
    return PyObject_CheckReadBuffer(pyObj) != 0;
}

void* Shiboken::Buffer::getPointer(PyObject* pyObj, Py_ssize_t* size)
{
    const void* buffer = 0;
#ifdef IS_PY3K
    Py_buffer view;
    if (PyObject_GetBuffer(pyObj, &view, PyBUF_ND) == 0) {
        if (size)
            *size = view.len;
        buffer = view.buf;
        PyBuffer_Release(&view);
        return view.buf;
    }
#else
    Py_ssize_t bufferSize = 0;

    PyObject_AsReadBuffer(pyObj, &buffer, &bufferSize);

    if (size)
        *size = bufferSize;
#endif
    return const_cast<void*>(buffer);
}

PyObject* Shiboken::Buffer::newObject(void* memory, Py_ssize_t size, Type type)
{
    if (size == 0)
        Py_RETURN_NONE;
#ifdef IS_PY3K
    Py_buffer view;
    memset(&view, 0, sizeof(Py_buffer));
    view.buf = memory;
    view.len = size;
    view.readonly = type == Shiboken::Buffer::ReadOnly;
    view.ndim = 1;
    view.itemsize = sizeof(char);
    Py_ssize_t shape[] = { size };
    view.shape = shape;
    // Pep384: This is way too complicated and impossible with the limited api:
    //return PyMemoryView_FromBuffer(&view);
    return PyMemoryView_FromMemory((char *)view.buf, size, type == ReadOnly ? PyBUF_READ : PyBUF_WRITE);
#else
    return type == ReadOnly ? PyBuffer_FromMemory(memory, size) : PyBuffer_FromReadWriteMemory(memory, size);
#endif
}

PyObject* Shiboken::Buffer::newObject(const void* memory, Py_ssize_t size)
{
    return newObject(const_cast<void*>(memory), size, ReadOnly);
}
