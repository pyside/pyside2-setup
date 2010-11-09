/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "basewrapper_p.h"
#include "sbkenum.h"
#include <cstddef>
#include <algorithm>
#include "autodecref.h"
#include "typeresolver.h"
#include <string>
#include <cstring>

namespace Shiboken
{

static void SbkBaseWrapperType_dealloc(PyObject* pyObj);
static PyObject* SbkBaseWrapperType_TpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds);
static void incRefPyObject(PyObject* pyObj);
static void decRefPyObjectlist(const std::list<SbkBaseWrapper*> &pyObj);

extern "C"
{

PyTypeObject SbkBaseWrapperType_Type = {
    PyObject_HEAD_INIT(0)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapperType",
    /*tp_basicsize*/        sizeof(SbkBaseWrapperType),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          SbkBaseWrapperType_dealloc,
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
    {const_cast<char*>("__dict__"), (getter)SbkBaseWrapper_get_dict, 0},
    {0} // Sentinel
};

SbkBaseWrapperType SbkBaseWrapper_Type = { { {
    PyObject_HEAD_INIT(&SbkBaseWrapperType_Type)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapper",
    /*tp_basicsize*/        sizeof(SbkBaseWrapper),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          deallocWrapperWithPrivateDtor,
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

} //extern "C"

void removeParent(SbkBaseWrapper* child)
{
    ParentInfo* pInfo = child->d->parentInfo;
    if (!pInfo || !pInfo->parent)
        return;

    ChildrenList& oldBrothers = pInfo->parent->d->parentInfo->children;
    oldBrothers.remove(child);
    pInfo->parent = 0;

    if (pInfo->hasWrapperRef) {
        Py_DECREF(child);
        pInfo->hasWrapperRef = false;
    }
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

    if (!parentIsNull) {
        if (!parent_->d->parentInfo)
            parent_->d->parentInfo = new ParentInfo;
        // do not re-add a child
        ChildrenList& children = parent_->d->parentInfo->children;
        if (std::find(children.begin(), children.end(), child_) != children.end())
            return;
    }

    ParentInfo* pInfo = child_->d->parentInfo;
    bool hasAnotherParent = pInfo && pInfo->parent && pInfo->parent != parent_;

    //Avoid destroy child during reparent operation
    Py_INCREF(child);

    // check if we need to remove this child from the old parent
    if (parentIsNull || hasAnotherParent)
        removeParent(child_);

    // Add the child to the new parent
    pInfo = child_->d->parentInfo;
    if (!parentIsNull) {
        if (!pInfo)
            pInfo = child_->d->parentInfo = new ParentInfo;
        pInfo->parent = parent_;
        parent_->d->parentInfo->children.push_back(child_);
        Py_INCREF(child_);
    }

    Py_DECREF(child);
}

static void _destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    ParentInfo* pInfo = obj->d->parentInfo;
    if (removeFromParent && pInfo && pInfo->parent)
        removeParent(obj);

    if (pInfo) {
        ChildrenList::iterator it = pInfo->children.begin();
        for (; it != pInfo->children.end(); ++it) {
            SbkBaseWrapper*& child = *it;

            // keep this, the wrapper still alive
            if (!obj->d->containsCppWrapper && child->d->containsCppWrapper && child->d->parentInfo) {
                child->d->parentInfo->parent = 0;
                child->d->parentInfo->hasWrapperRef = true;
                child->d->hasOwnership = false;
            } else {
                _destroyParentInfo(child, false);
                Py_DECREF(child);
            }
        }
        delete pInfo;
        obj->d->parentInfo = 0;
    }
}

void destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    BindingManager::instance().destroyWrapper(obj);
    _destroyParentInfo(obj, removeFromParent);
}

PyObject* SbkBaseWrapper_New(SbkBaseWrapperType* instanceType,
                             void* cptr,
                             bool hasOwnership,
                             bool isExactType, const char* typeName)
{
    // Try to find the exact type of cptr.
    if (!isExactType) {
        TypeResolver* tr = 0;
        if (typeName) {
            tr = TypeResolver::get(typeName);
            if (tr)
                instanceType = reinterpret_cast<SbkBaseWrapperType*>(tr->pythonType());
        }
        if (!tr)
            instanceType = BindingManager::instance().resolveType(cptr, instanceType);
    }

    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(SbkBaseWrapper_TpNew(reinterpret_cast<PyTypeObject*>(instanceType), 0, 0));
    self->d->cptr[0] = cptr;
    self->d->hasOwnership = hasOwnership;
    self->d->validCppObject = 1;
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
            if (sbkType->is_user_type)
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
    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(subtype->tp_alloc(subtype, 0));
    self->d = new SbkBaseWrapperPrivate;

    SbkBaseWrapperType* sbkType = reinterpret_cast<SbkBaseWrapperType*>(subtype);
    int numBases = sbkType->is_multicpp ? getNumberOfCppBaseClasses(subtype) : 1;
    self->d->cptr = new void*[numBases];
    std::memset(self->d->cptr, 0, sizeof(void*)*numBases);
    self->d->hasOwnership = 1;
    self->d->containsCppWrapper = 0;
    self->d->validCppObject = 0;
    self->d->parentInfo = 0;
    self->ob_dict = 0;
    self->weakreflist = 0;
    self->d->referredObjects = 0;
    return reinterpret_cast<PyObject*>(self);
}

void* getCppPointer(PyObject* wrapper, PyTypeObject* desiredType)
{
    PyTypeObject* type = wrapper->ob_type;
    int idx = 0;
    if (reinterpret_cast<SbkBaseWrapperType*>(type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);
    return reinterpret_cast<SbkBaseWrapper*>(wrapper)->d->cptr[idx];
}

bool setCppPointer(SbkBaseWrapper* wrapper, PyTypeObject* desiredType, void* cptr)
{
    int idx = 0;
    if (reinterpret_cast<SbkBaseWrapperType*>(wrapper->ob_type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(wrapper->ob_type, desiredType);

    bool alreadyInitialized = wrapper->d->cptr[idx];
    if (alreadyInitialized)
        PyErr_SetString(PyExc_RuntimeError, "You can't initialize an object twice!");
    else
        wrapper->d->cptr[idx] = cptr;

    return !alreadyInitialized;
}

bool cppObjectIsInvalid(PyObject* wrapper)
{
    if (!wrapper || wrapper == Py_None
        || wrapper->ob_type->ob_type != &Shiboken::SbkBaseWrapperType_Type
        || ((SbkBaseWrapper*)wrapper)->d->validCppObject) {
        return false;
    }
    PyErr_SetString(PyExc_RuntimeError, "Internal C++ object already deleted.");
    return true;
}

void setTypeUserData(SbkBaseWrapper* wrapper, void *user_data, DeleteUserDataFunc d_func)
{
    SbkBaseWrapperType* ob_type = reinterpret_cast<SbkBaseWrapperType*>(wrapper->ob_type);
    if (ob_type->user_data)
        ob_type->d_func(ob_type->user_data);

    ob_type->d_func = d_func;
    ob_type->user_data = user_data;
}

void* getTypeUserData(SbkBaseWrapper* wrapper)
{
    return reinterpret_cast<SbkBaseWrapperType*>(wrapper->ob_type)->user_data;
}

void deallocWrapperWithPrivateDtor(PyObject* self)
{
    if (((SbkBaseWrapper *)self)->weakreflist)
        PyObject_ClearWeakRefs(self);

    BindingManager::instance().releaseWrapper(self);
    clearReferences(reinterpret_cast<SbkBaseWrapper*>(self));
    Py_TYPE(reinterpret_cast<SbkBaseWrapper*>(self))->tp_free(self);
}

void keepReference(SbkBaseWrapper* self, const char* key, PyObject* referredObject, bool append)
{

    bool isNone = (!referredObject || (referredObject == Py_None));

    if (!self->d->referredObjects)
        self->d->referredObjects = new Shiboken::RefCountMap;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    if (!isNone)
        incRefPyObject(referredObject);

    RefCountMap::iterator iter = refCountMap.find(key);
    if (!append && (iter != refCountMap.end())) {
        decRefPyObjectlist(iter->second);
        refCountMap.erase(iter);
    }

    if (!isNone) {
        std::list<SbkBaseWrapper*> values = splitPyObject(referredObject);
        if (append && (iter != refCountMap.end()))
            refCountMap[key].insert(refCountMap[key].end(), values.begin(), values.end());
        else
            refCountMap[key] = values;
    }
}

void clearReferences(SbkBaseWrapper* self)
{
    if (!self->d->referredObjects)
        return;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    RefCountMap::iterator iter;
    for (iter = refCountMap.begin(); iter != refCountMap.end(); ++iter)
        decRefPyObjectlist(iter->second);
    delete self->d->referredObjects;
    self->d->referredObjects = 0;
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
        node->cpp_dtor(m_pyObj->d->cptr[m_count]);
        m_count++;
    }
private:
    int m_count;
    SbkBaseWrapper* m_pyObj;
};

void deallocWrapper(PyObject* pyObj)
{
    SbkBaseWrapper* sbkObj = reinterpret_cast<SbkBaseWrapper*>(pyObj);
    if (sbkObj->weakreflist)
        PyObject_ClearWeakRefs(pyObj);

    BindingManager::instance().releaseWrapper(pyObj);
    if (sbkObj->d->hasOwnership) {
        SbkBaseWrapperType* sbkType = reinterpret_cast<SbkBaseWrapperType*>(pyObj->ob_type);
        if (sbkType->is_multicpp) {
            DtorCallerVisitor visitor(sbkObj);
            walkThroughClassHierarchy(pyObj->ob_type, &visitor);
        } else {
            sbkType->cpp_dtor(sbkObj->d->cptr[0]);
        }
    }

    if (sbkObj->d->parentInfo)
        destroyParentInfo(sbkObj);

    clearReferences(sbkObj);

    Py_XDECREF(sbkObj->ob_dict);
    delete[] sbkObj->d->cptr;
    sbkObj->d->cptr = 0;
    delete sbkObj->d;
    Py_TYPE(pyObj)->tp_free(pyObj);

}

void SbkBaseWrapperType_dealloc(PyObject* pyObj)
{
    SbkBaseWrapperType *sbkType = reinterpret_cast<SbkBaseWrapperType*>(pyObj->ob_type);

    if(sbkType->user_data && sbkType->d_func) {
        sbkType->d_func(sbkType->user_data);
        sbkType->user_data = 0;
    }
}

PyObject* SbkBaseWrapperType_TpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds)
{
    // The meta type creates a new type when the Python programmer extends a wrapped C++ class.
    SbkBaseWrapperType* newType = reinterpret_cast<SbkBaseWrapperType*>(PyType_Type.tp_new(metatype, args, kwds));

    if (!newType)
        return 0;

    std::list<SbkBaseWrapperType*> bases = getCppBaseClasses(reinterpret_cast<PyTypeObject*>(newType));
    if (bases.size() == 1) {
        SbkBaseWrapperType* parentType = bases.front();
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
    if (bases.size() == 1)
        newType->original_name = bases.front()->original_name;
    else
        newType->original_name = "object";
    newType->user_data = 0;
    newType->d_func = 0;
    newType->is_user_type = 1;
    return reinterpret_cast<PyObject*>(newType);
}

void initShiboken()
{
    static bool shibokenAlreadInitialised = false;
    if (shibokenAlreadInitialised)
        return;

    initTypeResolver();
#ifdef WITH_THREAD
    PyEval_InitThreads();
#endif

    if (PyType_Ready(&SbkEnumType_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.SbkEnumType metatype.");

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
                PyObject* arg = PyTuple_GET_ITEM(args, i);
                if (PyCObject_Check(arg))
                    params += "pointer";
                else
                    params += arg->ob_type->tp_name;
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

class FindBaseTypeVisitor : public HierarchyVisitor
{
    public:
        FindBaseTypeVisitor(PyTypeObject* typeToFind) : m_found(false), m_typeToFind(typeToFind) {}
        virtual void visit(SbkBaseWrapperType* node)
        {
            if (reinterpret_cast<PyTypeObject*>(node) == m_typeToFind) {
                m_found = true;
                finish();
            }
        }
        bool found() const { return m_found; }

    private:
        bool m_found;
        PyTypeObject* m_typeToFind;
};

bool canCallConstructor(PyTypeObject* myType, PyTypeObject* ctorType)
{
    FindBaseTypeVisitor visitor(ctorType);
    walkThroughClassHierarchy(myType, &visitor);
    if (!visitor.found()) {
        PyErr_Format(PyExc_TypeError, "%s isn't a direct base class of %s", ctorType->tp_name, myType->tp_name);
        return false;
    }
    return true;
}

std::list<SbkBaseWrapper*> splitPyObject(PyObject* pyObj)
{
    std::list<SbkBaseWrapper*> result;
    if (PySequence_Check(pyObj)) {
        AutoDecRef lst(PySequence_Fast(pyObj, "Invalid keep reference object."));
        if (!lst.isNull()) {
            for(int i = 0, i_max = PySequence_Fast_GET_SIZE(lst.object()); i < i_max; i++) {
                PyObject* item = PySequence_Fast_GET_ITEM(lst.object(), i);
                if (isShibokenType(item))
                    result.push_back(reinterpret_cast<SbkBaseWrapper*>(item));
            }
        }
    } else {
        result.push_back(reinterpret_cast<SbkBaseWrapper*>(pyObj));
    }
    return result;
}

static void incRefPyObject(PyObject* pyObj)
{
    if (PySequence_Check(pyObj)) {
        for(int i = 0, i_max = PySequence_Size(pyObj); i < i_max; i++) {
            PySequence_GetItem(pyObj, i);
        }
    } else {
        Py_INCREF(pyObj);
    }
}

static void decRefPyObjectlist(const std::list<SbkBaseWrapper*> &lst)
{
    std::list<SbkBaseWrapper*>::const_iterator iter = lst.begin();
    while(iter != lst.end()) {
        Py_DECREF(*iter);
        ++iter;
    }
}

static void setSequenceOwnership(PyObject* pyObj, bool owner)
{
    if (PySequence_Check(pyObj)) {
        std::list<SbkBaseWrapper*> objs = splitPyObject(pyObj);
        std::list<SbkBaseWrapper*>::const_iterator it = objs.begin();
        for(; it != objs.end(); ++it)
            (*it)->d->hasOwnership = owner;
    } else if (isShibokenType(pyObj)) {
        reinterpret_cast<SbkBaseWrapper*>(pyObj)->d->hasOwnership = owner;
    }
}

namespace Wrapper
{

void setValidCpp(SbkBaseWrapper* pyObj, bool value)
{
    pyObj->d->validCppObject = value;
}

void setHasCppWrapper(SbkBaseWrapper* pyObj, bool value)
{
    pyObj->d->containsCppWrapper = value;
}

bool hasCppWrapper(SbkBaseWrapper* pyObj)
{
    return pyObj->d->containsCppWrapper;
}

void getOwnership(SbkBaseWrapper* pyObj)
{
    pyObj->d->hasOwnership = true;
}

void getOwnership(PyObject* pyObj)
{
    setSequenceOwnership(pyObj, true);
}

void releaseOwnership(SbkBaseWrapper* pyObj)
{
    pyObj->d->hasOwnership = false;
}

void releaseOwnership(PyObject* pyObj)
{
    setSequenceOwnership(pyObj, false);
}

} // namespace Wrapper

} // namespace Shiboken


