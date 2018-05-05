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

#ifndef SBK_CONVERTER_H
#define SBK_CONVERTER_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#include <limits>
#include <string>

struct SbkObject;
struct SbkObjectType;

/**
 *  This is a convenience macro identical to Python's PyObject_TypeCheck,
 *  except that the arguments have swapped places, for the great convenience
 *  of generator.
 */
#define SbkObject_TypeCheck(tp, ob) \
        (Py_TYPE(ob) == (tp) || PyType_IsSubtype(Py_TYPE(ob), (tp)))

extern "C"
{

/**
 *  SbkConverter is used to perform type conversions from C++
 *  to Python and vice-versa;.and it is also used for type checking.
 *  SbkConverter is a private structure that must be accessed
 *  using the functions provided by the converter API.
 */
struct SbkConverter;
struct SbkArrayConverter;

/**
 *  Given a void pointer to a C++ object, this function must return
 *  the proper Python object. It may be either an existing wrapper
 *  for the C++ object, or a newly create one. Or even the Python
 *  equivalent of the C++ value passed in the argument.
 *
 *  C++ -> Python
 */
typedef PyObject* (*CppToPythonFunc)(const void*);

/**
 *  This function converts a Python object to a C++ value, it may be
 *  a pointer, value, class, container or primitive type, passed via
 *  a void pointer, that will be cast properly inside the function.
 *  This function is usually returned by an IsConvertibleToCppFunc
 *  function, or obtained knowing the type of the Python object input,
 *  thus it will not check the Python object type, and will expect
 *  the void pointer to be pointing to a proper variable.
 *
 *  Python -> C++
 */
typedef void (*PythonToCppFunc)(PyObject*,void*);

/**
 *  Checks if the Python object passed in the argument is convertible to a
 *  C++ type defined inside the function, it returns the converter function
 *  that will transform a Python argument into a C++ value.
 *  It returns NULL if the Python object is not convertible to the C++ type
 *  that the function represents.
 *
 *  Python -> C++ ?
 */
typedef PythonToCppFunc (*IsConvertibleToCppFunc)(PyObject*);

} // extern "C"

namespace Shiboken {
namespace Conversions {


class LIBSHIBOKEN_API SpecificConverter
{
public:
    enum Type
    {
        InvalidConversion,
        CopyConversion,
        PointerConversion,
        ReferenceConversion
    };

    explicit SpecificConverter(const char* typeName);

    inline SbkConverter* converter() { return m_converter; }
    inline operator SbkConverter*() const { return m_converter; }

    inline bool isValid() { return m_type != InvalidConversion; }
    inline operator bool() const { return m_type != InvalidConversion; }

    inline Type conversionType() { return m_type; }

    PyObject* toPython(const void* cppIn);
    void toCpp(PyObject* pyIn, void* cppOut);
private:
    SbkConverter* m_converter;
    Type m_type;
};


/**
 *  Creates a converter for a wrapper type.
 *  \param type                  A Shiboken.ObjectType that will receive the new converter.
 *  \param toCppPointerConvFunc  Function to retrieve the C++ pointer held by a Python wrapper.
 *  \param toCppPointerCheckFunc Check and return the retriever function of the C++ pointer held by a Python wrapper.
 *  \param pointerToPythonFunc   Function to convert a C++ object to a Python \p type wrapper, keeping their identity.
 *  \param copyToPythonFunc      Function to convert a C++ object to a Python \p type, copying the object.
 *  \returns                     The new converter referred by the wrapper \p type.
 */
LIBSHIBOKEN_API SbkConverter* createConverter(SbkObjectType* type,
                                              PythonToCppFunc toCppPointerConvFunc,
                                              IsConvertibleToCppFunc toCppPointerCheckFunc,
                                              CppToPythonFunc pointerToPythonFunc,
                                              CppToPythonFunc copyToPythonFunc = 0);

/**
 *  Creates a converter for a non wrapper type (primitive or container type).
 *  \param type         Python type representing to the new converter.
 *  \param toPythonFunc Function to convert a C++ object to a Python \p type.
 *  \returns            A new type converter.
 */
LIBSHIBOKEN_API SbkConverter* createConverter(PyTypeObject* type, CppToPythonFunc toPythonFunc);

LIBSHIBOKEN_API void deleteConverter(SbkConverter* converter);

/// Sets the Python object to C++ pointer conversion function.
LIBSHIBOKEN_API void setCppPointerToPythonFunction(SbkConverter* converter, CppToPythonFunc pointerToPythonFunc);

/// Sets the C++ pointer to Python object conversion functions.
LIBSHIBOKEN_API void setPythonToCppPointerFunctions(SbkConverter* converter,
                                                    PythonToCppFunc toCppPointerConvFunc,
                                                    IsConvertibleToCppFunc toCppPointerCheckFunc);

/**
 *  Adds a new conversion of a Python object to a C++ value.
 *  This is used in copy and implicit conversions.
 */
LIBSHIBOKEN_API void addPythonToCppValueConversion(SbkConverter* converter,
                                                   PythonToCppFunc pythonToCppFunc,
                                                   IsConvertibleToCppFunc isConvertibleToCppFunc);
LIBSHIBOKEN_API void addPythonToCppValueConversion(SbkObjectType* type,
                                                   PythonToCppFunc pythonToCppFunc,
                                                   IsConvertibleToCppFunc isConvertibleToCppFunc);

// C++ -> Python ---------------------------------------------------------------------------

/**
 *  Retrieves the Python wrapper object for the given \p cppIn C++ pointer object.
 *  This function is used only for Value and Object Types.
 *  Example usage:
 *      TYPE* var;
 *      PyObject* pyVar = pointerToPython(SBKTYPE, &var);
 */
LIBSHIBOKEN_API PyObject* pointerToPython(const SbkObjectType *type, const void *cppIn);
LIBSHIBOKEN_API PyObject* pointerToPython(const SbkConverter *converter, const void *cppIn);

/**
 *  For the given \p cppIn C++ reference it returns the Python wrapper object,
 *  always for Object Types, and when they already exist for reference types;
 *  for when the latter doesn't have an existing wrapper type, the C++ object
 *  is copied to Python.
 *  Example usage:
 *      TYPE& var = SOMETHING;
 *      PyObject* pyVar = referenceToPython(SBKTYPE, &var);
 */
LIBSHIBOKEN_API PyObject* referenceToPython(const SbkObjectType *type, const void *cppIn);
LIBSHIBOKEN_API PyObject* referenceToPython(const SbkConverter *converter, const void *cppIn);

/**
 *  Retrieves the Python wrapper object for the given C++ value pointed by \p cppIn.
 *  This function is used only for Value Types.
 *  Example usage:
 *      TYPE var;
 *      PyObject* pyVar = copyToPython(SBKTYPE, &var);
 */
LIBSHIBOKEN_API PyObject* copyToPython(const SbkObjectType *type, const void *cppIn);
LIBSHIBOKEN_API PyObject* copyToPython(const SbkConverter *converter, const void *cppIn);

// Python -> C++ ---------------------------------------------------------------------------

/**
 *  Returns a Python to C++ conversion function if the Python object is convertible to a C++ pointer.
 *  It returns NULL if the Python object is not convertible to \p type.
 */
LIBSHIBOKEN_API PythonToCppFunc isPythonToCppPointerConvertible(const SbkObjectType *type, PyObject *pyIn);

/**
 *  Returns a Python to C++ conversion function if the Python object is convertible to a C++ value.
 *  The resulting converter function will create a copy of the Python object in C++, or implicitly
 *  convert the object to the expected \p type.
 *  It returns NULL if the Python object is not convertible to \p type.
 */
LIBSHIBOKEN_API PythonToCppFunc isPythonToCppValueConvertible(const SbkObjectType *type, PyObject *pyIn);

/**
 *  Returns a Python to C++ conversion function if the Python object is convertible to a C++ reference.
 *  The resulting converter function will return the underlying C++ object held by the Python wrapper,
 *  or a new C++ value if it must be a implicit conversion.
 *  It returns NULL if the Python object is not convertible to \p type.
 */
LIBSHIBOKEN_API PythonToCppFunc isPythonToCppReferenceConvertible(const SbkObjectType *type, PyObject *pyIn);

/// This is the same as isPythonToCppValueConvertible function.
LIBSHIBOKEN_API PythonToCppFunc isPythonToCppConvertible(const SbkConverter *converter, PyObject *pyIn);
LIBSHIBOKEN_API PythonToCppFunc isPythonToCppConvertible(const SbkArrayConverter *converter,
                                                         int dim1, int dim2, PyObject *pyIn);

/**
 *  Returns the C++ pointer for the \p pyIn object cast to the type passed via \p desiredType.
 *  It differs from Shiboken::Object::cppPointer because it casts the pointer to a proper
 *  memory offset depending on the desired type.
 */
LIBSHIBOKEN_API void* cppPointer(PyTypeObject* desiredType, SbkObject* pyIn);

/// Converts a Python object \p pyIn to C++ and stores the result in the C++ pointer passed in \p cppOut.
LIBSHIBOKEN_API void pythonToCppPointer(SbkObjectType* type, PyObject* pyIn, void* cppOut);
LIBSHIBOKEN_API void pythonToCppPointer(const SbkConverter *converter, PyObject *pyIn, void *cppOut);

/// Converts a Python object \p pyIn to C++, and copies the result in the C++ variable passed in \p cppOut.
LIBSHIBOKEN_API void pythonToCppCopy(const SbkObjectType *type, PyObject *pyIn, void *cppOut);
LIBSHIBOKEN_API void pythonToCppCopy(const SbkConverter *converter, PyObject *pyIn, void *cppOut);

/**
 *  Helper function returned by generated convertible checking functions
 *  that returns a C++ NULL when the input Python object is None.
 */
LIBSHIBOKEN_API void nonePythonToCppNullPtr(PyObject*, void* cppOut);

/**
 *  Returns true if the \p toCpp function passed is an implicit conversion of Python \p type.
 *  It is used when C++ expects a reference argument, so it may be the same object received
 *  from Python, or another created through implicit conversion.
 */
LIBSHIBOKEN_API bool isImplicitConversion(const SbkObjectType *type, PythonToCppFunc toCpp);

/// Registers a converter with a type name that may be used to retrieve the converter.
LIBSHIBOKEN_API void registerConverterName(SbkConverter* converter, const char* typeName);

/// Returns the converter for a given type name, or NULL if it wasn't registered before.
LIBSHIBOKEN_API SbkConverter* getConverter(const char* typeName);

/// Returns the converter for a primitive type.
LIBSHIBOKEN_API SbkConverter* primitiveTypeConverter(int index);

/// Returns true if a Python sequence is comprised of objects of the given \p type.
LIBSHIBOKEN_API bool checkSequenceTypes(PyTypeObject* type, PyObject* pyIn);

/// Returns true if a Python sequence is comprised of objects of a type convertible to the one represented by the given \p converter.
LIBSHIBOKEN_API bool convertibleSequenceTypes(const SbkConverter *converter, PyObject *pyIn);

/// Returns true if a Python sequence is comprised of objects of a type convertible to \p type.
LIBSHIBOKEN_API bool convertibleSequenceTypes(const SbkObjectType *type, PyObject *pyIn);

/// Returns true if a Python sequence can be converted to a C++ pair.
LIBSHIBOKEN_API bool checkPairTypes(PyTypeObject* firstType, PyTypeObject* secondType, PyObject* pyIn);

/// Returns true if a Python sequence can be converted to a C++ pair.
LIBSHIBOKEN_API bool convertiblePairTypes(const SbkConverter *firstConverter, bool firstCheckExact,
                                          const SbkConverter *secondConverter, bool secondCheckExact,
                                          PyObject *pyIn);

/// Returns true if a Python dictionary can be converted to a C++ hash or map.
LIBSHIBOKEN_API bool checkDictTypes(PyTypeObject* keyType, PyTypeObject* valueType, PyObject* pyIn);

/// Returns true if a Python dictionary can be converted to a C++ hash or map.
LIBSHIBOKEN_API bool convertibleDictTypes(const SbkConverter *keyConverter, bool keyCheckExact,
                                          const SbkConverter *valueConverter, bool valueCheckExact,
                                          PyObject *pyIn);

/// Returns the Python type object associated with the given \p converter.
LIBSHIBOKEN_API PyTypeObject* getPythonTypeObject(const SbkConverter *converter);

/// Returns the Python type object for the given \p typeName.
LIBSHIBOKEN_API PyTypeObject* getPythonTypeObject(const char* typeName);

/// Returns true if the Python type associated with the converter is a value type.
LIBSHIBOKEN_API bool pythonTypeIsValueType(const SbkConverter *converter);

/// Returns true if the Python type associated with the converter is an object type.
LIBSHIBOKEN_API bool pythonTypeIsObjectType(const SbkConverter *converter);

/// Returns true if the Python type associated with the converter is a wrapper type.
LIBSHIBOKEN_API bool pythonTypeIsWrapperType(const SbkConverter *converter);

#define SBK_PY_LONG_LONG_IDX            0
// Qt5: name collision in QtCore after QBool is replaced by bool
#define SBK_BOOL_IDX_1                  1
#define SBK_CHAR_IDX                    2
#define SBK_CONSTCHARPTR_IDX            3
#define SBK_DOUBLE_IDX                  4
#define SBK_FLOAT_IDX                   5
#define SBK_INT_IDX                     6
#define SBK_SIGNEDINT_IDX               6
#define SBK_LONG_IDX                    7
#define SBK_SHORT_IDX                   8
#define SBK_SIGNEDCHAR_IDX              9
#define SBK_STD_STRING_IDX             10
#define SBK_UNSIGNEDPY_LONG_LONG_IDX   11
#define SBK_UNSIGNEDCHAR_IDX           12
#define SBK_UNSIGNEDINT_IDX            13
#define SBK_UNSIGNEDLONG_IDX           14
#define SBK_UNSIGNEDSHORT_IDX          15
#define SBK_VOIDPTR_IDX                16

template<typename T> SbkConverter* PrimitiveTypeConverter() { return 0; }
template<> inline SbkConverter* PrimitiveTypeConverter<PY_LONG_LONG>() { return primitiveTypeConverter(SBK_PY_LONG_LONG_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<bool>() { return primitiveTypeConverter(SBK_BOOL_IDX_1); }
template<> inline SbkConverter* PrimitiveTypeConverter<char>() { return primitiveTypeConverter(SBK_CHAR_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<const char*>() { return primitiveTypeConverter(SBK_CONSTCHARPTR_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<double>() { return primitiveTypeConverter(SBK_DOUBLE_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<float>() { return primitiveTypeConverter(SBK_FLOAT_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<int>() { return primitiveTypeConverter(SBK_INT_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<long>() { return primitiveTypeConverter(SBK_LONG_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<short>() { return primitiveTypeConverter(SBK_SHORT_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<signed char>() { return primitiveTypeConverter(SBK_SIGNEDCHAR_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<std::string>() { return primitiveTypeConverter(SBK_STD_STRING_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<unsigned PY_LONG_LONG>() { return primitiveTypeConverter(SBK_UNSIGNEDPY_LONG_LONG_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<unsigned char>() { return primitiveTypeConverter(SBK_UNSIGNEDCHAR_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<unsigned int>() { return primitiveTypeConverter(SBK_UNSIGNEDINT_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<unsigned long>() { return primitiveTypeConverter(SBK_UNSIGNEDLONG_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<unsigned short>() { return primitiveTypeConverter(SBK_UNSIGNEDSHORT_IDX); }
template<> inline SbkConverter* PrimitiveTypeConverter<void*>() { return primitiveTypeConverter(SBK_VOIDPTR_IDX); }

} // namespace Shiboken::Conversions

/**
*   This function template is used to get the PyTypeObject of a C++ type T.
*   All implementations should be provided by template specializations generated by the generator when
*   T isn't a C++ primitive type.
*   \see SpecialCastFunction
*/
template<typename T> PyTypeObject* SbkType() { return 0; }

// Below are the template specializations for C++ primitive types.
template<> inline PyTypeObject* SbkType<PY_LONG_LONG>() { return &PyLong_Type; }
template<> inline PyTypeObject* SbkType<bool>() { return &PyBool_Type; }
template<> inline PyTypeObject* SbkType<char>() { return &PyInt_Type; }
template<> inline PyTypeObject* SbkType<double>() { return &PyFloat_Type; }
template<> inline PyTypeObject* SbkType<float>() { return &PyFloat_Type; }
template<> inline PyTypeObject* SbkType<int>() { return &PyInt_Type; }
template<> inline PyTypeObject* SbkType<long>() { return &PyLong_Type; }
template<> inline PyTypeObject* SbkType<short>() { return &PyInt_Type; }
template<> inline PyTypeObject* SbkType<signed char>() { return &PyInt_Type; }
template<> inline PyTypeObject* SbkType<unsigned PY_LONG_LONG>() { return &PyLong_Type; }
template<> inline PyTypeObject* SbkType<unsigned char>() { return &PyInt_Type; }
template<> inline PyTypeObject* SbkType<unsigned int>() { return &PyLong_Type; }
template<> inline PyTypeObject* SbkType<unsigned long>() { return &PyLong_Type; }
template<> inline PyTypeObject* SbkType<unsigned short>() { return &PyInt_Type; }

} // namespace Shiboken

// When the user adds a function with an argument unknown for the typesystem, the generator writes type checks as
// TYPENAME_Check, so this macro allows users to add PyObject arguments to their added functions.
#define PyObject_Check(X) true
#define SbkChar_Check(X) (SbkNumber_Check(X) || Shiboken::String::checkChar(X))

struct _SbkGenericType { PepTypeObject super; SbkConverter** converter; };
#define SBK_CONVERTER(pyType) (*reinterpret_cast<_SbkGenericType*>(pyType)->converter)


#endif // SBK_CONVERTER_H
