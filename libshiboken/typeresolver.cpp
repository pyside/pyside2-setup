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

#include "typeresolver.h"
#include "google/dense_hash_map"
#include "sbkdbg.h"
#include <cstdlib>
#include <string>

using namespace Shiboken;

typedef google::dense_hash_map<std::string, TypeResolver*> TypeResolverMap;
static TypeResolverMap typeResolverMap;

struct TypeResolver::TypeResolverPrivate
{
    const char* typeName; // maybe this is not needed anymore
    CppToPythonFunc cppToPython;
    PythonToCppFunc pythonToCpp;
    DeleteObjectFunc deleteObject;
    PyTypeObject* pyType;
};

static void deinitTypeResolver()
{
    for (TypeResolverMap::const_iterator it = typeResolverMap.begin(); it != typeResolverMap.end(); ++it)
        delete it->second;
    typeResolverMap.clear();
}

void Shiboken::initTypeResolver()
{
    assert(typeResolverMap.empty());
    typeResolverMap.set_empty_key("");
    typeResolverMap.set_deleted_key("?");
    std::atexit(deinitTypeResolver);
}

static void registerTypeResolver(TypeResolver* resolver)
{
    TypeResolver*& v = typeResolverMap[resolver->typeName()];
    if (!v)
        v = resolver;
    else
        delete resolver; // Discard type resolvers already registered
}

TypeResolver::TypeResolver(const char* typeName, TypeResolver::CppToPythonFunc cppToPy, TypeResolver::PythonToCppFunc pyToCpp, PyTypeObject* pyType, TypeResolver::DeleteObjectFunc deleter)
{
    m_d = new TypeResolverPrivate;
    m_d->typeName = typeName;
    m_d->cppToPython = cppToPy;
    m_d->pythonToCpp = pyToCpp;
    m_d->deleteObject = deleter;
    m_d->pyType = pyType;

    registerTypeResolver(this);
}

TypeResolver::~TypeResolver()
{
    delete m_d;
}

TypeResolver* TypeResolver::get(const char* typeName)
{
    TypeResolverMap::const_iterator it = typeResolverMap.find(typeName);
    if (it != typeResolverMap.end()) {
        return it->second;
    } else {
        SbkDbg() << "Can't find type resolver for " << typeName;
        return 0;
    }
}

const char* TypeResolver::typeName() const
{
    return m_d->typeName;
}

void* TypeResolver::toCpp(PyObject* pyObj, void** place, bool alloc)
{
    return m_d->pythonToCpp(pyObj, place, alloc);
}

void TypeResolver::deleteObject(void* object)
{
    if (m_d->deleteObject)
        m_d->deleteObject(object);
}

PyObject* TypeResolver::toPython(void* cppObj)
{
    return m_d->cppToPython(cppObj);
}

PyTypeObject* TypeResolver::pythonType()
{
    return m_d->pyType;
}

TypeResolver::Type TypeResolver::getType(const char* name)
{
    int len = strlen(name);
    bool isObjTypeName = name[len - 1] == '*';
    if (TypeResolver::get(name)) {
        // great, we found the type in our first attempt!
        return isObjTypeName ? ObjectType : ValueType;
    } else {
        // Type not found... let's copy the string.
        std::string typeName;
        typeName.reserve(len + 2);
        if (isObjTypeName)
            typeName.erase(len - 1, 1);
        else
            typeName += '*';
        isObjTypeName = !isObjTypeName;

        if (TypeResolver::get(typeName.c_str()))
            return isObjTypeName ? ObjectType : ValueType;
        else
            return UnknownType;
    }
}

