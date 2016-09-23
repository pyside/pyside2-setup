/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef TYPERESOLVER_H
#define TYPERESOLVER_H

#include "shibokenmacros.h"
#include "conversions.h"

namespace Shiboken
{

/* To C++ convertion functions. */
template <typename T>
inline void pythonToValueType(PyObject* pyobj, void** data)
{
    *reinterpret_cast<T*>(*data) = Shiboken::Converter<T>::toCpp(pyobj);
}

template <typename T>
inline void pythonToObjectType(PyObject* pyobj, void** data)
{
    *reinterpret_cast<T**>(*data) = Shiboken::Converter<T*>::toCpp(pyobj);
}

template <typename T>
inline PyObject* objectTypeToPython(void* cptr)
{
    return Shiboken::Converter<T*>::toPython(*reinterpret_cast<T**>(cptr));
}

template <typename T>
inline PyObject* referenceTypeToPython(void* cptr)
{
    // cptr comes the same way it come when we have a value type, but
    // we deliver a Python object of a reference
    return Shiboken::Converter<T&>::toPython(*reinterpret_cast<T*>(cptr));
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
    typedef void (*PythonToCppFunc)(PyObject*, void**);

    ~TypeResolver();

    template<typename T>
    static TypeResolver* createValueTypeResolver(const char* typeName)
    {
        return createTypeResolver(typeName, &Shiboken::Converter<T>::toPython, &pythonToValueType<T>, SbkType<T>());
    }

    template<typename T>
    static TypeResolver* createObjectTypeResolver(const char* typeName)
    {
        return createTypeResolver(typeName, &objectTypeToPython<T>, &pythonToObjectType<T>, SbkType<T>());
    }

    /**
     * This kind of type resolver is used only when we have a signal with a reference in their arguments
     * like on QSqlTableModel::primeInsert.
     */
    template<typename T>
    static TypeResolver* createReferenceTypeResolver(const char* typeName)
    {
        return createTypeResolver(typeName, &referenceTypeToPython<T>, &pythonToValueType<T>, SbkType<T>());
    }

    static Type getType(const char* name);
    static TypeResolver* get(const char* typeName);

    PyObject* toPython(void* cppObj);
    void toCpp(PyObject* pyObj, void** place);
    PyTypeObject* pythonType();

private:
    struct TypeResolverPrivate;
    TypeResolverPrivate* m_d;

    TypeResolver();
    // disable object copy
    TypeResolver(const TypeResolver&);
    TypeResolver& operator=(const TypeResolver&);

    static TypeResolver* createTypeResolver(const char* typeName, CppToPythonFunc cppToPy, PythonToCppFunc pyToCpp, PyTypeObject* pyType);
};
}

#endif
