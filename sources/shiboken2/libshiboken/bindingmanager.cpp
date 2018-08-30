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

#include "basewrapper.h"
#include "basewrapper_p.h"
#include "bindingmanager.h"
#include "sbkdbg.h"
#include "gilstate.h"
#include "sbkstring.h"
#include "debugfreehook.h"

#include <cstddef>
#include <fstream>
#include <unordered_map>

namespace Shiboken
{

typedef std::unordered_map<const void *, SbkObject *> WrapperMap;

class Graph
{
public:
    typedef std::vector<SbkObjectType *> NodeList;
    typedef std::unordered_map<SbkObjectType *, NodeList> Edges;

    Edges m_edges;

    Graph()
    {
    }

    void addEdge(SbkObjectType* from, SbkObjectType* to)
    {
        m_edges[from].push_back(to);
    }

#ifndef NDEBUG
    void dumpDotGraph()
    {
        std::ofstream file("/tmp/shiboken_graph.dot");

        file << "digraph D {\n";

        for (auto i = m_edges.begin(), end = m_edges.end(); i != end; ++i) {
            auto node1 = reinterpret_cast<const PyTypeObject *>(i->first);
            const NodeList& nodeList = i->second;
            for (const SbkObjectType *o : nodeList) {
                auto node2 = reinterpret_cast<const PyTypeObject *>(o);
                file << '"' << node2->tp_name << "\" -> \""
                    << node1->tp_name << "\"\n";
            }
        }
        file << "}\n";
    }
#endif

    SbkObjectType* identifyType(void** cptr, SbkObjectType* type, SbkObjectType* baseType) const
    {
        Edges::const_iterator edgesIt = m_edges.find(type);
        if (edgesIt != m_edges.end()) {
            const NodeList& adjNodes = m_edges.find(type)->second;
            for (SbkObjectType *node : adjNodes) {
                SbkObjectType* newType = identifyType(cptr, node, baseType);
                if (newType)
                    return newType;
            }
        }
        void *typeFound = nullptr;
        if (PepType_SOTP(type) && PepType_SOTP(type)->type_discovery) {
            typeFound = PepType_SOTP(type)->type_discovery(*cptr, baseType);
        }
        if (typeFound) {
            // This "typeFound != type" is needed for backwards compatibility with old modules using a newer version of
            // libshiboken because old versions of type_discovery function used to return a SbkObjectType* instead of
            // a possible variation of the C++ instance pointer (*cptr).
            if (typeFound != type)
                *cptr = typeFound;
            return type;
        }
        return nullptr;
    }
};


#ifndef NDEBUG
static void showWrapperMap(const WrapperMap& wrapperMap)
{
    if (Py_VerboseFlag > 0) {
        fprintf(stderr, "-------------------------------\n");
        fprintf(stderr, "WrapperMap: %p (size: %d)\n", &wrapperMap, (int) wrapperMap.size());
        for (auto it = wrapperMap.begin(), end = wrapperMap.end(); it != end; ++it) {
            const SbkObject *sbkObj = it->second;
            fprintf(stderr, "key: %p, value: %p (%s, refcnt: %d)\n", it->first,
                    static_cast<const void *>(sbkObj),
                    (Py_TYPE(sbkObj))->tp_name,
                    int(reinterpret_cast<const PyObject *>(sbkObj)->ob_refcnt));
        }
        fprintf(stderr, "-------------------------------\n");
    }
}
#endif

struct BindingManager::BindingManagerPrivate {
    WrapperMap wrapperMapper;
    Graph classHierarchy;
    bool destroying;

    BindingManagerPrivate() : destroying(false) {}
    bool releaseWrapper(void* cptr, SbkObject* wrapper);
    void assignWrapper(SbkObject* wrapper, const void* cptr);

};

bool BindingManager::BindingManagerPrivate::releaseWrapper(void* cptr, SbkObject* wrapper)
{
    // The wrapper argument is checked to ensure that the correct wrapper is released.
    // Returns true if the correct wrapper is found and released.
    // If wrapper argument is NULL, no such check is performed.
    WrapperMap::iterator iter = wrapperMapper.find(cptr);
    if (iter != wrapperMapper.end() && (wrapper == 0 || iter->second == wrapper)) {
        wrapperMapper.erase(iter);
        return true;
    }
    return false;
}

void BindingManager::BindingManagerPrivate::assignWrapper(SbkObject* wrapper, const void* cptr)
{
    assert(cptr);
    WrapperMap::iterator iter = wrapperMapper.find(cptr);
    if (iter == wrapperMapper.end())
        wrapperMapper.insert(std::make_pair(cptr, wrapper));
}

BindingManager::BindingManager()
{
    m_d = new BindingManager::BindingManagerPrivate;

#ifdef SHIBOKEN_INSTALL_FREE_DEBUG_HOOK
    debugInstallFreeHook();
#endif
}

BindingManager::~BindingManager()
{
#ifdef SHIBOKEN_INSTALL_FREE_DEBUG_HOOK
    debugRemoveFreeHook();
#endif
#ifndef NDEBUG
    showWrapperMap(m_d->wrapperMapper);
#endif
    /* Cleanup hanging references. We just invalidate them as when
     * the BindingManager is being destroyed the interpreter is alredy
     * shutting down. */
    if (Py_IsInitialized()) {  // ensure the interpreter is still valid
        while (!m_d->wrapperMapper.empty()) {
            Object::destroy(m_d->wrapperMapper.begin()->second, const_cast<void*>(m_d->wrapperMapper.begin()->first));
        }
        assert(m_d->wrapperMapper.size() == 0);
    }
    delete m_d;
}

BindingManager& BindingManager::instance() {
    static BindingManager singleton;
    return singleton;
}

bool BindingManager::hasWrapper(const void* cptr)
{
    return m_d->wrapperMapper.find(cptr) != m_d->wrapperMapper.end();
}

void BindingManager::registerWrapper(SbkObject* pyObj, void* cptr)
{
    SbkObjectType* instanceType = reinterpret_cast<SbkObjectType*>(Py_TYPE(pyObj));
    SbkObjectTypePrivate* d = PepType_SOTP(instanceType);

    if (!d)
        return;

    if (d->mi_init && !d->mi_offsets)
        d->mi_offsets = d->mi_init(cptr);
    m_d->assignWrapper(pyObj, cptr);
    if (d->mi_offsets) {
        int* offset = d->mi_offsets;
        while (*offset != -1) {
            if (*offset > 0)
                m_d->assignWrapper(pyObj, reinterpret_cast<void*>((std::size_t) cptr + (*offset)));
            offset++;
        }
    }
}

void BindingManager::releaseWrapper(SbkObject* sbkObj)
{
    SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(Py_TYPE(sbkObj));
    SbkObjectTypePrivate* d = PepType_SOTP(sbkType);
    int numBases = ((d && d->is_multicpp) ? getNumberOfCppBaseClasses(Py_TYPE(sbkObj)) : 1);

    void** cptrs = reinterpret_cast<SbkObject*>(sbkObj)->d->cptr;
    for (int i = 0; i < numBases; ++i) {
        unsigned char *cptr = reinterpret_cast<unsigned char *>(cptrs[i]);
        m_d->releaseWrapper(cptr, sbkObj);
        if (d && d->mi_offsets) {
            int* offset = d->mi_offsets;
            while (*offset != -1) {
                if (*offset > 0)
                    m_d->releaseWrapper(reinterpret_cast<void *>((std::size_t) cptr + (*offset)), sbkObj);
                offset++;
            }
        }
    }
    sbkObj->d->validCppObject = false;
}

SbkObject* BindingManager::retrieveWrapper(const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter == m_d->wrapperMapper.end())
        return 0;
    return iter->second;
}

PyObject* BindingManager::getOverride(const void* cptr, const char* methodName)
{
    SbkObject* wrapper = retrieveWrapper(cptr);
    // The refcount can be 0 if the object is dieing and someone called
    // a virtual method from the destructor
    if (!wrapper || reinterpret_cast<const PyObject *>(wrapper)->ob_refcnt == 0)
        return 0;

    if (wrapper->ob_dict) {
        PyObject* method = PyDict_GetItemString(wrapper->ob_dict, methodName);
        if (method) {
            Py_INCREF(reinterpret_cast<PyObject *>(method));
            return method;
        }
    }

    PyObject* pyMethodName = Shiboken::String::fromCString(methodName);
    PyObject *method = PyObject_GetAttr(reinterpret_cast<PyObject *>(wrapper), pyMethodName);

    if (method && PyMethod_Check(method)
        && PyMethod_GET_SELF(method) == reinterpret_cast<PyObject*>(wrapper)) {
        PyObject* defaultMethod;
        PyObject* mro = Py_TYPE(wrapper)->tp_mro;

        // The first class in the mro (index 0) is the class being checked and it should not be tested.
        // The last class in the mro (size - 1) is the base Python object class which should not be tested also.
        for (int i = 1; i < PyTuple_GET_SIZE(mro) - 1; i++) {
            PyTypeObject* parent = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(mro, i));
            if (parent->tp_dict) {
                defaultMethod = PyDict_GetItem(parent->tp_dict, pyMethodName);
                if (defaultMethod && PyMethod_GET_FUNCTION(method) != defaultMethod) {
                    Py_DECREF(pyMethodName);
                    return method;
                }
            }
        }
    }

    Py_XDECREF(method);
    Py_DECREF(pyMethodName);
    return 0;
}

void BindingManager::addClassInheritance(SbkObjectType* parent, SbkObjectType* child)
{
    m_d->classHierarchy.addEdge(parent, child);
}

SbkObjectType* BindingManager::resolveType(void* cptr, SbkObjectType* type)
{
    return resolveType(&cptr, type);
}

SbkObjectType* BindingManager::resolveType(void** cptr, SbkObjectType* type)
{
    SbkObjectType* identifiedType = m_d->classHierarchy.identifyType(cptr, type, type);
    return identifiedType ? identifiedType : type;
}

std::set<PyObject*> BindingManager::getAllPyObjects()
{
    std::set<PyObject*> pyObjects;
    const WrapperMap& wrappersMap = m_d->wrapperMapper;
    WrapperMap::const_iterator it = wrappersMap.begin();
    for (; it != wrappersMap.end(); ++it)
        pyObjects.insert(reinterpret_cast<PyObject*>(it->second));

    return pyObjects;
}

void BindingManager::visitAllPyObjects(ObjectVisitor visitor, void* data)
{
    WrapperMap copy = m_d->wrapperMapper;
    for (WrapperMap::iterator it = copy.begin(); it != copy.end(); ++it) {
        if (hasWrapper(it->first))
            visitor(it->second, data);
    }
}

} // namespace Shiboken

