/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

#include "sbkarrayconverter.h"
#include "helper.h"
#include "sbkconverter.h"
#include "sbkconverter_p.h"
#include "sbkarrayconverter_p.h"

#include <numpy/arrayobject.h>

#include <algorithm>
#include <iostream>
#include <cstdint>

enum { debugNumPy = 0 };

struct TypeCharMapping
{
    NPY_TYPES type;
    const char *name;
};

static const TypeCharMapping typeCharMappings[] = {
{NPY_BYTE, "NPY_BYTE"},
{NPY_UBYTE, "NPY_UBYTE"},
{NPY_SHORT, "NPY_SHORT"},
{NPY_USHORT, "NPY_USHORT"},
{NPY_INT, "NPY_INT"},
{NPY_UINT, "NPY_UINT"},
{NPY_LONG, "NPY_LONG"},
{NPY_ULONG, "NPY_ULONG"},
{NPY_LONGLONG, "NPY_LONGLONG"},
{NPY_ULONGLONG, "NPY_ULONGLONG"},
{NPY_FLOAT, "NPY_FLOAT"},
{NPY_DOUBLE, "NPY_DOUBLE"}
};

const char *npTypeName(npy_intp t)
{
    const TypeCharMapping *end = typeCharMappings + sizeof(typeCharMappings) / sizeof(typeCharMappings[0]);
    const TypeCharMapping *result =
        std::find_if(typeCharMappings, end,
                     [t] (const TypeCharMapping &m) { return m.type == t; });
    return result != end ? result->name : nullptr;
}

std::ostream &operator<<(std::ostream &str, PyArrayObject *o)
{
    str << "PyArrayObject(";
    if (o) {
        const npy_intp npType = PyArray_TYPE(o);
        if (const char *name = npTypeName(npType))
            str << name;
        else
            str << "type=" << npType;
        const int nDim = PyArray_NDIM(o);
        const npy_intp *dims = PyArray_DIMS(o);
        for (int d = 0; d < nDim; ++d)
            str << '[' << dims[d] << ']';
        str << ", ";
        const int flags = PyArray_FLAGS(o);
        if ((flags & NPY_ARRAY_C_CONTIGUOUS) != 0)
            str << " NPY_ARRAY_C_CONTIGUOUS";
        if ((flags & NPY_ARRAY_F_CONTIGUOUS) != 0)
            str << " NPY_ARRAY_F_CONTIGUOUS";
        if ((flags & NPY_ARRAY_OWNDATA) != 0)
            str << " NPY_ARRAY_OWNDATA";
        if ((flags & NPY_ARRAY_FORCECAST) != 0)
            str << " NPY_ARRAY_FORCECAST";
        if ((flags & NPY_ARRAY_ENSURECOPY) != 0)
            str << " NPY_ARRAY_ENSURECOPY";
        if ((flags & NPY_ARRAY_ENSUREARRAY) != 0)
            str << " NPY_ARRAY_ENSUREARRAY";
        if ((flags & NPY_ARRAY_ELEMENTSTRIDES) != 0)
            str << " NPY_ARRAY_ELEMENTSTRIDES";
        if ((flags & NPY_ARRAY_ALIGNED) != 0)
            str << " NPY_ARRAY_ALIGNED";
        if ((flags & NPY_ARRAY_NOTSWAPPED) != 0)
            str << " NPY_ARRAY_NOTSWAPPED";
        if ((flags & NPY_ARRAY_WRITEABLE) != 0)
            str << " NPY_ARRAY_WRITEABLE";
        if ((flags & NPY_ARRAY_UPDATEIFCOPY) != 0)
            str << " NPY_ARRAY_UPDATEIFCOPY";
    } else {
        str << '0';
    }
    str << ')';
    return str;
}

namespace Shiboken {
namespace Conversions {

// Internals from sbkarrayconverter.cpp
SbkArrayConverter *createArrayConverter(IsArrayConvertibleToCppFunc toCppCheckFunc);
void setArrayTypeConverter(int index, int dimension, SbkArrayConverter *c);
SbkArrayConverter *unimplementedArrayConverter();

template <int dimension>
static bool isPrimitiveArray(PyObject *pyIn, int expectedNpType)
{
    if (!PyArray_Check(pyIn))
        return false;
    PyArrayObject *pya = reinterpret_cast<PyArrayObject *>(pyIn);
    if (debugNumPy) {
        std::cerr << __FUNCTION__ << "(expectedNpType=" << expectedNpType;
        if (const char *name = npTypeName(expectedNpType))
            std::cerr << " (" << name << ')';
        std::cerr << ' ' << pya << '\n';
    }

    const int dim = PyArray_NDIM(pya);
    if (dim != dimension) {
        warning(PyExc_RuntimeWarning, 0,
                "%d dimensional numpy array passed to a function expecting a %d dimensional array.",
                dim, dimension);
        return false;
    }
    if ((PyArray_FLAGS(pya) & NPY_ARRAY_C_CONTIGUOUS) == 0) {
        warning(PyExc_RuntimeWarning, 0,
                "Cannot handle numpy arrays that do not have NPY_ARRAY_C_CONTIGUOUS set.");
        return false;
    }
    const int actualNpType = PyArray_TYPE(pya);
    if (actualNpType != expectedNpType) {
        const char *actualName = npTypeName(actualNpType);
        const char *expectedName = npTypeName(expectedNpType);
        warning(PyExc_RuntimeWarning, 0,
                "A numpy array of type %d (%s) was passed to a function expecting type %d (%s).",
                actualNpType, actualName ? actualName : "",
                expectedNpType, expectedName ? expectedName : "");
        return false;
    }
    return true;
}

static inline bool primitiveArrayCheck1(PyObject *pyIn, int expectedNpType, int expectedSize)
{
    if (!isPrimitiveArray<1>(pyIn, expectedNpType))
        return false;
    if (expectedSize >= 0) {
        PyArrayObject *pya = reinterpret_cast<PyArrayObject *>(pyIn);
        const int size = int(PyArray_DIMS(pya)[0]);
        if (size < expectedSize) {
            warning(PyExc_RuntimeWarning, 0, "A numpy array of size %d was passed to a function expects %d.",
                    size, expectedSize);
            return false;
        }
    }
    return true;
}

// Convert one-dimensional array
template <class T>
static void convertArray1(PyObject *pyIn, void *cppOut)
{
    ArrayHandle<T> *handle = reinterpret_cast<ArrayHandle<T> *>(cppOut);
    PyArrayObject *pya = reinterpret_cast<PyArrayObject *>(pyIn);
    const npy_intp size = PyArray_DIMS(pya)[0];
    if (debugNumPy)
        std::cerr << __FUNCTION__ << ' ' << size << '\n';
    handle->setData(reinterpret_cast<T *>(PyArray_DATA(pya)), size_t(size));
}

// Convert 2 dimensional array
template <class T>
static void convertArray2(PyObject *pyIn, void *cppOut)
{
    typedef typename Array2Handle<T, 1>::RowType RowType;
    Array2Handle<T, 1> *handle = reinterpret_cast<Array2Handle<T, 1> *>(cppOut);
    PyArrayObject *pya = reinterpret_cast<PyArrayObject *>(pyIn);
    handle->setData(reinterpret_cast<RowType *>(PyArray_DATA(pya)));
}

template <class T, int NumPyType>
static PythonToCppFunc checkArray1(PyObject *pyIn, int dim1, int /* dim2 */)
{
    return primitiveArrayCheck1(pyIn, NumPyType, dim1) ? convertArray1<T> : nullptr;
}

static inline bool primitiveArrayCheck2(PyObject *pyIn, int expectedNpType, int expectedDim1, int expectedDim2)
{
    if (!isPrimitiveArray<2>(pyIn, expectedNpType))
        return false;
    if (expectedDim2 >= 0) {
        PyArrayObject *pya = reinterpret_cast<PyArrayObject *>(pyIn);
        const int dim1 = int(PyArray_DIMS(pya)[0]);
        const int dim2 = int(PyArray_DIMS(pya)[1]);
        if (dim1 != expectedDim1 || dim2 != expectedDim2) {
            warning(PyExc_RuntimeWarning, 0, "A numpy array of size %dx%d was passed to a function that expects %dx%d.",
                    dim1, dim2, expectedDim1, expectedDim2);
            return false;
        }
    }
    return true;
}

template <class T, int NumPyType>
static PythonToCppFunc checkArray2(PyObject *pyIn, int dim1, int dim2)
{
    return primitiveArrayCheck2(pyIn, NumPyType, dim1, dim2) ? convertArray2<T> : nullptr;
}

template <class T>
static void setOrExtendArrayConverter(int dimension, IsArrayConvertibleToCppFunc toCppCheckFunc)
{
    SbkArrayConverter *arrayConverter = ArrayTypeConverter<T>(dimension);
    if (arrayConverter == unimplementedArrayConverter()) {
        arrayConverter = createArrayConverter(toCppCheckFunc);
        setArrayTypeConverter(ArrayTypeIndex<T>::index, dimension, arrayConverter);
    } else {
        arrayConverter->toCppConversions.push_back(toCppCheckFunc);
    }
}

// Extend the converters for primitive type one-dimensional arrays by NumPy ones.
template <class T, int NumPyType>
static inline void extendArrayConverter1()
{
    setOrExtendArrayConverter<T>(1, checkArray1<T, NumPyType>);
}

// Extend the converters for primitive type one-dimensional arrays by NumPy ones.
template <class T, int NumPyType>
static inline void extendArrayConverter2()
{
    setOrExtendArrayConverter<T>(2, checkArray2<T, NumPyType>);
}

void initNumPyArrayConverters()
{
    // Expanded from macro "import_array" in __multiarray_api.h
    // Make sure to read about the magic defines PY_ARRAY_UNIQUE_SYMBOL etc.,
    // when changing this or spreading the code over several source files.
    if (_import_array() < 0) {
        PyErr_Print();
        PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
        return;
    }

    // Extend the converters for primitive types by NumPy ones.
    extendArrayConverter1<short, NPY_SHORT>();
    extendArrayConverter2<short, NPY_SHORT>();
    extendArrayConverter1<unsigned short, NPY_SHORT>();
    extendArrayConverter2<unsigned short, NPY_SHORT>();
    extendArrayConverter1<int, NPY_INT>();
    extendArrayConverter2<int, NPY_INT>();
    extendArrayConverter1<unsigned int, NPY_UINT>();
    extendArrayConverter2<unsigned int, NPY_UINT>();
    extendArrayConverter1<long long, NPY_LONGLONG>();
    extendArrayConverter2<long long, NPY_LONGLONG>();
    extendArrayConverter1<unsigned long long, NPY_ULONGLONG>();
    if (sizeof(long) == 8) { // UNIX/LP64: ints typically come as long
        extendArrayConverter1<long long, NPY_LONG>();
        extendArrayConverter2<long long, NPY_LONG>();
        extendArrayConverter1<unsigned long long, NPY_ULONG>();
        extendArrayConverter2<unsigned long long, NPY_ULONG>();
    } else if (sizeof(long) == sizeof(int)) {
        extendArrayConverter1<int, NPY_LONG>();
        extendArrayConverter1<unsigned, NPY_ULONG>();
        extendArrayConverter2<int, NPY_LONG>();
        extendArrayConverter2<unsigned, NPY_ULONG>();
    }
    extendArrayConverter1<float, NPY_FLOAT>();
    extendArrayConverter2<float, NPY_FLOAT>();
    extendArrayConverter1<double, NPY_DOUBLE>();
    extendArrayConverter2<double, NPY_DOUBLE>();
}

} // namespace Conversions
} // namespace Shiboken
