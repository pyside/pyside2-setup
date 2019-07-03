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

/*****************************************************************************
 *
 * Copied from abstract.c
 *
 * Py_buffer has been replaced by Pep_buffer
 *
 */

#ifdef Py_LIMITED_API

#include "pep384impl.h"
/* Buffer C-API for Python 3.0 */

int
PyObject_GetBuffer(PyObject *obj, Pep_buffer *view, int flags)
{
    PyBufferProcs *pb = PepType_AS_BUFFER(Py_TYPE(obj));

    if (pb == NULL || pb->bf_getbuffer == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "a bytes-like object is required, not '%.100s'",
                     Py_TYPE(obj)->tp_name);
        return -1;
    }
    return (*pb->bf_getbuffer)(obj, view, flags);
}

static int
_IsFortranContiguous(const Pep_buffer *view)
{
    Py_ssize_t sd, dim;
    int i;

    /* 1) len = product(shape) * itemsize
       2) itemsize > 0
       3) len = 0 <==> exists i: shape[i] = 0 */
    if (view->len == 0) return 1;
    if (view->strides == NULL) {  /* C-contiguous by definition */
        /* Trivially F-contiguous */
        if (view->ndim <= 1) return 1;

        /* ndim > 1 implies shape != NULL */
        assert(view->shape != NULL);

        /* Effectively 1-d */
        sd = 0;
        for (i=0; i<view->ndim; i++) {
            if (view->shape[i] > 1) sd += 1;
        }
        return sd <= 1;
    }

    /* strides != NULL implies both of these */
    assert(view->ndim > 0);
    assert(view->shape != NULL);

    sd = view->itemsize;
    for (i=0; i<view->ndim; i++) {
        dim = view->shape[i];
        if (dim > 1 && view->strides[i] != sd) {
            return 0;
        }
        sd *= dim;
    }
    return 1;
}

static int
_IsCContiguous(const Pep_buffer *view)
{
    Py_ssize_t sd, dim;
    int i;

    /* 1) len = product(shape) * itemsize
       2) itemsize > 0
       3) len = 0 <==> exists i: shape[i] = 0 */
    if (view->len == 0) return 1;
    if (view->strides == NULL) return 1; /* C-contiguous by definition */

    /* strides != NULL implies both of these */
    assert(view->ndim > 0);
    assert(view->shape != NULL);

    sd = view->itemsize;
    for (i=view->ndim-1; i>=0; i--) {
        dim = view->shape[i];
        if (dim > 1 && view->strides[i] != sd) {
            return 0;
        }
        sd *= dim;
    }
    return 1;
}

int
PyBuffer_IsContiguous(const Pep_buffer *view, char order)
{

    if (view->suboffsets != NULL) return 0;

    if (order == 'C')
        return _IsCContiguous(view);
    else if (order == 'F')
        return _IsFortranContiguous(view);
    else if (order == 'A')
        return (_IsCContiguous(view) || _IsFortranContiguous(view));
    return 0;
}


void *
PyBuffer_GetPointer(Pep_buffer *view, Py_ssize_t *indices)
{
    int i;
    auto pointer = reinterpret_cast<char *>(view->buf);
    for (i = 0; i < view->ndim; i++) {
        pointer += view->strides[i]*indices[i];
        if ((view->suboffsets != NULL) && (view->suboffsets[i] >= 0)) {
            pointer = *reinterpret_cast<char **>(pointer) + view->suboffsets[i];
        }
    }
    return pointer;
}


void
_Py_add_one_to_index_F(int nd, Py_ssize_t *index, const Py_ssize_t *shape)
{
    int k;

    for (k=0; k<nd; k++) {
        if (index[k] < shape[k]-1) {
            index[k]++;
            break;
        }
        else {
            index[k] = 0;
        }
    }
}

void
_Py_add_one_to_index_C(int nd, Py_ssize_t *index, const Py_ssize_t *shape)
{
    int k;

    for (k=nd-1; k>=0; k--) {
        if (index[k] < shape[k]-1) {
            index[k]++;
            break;
        }
        else {
            index[k] = 0;
        }
    }
}

int
PyBuffer_FromContiguous(Pep_buffer *view, void *buf, Py_ssize_t len, char fort)
{
    int k;
    void (*addone)(int, Py_ssize_t *, const Py_ssize_t *);
    Py_ssize_t *indices, elements;
    char *src, *ptr;

    if (len > view->len) {
        len = view->len;
    }

    if (PyBuffer_IsContiguous(view, fort)) {
        /* simplest copy is all that is needed */
        memcpy(view->buf, buf, len);
        return 0;
    }

    /* Otherwise a more elaborate scheme is needed */

    /* view->ndim <= 64 */
    indices = (Py_ssize_t *)PyMem_Malloc(sizeof(Py_ssize_t)*(view->ndim));
    if (indices == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    for (k=0; k<view->ndim; k++) {
        indices[k] = 0;
    }

    if (fort == 'F') {
        addone = _Py_add_one_to_index_F;
    }
    else {
        addone = _Py_add_one_to_index_C;
    }
    src = (char *)buf; // patched by CT
    /* XXX : This is not going to be the fastest code in the world
             several optimizations are possible.
     */
    elements = len / view->itemsize;
    while (elements--) {
        ptr = (char *)PyBuffer_GetPointer(view, indices); // patched by CT
        memcpy(ptr, src, view->itemsize);
        src += view->itemsize;
        addone(view->ndim, indices, view->shape);
    }

    PyMem_Free(indices);
    return 0;
}

int PyObject_CopyData(PyObject *dest, PyObject *src)
{
    Pep_buffer view_dest, view_src;
    int k;
    Py_ssize_t *indices, elements;
    char *dptr, *sptr;

    if (!PyObject_CheckBuffer(dest) ||
        !PyObject_CheckBuffer(src)) {
        PyErr_SetString(PyExc_TypeError,
                        "both destination and source must be "\
                        "bytes-like objects");
        return -1;
    }

    if (PyObject_GetBuffer(dest, &view_dest, PyBUF_FULL) != 0) return -1;
    if (PyObject_GetBuffer(src, &view_src, PyBUF_FULL_RO) != 0) {
        PyBuffer_Release(&view_dest);
        return -1;
    }

    if (view_dest.len < view_src.len) {
        PyErr_SetString(PyExc_BufferError,
                        "destination is too small to receive data from source");
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return -1;
    }

    if ((PyBuffer_IsContiguous(&view_dest, 'C') &&
         PyBuffer_IsContiguous(&view_src, 'C')) ||
        (PyBuffer_IsContiguous(&view_dest, 'F') &&
         PyBuffer_IsContiguous(&view_src, 'F'))) {
        /* simplest copy is all that is needed */
        memcpy(view_dest.buf, view_src.buf, view_src.len);
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return 0;
    }

    /* Otherwise a more elaborate copy scheme is needed */

    /* XXX(nnorwitz): need to check for overflow! */
    indices = (Py_ssize_t *)PyMem_Malloc(sizeof(Py_ssize_t)*view_src.ndim);
    if (indices == NULL) {
        PyErr_NoMemory();
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return -1;
    }
    for (k=0; k<view_src.ndim;k++) {
        indices[k] = 0;
    }
    elements = 1;
    for (k=0; k<view_src.ndim; k++) {
        /* XXX(nnorwitz): can this overflow? */
        elements *= view_src.shape[k];
    }
    while (elements--) {
        _Py_add_one_to_index_C(view_src.ndim, indices, view_src.shape);
        dptr = (char *)PyBuffer_GetPointer(&view_dest, indices); // patched by CT
        sptr = (char *)PyBuffer_GetPointer(&view_src, indices); // patched by CT
        memcpy(dptr, sptr, view_src.itemsize);
    }
    PyMem_Free(indices);
    PyBuffer_Release(&view_dest);
    PyBuffer_Release(&view_src);
    return 0;
}

void
PyBuffer_FillContiguousStrides(int nd, Py_ssize_t *shape,
                               Py_ssize_t *strides, int itemsize,
                               char fort)
{
    int k;
    Py_ssize_t sd;

    sd = itemsize;
    if (fort == 'F') {
        for (k=0; k<nd; k++) {
            strides[k] = sd;
            sd *= shape[k];
        }
    }
    else {
        for (k=nd-1; k>=0; k--) {
            strides[k] = sd;
            sd *= shape[k];
        }
    }
    return;
}

int
PyBuffer_FillInfo(Pep_buffer *view, PyObject *obj, void *buf, Py_ssize_t len,
                  int readonly, int flags)
{
    if (view == NULL) {
        PyErr_SetString(PyExc_BufferError,
            "PyBuffer_FillInfo: view==NULL argument is obsolete");
        return -1;
    }

    if (((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) &&
        (readonly == 1)) {
        PyErr_SetString(PyExc_BufferError,
                        "Object is not writable.");
        return -1;
    }

    view->obj = obj;
    if (obj)
        Py_INCREF(obj);
    view->buf = buf;
    view->len = len;
    view->readonly = readonly;
    view->itemsize = 1;
    view->format = NULL;
    if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT)
        view->format = (char *)"B"; // patched by CT
    view->ndim = 1;
    view->shape = NULL;
    if ((flags & PyBUF_ND) == PyBUF_ND)
        view->shape = &(view->len);
    view->strides = NULL;
    if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES)
        view->strides = &(view->itemsize);
    view->suboffsets = NULL;
    view->internal = NULL;
    return 0;
}

void
PyBuffer_Release(Pep_buffer *view)
{
    PyObject *obj = view->obj;
    PyBufferProcs *pb;
    if (obj == NULL)
        return;
    pb = PepType_AS_BUFFER(Py_TYPE(obj));
    if (pb && pb->bf_releasebuffer)
        pb->bf_releasebuffer(obj, view);
    view->obj = NULL;
    Py_DECREF(obj);
}

#endif // Py_LIMITED_API
