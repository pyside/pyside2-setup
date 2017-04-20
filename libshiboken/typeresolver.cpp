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

#include "typeresolver.h"
#include "sbkdbg.h"
#include <cstdlib>
#include <string>
#include <unordered_map>
#include "basewrapper_p.h"

using namespace Shiboken;

typedef std::unordered_map<std::string, TypeResolver *> TypeResolverMap;
static TypeResolverMap typeResolverMap;

struct TypeResolver::TypeResolverPrivate
{
    CppToPythonFunc cppToPython;
    PythonToCppFunc pythonToCpp;
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
    std::atexit(deinitTypeResolver);
}

TypeResolver::TypeResolver() : m_d(new TypeResolverPrivate)
{
}

TypeResolver* TypeResolver::createTypeResolver(const char* typeName,
                                               CppToPythonFunc cppToPy,
                                               PythonToCppFunc pyToCpp,
                                               PyTypeObject* pyType)
{
    TypeResolver*& tr = typeResolverMap[typeName];
    if (!tr) {
        tr = new TypeResolver;
        tr->m_d->cppToPython = cppToPy;
        tr->m_d->pythonToCpp = pyToCpp;
        tr->m_d->pyType = pyType;

        /*
         * Note:
         *
         *     Value types are also registered as object types, but the generator *always* first register the value
         *     type version in the TypeResolver and it *must* always do it! otherwise this code wont work.
         */
        if (pyType && PyType_IsSubtype(pyType, reinterpret_cast<PyTypeObject*>(&SbkObject_Type))) {
            SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(pyType);
            // TODO-CONVERTERS: to be deprecated
            if (!sbkType->d->type_behaviour) {
                const size_t len = strlen(typeName);
                sbkType->d->type_behaviour = typeName[len -1] == '*' ? BEHAVIOUR_OBJECTTYPE : BEHAVIOUR_VALUETYPE;
            }
        }
    }
    return tr;
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
        if (Py_VerboseFlag > 0)
            SbkDbg() << "Can't find type resolver for " << typeName;
        return 0;
    }
}

void TypeResolver::toCpp(PyObject* pyObj, void** place)
{
    m_d->pythonToCpp(pyObj, place);
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
    const size_t len = strlen(name);
    bool isObjTypeName = name[len - 1] == '*';
    if (TypeResolver::get(name)) {
        // great, we found the type in our first attempt!
        return isObjTypeName ? ObjectType : ValueType;
    } else {
        // Type not found... let's copy the string.
        std::string typeName(name);
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

