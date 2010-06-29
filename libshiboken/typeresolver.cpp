/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
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
    typeResolverMap[resolver->typeName()] = resolver;
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

void* TypeResolver::toCpp(PyObject* pyObj)
{
    return m_d->pythonToCpp(pyObj);
}

PyObject* TypeResolver::toPython(void* cppObj)
{
    return m_d->cppToPython(cppObj);
}

void TypeResolver::deleteObject(void* object)
{
    if (m_d->deleteObject)
        m_d->deleteObject(object);
}

PyTypeObject* TypeResolver::pythonType()
{
    return m_d->pyType;
}

TypeResolver::Type TypeResolver::getType(const char* name)
{
    std::string typeName(name);
    int len = typeName.size() - 1;
    if (len > 1) {
        if (typeName[len] == '*')
            typeName.erase(len, 1);

        TypeResolver *resolver = TypeResolver::get(typeName.c_str());
        if (resolver)
            return TypeResolver::ValueType;

        typeName += '*';
        resolver = TypeResolver::get(typeName.c_str());
        if (resolver)
            return TypeResolver::ObjectType;
    }

    return TypeResolver::UnknownType;
}

