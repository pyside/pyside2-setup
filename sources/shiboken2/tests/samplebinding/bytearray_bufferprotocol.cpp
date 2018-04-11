/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#if PY_VERSION_HEX < 0x03000000
// ByteArray buffer protocol functions
// See: http://www.python.org/dev/peps/pep-3118/
extern "C" {
static Py_ssize_t SbkByteArray_segcountproc(PyObject* self, Py_ssize_t* lenp)
{
    if (lenp)
        *lenp = self->ob_type->tp_as_sequence->sq_length(self);
    return 1;
}
static Py_ssize_t SbkByteArray_readbufferproc(PyObject* self, Py_ssize_t segment, void** ptrptr)
{
    if (segment || !Shiboken::Object::isValid(self))
        return -1;

    ByteArray* cppSelf = %CONVERTTOCPP[ByteArray*](self);
    *ptrptr = reinterpret_cast<void*>(const_cast<char*>(cppSelf->data()));
    return cppSelf->size();
}
PyBufferProcs SbkByteArrayBufferProc = {
    /*bf_getreadbuffer*/    &SbkByteArray_readbufferproc,
    /*bf_getwritebuffer*/   (writebufferproc)&SbkByteArray_readbufferproc,
    /*bf_getsegcount*/      &SbkByteArray_segcountproc,
    /*bf_getcharbuffer*/    (charbufferproc)&SbkByteArray_readbufferproc
};
}
#endif
