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
#include "sbkconverter.h"
#include "sbkenum.h"
#include "sbkstring.h"
#include "autodecref.h"
#include "gilstate.h"
#include <string>
#include <cstring>
#include <cstddef>
#include <set>
#include <sstream>
#include <algorithm>
#include "threadstatesaver.h"
#include "signature.h"
#include "qapp_macro.h"
#include "voidptr.h"

namespace {
    void _destroyParentInfo(SbkObject* obj, bool keepReference);
}

extern "C"
{

static void SbkObjectTypeDealloc(PyObject* pyObj);
static PyObject* SbkObjectTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds);

static PyType_Slot SbkObjectType_Type_slots[] = {
    {Py_tp_dealloc, (void *)SbkObjectTypeDealloc},
    {Py_tp_setattro, (void *)PyObject_GenericSetAttr},
    {Py_tp_base, (void *)&PyType_Type},
    {Py_tp_alloc, (void *)PyType_GenericAlloc},
    {Py_tp_new, (void *)SbkObjectTypeTpNew},
    {Py_tp_free, (void *)PyObject_GC_Del},
    {0, 0}
};
static PyType_Spec SbkObjectType_Type_spec = {
    "Shiboken.ObjectType",
    0,   // basicsize (inserted later)
    sizeof(PyMemberDef),
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    SbkObjectType_Type_slots,
};


PyTypeObject *SbkObjectType_TypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        SbkObjectType_Type_spec.basicsize =
            PepHeapType_SIZE + sizeof(SbkObjectTypePrivate);
        type = reinterpret_cast<PyTypeObject *>(PyType_FromSpec(&SbkObjectType_Type_spec));
    }
    return type;
}

static PyObject *SbkObjectGetDict(PyObject* pObj, void *)
{
    SbkObject *obj = reinterpret_cast<SbkObject *>(pObj);
    if (!obj->ob_dict)
        obj->ob_dict = PyDict_New();
    if (!obj->ob_dict)
        return 0;
    Py_INCREF(obj->ob_dict);
    return obj->ob_dict;
}

static PyGetSetDef SbkObjectGetSetList[] = {
    {const_cast<char*>("__dict__"), SbkObjectGetDict, 0, 0, 0},
    {0, 0, 0, 0, 0} // Sentinel
};

static int SbkObject_traverse(PyObject* self, visitproc visit, void* arg)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);

    //Visit children
    Shiboken::ParentInfo* pInfo = sbkSelf->d->parentInfo;
    if (pInfo) {
        std::set<SbkObject*>::const_iterator it = pInfo->children.begin();
        for(; it != pInfo->children.end(); ++it)
            Py_VISIT(*it);
    }

    //Visit refs
    Shiboken::RefCountMap* rInfo = sbkSelf->d->referredObjects;
    if (rInfo) {
        Shiboken::RefCountMap::const_iterator it = rInfo->begin();
        for (; it != rInfo->end(); ++it) {
            std::list<PyObject*>::const_iterator ref = it->second.begin();
            for(; ref != it->second.end(); ++ref)
                Py_VISIT(*ref);
        }
    }

    if (sbkSelf->ob_dict)
        Py_VISIT(sbkSelf->ob_dict);
    return 0;
}

static int SbkObject_clear(PyObject* self)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);

    Shiboken::Object::removeParent(sbkSelf);

    if (sbkSelf->d->parentInfo)
        _destroyParentInfo(sbkSelf, true);

    Shiboken::Object::clearReferences(sbkSelf);

    if (sbkSelf->ob_dict)
        Py_CLEAR(sbkSelf->ob_dict);
    return 0;
}

static PyType_Slot SbkObject_Type_slots[] = {
    {Py_tp_dealloc, (void *)SbkDeallocWrapperWithPrivateDtor},
    {Py_tp_traverse, (void *)SbkObject_traverse},
    {Py_tp_clear, (void *)SbkObject_clear},
    // unsupported: {Py_tp_weaklistoffset, (void *)offsetof(SbkObject, weakreflist)},
    {Py_tp_getset, (void *)SbkObjectGetSetList},
    // unsupported: {Py_tp_dictoffset, (void *)offsetof(SbkObject, ob_dict)},
    {0, 0}
};
static PyType_Spec SbkObject_Type_spec = {
    "Shiboken.Object",
    sizeof(SbkObject),
    0,
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_HAVE_GC,
    SbkObject_Type_slots,
};


SbkObjectType *SbkObject_TypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        type = reinterpret_cast<PyTypeObject *>(PyType_FromSpec(&SbkObject_Type_spec));
        Py_TYPE(type) = SbkObjectType_TypeF();
        Py_INCREF(Py_TYPE(type));
        type->tp_weaklistoffset = offsetof(SbkObject, weakreflist);
        type->tp_dictoffset = offsetof(SbkObject, ob_dict);
    }
    return reinterpret_cast<SbkObjectType *>(type);
}


static void SbkDeallocWrapperCommon(PyObject* pyObj, bool canDelete)
{
    SbkObject* sbkObj = reinterpret_cast<SbkObject*>(pyObj);
    PyTypeObject* pyType = Py_TYPE(pyObj);

    // Need to decref the type if this is the dealloc func; if type
    // is subclassed, that dealloc func will decref (see subtype_dealloc
    // in typeobject.c in the python sources)
    bool needTypeDecref = (PyType_GetSlot(pyType, Py_tp_dealloc) == SbkDeallocWrapper
                           || PyType_GetSlot(pyType, Py_tp_dealloc) == SbkDeallocWrapperWithPrivateDtor);

    // Ensure that the GC is no longer tracking this object to avoid a
    // possible reentrancy problem.  Since there are multiple steps involved
    // in deallocating a SbkObject it is possible for the garbage collector to
    // be invoked and it trying to delete this object while it is still in
    // progress from the first time around, resulting in a double delete and a
    // crash.
    // PYSIDE-571: Some objects do not use GC, so check this!
    if (PyObject_IS_GC(pyObj))
        PyObject_GC_UnTrack(pyObj);

    // Check that Python is still initialized as sometimes this is called by a static destructor
    // after Python interpeter is shutdown.
    if (sbkObj->weakreflist && Py_IsInitialized())
        PyObject_ClearWeakRefs(pyObj);

    // If I have ownership and is valid delete C++ pointer
    if (canDelete && sbkObj->d->hasOwnership && sbkObj->d->validCppObject) {
        SbkObjectTypePrivate *sotp = PepType_SOTP(pyType);
        if (sotp->is_multicpp) {
            Shiboken::DeallocVisitor visitor(sbkObj);
            Shiboken::walkThroughClassHierarchy(Py_TYPE(pyObj), &visitor);
        } else {
            void* cptr = sbkObj->d->cptr[0];
            Shiboken::Object::deallocData(sbkObj, true);

            Shiboken::ThreadStateSaver threadSaver;
            if (Py_IsInitialized())
                threadSaver.save();
            sotp->cpp_dtor(cptr);
        }
    } else {
        Shiboken::Object::deallocData(sbkObj, true);
    }

    if (needTypeDecref)
        Py_DECREF(pyType);
}

void SbkDeallocWrapper(PyObject* pyObj)
{
    SbkDeallocWrapperCommon(pyObj, true);
}

void SbkDeallocQAppWrapper(PyObject* pyObj)
{
    SbkDeallocWrapper(pyObj);
    // PYSIDE-571: make sure to create a singleton deleted qApp.
    MakeSingletonQAppWrapper(NULL);
}

void SbkDeallocWrapperWithPrivateDtor(PyObject* self)
{
    SbkDeallocWrapperCommon(self, false);
}

void SbkObjectTypeDealloc(PyObject* pyObj)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(pyObj);
    PyTypeObject *type = reinterpret_cast<PyTypeObject*>(pyObj);

    PyObject_GC_UnTrack(pyObj);
#ifndef Py_LIMITED_API
    Py_TRASHCAN_SAFE_BEGIN(pyObj);
#endif
    if (sotp) {
        if (sotp->user_data && sotp->d_func) {
            sotp->d_func(sotp->user_data);
            sotp->user_data = nullptr;
        }
        free(sotp->original_name);
        sotp->original_name = nullptr;
        if (!Shiboken::ObjectType::isUserType(type))
            Shiboken::Conversions::deleteConverter(sotp->converter);
        delete sotp;
        sotp = nullptr;
    }
#ifndef Py_LIMITED_API
    Py_TRASHCAN_SAFE_END(pyObj);
#endif
}

PyObject* SbkObjectTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds)
{
    // Check if all bases are new style before calling type.tp_new
    // Was causing gc assert errors in test_bug704.py when
    // this check happened after creating the type object.
    // Argument parsing take from type.tp_new code.

    // PYSIDE-595: Also check if all bases allow inheritance.
    // Before we changed to heap types, it was sufficient to remove the
    // Py_TPFLAGS_BASETYPE flag. That does not work, because PySide does
    // not respect this flag itself!
    PyObject* name;
    PyObject* pyBases;
    PyObject* dict;
    static const char* kwlist[] = { "name", "bases", "dict", 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO!O!:sbktype", (char**)kwlist,
                                     &name,
                                     &PyTuple_Type, &pyBases,
                                     &PyDict_Type, &dict))
        return NULL;

    for (int i=0, i_max=PyTuple_GET_SIZE(pyBases); i < i_max; i++) {
        PyObject* baseType = PyTuple_GET_ITEM(pyBases, i);
#ifndef IS_PY3K
        if (PyClass_Check(baseType)) {
            PyErr_Format(PyExc_TypeError, "Invalid base class used in type %s. "
                "PySide only support multiple inheritance from python new style class.", metatype->tp_name);
            return 0;
        }
#endif
        if (reinterpret_cast<PyTypeObject *>(baseType)->tp_new == SbkDummyNew) {
            // PYSIDE-595: A base class does not allow inheritance.
            return SbkDummyNew(metatype, args, kwds);
        }
    }

    // The meta type creates a new type when the Python programmer extends a wrapped C++ class.
    newfunc type_new = reinterpret_cast<newfunc>(PyType_Type.tp_new);
    SbkObjectType *newType = reinterpret_cast<SbkObjectType*>(type_new(metatype, args, kwds));
    if (!newType)
        return 0;

    Shiboken::ObjectType::initPrivateData(newType);
    SbkObjectTypePrivate *sotp = PepType_SOTP(newType);

    std::list<SbkObjectType*> bases = Shiboken::getCppBaseClasses(reinterpret_cast<PyTypeObject*>(newType));
    if (bases.size() == 1) {
        SbkObjectTypePrivate *parentType = PepType_SOTP(bases.front());
        sotp->mi_offsets = parentType->mi_offsets;
        sotp->mi_init = parentType->mi_init;
        sotp->mi_specialcast = parentType->mi_specialcast;
        sotp->type_discovery = parentType->type_discovery;
        sotp->cpp_dtor = parentType->cpp_dtor;
        sotp->is_multicpp = 0;
        sotp->converter = parentType->converter;
    } else {
        sotp->mi_offsets = nullptr;
        sotp->mi_init = nullptr;
        sotp->mi_specialcast = nullptr;
        sotp->type_discovery = nullptr;
        sotp->cpp_dtor = nullptr;
        sotp->is_multicpp = 1;
        sotp->converter = nullptr;
    }
    if (bases.size() == 1)
        sotp->original_name = strdup(PepType_SOTP(bases.front())->original_name);
    else
        sotp->original_name = strdup("object");
    sotp->user_data = nullptr;
    sotp->d_func = nullptr;
    sotp->is_user_type = 1;

    std::list<SbkObjectType*>::const_iterator it = bases.begin();
    for (; it != bases.end(); ++it) {
        if (PepType_SOTP(*it)->subtype_init)
            PepType_SOTP(*it)->subtype_init(newType, args, kwds);
    }

    return reinterpret_cast<PyObject*>(newType);
}

static PyObject *_setupNew(SbkObject *self, PyTypeObject *subtype)
{
    Py_INCREF(reinterpret_cast<PyObject*>(subtype));
    SbkObjectPrivate* d = new SbkObjectPrivate;

    SbkObjectTypePrivate * sotp = PepType_SOTP(subtype);
    int numBases = ((sotp && sotp->is_multicpp) ?
        Shiboken::getNumberOfCppBaseClasses(subtype) : 1);
    d->cptr = new void*[numBases];
    std::memset(d->cptr, 0, sizeof(void*) * size_t(numBases));
    d->hasOwnership = 1;
    d->containsCppWrapper = 0;
    d->validCppObject = 0;
    d->parentInfo = nullptr;
    d->referredObjects = nullptr;
    d->cppObjectCreated = 0;
    self->ob_dict = nullptr;
    self->weakreflist = nullptr;
    self->d = d;
    return reinterpret_cast<PyObject*>(self);
}

PyObject* SbkObjectTpNew(PyTypeObject *subtype, PyObject *, PyObject *)
{
    SbkObject *self = PyObject_GC_New(SbkObject, subtype);
    PyObject *res = _setupNew(self, subtype);
    PyObject_GC_Track(reinterpret_cast<PyObject*>(self));
    return res;
}

PyObject* SbkQAppTpNew(PyTypeObject* subtype, PyObject *, PyObject *)
{
    // PYSIDE-571:
    // For qApp, we need to create a singleton Python object.
    // We cannot track this with the GC, because it is a static variable!

    // Python 2 has a weird handling of flags in derived classes that Python 3
    // does not have. Observed with bug_307.py.
    // But it could theoretically also happen with Python3.
    // Therefore we enforce that there is no GC flag, ever!

    // PYSIDE-560:
    // We avoid to use this in Python 3, because we have a hard time to get
    // write access to these flags
#ifndef IS_PY3K
    if (PyType_HasFeature(subtype, Py_TPFLAGS_HAVE_GC)) {
        subtype->tp_flags &= ~Py_TPFLAGS_HAVE_GC;
        subtype->tp_free = PyObject_Del;
    }
#endif
    SbkObject* self = reinterpret_cast<SbkObject*>(MakeSingletonQAppWrapper(subtype));
    return self == 0 ? 0 : _setupNew(self, subtype);
}

void
SbkDummyDealloc(PyObject *)
{}

PyObject *
SbkDummyNew(PyTypeObject *type, PyObject*, PyObject*)
{
    // PYSIDE-595: Give the same error as type_call does when tp_new is NULL.
    PyErr_Format(PyExc_TypeError,
                 "cannot create '%.100s' instances ¯\\_(ツ)_/¯",
                 type->tp_name);
    return nullptr;
}

} //extern "C"


namespace
{

void _destroyParentInfo(SbkObject* obj, bool keepReference)
{
    Shiboken::ParentInfo* pInfo = obj->d->parentInfo;
    if (pInfo) {
        while(!pInfo->children.empty()) {
            SbkObject* first = *pInfo->children.begin();
            // Mark child as invalid
            Shiboken::Object::invalidate(first);
            Shiboken::Object::removeParent(first, false, keepReference);
        }
        Shiboken::Object::removeParent(obj, false);
    }
}

}

namespace Shiboken
{

static void decRefPyObjectList(const std::list<PyObject*> &pyObj, PyObject* skip = 0);

static void _walkThroughClassHierarchy(PyTypeObject* currentType, HierarchyVisitor* visitor)
{
    PyObject* bases = currentType->tp_bases;
    Py_ssize_t numBases = PyTuple_GET_SIZE(bases);
    for (int i = 0; i < numBases; ++i) {
        PyTypeObject* type = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(bases, i));

        if (!PyType_IsSubtype(type, reinterpret_cast<PyTypeObject*>(SbkObject_TypeF()))) {
            continue;
        } else {
            SbkObjectType* sbkType = reinterpret_cast<SbkObjectType*>(type);
            if (PepType_SOTP(sbkType)->is_user_type)
                _walkThroughClassHierarchy(type, visitor);
            else
                visitor->visit(sbkType);
        }
        if (visitor->wasFinished())
            break;
    }
}

void walkThroughClassHierarchy(PyTypeObject* currentType, HierarchyVisitor* visitor)
{
    _walkThroughClassHierarchy(currentType, visitor);
    visitor->done();
}


bool importModule(const char* moduleName, PyTypeObject*** cppApiPtr)
{
    PyObject* sysModules = PyImport_GetModuleDict();
    PyObject* module = PyDict_GetItemString(sysModules, moduleName);
    if (!module) {
        module = PyImport_ImportModule(moduleName);
        if (!module)
            return false;
    } else {
        Py_INCREF(module);
    }

    Shiboken::AutoDecRef cppApi(PyObject_GetAttrString(module, "_Cpp_Api"));
    Py_DECREF(module);

    if (cppApi.isNull())
        return false;

#ifdef IS_PY3K
    if (PyCapsule_CheckExact(cppApi))
        *cppApiPtr = reinterpret_cast<PyTypeObject**>(PyCapsule_GetPointer(cppApi, 0));
#else
    // Python 2.6 doesn't have PyCapsule API, so let's keep usign PyCObject on all Python 2.x
    if (PyCObject_Check(cppApi))
        *cppApiPtr = reinterpret_cast<PyTypeObject**>(PyCObject_AsVoidPtr(cppApi));
#endif
    return true;
}

// Wrapper metatype and base type ----------------------------------------------------------

void DtorCallerVisitor::visit(SbkObjectType* node)
{
    m_ptrs.push_back(std::make_pair(m_pyObj->d->cptr[m_ptrs.size()], node));
}

void DtorCallerVisitor::done()
{
    std::list<std::pair<void*, SbkObjectType*> >::const_iterator it = m_ptrs.begin();
    for (; it != m_ptrs.end(); ++it) {
        Shiboken::ThreadStateSaver threadSaver;
        threadSaver.save();
        PepType_SOTP(it->second)->cpp_dtor(it->first);
    }
}

void DeallocVisitor::done()
{
    Shiboken::Object::deallocData(m_pyObj, true);
    DtorCallerVisitor::done();
}

namespace Conversions { void init(); }

void init()
{
    static bool shibokenAlreadInitialised = false;
    if (shibokenAlreadInitialised)
        return;

    Conversions::init();

    PyEval_InitThreads();

    //Init private data
    Pep384_Init();

    Shiboken::ObjectType::initPrivateData(SbkObject_TypeF());

    if (PyType_Ready(SbkEnumType_TypeF()) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.SbkEnumType metatype.");

    if (PyType_Ready(SbkObjectType_TypeF()) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapperType metatype.");

    if (PyType_Ready(reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapper type.");

    VoidPtr::init();

    shibokenAlreadInitialised = true;
}

void setErrorAboutWrongArguments(PyObject* args, const char* funcName, const char** cppOverloads)
{
    std::string msg;
    std::string params;
    if (args) {
        if (PyTuple_Check(args)) {
            for (Py_ssize_t i = 0, max = PyTuple_GET_SIZE(args); i < max; ++i) {
                if (i)
                    params += ", ";
                PyObject* arg = PyTuple_GET_ITEM(args, i);
                params += Py_TYPE(arg)->tp_name;
            }
        } else {
            params = Py_TYPE(args)->tp_name;
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
            for (Py_ssize_t i = 0, i_max = PySequence_Fast_GET_SIZE(lst.object()); i < i_max; ++i) {
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

static void decRefPyObjectList(const std::list<PyObject*>& lst, PyObject *skip)
{
    std::list<PyObject*>::const_iterator iter = lst.begin();
    while(iter != lst.end()) {
        if (*iter != skip)
            Py_DECREF(*iter);
        ++iter;
    }
}

namespace ObjectType
{

bool checkType(PyTypeObject* type)
{
    return PyType_IsSubtype(type, reinterpret_cast<PyTypeObject*>(SbkObject_TypeF())) != 0;
}

bool isUserType(PyTypeObject* type)
{
    return checkType(type) && PepType_SOTP(type)->is_user_type;
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

bool hasCast(SbkObjectType* type)
{
    return PepType_SOTP(type)->mi_specialcast != 0;
}

void* cast(SbkObjectType* sourceType, SbkObject* obj, PyTypeObject* targetType)
{
    return PepType_SOTP(sourceType)->mi_specialcast(Object::cppPointer(obj, targetType),
        reinterpret_cast<SbkObjectType*>(targetType));
}

void setCastFunction(SbkObjectType* type, SpecialCastFunction func)
{
    PepType_SOTP(type)->mi_specialcast = func;
}

void setOriginalName(SbkObjectType* type, const char* name)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(type);
    if (sotp->original_name)
        free(sotp->original_name);
    sotp->original_name = strdup(name);
}

const char* getOriginalName(SbkObjectType* type)
{
    return PepType_SOTP(type)->original_name;
}

void setTypeDiscoveryFunctionV2(SbkObjectType* type, TypeDiscoveryFuncV2 func)
{
    PepType_SOTP(type)->type_discovery = func;
}

void copyMultimpleheritance(SbkObjectType* type, SbkObjectType* other)
{
    PepType_SOTP(type)->mi_init = PepType_SOTP(other)->mi_init;
    PepType_SOTP(type)->mi_offsets = PepType_SOTP(other)->mi_offsets;
    PepType_SOTP(type)->mi_specialcast = PepType_SOTP(other)->mi_specialcast;
}

void setMultipleInheritanceFunction(SbkObjectType* type, MultipleInheritanceInitFunction function)
{
    PepType_SOTP(type)->mi_init = function;
}

MultipleInheritanceInitFunction getMultipleIheritanceFunction(SbkObjectType* type)
{
    return PepType_SOTP(type)->mi_init;
}

void setDestructorFunction(SbkObjectType* type, ObjectDestructor func)
{
    PepType_SOTP(type)->cpp_dtor = func;
}

void initPrivateData(SbkObjectType* type)
{
    PepType_SOTP(type) = new SbkObjectTypePrivate;
    memset(PepType_SOTP(type), 0, sizeof(SbkObjectTypePrivate));
}

SbkObjectType *
introduceWrapperType(PyObject *enclosingObject,
                     const char *typeName,
                     const char *originalName,
                     PyType_Spec *typeSpec,
                     const char *signaturesString,
                     ObjectDestructor cppObjDtor,
                     SbkObjectType *baseType,
                     PyObject *baseTypes,
                     bool isInnerClass)
{
    if (baseType) {
        typeSpec->slots[0].pfunc = reinterpret_cast<void *>(baseType);
    }
    else {
        typeSpec->slots[0].pfunc = reinterpret_cast<void *>(SbkObject_TypeF());
    }
    PyObject *heaptype = PyType_FromSpecWithBases(typeSpec, baseTypes);
    Py_TYPE(heaptype) = SbkObjectType_TypeF();
    Py_INCREF(Py_TYPE(heaptype));
    SbkObjectType *type = reinterpret_cast<SbkObjectType *>(heaptype);
    if (baseType) {
        if (baseTypes) {
            for (int i = 0; i < PySequence_Fast_GET_SIZE(baseTypes); ++i)
                BindingManager::instance().addClassInheritance(reinterpret_cast<SbkObjectType *>(PySequence_Fast_GET_ITEM(baseTypes, i)), type);
        } else {
            BindingManager::instance().addClassInheritance(baseType, type);
        }
    }
    // PYSIDE-510: Here is the single change to support signatures.
    if (SbkSpecial_Type_Ready(enclosingObject, reinterpret_cast<PyTypeObject *>(type), signaturesString) < 0)
        return nullptr;

    initPrivateData(type);
    setOriginalName(type, originalName);
    setDestructorFunction(type, cppObjDtor);

    if (isInnerClass) {
        if (PyDict_SetItemString(enclosingObject, typeName, reinterpret_cast<PyObject *>(type)) == 0)
            return type;
        else
            return nullptr;
    }

    //PyModule_AddObject steals type's reference.
    Py_INCREF(reinterpret_cast<PyObject *>(type));
    if (PyModule_AddObject(enclosingObject, typeName, reinterpret_cast<PyObject *>(type)) == 0) {
        return type;
    }
    return nullptr;
}

void setSubTypeInitHook(SbkObjectType* type, SubTypeInitHook func)
{
    PepType_SOTP(type)->subtype_init = func;
}

void* getTypeUserData(SbkObjectType* type)
{
    return PepType_SOTP(type)->user_data;
}

void setTypeUserData(SbkObjectType* type, void* userData, DeleteUserDataFunc d_func)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(type);
    sotp->user_data = userData;
    sotp->d_func = d_func;
}

} // namespace ObjectType


namespace Object
{

static void recursive_invalidate(SbkObject* self, std::set<SbkObject*>& seen);

bool checkType(PyObject* pyObj)
{
    return ObjectType::checkType(Py_TYPE(pyObj));
}

bool isUserType(PyObject* pyObj)
{
    return ObjectType::isUserType(Py_TYPE(pyObj));
}

Py_hash_t hash(PyObject* pyObj)
{
    assert(Shiboken::Object::checkType(pyObj));
    return reinterpret_cast<Py_hash_t>(pyObj);
}

static void setSequenceOwnership(PyObject* pyObj, bool owner)
{

    bool has_length = true;
    if (PySequence_Size(pyObj) < 0) {
        PyErr_Clear();
        has_length = false;
    }

    if (PySequence_Check(pyObj) && has_length) {
        Py_ssize_t size = PySequence_Size(pyObj);
        if (size > 0) {
            std::list<SbkObject*> objs = splitPyObject(pyObj);
            for (auto it = objs.begin(), end = objs.end(); it != end; ++it) {
                if (owner)
                    getOwnership(*it);
                else
                    releaseOwnership(*it);
            }
        }
    } else if (Object::checkType(pyObj)) {
        if (owner)
            getOwnership(reinterpret_cast<SbkObject*>(pyObj));
        else
            releaseOwnership(reinterpret_cast<SbkObject*>(pyObj));
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

bool wasCreatedByPython(SbkObject* pyObj)
{
    return pyObj->d->cppObjectCreated;
}

void callCppDestructors(SbkObject* pyObj)
{
    PyTypeObject *type = Py_TYPE(pyObj);
    SbkObjectTypePrivate * sotp = PepType_SOTP(type);
    if (sotp->is_multicpp) {
        Shiboken::DtorCallerVisitor visitor(pyObj);
        Shiboken::walkThroughClassHierarchy(type, &visitor);
    } else {
        Shiboken::ThreadStateSaver threadSaver;
        threadSaver.save();
        sotp->cpp_dtor(pyObj->d->cptr[0]);
    }

    /* invalidate needs to be called before deleting pointer array because
       it needs to delete entries for them from the BindingManager hash table;
       also release wrapper explicitly if object contains C++ wrapper because
       invalidate doesn't */
    invalidate(pyObj);
    if (pyObj->d->validCppObject && pyObj->d->containsCppWrapper) {
      BindingManager::instance().releaseWrapper(pyObj);
    }

    delete[] pyObj->d->cptr;
    pyObj->d->cptr = 0;
    pyObj->d->validCppObject = false;
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
        Py_DECREF(reinterpret_cast<PyObject *>(self)); // Remove extra ref
    else
        makeValid(self); // Make the object valid again
}

void getOwnership(PyObject* pyObj)
{
    if (pyObj)
        setSequenceOwnership(pyObj, true);
}

void releaseOwnership(SbkObject* self)
{
    // skip if the ownership have already moved to c++
    SbkObjectType* selfType = reinterpret_cast<SbkObjectType*>(Py_TYPE(self));
    if (!self->d->hasOwnership || Shiboken::Conversions::pythonTypeIsValueType(PepType_SOTP(selfType)->converter))
        return;

    // remove object ownership
    self->d->hasOwnership = false;

    // If We have control over object life
    if (self->d->containsCppWrapper)
        Py_INCREF(reinterpret_cast<PyObject *>(self)); // keep the python object alive until the wrapper destructor call
    else
        invalidate(self); // If I do not know when this object will die We need to invalidate this to avoid use after
}

void releaseOwnership(PyObject* self)
{
    setSequenceOwnership(self, false);
}

/* Needed forward declarations */
static void recursive_invalidate(PyObject* pyobj, std::set<SbkObject*>& seen);
static void recursive_invalidate(SbkObject* self, std::set<SbkObject*>& seen);

void invalidate(PyObject* pyobj)
{
    std::set<SbkObject*> seen;
    recursive_invalidate(pyobj, seen);
}

void invalidate(SbkObject* self)
{
    std::set<SbkObject*> seen;
    recursive_invalidate(self, seen);
}

static void recursive_invalidate(PyObject* pyobj, std::set<SbkObject*>& seen)
{
    std::list<SbkObject*> objs = splitPyObject(pyobj);
    std::list<SbkObject*>::const_iterator it = objs.begin();
    for (; it != objs.end(); it++)
        recursive_invalidate(*it, seen);
}

static void recursive_invalidate(SbkObject* self, std::set<SbkObject*>& seen)
{
    // Skip if this object not is a valid object or if it's already been seen
    if (!self || reinterpret_cast<PyObject *>(self) == Py_None || seen.find(self) != seen.end())
        return;
    seen.insert(self);

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
            recursive_invalidate(*it, seen);

            // if the parent not is a wrapper class, then remove children from him, because We do not know when this object will be destroyed
            if (!self->d->validCppObject)
                removeParent(*it, true, true);
        }
    }

    // If has ref to other objects invalidate all
    if (self->d->referredObjects) {
        RefCountMap& refCountMap = *(self->d->referredObjects);
        RefCountMap::iterator iter;
        for (iter = refCountMap.begin(); iter != refCountMap.end(); ++iter) {
            const std::list<PyObject*> lst = iter->second;
            std::list<PyObject*>::const_iterator it = lst.begin();
            while(it != lst.end()) {
                recursive_invalidate(*it, seen);
                ++it;
            }
        }
    }
}

void makeValid(SbkObject* self)
{
    // Skip if this object not is a valid object
    if (!self || reinterpret_cast<PyObject *>(self) == Py_None || self->d->validCppObject)
        return;

    // Mark object as invalid only if this is not a wrapper class
    self->d->validCppObject = true;

    // If it is a parent make  all children valid
    if (self->d->parentInfo) {
        ChildrenList::iterator it = self->d->parentInfo->children.begin();
        for (; it != self->d->parentInfo->children.end(); ++it)
            makeValid(*it);
    }

    // If has ref to other objects make all valid again
    if (self->d->referredObjects) {
        RefCountMap& refCountMap = *(self->d->referredObjects);
        RefCountMap::iterator iter;
        for (iter = refCountMap.begin(); iter != refCountMap.end(); ++iter) {
            const std::list<PyObject*> lst = iter->second;
            std::list<PyObject*>::const_iterator it = lst.begin();
            while(it != lst.end()) {
                if (Shiboken::Object::checkType(*it))
                    makeValid(reinterpret_cast<SbkObject*>(*it));
                ++it;
            }
        }
    }
}

void* cppPointer(SbkObject* pyObj, PyTypeObject* desiredType)
{
    PyTypeObject* type = Py_TYPE(pyObj);
    int idx = 0;
    if (PepType_SOTP(reinterpret_cast<SbkObjectType*>(type))->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);
    if (pyObj->d->cptr)
        return pyObj->d->cptr[idx];
    return 0;
}

std::vector<void*> cppPointers(SbkObject* pyObj)
{
    int n = getNumberOfCppBaseClasses(Py_TYPE(pyObj));
    std::vector<void*> ptrs(n);
    for (int i = 0; i < n; ++i)
        ptrs[i] = pyObj->d->cptr[i];
    return ptrs;
}


bool setCppPointer(SbkObject* sbkObj, PyTypeObject* desiredType, void* cptr)
{
    int idx = 0;
    PyTypeObject *type = Py_TYPE(sbkObj);
    if (PepType_SOTP(type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);

    const bool alreadyInitialized = sbkObj->d->cptr[idx] != 0;
    if (alreadyInitialized)
        PyErr_SetString(PyExc_RuntimeError, "You can't initialize an object twice!");
    else
        sbkObj->d->cptr[idx] = cptr;

    sbkObj->d->cppObjectCreated = true;
    return !alreadyInitialized;
}

bool isValid(PyObject* pyObj)
{
    if (!pyObj || pyObj == Py_None
        || Py_TYPE(Py_TYPE(pyObj)) != SbkObjectType_TypeF()) {
        return true;
    }

    SbkObjectPrivate* priv = reinterpret_cast<SbkObject*>(pyObj)->d;

    if (!priv->cppObjectCreated && isUserType(pyObj)) {
        PyErr_Format(PyExc_RuntimeError, "'__init__' method of object's base class (%s) not called.",
                     Py_TYPE(pyObj)->tp_name);
        return false;
    }

    if (!priv->validCppObject) {
        PyErr_Format(PyExc_RuntimeError, "Internal C++ object (%s) already deleted.",
                     Py_TYPE(pyObj)->tp_name);
        return false;
    }

    return true;
}

bool isValid(SbkObject* pyObj, bool throwPyError)
{
    if (!pyObj)
        return false;

    SbkObjectPrivate* priv = pyObj->d;
    if (!priv->cppObjectCreated && isUserType(reinterpret_cast<PyObject*>(pyObj))) {
        if (throwPyError)
            PyErr_Format(PyExc_RuntimeError, "Base constructor of the object (%s) not called.",
                         Py_TYPE(pyObj)->tp_name);
        return false;
    }

    if (!priv->validCppObject) {
        if (throwPyError)
            PyErr_Format(PyExc_RuntimeError, "Internal C++ object (%s) already deleted.",
                         (Py_TYPE(pyObj))->tp_name);
        return false;
    }

    return true;
}

bool isValid(PyObject* pyObj, bool throwPyError)
{
    if (!pyObj || pyObj == Py_None ||
        !PyType_IsSubtype(Py_TYPE(pyObj), reinterpret_cast<PyTypeObject*>(SbkObject_TypeF()))) {
        return true;
    }
    return isValid(reinterpret_cast<SbkObject*>(pyObj), throwPyError);
}

SbkObject *findColocatedChild(SbkObject *wrapper,
                              const SbkObjectType *instanceType)
{
    // Degenerate case, wrapper is the correct wrapper.
    if (reinterpret_cast<const void *>(Py_TYPE(wrapper)) == reinterpret_cast<const void *>(instanceType))
        return wrapper;

    if (!(wrapper->d && wrapper->d->cptr))
        return 0;

    ParentInfo* pInfo = wrapper->d->parentInfo;
    if (!pInfo)
        return 0;

    ChildrenList& children = pInfo->children;

    ChildrenList::iterator childrenEnd = children.end();
    for (ChildrenList::iterator iChild = children.begin(); iChild != childrenEnd; ++iChild) {
        if (!((*iChild)->d && (*iChild)->d->cptr))
            continue;
        if ((*iChild)->d->cptr[0] == wrapper->d->cptr[0]) {
            if (reinterpret_cast<const void *>(Py_TYPE(*iChild)) == reinterpret_cast<const void *>(instanceType))
                return const_cast<SbkObject *>((*iChild));
            else
                return findColocatedChild(const_cast<SbkObject *>(*iChild), instanceType);
        }
    }
    return 0;
}


PyObject *newObject(SbkObjectType* instanceType,
                    void* cptr,
                    bool hasOwnership,
                    bool isExactType,
                    const char* typeName)
{
    // Try to find the exact type of cptr.
    if (!isExactType) {
        PyTypeObject* exactType = 0;
        if (typeName) {
            exactType = Shiboken::Conversions::getPythonTypeObject(typeName);
            if (exactType)
                instanceType = reinterpret_cast<SbkObjectType*>(exactType);
        }
        if (!exactType)
            instanceType = BindingManager::instance().resolveType(&cptr, instanceType);
    }

    bool shouldCreate = true;
    bool shouldRegister = true;
    SbkObject* self = 0;

    // Some logic to ensure that colocated child field does not overwrite the parent
    if (BindingManager::instance().hasWrapper(cptr)) {
        SbkObject* existingWrapper = BindingManager::instance().retrieveWrapper(cptr);

        self = findColocatedChild(existingWrapper, instanceType);
        if (self) {
            // Wrapper already registered for cptr.
            // This should not ideally happen, binding code should know when a wrapper
            // already exists and retrieve it instead.
            shouldRegister = shouldCreate = false;
        } else if (hasOwnership &&
                  (!(Shiboken::Object::hasCppWrapper(existingWrapper) ||
                     Shiboken::Object::hasOwnership(existingWrapper)))) {
            // Old wrapper is likely junk, since we have ownership and it doesn't.
            BindingManager::instance().releaseWrapper(existingWrapper);
        } else {
            // Old wrapper may be junk caused by some bug in identifying object deletion
            // but it may not be junk when a colocated field is accessed for an
            // object which was not created by python (returned from c++ factory function).
            // Hence we cannot release the wrapper confidently so we do not register.
            shouldRegister = false;
        }
    }

    if (shouldCreate) {
        self = reinterpret_cast<SbkObject*>(SbkObjectTpNew(reinterpret_cast<PyTypeObject*>(instanceType), 0, 0));
        self->d->cptr[0] = cptr;
        self->d->hasOwnership = hasOwnership;
        self->d->validCppObject = 1;
        if (shouldRegister) {
            BindingManager::instance().registerWrapper(self, cptr);
        }
    } else {
        Py_IncRef(reinterpret_cast<PyObject*>(self));
    }
    return reinterpret_cast<PyObject*>(self);
}

void destroy(SbkObject* self)
{
    destroy(self, 0);
}

void destroy(SbkObject* self, void* cppData)
{
    // Skip if this is called with NULL pointer this can happen in derived classes
    if (!self)
        return;

    // This can be called in c++ side
    Shiboken::GilState gil;

    // Remove all references attached to this object
    clearReferences(self);

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
        Py_DECREF(reinterpret_cast<PyObject *>(self));
    }

    //Python Object is not destroyed yet
    if (cppData && Shiboken::BindingManager::instance().hasWrapper(cppData)) {
        // Remove from BindingManager
        Shiboken::BindingManager::instance().releaseWrapper(self);
        self->d->hasOwnership = false;

        // the cpp object instance was deleted
        delete[] self->d->cptr;
        self->d->cptr = 0;
    }

    // After this point the object can be death do not use the self pointer bellow
}

void removeParent(SbkObject* child, bool giveOwnershipBack, bool keepReference)
{
    ParentInfo* pInfo = child->d->parentInfo;
    if (!pInfo || !pInfo->parent) {
        if (pInfo && pInfo->hasWrapperRef) {
            pInfo->hasWrapperRef = false;
        }
        return;
    }

    ChildrenList& oldBrothers = pInfo->parent->d->parentInfo->children;
    // Verify if this child is part of parent list
    auto iChild = oldBrothers.find(child);
    if (iChild == oldBrothers.end())
        return;

    oldBrothers.erase(iChild);

    pInfo->parent = 0;

    // This will keep the wrapper reference, will wait for wrapper destruction to remove that
    if (keepReference &&
        child->d->containsCppWrapper) {
        //If have already a extra ref remove this one
        if (pInfo->hasWrapperRef)
            Py_DECREF(child);
        else
            pInfo->hasWrapperRef = true;
        return;
    }

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
        for (Py_ssize_t i = 0, max = PySequence_Size(seq); i < max; ++i)
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
        if (child_->d->parentInfo && (child_->d->parentInfo->parent == parent_))
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
        parent_->d->parentInfo->children.insert(child_);

        // Add Parent ref
        Py_INCREF(child_);

        // Remove ownership
        child_->d->hasOwnership = false;
    }

    // Remove previous safe ref
    Py_DECREF(child);
}

void deallocData(SbkObject* self, bool cleanup)
{
    // Make cleanup if this is not a wrapper otherwise this will be done on wrapper destructor
    if(cleanup) {
        removeParent(self);

        if (self->d->parentInfo)
            _destroyParentInfo(self, true);

        clearReferences(self);
    }

    if (self->d->cptr) {
        // Remove from BindingManager
        Shiboken::BindingManager::instance().releaseWrapper(self);
        delete[] self->d->cptr;
        self->d->cptr = 0;
        // delete self->d; PYSIDE-205: wrong!
    }
    delete self->d; // PYSIDE-205: always delete d.
    Py_XDECREF(self->ob_dict);

    // PYSIDE-571: qApp is no longer allocated.
    if (PyObject_IS_GC(reinterpret_cast<PyObject*>(self)))
        Py_TYPE(self)->tp_free(self);
}

void setTypeUserData(SbkObject* wrapper, void* userData, DeleteUserDataFunc d_func)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(Py_TYPE(wrapper));
    if (sotp->user_data)
        sotp->d_func(sotp->user_data);

    sotp->d_func = d_func;
    sotp->user_data = userData;
}

void* getTypeUserData(SbkObject* wrapper)
{
    return PepType_SOTP(Py_TYPE(wrapper))->user_data;
}

void keepReference(SbkObject* self, const char* key, PyObject* referredObject, bool append)
{
    bool isNone = (!referredObject || (referredObject == Py_None));

    if (!self->d->referredObjects)
        self->d->referredObjects = new Shiboken::RefCountMap;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    RefCountMap::iterator iter = refCountMap.find(key);
    std::list<PyObject*> objects;
    if (iter != refCountMap.end()) {
        objects = (*iter).second;
        std::list<PyObject*>::const_iterator found = std::find(objects.begin(), objects.end(), referredObject);

        // skip if objects already exists
        if (found != objects.end())
            return;
    }

    if (append && !isNone) {
        refCountMap[key].push_back(referredObject);
        Py_INCREF(referredObject);
    } else if (!append) {
        if (objects.size() > 0)
            decRefPyObjectList(objects, isNone ? 0 : referredObject);
        if (isNone) {
            if (iter != refCountMap.end())
                refCountMap.erase(iter);
        } else {
            objects.clear();
            objects.push_back(referredObject);
            refCountMap[key] = objects;
            Py_INCREF(referredObject);
        }
    }
}

void removeReference(SbkObject* self, const char* key, PyObject* referredObject)
{
    if (!referredObject || (referredObject == Py_None))
        return;

    if (!self->d->referredObjects)
        return;

    RefCountMap& refCountMap = *(self->d->referredObjects);
    RefCountMap::iterator iter = refCountMap.find(key);
    if (iter != refCountMap.end()) {
        decRefPyObjectList(iter->second);
        refCountMap.erase(iter);
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
    self->d->referredObjects->clear();
}

std::string info(SbkObject* self)
{
    std::ostringstream s;
    std::list<SbkObjectType*> bases;

    if (self->d && self->d->cptr) {
        if (ObjectType::isUserType(Py_TYPE(self)))
            bases = getCppBaseClasses(Py_TYPE(self));
        else
            bases.push_back(reinterpret_cast<SbkObjectType*>(Py_TYPE(self)));

        s << "C++ address....... ";
        std::list<SbkObjectType*>::const_iterator it = bases.begin();
        for (int i = 0; it != bases.end(); ++it, ++i)
            s << reinterpret_cast<PyTypeObject *>(*it)->tp_name << '/' << self->d->cptr[i] << ' ';
        s << "\n";
    }
    else {
        s << "C++ address....... <<Deleted>>\n";
    }

    s << "hasOwnership...... " << bool(self->d->hasOwnership) << "\n"
         "containsCppWrapper " << self->d->containsCppWrapper << "\n"
         "validCppObject.... " << self->d->validCppObject << "\n"
         "wasCreatedByPython " << self->d->cppObjectCreated << "\n";


    if (self->d->parentInfo && self->d->parentInfo->parent) {
        s << "parent............ ";
        Shiboken::AutoDecRef parent(PyObject_Str(reinterpret_cast<PyObject *>(self->d->parentInfo->parent)));
        s << String::toCString(parent) << "\n";
    }

    if (self->d->parentInfo && !self->d->parentInfo->children.empty()) {
        s << "children.......... ";
        ChildrenList& children = self->d->parentInfo->children;
        for (ChildrenList::const_iterator it = children.begin(); it != children.end(); ++it) {
            Shiboken::AutoDecRef child(PyObject_Str(reinterpret_cast<PyObject *>(*it)));
            s << String::toCString(child) << ' ';
        }
        s << '\n';
    }

    if (self->d->referredObjects && !self->d->referredObjects->empty()) {
        Shiboken::RefCountMap& map = *self->d->referredObjects;
        s << "referred objects.. ";
        Shiboken::RefCountMap::const_iterator it = map.begin();
        for (; it != map.end(); ++it) {
            if (it != map.begin())
                s << "                   ";
            s << '"' << it->first << "\" => ";
            std::list<PyObject*>::const_iterator j = it->second.begin();
            for (; j != it->second.end(); ++j) {
                Shiboken::AutoDecRef obj(PyObject_Str(*j));
                s << String::toCString(obj) << ' ';
            }
            s << ' ';
        }
        s << '\n';
    }
    return s.str();
}

} // namespace Object

} // namespace Shiboken
