/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "basewrapper.h"
#include "basewrapper_p.h"
#include <cstddef>
#include <algorithm>
#include "autodecref.h"
#include "typeresolver.h"
#include <string>
#include <cstring>

namespace Shiboken
{

void removeParent(SbkBaseWrapper* child)
{
    if (!child->parentInfo->parent)
        return;

    ChildrenList& oldBrothers = child->parentInfo->parent->parentInfo->children;
    oldBrothers.remove(child);
    child->parentInfo->parent = 0;
    Py_DECREF(child);
}

void setParent(PyObject* parent, PyObject* child)
{
    if (!child || child == Py_None || child == parent)
        return;

    /*
     *  setParent is recursive when the child is a native Python sequence, i.e. objects not binded by Shiboken
     *  like tuple and list.
     *
     *  This "limitation" exists to fix the following problem: A class multiple inherits QObject and QString,
     *  so if you pass this class to someone that takes the ownership, we CAN'T enter in this if, but hey! QString
     *  follows the sequence protocol.
     */
    if (PySequence_Check(child) && !isShibokenType(child)) {
        Shiboken::AutoDecRef seq(PySequence_Fast(child, 0));
        for (int i = 0, max = PySequence_Size(seq); i < max; ++i)
            setParent(parent, PySequence_Fast_GET_ITEM(seq.object(), i));
        return;
    }

    bool parentIsNull = !parent || parent == Py_None;
    SbkBaseWrapper* parent_ = reinterpret_cast<SbkBaseWrapper*>(parent);
    SbkBaseWrapper* child_ = reinterpret_cast<SbkBaseWrapper*>(child);
    if (!child_->parentInfo)
        child_->parentInfo = new ParentInfo;

    if (!parentIsNull) {
        if (!parent_->parentInfo)
            parent_->parentInfo = new ParentInfo;
        // do not re-add a child
        ChildrenList& children = parent_->parentInfo->children;
        if (std::find(children.begin(), children.end(), child_) != children.end())
            return;
    }

    bool hasAnotherParent = child_->parentInfo->parent && child_->parentInfo->parent != parent_;

    //Avoid destroy child during reparent operation
    Py_INCREF(child);

    // check if we need to remove this child from the old parent
    if (parentIsNull || hasAnotherParent)
        removeParent(child_);

    // Add the child to the new parent
    if (!parentIsNull) {
        child_->parentInfo->parent = parent_;
        parent_->parentInfo->children.push_back(child_);
        Py_INCREF(child_);
    }

    Py_DECREF(child);
}

static void _destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    if (removeFromParent && obj->parentInfo->parent)
        removeParent(obj);
    ChildrenList::iterator it = obj->parentInfo->children.begin();
    for (; it != obj->parentInfo->children.end(); ++it) {
        SbkBaseWrapper*& child = *it;
        _destroyParentInfo(child, false);
        Py_DECREF(child);
    }
    delete obj->parentInfo;
    obj->parentInfo = 0;
}

void destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    BindingManager::instance().invalidateWrapper(obj);
    _destroyParentInfo(obj, removeFromParent);
}

PyObject* SbkBaseWrapper_New(SbkBaseWrapperType* instanceType,
                             void* cptr,
                             bool hasOwnership,
                             bool isExactType)
{
    // Try to find the exact type of cptr.
    if (!isExactType && instanceType->type_discovery)
        instanceType = instanceType->type_discovery->getType(cptr, instanceType);

    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(SbkBaseWrapper_TpNew(reinterpret_cast<PyTypeObject*>(instanceType), 0, 0));
    self->cptr[0] = cptr;
    self->hasOwnership = hasOwnership;
    self->validCppObject = 1;
    BindingManager::instance().registerWrapper(self, cptr);
    return reinterpret_cast<PyObject*>(self);
}

void walkThroughClassHierarchy(PyTypeObject* currentType, HierarchyVisitor* visitor)
{
    PyObject* bases = currentType->tp_bases;
    Py_ssize_t numBases = PyTuple_GET_SIZE(bases);
    for (int i = 0; i < numBases; ++i) {
        PyTypeObject* type = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(bases, i));

        if (type->ob_type != &Shiboken::SbkBaseWrapperType_Type) {
            continue;
        } else {
            SbkBaseWrapperType* sbkType = reinterpret_cast<SbkBaseWrapperType*>(type);
            if (sbkType->is_multicpp)
                walkThroughClassHierarchy(type, visitor);
            else
                visitor->visit(sbkType);
        }
        if (visitor->wasFinished())
            return;
    }
}

PyObject* SbkBaseWrapper_TpNew(PyTypeObject* subtype, PyObject*, PyObject*)
{
    Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(PyBaseObject_Type.tp_new(subtype, emptyTuple, 0));

    int numBases = reinterpret_cast<SbkBaseWrapperType*>(subtype)->is_multicpp ? getNumberOfCppBaseClasses(subtype) : 1;
    self->cptr = new void*[numBases];
    std::memset(self->cptr, 0, sizeof(void*)*numBases);
    self->hasOwnership = 1;
    self->containsCppWrapper = 0;
    self->validCppObject = 0;
    self->parentInfo = 0;
    self->ob_dict = 0;
    self->weakreflist = 0;
    self->referredObjects = 0;
    return reinterpret_cast<PyObject*>(self);
}

void* getCppPointer(PyObject* wrapper, PyTypeObject* desiredType)
{
    PyTypeObject* type = wrapper->ob_type;
    int idx = 0;
    if (reinterpret_cast<SbkBaseWrapperType*>(type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);
    return reinterpret_cast<Shiboken::SbkBaseWrapper*>(wrapper)->cptr[idx];
}

bool setCppPointer(SbkBaseWrapper* wrapper, PyTypeObject* desiredType, void* cptr)
{
    int idx = 0;
    if (reinterpret_cast<SbkBaseWrapperType*>(wrapper->ob_type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(wrapper->ob_type, desiredType);

    bool alreadyInitialized = wrapper->cptr[idx];
    if (alreadyInitialized)
        PyErr_SetString(PyExc_RuntimeError, "You can't initialize an object twice!");
    else
        wrapper->cptr[idx] = cptr;

    return !alreadyInitialized;
}

bool cppObjectIsInvalid(PyObject* wrapper)
{
    if (wrapper == Py_None
        || wrapper->ob_type->ob_type != &Shiboken::SbkBaseWrapperType_Type
        || ((Shiboken::SbkBaseWrapper*)wrapper)->validCppObject) {
        return false;
    }
    PyErr_SetString(PyExc_RuntimeError, "Internal C++ object already deleted.");
    return true;
}

void SbkBaseWrapper_Dealloc_PrivateDtor(PyObject* self)
{
    if (((SbkBaseWrapper *)self)->weakreflist)
        PyObject_ClearWeakRefs(self);

    BindingManager::instance().releaseWrapper(self);
    SbkBaseWrapper_clearReferences(reinterpret_cast<SbkBaseWrapper*>(self));
    Py_TYPE(reinterpret_cast<SbkBaseWrapper*>(self))->tp_free(self);
}

void SbkBaseWrapper_keepReference(SbkBaseWrapper* self, const char* key, PyObject* referredObject)
{
    if (!self->referredObjects)
        return;
    RefCountMap& refCountMap = *(self->referredObjects);
    Py_INCREF(referredObject);
    RefCountMap::iterator iter = refCountMap.find(key);
    if (iter != refCountMap.end())
        Py_DECREF(iter->second);
    refCountMap[key] = referredObject;
}

void SbkBaseWrapper_clearReferences(SbkBaseWrapper* self)
{
    if (!self->referredObjects)
        return;
    RefCountMap& refCountMap = *(self->referredObjects);
    RefCountMap::iterator iter;
    for (iter = refCountMap.begin(); iter != refCountMap.end(); ++iter)
        Py_DECREF(iter->second);
    delete self->referredObjects;
}

bool importModule(const char* moduleName, PyTypeObject*** cppApiPtr)
{
    Shiboken::AutoDecRef module(PyImport_ImportModule(moduleName));
    if (module.isNull())
        return false;

    Shiboken::AutoDecRef cppApi(PyObject_GetAttrString(module, "_Cpp_Api"));
    if (cppApi.isNull())
        return false;

    if (PyCObject_Check(cppApi))
        *cppApiPtr = reinterpret_cast<PyTypeObject**>(PyCObject_AsVoidPtr(cppApi));

    return true;
}

// Wrapper metatype and base type ----------------------------------------------------------

class DtorCallerVisitor : public HierarchyVisitor
{
public:
    DtorCallerVisitor(SbkBaseWrapper* pyObj) : m_count(0), m_pyObj(pyObj) {}
    virtual void visit(SbkBaseWrapperType* node)
    {
        node->cpp_dtor(m_pyObj->cptr[m_count]);
        m_count++;
    }
private:
    int m_count;
    SbkBaseWrapper* m_pyObj;
};

static void deallocPythonTypes(PyObject* pyObj)
{
    SbkBaseWrapper* sbkObj = reinterpret_cast<SbkBaseWrapper*>(pyObj);
    if (sbkObj->weakreflist)
        PyObject_ClearWeakRefs(pyObj);

    BindingManager::instance().releaseWrapper(pyObj);
    if (SbkBaseWrapper_hasOwnership(sbkObj)) {
        DtorCallerVisitor visitor(sbkObj);
        walkThroughClassHierarchy(pyObj->ob_type, &visitor);
    }

    if (SbkBaseWrapper_hasParentInfo(sbkObj))
        destroyParentInfo(sbkObj);
    SbkBaseWrapper_clearReferences(sbkObj);

    delete[] sbkObj->cptr;
    sbkObj->cptr = 0;
    Py_TYPE(pyObj)->tp_free(pyObj);
}

void deallocWrapper(PyObject* pyObj)
{
    SbkBaseWrapper* sbkObj = reinterpret_cast<SbkBaseWrapper*>(pyObj);
    if (sbkObj->weakreflist)
        PyObject_ClearWeakRefs(pyObj);

    BindingManager::instance().releaseWrapper(pyObj);
    if (SbkBaseWrapper_hasOwnership(pyObj)) {
        SbkBaseWrapperType* sbkType = reinterpret_cast<SbkBaseWrapperType*>(pyObj->ob_type);
        assert(!sbkType->is_multicpp);
        sbkType->cpp_dtor(sbkObj->cptr[0]);
    }

    if (SbkBaseWrapper_hasParentInfo(pyObj))
        destroyParentInfo(sbkObj);
    SbkBaseWrapper_clearReferences(sbkObj);

    delete[] sbkObj->cptr;
    sbkObj->cptr = 0;
    Py_TYPE(pyObj)->tp_free(pyObj);
}

static PyObject*
SbkBaseWrapperType_TpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds)
{
    // The meta type creates a new type when the Python programmer extends a wrapped C++ class.
    SbkBaseWrapperType* newType = reinterpret_cast<SbkBaseWrapperType*>(PyType_Type.tp_new(metatype, args, kwds));

    if (!newType)
        return 0;

    std::list<SbkBaseWrapperType*> bases = getCppBaseClasses(reinterpret_cast<PyTypeObject*>(newType));
    if (bases.size() == 1) {
        SbkBaseWrapperType* parentType = bases.front();
        newType->super.ht_type.tp_dealloc = parentType->super.ht_type.tp_dealloc;
        newType->mi_offsets = parentType->mi_offsets;
        newType->mi_init = parentType->mi_init;
        newType->mi_specialcast = parentType->mi_specialcast;
        newType->ext_isconvertible = parentType->ext_isconvertible;
        newType->ext_tocpp = parentType->ext_tocpp;
        newType->type_discovery = parentType->type_discovery;
        newType->obj_copier = parentType->obj_copier;
        newType->cpp_dtor = parentType->cpp_dtor;
        newType->is_multicpp = 0;
    } else {
        newType->super.ht_type.tp_dealloc = &deallocPythonTypes;
        newType->mi_offsets = 0;
        newType->mi_init = 0;
        newType->mi_specialcast = 0;
        newType->ext_isconvertible = 0;
        newType->ext_tocpp = 0;
        newType->type_discovery = 0;
        newType->obj_copier = 0;
        newType->cpp_dtor = 0;
        newType->is_multicpp = 1;
    }
    return reinterpret_cast<PyObject*>(newType);
}

PyTypeObject SbkBaseWrapperType_Type = {
    PyObject_HEAD_INIT(0)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapperType",
    /*tp_basicsize*/        sizeof(SbkBaseWrapperType),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    /*tp_doc*/              0,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             &PyType_Type,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              SbkBaseWrapperType_TpNew,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
};

static PyObject* SbkBaseWrapper_get_dict(SbkBaseWrapper* obj)
{
    if (!obj->ob_dict)
        obj->ob_dict = PyDict_New();
    if (!obj->ob_dict)
        return 0;
    Py_INCREF(obj->ob_dict);
    return obj->ob_dict;
}

static PyGetSetDef SbkBaseWrapper_getsetlist[] = {
    {"__dict__", (getter)SbkBaseWrapper_get_dict, 0},
    {0} // Sentinel
};

SbkBaseWrapperType SbkBaseWrapper_Type = { { {
    PyObject_HEAD_INIT(&SbkBaseWrapperType_Type)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapper",
    /*tp_basicsize*/        sizeof(SbkBaseWrapper),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    /*tp_doc*/              0,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   offsetof(SbkBaseWrapper, weakreflist),
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           SbkBaseWrapper_getsetlist,
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       offsetof(SbkBaseWrapper, ob_dict),
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              0,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
}, },
    /*mi_offsets*/          0,
    /*mi_init*/             0,
    /*mi_specialcast*/      0,
    /*type_name_func*/      0,
    /*ext_isconvertible*/   0,
    /*ext_tocpp*/           0
};

void initShiboken()
{
    static bool shibokenAlreadInitialised = false;
    if (shibokenAlreadInitialised)
        return;

#ifdef WITH_THREAD
    PyEval_InitThreads();
#endif

    if (PyType_Ready(&SbkBaseWrapperType_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapperType metatype.");

    if (PyType_Ready((PyTypeObject *)&SbkBaseWrapper_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapper type.");

    shibokenAlreadInitialised = true;
}

void setErrorAboutWrongArguments(PyObject* args, const char* funcName, const char** cppOverloads)
{
    std::string msg;
    std::string params;
    if (args) {
        if (PyTuple_Check(args)) {
            for (int i = 0, max = PyTuple_GET_SIZE(args); i < max; ++i) {
                if (i)
                    params += ", ";
                params += PyTuple_GET_ITEM(args, i)->ob_type->tp_name;
            }
        } else {
            params = args->ob_type->tp_name;
        }
    }

    if (!cppOverloads) {
        msg = "'" + std::string(funcName) + "' called with wrong argument types: " + params;
    } else {
        msg = "'" + std::string(funcName) + "' called with wrong argument types:\n  ";
        msg += funcName;
        msg += '(';
        msg += params;
        msg += ")\n";
        msg += "Supported signatures:";
        for (int i = 0; cppOverloads[i]; ++i) {
            msg += "\n  ";
            msg += funcName;
            msg += '(';
            msg += cppOverloads[i];
            msg += ')';
        }
    }
    PyErr_SetString(PyExc_TypeError, msg.c_str());

}

SbkBaseWrapperType* TypeDiscovery::getType(const void* cptr, SbkBaseWrapperType* instanceType) const
{
    TypeDiscoveryFuncList::const_reverse_iterator it = m_discoveryFunctions.rbegin();
    for (; it != m_discoveryFunctions.rend(); ++it) {
        SbkBaseWrapperType* type = (*it)(const_cast<void*>(cptr), instanceType);
        if (type)
            return type;
    }
    return instanceType;
}

void TypeDiscovery::addTypeDiscoveryFunction(Shiboken::TypeDiscoveryFunc func)
{
    m_discoveryFunctions.push_back(func);
}

} // namespace Shiboken


