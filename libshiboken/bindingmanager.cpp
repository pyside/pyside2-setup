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

#include "basewrapper.h"
#include <cstddef>
#include <fstream>
#include "basewrapper_p.h"
#include "bindingmanager.h"
#include "google/dense_hash_map"
#include "sbkdbg.h"
#include "gilstate.h"

namespace Shiboken
{

typedef google::dense_hash_map<const void*, SbkObject*> WrapperMap;

class Graph
{
public:
    typedef std::list<SbkBaseWrapperType*> NodeList;
    typedef google::dense_hash_map<SbkBaseWrapperType*, NodeList> Edges;

    Edges m_edges;

    Graph()
    {
        m_edges.set_empty_key(0);
    }

    void addEdge(SbkBaseWrapperType* from, SbkBaseWrapperType* to)
    {
        m_edges[from].push_back(to);
    }

#ifndef NDEBUG
    void dumpDotGraph()
    {
        std::ofstream file("/tmp/shiboken_graph.dot");

        file << "digraph D {\n";

        Edges::const_iterator i = m_edges.begin();
        for (; i != m_edges.end(); ++i) {
            SbkBaseWrapperType* node1 = i->first;
            const NodeList& nodeList = i->second;
            NodeList::const_iterator j = nodeList.begin();
            for (; j != nodeList.end(); ++j)
                file << '"' << (*j)->super.ht_type.tp_name << "\" -> \"" << node1->super.ht_type.tp_name << "\"\n";
        }
        file << "}\n";
    }
#endif

    SbkBaseWrapperType* identifyType(void* cptr, SbkBaseWrapperType* type, SbkBaseWrapperType* baseType) const
    {
        Edges::const_iterator edgesIt = m_edges.find(type);
        if (edgesIt != m_edges.end()) {
            const NodeList& adjNodes = m_edges.find(type)->second;
            NodeList::const_iterator i = adjNodes.begin();
            for (; i != adjNodes.end(); ++i) {
                SbkBaseWrapperType* newType = identifyType(cptr, *i, baseType);
                if (newType)
                    return newType;
            }
        }
        return type->type_discovery ? type->type_discovery(cptr, baseType) : 0;
    }
};


#ifndef NDEBUG
static void showWrapperMap(const WrapperMap& wrapperMap)
{
    printf("-------------------------------\n");
    printf("WrapperMap: %p (size: %d)\n", &wrapperMap, (int) wrapperMap.size());
    WrapperMap::const_iterator iter;
    for (iter = wrapperMap.begin(); iter != wrapperMap.end(); ++iter) {
        printf("key: %p, value: %p (%s, refcnt: %d)\n", iter->first,
                                                        iter->second,
                                                        iter->second->ob_type->tp_name,
                                                        (int) iter->second->ob_refcnt);
    }
    printf("-------------------------------\n");
}
#endif

struct BindingManager::BindingManagerPrivate {
    WrapperMap wrapperMapper;
    Graph classHierarchy;
    bool destroying;

    BindingManagerPrivate() : destroying(false) {}
    void releaseWrapper(void* cptr);
    void assignWrapper(SbkObject* wrapper, const void* cptr);

};

void BindingManager::BindingManagerPrivate::releaseWrapper(void* cptr)
{
    WrapperMap::iterator iter = wrapperMapper.find(cptr);
    if (iter != wrapperMapper.end())
        wrapperMapper.erase(iter);
}

void BindingManager::BindingManagerPrivate::assignWrapper(SbkObject* wrapper, const void* cptr)
{
    assert(cptr);
    WrapperMap::iterator iter = wrapperMapper.find(cptr);
    if (iter == wrapperMapper.end())
        wrapperMapper.insert(std::make_pair(cptr, wrapper));
    else
        iter->second = wrapper;
}

BindingManager::BindingManager()
{
    m_d = new BindingManager::BindingManagerPrivate;
    m_d->wrapperMapper.set_empty_key((WrapperMap::key_type)0);
    m_d->wrapperMapper.set_deleted_key((WrapperMap::key_type)1);
}

BindingManager::~BindingManager()
{
#ifndef NDEBUG
    showWrapperMap(m_d->wrapperMapper);
#endif
    /* Cleanup hanging references. We just invalidate them as when
     * the BindingManager is being destroyed the interpreter is alredy
     * shutting down. */
    while (!m_d->wrapperMapper.empty())
        invalidateWrapper(m_d->wrapperMapper.begin()->second);
    assert(m_d->wrapperMapper.size() == 0);
    delete m_d;
}

BindingManager& BindingManager::instance() {
    static BindingManager singleton;
    return singleton;
}

bool BindingManager::hasWrapper(const void* cptr)
{
    return m_d->wrapperMapper.count(cptr);
}
void BindingManager::registerWrapper(SbkObject* pyObj, void* cptr)
{
    SbkBaseWrapperType* instanceType = reinterpret_cast<SbkBaseWrapperType*>(pyObj->ob_type);

    if (instanceType->mi_init && !instanceType->mi_offsets)
        instanceType->mi_offsets = instanceType->mi_init(cptr);
    m_d->assignWrapper(pyObj, cptr);
    if (instanceType->mi_offsets) {
        int* offset = instanceType->mi_offsets;
        while (*offset != -1) {
            if (*offset > 0)
                m_d->assignWrapper(pyObj, reinterpret_cast<void*>((std::size_t) cptr + (*offset)));
            offset++;
        }
    }
}

void BindingManager::releaseWrapper(SbkObject* sbkObj)
{
    SbkBaseWrapperType* sbkType = reinterpret_cast<SbkBaseWrapperType*>(sbkObj->ob_type);
    int numBases = sbkType->is_multicpp ? getNumberOfCppBaseClasses(sbkObj->ob_type) : 1;

    void** cptrs = reinterpret_cast<SbkObject*>(sbkObj)->d->cptr;
    for (int i = 0; i < numBases; ++i) {
        void* cptr = cptrs[i];
        m_d->releaseWrapper(cptr);
        if (sbkType->mi_offsets) {
            int* offset = sbkType->mi_offsets;
            while (*offset != -1) {
                if (*offset > 0)
                    m_d->releaseWrapper((void*) ((std::size_t) cptr + (*offset)));
                offset++;
            }
        }
    }
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
    if (!wrapper)
        return 0;

    if (wrapper->ob_dict) {
        PyObject* method = PyDict_GetItemString(wrapper->ob_dict, methodName);
        if (method) {
            Py_INCREF((PyObject*)method);
            return method;
        }
    }

    PyObject* pyMethodName = PyString_FromString(methodName);
    PyObject* method = PyObject_GetAttr((PyObject*)wrapper, pyMethodName);

    if (method && PyMethod_Check(method)
        && reinterpret_cast<PyMethodObject*>(method)->im_self == reinterpret_cast<PyObject*>(wrapper)) {
        PyObject* defaultMethod;
        PyObject* mro = wrapper->ob_type->tp_mro;

        // The first class in the mro (index 0) is the class being checked and it should not be tested.
        // The last class in the mro (size - 1) is the base Python object class which should not be tested also.
        for (int i = 1; i < PyTuple_GET_SIZE(mro) - 1; i++) {
            PyTypeObject* parent = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(mro, i));
            if (parent->tp_dict) {
                defaultMethod = PyDict_GetItem(parent->tp_dict, pyMethodName);
                if (defaultMethod && reinterpret_cast<PyMethodObject*>(method)->im_func != defaultMethod) {
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


void BindingManager::invalidateWrapper(PyObject* pyobj)
{
    std::list<SbkObject*> objs = splitPyObject(pyobj);
    std::list<SbkObject*>::const_iterator it = objs.begin();
    for(; it != objs.end(); it++)
        invalidateWrapper(*it);
}

void BindingManager::invalidateWrapper(SbkObject* wrapper)
{
    if (!wrapper || ((PyObject*)wrapper == Py_None) || !wrapper->d->validCppObject)
        return;

    GilState gil; // lock the gil to assure no one is changing the value of m_d->destroying

    // skip this if the object is a wrapper class and this is not a destructor call
    if (wrapper->d->containsCppWrapper && !m_d->destroying) {
        ParentInfo* pInfo = wrapper->d->parentInfo;
        // this meaning the object has a extra ref and we will remove this now
        if (pInfo && pInfo->hasWrapperRef) {
            delete pInfo;
            wrapper->d->parentInfo = 0;
            Py_XDECREF((PyObject*) wrapper);
        }
        return;
    }

    wrapper->d->validCppObject = false;
    wrapper->d->hasOwnership = false;

    // If it is a parent invalidate all children.
    if (wrapper->d->parentInfo) {
        ChildrenList::iterator it = wrapper->d->parentInfo->children.begin();
        bool parentDestroying = m_d->destroying;
        m_d->destroying = false;
        for (; it != wrapper->d->parentInfo->children.end(); ++it)
            invalidateWrapper(*it);
        m_d->destroying = parentDestroying;
    }

    releaseWrapper(wrapper);
}

void BindingManager::invalidateWrapper(const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter != m_d->wrapperMapper.end())
        invalidateWrapper(iter->second);
}

void BindingManager::destroyWrapper(const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter != m_d->wrapperMapper.end())
        destroyWrapper(reinterpret_cast<SbkObject*>(iter->second));
}

void BindingManager::destroyWrapper(SbkObject* wrapper)
{
    GilState gil;
    m_d->destroying = true;
    invalidateWrapper(wrapper);
    m_d->destroying = false;
}

void BindingManager::transferOwnershipToCpp(PyObject* wrapper)
{
    std::list<SbkObject*> objs = splitPyObject(wrapper);
    std::list<SbkObject*>::const_iterator it = objs.begin();
    for(; it != objs.end(); it++)
        transferOwnershipToCpp(*it);
}

void BindingManager::transferOwnershipToCpp(SbkObject* wrapper)
{
    if (wrapper->d->parentInfo)
        Shiboken::removeParent(wrapper);

    if (wrapper->d->containsCppWrapper)
        wrapper->d->hasOwnership = false;
    else
        invalidateWrapper(wrapper);
}

void BindingManager::addClassInheritance(SbkBaseWrapperType* parent, SbkBaseWrapperType* child)
{
    m_d->classHierarchy.addEdge(parent, child);
}

SbkBaseWrapperType* BindingManager::resolveType(void* cptr, SbkBaseWrapperType* type)
{
    SbkBaseWrapperType* identifiedType = m_d->classHierarchy.identifyType(cptr, type, type);
    return identifiedType ? identifiedType : type;
}

std::set<SbkObject*> BindingManager::getAllPyObjects()
{
    std::set<SbkObject*> pyObjects;
    const WrapperMap& wrappersMap = m_d->wrapperMapper;
    WrapperMap::const_iterator it = wrappersMap.begin();
    for (; it != wrappersMap.end(); ++it)
        pyObjects.insert(it->second);

    return pyObjects;
}

} // namespace Shiboken

