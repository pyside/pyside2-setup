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

#if PY_VERSION_HEX < 0x03000000

// QByteArray buffer protocol functions
// see: http://www.python.org/dev/peps/pep-3118/

extern "C" {

static Py_ssize_t SbkQByteArray_segcountproc(PyObject* self, Py_ssize_t* lenp)
{
    if (lenp)
        *lenp = self->ob_type->tp_as_sequence->sq_length(self);
    return 1;
}

static Py_ssize_t SbkQByteArray_readbufferproc(PyObject* self, Py_ssize_t segment, void** ptrptr)
{
    if (segment || !Shiboken::Object::isValid(self))
        return -1;

    QByteArray* cppSelf = %CONVERTTOCPP[QByteArray*](self);
    *ptrptr = reinterpret_cast<void*>(cppSelf->data());
    return cppSelf->size();
}

PyBufferProcs SbkQByteArrayBufferProc = {
    /*bf_getreadbuffer*/  &SbkQByteArray_readbufferproc,
    /*bf_getwritebuffer*/ (writebufferproc) &SbkQByteArray_readbufferproc,
    /*bf_getsegcount*/    &SbkQByteArray_segcountproc,
    /*bf_getcharbuffer*/  (charbufferproc) &SbkQByteArray_readbufferproc
};

}

#endif
