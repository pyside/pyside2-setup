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
#include "autodecref.h"
#include "typeresolver.h"
#include "gilstate.h"
#include <string>
#include <cstring>
#include <cstddef>
#include <algorithm>

extern "C"
{

static void SbkObjectTypeDealloc(PyObject* pyObj);
static PyObject* SbkObjectTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds);

PyTypeObject SbkObjectType_Type = {
    PyObject_HEAD_INIT(0)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.ObjectType",
    /*tp_basicsize*/        sizeof(SbkObjectType),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          SbkObjectTypeDealloc,
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
    /*tp_new*/              SbkObjectTypeTpNew,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
};

static PyObject* SbkObjectGetDict(SbkObject* obj)
{
    if (!obj->ob_dict)
        obj->ob_dict = PyDict_New();
    if (!obj->ob_dict)
        return 0;
    Py_INCREF(obj->ob_dict);
    return obj->ob_dict;
}

static PyGetSetDef SbkObjectGetSetList[] = {
    {const_cast<char*>("__dict__"), (getter)SbkObjectGetDict, 0},
    {0} // Sentinel
};

SbkObjectType SbkObject_Type = { { {
    PyObject_HEAD_INIT(&SbkObjectType_Type)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.Object",
    /*tp_basicsize*/        sizeof(SbkObject),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          SbkDeallocWrapperWithPrivateDtor,
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
    /*tp_weaklistoffset*/   offsetof(SbkObject, weakreflist),
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           SbkObjectGetSetList,
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       offsetof(SbkObject, ob_dict),
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
    /*priv_data*/           0
};


void SbkDeallocWrapper(PyObject* pyObj)
{
    SbkObject* sbkObj = reinterpret_cast<SbkObject*>(pyObj);
    if (sbkObj->weakreflist)
        PyObject_ClearWeakRefs(pyObj);

    // If I have ownership and is valid delete C++ pointer
    if (sbkObj->d->hasOwnership && sbkObj->d->validCppObject) {
        SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(pyObj->ob_type);
        if (sbkType->d->is_multicpp) {
            Shiboken::DtorCallerVisitor visitor(sbkObj);
            Shiboken::walkThroughClassHierarchy(pyObj->ob_type, &visitor);
        } else {
            sbkType->d->cpp_dtor(sbkObj->d->cptr[0]);
        }
    }

    Shiboken::Object::deallocData(sbkObj);
}

void SbkDeallocWrapperWithPrivateDtor(PyObject* self)
{
    SbkObject* sbkObj = reinterpret_cast<SbkObject*>(self);
    if (sbkObj->weakreflist)
        PyObject_ClearWeakRefs(self);

    Shiboken::BindingManager::instance().releaseWrapper(sbkObj);
    Shiboken::Object::deallocData(sbkObj);
}

void SbkObjectTypeDealloc(PyObject* pyObj)
{
    SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(pyObj->ob_type);
    if (!sbkType->d)
        return;

    if(sbkType->d->user_data && sbkType->d->d_func) {
        sbkType->d->d_func(sbkType->d->user_data);
        sbkType->d->user_data = 0;
    }
    free(sbkType->d->original_name);
    sbkType->d->original_name = 0;
    delete sbkType->d;
    sbkType->d = 0;
}

PyObject* SbkObjectTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds)
{
    // The meta type creates a new type when the Python programmer extends a wrapped C++ class.
    SbkObjectType* newType = reinterpret_cast<SbkObjectType*>(PyType_Type.tp_new(metatype, args, kwds));

    if (!newType)
        return 0;

    SbkObjectTypePrivate* d = new SbkObjectTypePrivate;
    memset(d, 0, sizeof(SbkObjectTypePrivate));

    std::list<SbkObjectType*> bases = Shiboken::getCppBaseClasses(reinterpret_cast<PyTypeObject*>(newType));
    if (bases.size() == 1) {
        SbkObjectTypePrivate* parentType = bases.front()->d;
        d->mi_offsets = parentType->mi_offsets;
        d->mi_init = parentType->mi_init;
        d->mi_specialcast = parentType->mi_specialcast;
        d->ext_isconvertible = parentType->ext_isconvertible;
        d->ext_tocpp = parentType->ext_tocpp;
        d->type_discovery = parentType->type_discovery;
        d->obj_copier = parentType->obj_copier;
        d->cpp_dtor = parentType->cpp_dtor;
        d->is_multicpp = 0;
    } else {
        d->mi_offsets = 0;
        d->mi_init = 0;
        d->mi_specialcast = 0;
        d->ext_isconvertible = 0;
        d->ext_tocpp = 0;
        d->type_discovery = 0;
        d->obj_copier = 0;
        d->cpp_dtor = 0;
        d->is_multicpp = 1;
    }
    if (bases.size() == 1)
        d->original_name = strdup(bases.front()->d->original_name);
    else
        d->original_name = strdup("object");
    d->user_data = 0;
    d->d_func = 0;
    d->is_user_type = 1;
    newType->d = d;
    return reinterpret_cast<PyObject*>(newType);
}

PyObject* SbkObjectTpNew(PyTypeObject* subtype, PyObject*, PyObject*)
{
    SbkObject* self = reinterpret_cast<SbkObject*>(subtype->tp_alloc(subtype, 0));
    self->d = new SbkObjectPrivate;

    SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(subtype);
    int numBases = ((sbkType->d && sbkType->d->is_multicpp) ? Shiboken::getNumberOfCppBaseClasses(subtype) : 1);
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


} //extern "C"

namespace Shiboken
{

static void incRefPyObject(PyObject* pyObj);
static void decRefPyObjectList(const std::list<SbkObject*> &pyObj);

void walkThroughClassHierarchy(PyTypeObject* currentType, HierarchyVisitor* visitor)
{
    PyObject* bases = currentType->tp_bases;
    Py_ssize_t numBases = PyTuple_GET_SIZE(bases);
    for (int i = 0; i < numBases; ++i) {
        PyTypeObject* type = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(bases, i));

        if (type->ob_type != &SbkObjectType_Type) {
            continue;
        } else {
            SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(type);
            if (sbkType->d->is_user_type)
                walkThroughClassHierarchy(type, visitor);
            else
                visitor->visit(sbkType);
        }
        if (visitor->wasFinished())
            return;
    }
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

void DtorCallerVisitor::visit(SbkObjectType* node)
{
    node->d->cpp_dtor(m_pyObj->d->cptr[m_count]);
    m_count++;
}

void init()
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

    if (PyType_Ready(&SbkObjectType_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapperType metatype.");

    if (PyType_Ready((PyTypeObject *)&SbkObject_Type) < 0)
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
        virtual void visit(SbkObjectType* node)
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

std::list<SbkObject*> splitPyObject(PyObject* pyObj)
{
    std::list<SbkObject*> result;
    if (PySequence_Check(pyObj)) {
        AutoDecRef lst(PySequence_Fast(pyObj, "Invalid keep reference object."));
        if (!lst.isNull()) {
            for(int i = 0, i_max = PySequence_Fast_GET_SIZE(lst.object()); i < i_max; i++) {
                PyObject* item = PySequence_Fast_GET_ITEM(lst.object(), i);
                if (Object::checkType(item))
                    result.push_back(reinterpret_cast<SbkObject*>(item));
            }
        }
    } else {
        result.push_back(reinterpret_cast<SbkObject*>(pyObj));
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

static void decRefPyObjectList(const std::list<SbkObject*>& lst)
{
    std::list<SbkObject*>::const_iterator iter = lst.begin();
    while(iter != lst.end()) {
        Py_DECREF(*iter);
        ++iter;
    }
}

namespace ObjectType
{

bool checkType(PyTypeObject* type)
{
    return type->ob_type == &SbkObjectType_Type;
}

bool isUserType(PyTypeObject* type)
{
    return checkType(type) && reinterpret_cast<SbkObjectType*>(type)->d->is_user_type;
}

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

void* copy(SbkObjectType* self, const void* obj)
{
    return self->d->obj_copier(obj);
}

void setCopyFunction(SbkObjectType* self, ObjectCopierFunction func)
{
    self->d->obj_copier = func;
}

bool hasExternalCppConversions(SbkObjectType* self)
{
    return self->d->ext_tocpp;
}

void* callExternalCppConversion(SbkObjectType* self, PyObject* obj)
{
    return self->d->ext_tocpp(obj);
}

void setExternalCppConversionFunction(SbkObjectType* self, ExtendedToCppFunc func)
{
    self->d->ext_tocpp = func;
}

void setExternalIsConvertibleFunction(SbkObjectType* self, ExtendedIsConvertibleFunc func)
{
    self->d->ext_isconvertible = func;
}

bool isExternalConvertible(SbkObjectType* self, PyObject* obj)
{
    return self->d->ext_isconvertible && self->d->ext_isconvertible(obj);
}

bool hasCast(SbkObjectType* self)
{
    return self->d->mi_specialcast;
}

void* cast(SbkObjectType* self, SbkObject* obj, PyTypeObject *target)
{
    return self->d->mi_specialcast(Object::cppPointer(obj, target), reinterpret_cast<SbkObjectType*>(target));
}

void setCastFunction(SbkObjectType* self, SpecialCastFunction func)
{
    self->d->mi_specialcast = func;
}

void setOriginalName(SbkObjectType* self, const char* name)
{
    if (self->d->original_name)
        free(self->d->original_name);
    self->d->original_name = strdup(name);
}

const char* getOriginalName(SbkObjectType* self)
{
    return self->d->original_name;
}

void setTypeDiscoveryFunction(SbkObjectType* self, TypeDiscoveryFunc func)
{
    self->d->type_discovery = func;
}

TypeDiscoveryFunc getTypeDiscoveryFunction(SbkObjectType* self)
{
    return self->d->type_discovery;
}

void copyMultimpleheritance(SbkObjectType* self, SbkObjectType* other)
{
    self->d->mi_init = other->d->mi_init;
    self->d->mi_offsets = other->d->mi_offsets;
    self->d->mi_specialcast = other->d->mi_specialcast;
}

void setMultipleIheritanceFunction(SbkObjectType* self, MultipleInheritanceInitFunction function)
{
    self->d->mi_init = function;
}

MultipleInheritanceInitFunction getMultipleIheritanceFunction(SbkObjectType* self)
{
    return self->d->mi_init;
}

void setDestructorFunction(SbkObjectType* self, ObjectDestructor func)
{
    self->d->cpp_dtor = func;
}

void initPrivateData(SbkObjectType* self)
{
    self->d = new SbkObjectTypePrivate;
    memset(self->d, 0, sizeof(SbkObjectTypePrivate));
}

} // namespace ObjectType

namespace Object
{

bool checkType(PyObject* pyObj)
{
    return ObjectType::checkType(pyObj->ob_type);
}

bool isUserType(PyObject* pyObj)
{
    return ObjectType::isUserType(pyObj->ob_type);
}

static void setSequenceOwnership(PyObject* pyObj, bool owner)
{
    if (PySequence_Check(pyObj)) {
        std::list<SbkObject*> objs = splitPyObject(pyObj);
        std::list<SbkObject*>::const_iterator it = objs.begin();
        for(; it != objs.end(); ++it) {
            if (owner)
                getOwnership(*it);
            else
                releaseOwnership(*it);
        }
    } else if (Object::checkType(pyObj)) {
        if (owner)
            getOwnership(reinterpret_cast<SbkObject*>(pyObj));
        else
            releaseOwnership(reinterpret_cast<SbkObject*>(pyObj));
    }
}


static void _destroyParentInfo(SbkObject* obj, bool keepReference)
{
    ParentInfo* pInfo = obj->d->parentInfo;
    if (pInfo) {
        while(!pInfo->children.empty()) {
            SbkObject* first = pInfo->children.front();
            // Mark child as invalid
            Shiboken::Object::invalidate(first);
            removeParent(first, false, keepReference);
       }
       removeParent(obj, false);
       delete pInfo;
       obj->d->parentInfo = 0;
    }
}


void setValidCpp(SbkObject* pyObj, bool value)
{
    pyObj->d->validCppObject = value;
}

void setHasCppWrapper(SbkObject* pyObj, bool value)
{
    pyObj->d->containsCppWrapper = value;
}

bool hasCppWrapper(SbkObject* pyObj)
{
    return pyObj->d->containsCppWrapper;
}

bool hasOwnership(SbkObject* pyObj)
{
    return pyObj->d->hasOwnership;
}

void getOwnership(SbkObject* self)
{
    // skip if already have the ownership
    if (self->d->hasOwnership)
        return;

    // skip if this object has parent
    if (self->d->parentInfo && self->d->parentInfo->parent)
        return;

    // Get back the ownership
    self->d->hasOwnership = true;

    if (self->d->containsCppWrapper)
        Py_DECREF((PyObject*) self); // Remove extra ref
    else
        makeValid(self); // Make the object valid again
}

void getOwnership(PyObject* pyObj)
{
    setSequenceOwnership(pyObj, true);
}

void releaseOwnership(SbkObject* self)
{
    // skip if the ownership have already moved to c++
    if (!self->d->hasOwnership)
        return;

    // remove object ownership
    self->d->hasOwnership = false;

    // If We have control over object life
    if (self->d->containsCppWrapper)
        Py_INCREF((PyObject*) self); // keep the python object alive until the wrapper destructor call
    else
        invalidate(self); // If I do not know when this object will die We need to invalidate this to avoid use after
}

void releaseOwnership(PyObject* self)
{
    setSequenceOwnership(self, false);
}

void invalidate(PyObject* pyobj)
{
    std::list<SbkObject*> objs = splitPyObject(pyobj);
    std::list<SbkObject*>::const_iterator it = objs.begin();
    for(; it != objs.end(); it++)
        invalidate(*it);
}

void invalidate(SbkObject* self)
{
    // Skip if this object not is a valid object
    if (!self || ((PyObject*)self == Py_None))
        return;

    if (!self->d->containsCppWrapper) {
        self->d->validCppObject = false; // Mark object as invalid only if this is not a wrapper class
        BindingManager::instance().releaseWrapper(self);
    }

    // If it is a parent invalidate all children.
    if (self->d->parentInfo) {
        // Create a copy because this list can be changed during the process
        ChildrenList copy = self->d->parentInfo->children;
        ChildrenList::iterator it = copy.begin();

        for (; it != copy.end(); ++it) {
            // invalidate the child
            invalidate(*it);

            // if the parent not is a wrapper class, then remove children from him, because We do not know when this object will be destroyed
            if (!self->d->validCppObject)
                removeParent(*it, true, true);
        }
    }
}

void makeValid(SbkObject* self)
{
    // Skip if this object not is a valid object
    if (!self || ((PyObject*)self == Py_None))
        return;

    // Mark object as invalid only if this is not a wrapper class
    self->d->validCppObject = true;

    // If it is a parent make  all children valid
    if (self->d->parentInfo) {
        ChildrenList::iterator it = self->d->parentInfo->children.begin();
        for (; it != self->d->parentInfo->children.end(); ++it)
            makeValid(*it);
    }
}

bool hasParentInfo(SbkObject* pyObj)
{
    return pyObj->d->parentInfo;
}

void* cppPointer(SbkObject* pyObj, PyTypeObject* desiredType)
{
    PyTypeObject* type = pyObj->ob_type;
    int idx = 0;
    if (reinterpret_cast<SbkObjectType*>(type)->d->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);
    return pyObj->d->cptr[idx];
}

bool setCppPointer(SbkObject* sbkObj, PyTypeObject* desiredType, void* cptr)
{
    int idx = 0;
    if (reinterpret_cast<SbkObjectType*>(sbkObj->ob_type)->d->is_multicpp)
        idx = getTypeIndexOnHierarchy(sbkObj->ob_type, desiredType);

    bool alreadyInitialized = sbkObj->d->cptr[idx];
    if (alreadyInitialized)
        PyErr_SetString(PyExc_RuntimeError, "You can't initialize an object twice!");
    else
        sbkObj->d->cptr[idx] = cptr;

    return !alreadyInitialized;
}

bool isValid(PyObject* pyObj)
{
    if (!pyObj || pyObj == Py_None
        || pyObj->ob_type->ob_type != &SbkObjectType_Type
        || ((SbkObject*)pyObj)->d->validCppObject) {
        return true;
    }
    PyErr_Format(PyExc_RuntimeError, "Internal C++ object (%s) already deleted.", pyObj->ob_type->tp_name);
    return false;
}

PyObject* newObject(SbkObjectType* instanceType,
                    void* cptr,
                    bool hasOwnership,
                    bool isExactType,
                    const char* typeName)
{
    // Try to find the exact type of cptr.
    if (!isExactType) {
        TypeResolver* tr = 0;
        if (typeName) {
            tr = TypeResolver::get(typeName);
            if (tr)
                instanceType = reinterpret_cast<SbkObjectType*>(tr->pythonType());
        }
        if (!tr)
            instanceType = BindingManager::instance().resolveType(cptr, instanceType);
    }

    SbkObject* self = reinterpret_cast<SbkObject*>(SbkObjectTpNew(reinterpret_cast<PyTypeObject*>(instanceType), 0, 0));
    self->d->cptr[0] = cptr;
    self->d->hasOwnership = hasOwnership;
    self->d->validCppObject = 1;
    BindingManager::instance().registerWrapper(self, cptr);
    return reinterpret_cast<PyObject*>(self);
}

void destroy(SbkObject* self)
{
    // Skip if this is called with NULL pointer this can happen in derived classes
    if (!self)
        return;

    // This can be called in c++ side
    Shiboken::GilState gil;

    // We will marks this object as invalid because this function will be called from wrapper destructor
    // If The object has ownership and this was destroyed then is necessary invalidate to avoid future used by Python
    self->d->validCppObject = false;

    // Remove all references attached to this object
    clearReferences(self);

    // Remove from BindinManager
    Shiboken::BindingManager::instance().releaseWrapper(self);

    // Remove the object from parent control

    // Verify if this object has parent
    bool hasParent = (self->d->parentInfo && self->d->parentInfo->parent);

    if (self->d->parentInfo) {
        // Check for children information and make all invalid if they exists
        _destroyParentInfo(self, true);
        // If this object has parent then the pyobject can be invalid now, because we remove the last ref after remove from parent
    }

    //if !hasParent this object could still alive
    if (!hasParent && self->d->containsCppWrapper && !self->d->hasOwnership) {
        // Remove extra ref used by c++ object this will case the pyobject destruction
        // This can cause the object death
        Py_DECREF((PyObject*)self);
    }
    // After this point the object can be death do not use the self pointer bellow
}

void removeParent(SbkObject* child, bool giveOwnershipBack, bool keepReference)
{
    ParentInfo* pInfo = child->d->parentInfo;
    if (!pInfo || !pInfo->parent) {
        return;
    }

    ChildrenList& oldBrothers = pInfo->parent->d->parentInfo->children;
    // Verify if this child is part of parent list
    ChildrenList::iterator iChild = std::find(oldBrothers.begin(), oldBrothers.end(), child);
    if (iChild == oldBrothers.end())
        return;

    oldBrothers.erase(iChild);

    pInfo->parent = 0;

    // This will keep the wrapper reference, will wait for wrapper destruction to remove that
    if (keepReference && child->d->containsCppWrapper)
        return;

    // Transfer ownership back to Python
    child->d->hasOwnership = giveOwnershipBack;

    // Remove parent ref
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
    if (PySequence_Check(child) && !Object::checkType(child)) {
        Shiboken::AutoDecRef seq(PySequence_Fast(child, 0));
        for (int i = 0, max = PySequence_Size(seq); i < max; ++i)
            setParent(parent, PySequence_Fast_GET_ITEM(seq.object(), i));
        return;
    }

    bool parentIsNull = !parent || parent == Py_None;
    SbkObject* parent_ = reinterpret_cast<SbkObject*>(parent);
    SbkObject* child_ = reinterpret_cast<SbkObject*>(child);

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

        // Add Parent ref
        Py_INCREF(child_);

        // Remove ownership
        child_->d->hasOwnership = false;
    }

    // Remove previous safe ref
    Py_DECREF(child);
}

void deallocData(SbkObject* self)
{
    // Make cleanup if this is not a wrapper otherwise this will be done on wrapper destructor
    if(!self->d->containsCppWrapper) {
        removeParent(self);

        if (self->d->parentInfo)
            _destroyParentInfo(self, true);

        clearReferences(self);

        // Remove from BindinManager
        Shiboken::BindingManager::instance().releaseWrapper(self);
    }

    Py_XDECREF(self->ob_dict);
    delete[] self->d->cptr;
    self->d->cptr = 0;
    delete self->d;
    Py_TYPE(self)->tp_free(self);
}

void setTypeUserData(SbkObject* wrapper, void* userData, DeleteUserDataFunc d_func)
{
    SbkObjectType* ob_type = reinterpret_cast<SbkObjectType*>(wrapper->ob_type);
    if (ob_type->d->user_data)
        ob_type->d->d_func(ob_type->d->user_data);

    ob_type->d->d_func = d_func;
    ob_type->d->user_data = userData;
}

void* getTypeUserData(SbkObject* wrapper)
{
    return reinterpret_cast<SbkObjectType*>(wrapper->ob_type)->d->user_data;
}

void keepReference(SbkObject* self, const char* key, PyObject* referredObject, bool append)
{

    bool isNone = (!referredObject || (referredObject == Py_None));

    if (!self->d->referredObjects)
        self->d->referredObjects = new Shiboken::RefCountMap;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    if (!isNone)
        incRefPyObject(referredObject);

    RefCountMap::iterator iter = refCountMap.find(key);
    if (!append && (iter != refCountMap.end())) {
        decRefPyObjectList(iter->second);
        refCountMap.erase(iter);
    }

    if (!isNone) {
        std::list<SbkObject*> values = splitPyObject(referredObject);
        if (append && (iter != refCountMap.end()))
            refCountMap[key].insert(refCountMap[key].end(), values.begin(), values.end());
        else
            refCountMap[key] = values;
    }
}

void clearReferences(SbkObject* self)
{
    if (!self->d->referredObjects)
        return;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    RefCountMap::iterator iter;
    for (iter = refCountMap.begin(); iter != refCountMap.end(); ++iter)
        decRefPyObjectList(iter->second);
    delete self->d->referredObjects;
    self->d->referredObjects = 0;
}

} // namespace Wrapper

} // namespace Shiboken


