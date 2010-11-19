/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TYPERESOLVER_H
#define TYPERESOLVER_H

#include "shibokenmacros.h"
#include "conversions.h"

namespace Shiboken
{

/* To C++ convertion functions. */
template <typename T>
inline void* pythonToValueType(PyObject* pyobj, void** data, bool alloc)
{
    if (alloc)
        *data = Shiboken::CppObjectCopier<T>::copy(Shiboken::Converter<T>::toCpp(pyobj));

    *reinterpret_cast< T*>(*data) = Shiboken::Converter<T>::toCpp(pyobj);
    return *data;
}

template <typename T>
inline void* pythonToObjectType(PyObject* pyobj, void** data, bool)
{
    *data = Shiboken::Converter<T*>::toCpp(pyobj);
    return *data;
}

template <typename T>
inline void objectDeleter(void* data)
{
    delete reinterpret_cast<T*>(data);
}

template <typename T>
inline PyObject* objectTypeToPython(void* cptr)
{
    return Shiboken::Converter<T*>::toPython(*reinterpret_cast<T**>(cptr));
}

/**
*   \internal This function is not part of the public API.
*   Initialize the TypeResource internal cache.
*/
void initTypeResolver();

class LIBSHIBOKEN_API TypeResolver
{
public:
    enum Type
    {
        ObjectType,
        ValueType,
        UnknownType
    };

    typedef PyObject* (*CppToPythonFunc)(void*);
    typedef void* (*PythonToCppFunc)(PyObject*, void**, bool);
    typedef void (*DeleteObjectFunc)(void*);

    ~TypeResolver();

    template<typename T>
    static TypeResolver* createValueTypeResolver(const char* typeName)
    {
        return new TypeResolver(typeName, &Shiboken::Converter<T>::toPython, &pythonToValueType<T>, SbkType<T>(), &objectDeleter<T>);
    }

    template<typename T>
    static TypeResolver* createObjectTypeResolver(const char* typeName)
    {
        return new TypeResolver(typeName, &objectTypeToPython<T>, &pythonToObjectType<T>, SbkType<T>());
    }

    static Type getType(const char* name);
    static TypeResolver* get(const char* typeName);

    const char* typeName() const;
    PyObject* toPython(void* cppObj);
    void* toCpp(PyObject* pyObj, void** place, bool alloc=false);
    void deleteObject(void* object);
    PyTypeObject* pythonType();

private:
    struct TypeResolverPrivate;
    TypeResolverPrivate* m_d;

    // disable object copy
    TypeResolver(const TypeResolver&);
    TypeResolver& operator=(const TypeResolver&);

    TypeResolver(const char* typeName, CppToPythonFunc cppToPy, PythonToCppFunc pyToCpp, PyTypeObject* pyType, DeleteObjectFunc deleter = 0);
};
}

#endif
