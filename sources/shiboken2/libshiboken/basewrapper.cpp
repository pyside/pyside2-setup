/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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
#include "helper.h"
#include "sbkconverter.h"
#include "sbkenum.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"
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

#include <iostream>

#if defined(__APPLE__)
#include <dlfcn.h>
#endif

namespace {
    void _destroyParentInfo(SbkObject *obj, bool keepReference);
}

static void callDestructor(const Shiboken::DtorAccumulatorVisitor::DestructorEntries &dts)
{
    for (const auto &e : dts) {
        Shiboken::ThreadStateSaver threadSaver;
        threadSaver.save();
        e.destructor(e.cppInstance);
    }
}

extern "C"
{

// PYSIDE-939: A general replacement for object_dealloc.
void Sbk_object_dealloc(PyObject *self)
{
    if (PepRuntime_38_flag) {
        // PYSIDE-939: Handling references correctly.
        // This was not needed before Python 3.8 (Python issue 35810)
        Py_DECREF(Py_TYPE(self));
    }
    Py_TYPE(self)->tp_free(self);
}

static void SbkObjectTypeDealloc(PyObject *pyObj);
static PyObject *SbkObjectTypeTpNew(PyTypeObject *metatype, PyObject *args, PyObject *kwds);

// PYSIDE-908: The function PyType_Modified does not work in PySide, so we need to
// explicitly pass __doc__. For __signature__ it _did_ actually work, because
// it was not existing before. We add them both for clarity.
static PyGetSetDef SbkObjectType_Type_getsetlist[] = {
    {const_cast<char *>("__signature__"), (getter)Sbk_TypeGet___signature__},
    {const_cast<char *>("__doc__"),       (getter)Sbk_TypeGet___doc__},
    {nullptr}  // Sentinel
};

#if PY_VERSION_HEX < 0x03000000

static PyObject *SbkObjectType_repr(PyObject *type)
{
    Shiboken::AutoDecRef mod(PyObject_GetAttr(type, Shiboken::PyMagicName::module()));
    if (mod.isNull())
        return nullptr;
    Shiboken::AutoDecRef name(PyObject_GetAttr(type, Shiboken::PyMagicName::qualname()));
    if (name.isNull())
        return nullptr;
    return PyString_FromFormat("<class '%s.%s'>",
                               PyString_AS_STRING(mod.object()),
                               PyString_AS_STRING(name.object()));
}

#endif // PY_VERSION_HEX < 0x03000000

static PyType_Slot SbkObjectType_Type_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *>(SbkObjectTypeDealloc)},
    {Py_tp_setattro, reinterpret_cast<void *>(PyObject_GenericSetAttr)},
    {Py_tp_base, static_cast<void *>(&PyType_Type)},
    {Py_tp_alloc, reinterpret_cast<void *>(PyType_GenericAlloc)},
    {Py_tp_new, reinterpret_cast<void *>(SbkObjectTypeTpNew)},
    {Py_tp_free, reinterpret_cast<void *>(PyObject_GC_Del)},
    {Py_tp_getset, reinterpret_cast<void *>(SbkObjectType_Type_getsetlist)},
#if PY_VERSION_HEX < 0x03000000
    {Py_tp_repr, reinterpret_cast<void *>(SbkObjectType_repr)},
#endif
    {0, nullptr}
};
static PyType_Spec SbkObjectType_Type_spec = {
    "1:Shiboken.ObjectType",
    0,   // basicsize (inserted later)
    sizeof(PyMemberDef),
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    SbkObjectType_Type_slots,
};


#if PY_VERSION_HEX < 0x03000000
/*****************************************************************************
 *
 * PYSIDE-816: Workaround for Python 2.7 for SbkObjectType_TypeF().
 *
 * This is an add-on for function typeobject.c:tp_new_wrapper from Python 2.7 .
 * Problem:
 * In Python 3.X, tp_new_wrapper uses this check:

    while (staticbase && (staticbase->tp_new == slot_tp_new))

 * In Python 2.7, it uses this, instead:

    while (staticbase && (staticbase->tp_flags & Py_TPFLAGS_HEAPTYPE))

 * The problem is that heap types have this unwanted dependency.
 * But we cannot get at static slot_tp_new, and so we have to use
 * the original function and patch Py_TPFLAGS_HEAPTYPE away during the call.
 *
 * PYSIDE-1051: The same problem holds for all dynamic metatypes generated by
 *              SbkObjectTypeTpNew()   and all types generated by
 *              introduceWrapperType() .
 *
 * This led to a drastic overhaul of patch_tp_new_wrapper() which now adds
 * the new wrapper to exactly those types which have the old wrapper.
 */

ternaryfunc old_tp_new_wrapper = nullptr;

static PyObject *
tp_new_wrapper(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyTypeObject *type = reinterpret_cast<PyTypeObject *>(self);
    Py_ssize_t orig_flags = type->tp_flags;
    type->tp_flags &= ~Py_TPFLAGS_HEAPTYPE;
    PyObject *ret = reinterpret_cast<ternaryfunc>(old_tp_new_wrapper)(self, args, kwds);
    type->tp_flags = orig_flags;
    return ret;
}

// This is intentionally the __new__ docstring of Python 3.7 .
static struct PyMethodDef tp_new_methoddef[] = {
    {"__new__", reinterpret_cast<PyCFunction>(tp_new_wrapper), METH_VARARGS|METH_KEYWORDS,
     PyDoc_STR("__new__($type, *args, **kwargs)\n--\n\n"
               "Create and return a new object.  "
               "See help(type) for accurate signature.")},
    {0}
};

static int
patch_tp_new_wrapper(PyTypeObject *type)
{
    /*
     * The old tp_new_wrapper is added to all types that have tp_new.
     * We patch that with a version that ignores the heaptype flag.
     */
    auto newMethod = Shiboken::PyMagicName::new_();
    if (old_tp_new_wrapper == nullptr) {
        PyObject *func = PyDict_GetItem(PyType_Type.tp_dict, newMethod);
        assert(func);
        PyCFunctionObject *pycf_ob = reinterpret_cast<PyCFunctionObject *>(func);
        old_tp_new_wrapper = reinterpret_cast<ternaryfunc>(pycf_ob->m_ml->ml_meth);
    }
    PyObject *mro = type->tp_mro;
    Py_ssize_t i, n = PyTuple_GET_SIZE(mro);
    for (i = 0; i < n; i++) {
        type = reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(mro, i));
        PyObject *existing = PyDict_GetItem(type->tp_dict, newMethod);
        if (existing && PyCFunction_Check(existing)
                     && type->tp_flags & Py_TPFLAGS_HEAPTYPE) {
            auto *pycf_ob = reinterpret_cast<PyCFunctionObject *>(existing);
            auto existing_wrapper = reinterpret_cast<ternaryfunc>(pycf_ob->m_ml->ml_meth);
            if (existing_wrapper == tp_new_wrapper)
                break;
            if (existing_wrapper == old_tp_new_wrapper) {
                PyObject *ob_type = reinterpret_cast<PyObject *>(type);
                Shiboken::AutoDecRef func(PyCFunction_New(tp_new_methoddef, ob_type));
                if (func.isNull() || PyDict_SetItem(type->tp_dict, newMethod, func))
                    return -1;
            }
        }
    }
    return 0;
}
/*****************************************************************************/
#endif // PY_VERSION_HEX < 0x03000000


PyTypeObject *SbkObjectType_TypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        SbkObjectType_Type_spec.basicsize =
            PepHeapType_SIZE + sizeof(SbkObjectTypePrivate);
        type = reinterpret_cast<PyTypeObject *>(SbkType_FromSpec(&SbkObjectType_Type_spec));
#if PY_VERSION_HEX < 0x03000000
        if (patch_tp_new_wrapper(type) < 0)
            return nullptr;
#endif
    }
    return type;
}

static PyObject *SbkObjectGetDict(PyObject *pObj, void *)
{
    auto *obj = reinterpret_cast<SbkObject *>(pObj);
    if (!obj->ob_dict)
        obj->ob_dict = PyDict_New();
    if (!obj->ob_dict)
        return nullptr;
    Py_INCREF(obj->ob_dict);
    return obj->ob_dict;
}

static PyGetSetDef SbkObjectGetSetList[] = {
    {const_cast<char *>("__dict__"), SbkObjectGetDict, nullptr, nullptr, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // Sentinel
};

static int SbkObject_traverse(PyObject *self, visitproc visit, void *arg)
{
    auto *sbkSelf = reinterpret_cast<SbkObject *>(self);

    //Visit children
    Shiboken::ParentInfo *pInfo = sbkSelf->d->parentInfo;
    if (pInfo) {
        for (SbkObject *c : pInfo->children)
             Py_VISIT(c);
    }

    //Visit refs
    Shiboken::RefCountMap *rInfo = sbkSelf->d->referredObjects;
    if (rInfo) {
        for (auto it = rInfo->begin(), end = rInfo->end(); it != end; ++it)
            Py_VISIT(it->second);
    }

    if (sbkSelf->ob_dict)
        Py_VISIT(sbkSelf->ob_dict);
    return 0;
}

static int SbkObject_clear(PyObject *self)
{
    auto *sbkSelf = reinterpret_cast<SbkObject *>(self);

    Shiboken::Object::removeParent(sbkSelf);

    if (sbkSelf->d->parentInfo)
        _destroyParentInfo(sbkSelf, true);

    Shiboken::Object::clearReferences(sbkSelf);

    if (sbkSelf->ob_dict)
        Py_CLEAR(sbkSelf->ob_dict);
    return 0;
}

static PyType_Slot SbkObject_Type_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *>(SbkDeallocWrapperWithPrivateDtor)},
    {Py_tp_traverse, reinterpret_cast<void *>(SbkObject_traverse)},
    {Py_tp_clear, reinterpret_cast<void *>(SbkObject_clear)},
    // unsupported: {Py_tp_weaklistoffset, (void *)offsetof(SbkObject, weakreflist)},
    {Py_tp_getset, reinterpret_cast<void *>(SbkObjectGetSetList)},
    // unsupported: {Py_tp_dictoffset, (void *)offsetof(SbkObject, ob_dict)},
    {0, nullptr}
};
static PyType_Spec SbkObject_Type_spec = {
    "1:Shiboken.Object",
    sizeof(SbkObject),
    0,
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_HAVE_GC,
    SbkObject_Type_slots,
};


SbkObjectType *SbkObject_TypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        type = reinterpret_cast<PyTypeObject *>(SbkType_FromSpec(&SbkObject_Type_spec));
        Py_TYPE(type) = SbkObjectType_TypeF();
        Py_INCREF(Py_TYPE(type));
        type->tp_weaklistoffset = offsetof(SbkObject, weakreflist);
        type->tp_dictoffset = offsetof(SbkObject, ob_dict);
    }
    return reinterpret_cast<SbkObjectType *>(type);
}

static int mainThreadDeletionHandler(void *)
{
    if (Py_IsInitialized())
        Shiboken::BindingManager::instance().runDeletionInMainThread();
    return 0;
}

static void SbkDeallocWrapperCommon(PyObject *pyObj, bool canDelete)
{
    auto *sbkObj = reinterpret_cast<SbkObject *>(pyObj);
    PyTypeObject *pyType = Py_TYPE(pyObj);

    // Need to decref the type if this is the dealloc func; if type
    // is subclassed, that dealloc func will decref (see subtype_dealloc
    // in typeobject.c in the python sources)
    bool needTypeDecref = (false
                           || PyType_GetSlot(pyType, Py_tp_dealloc) == SbkDeallocWrapper
                           || PyType_GetSlot(pyType, Py_tp_dealloc) == SbkDeallocWrapperWithPrivateDtor);
    if (PepRuntime_38_flag) {
        // PYSIDE-939: Additional rule: Also when a subtype is heap allocated,
        // then the subtype_dealloc deref will be suppressed, and we need again
        // to supply a decref.
        needTypeDecref |= (pyType->tp_base->tp_flags & Py_TPFLAGS_HEAPTYPE) != 0;
    }

#if defined(__APPLE__)
    // Just checking once that our assumptions are right.
    if (false) {
        void *p = PyType_GetSlot(pyType, Py_tp_dealloc);
        Dl_info dl_info;
        dladdr(p, &dl_info);
        fprintf(stderr, "tp_dealloc is %s\n", dl_info.dli_sname);
    }
    // Gives one of our functions
    //  "Sbk_object_dealloc"
    //  "SbkDeallocWrapperWithPrivateDtor"
    //  "SbkDeallocQAppWrapper"
    //  "SbkDeallocWrapper"
    // but for typedealloc_test.py we get
    //  "subtype_dealloc"
#endif

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
    SbkObjectTypePrivate *sotp{nullptr};
    canDelete &= sbkObj->d->hasOwnership && sbkObj->d->validCppObject;
    if (canDelete) {
        sotp = PepType_SOTP(pyType);
        if (sotp->delete_in_main_thread && Shiboken::currentThreadId() != Shiboken::mainThreadId()) {
            auto &bindingManager = Shiboken::BindingManager::instance();
            if (sotp->is_multicpp) {
                 Shiboken::DtorAccumulatorVisitor visitor(sbkObj);
                 Shiboken::walkThroughClassHierarchy(Py_TYPE(pyObj), &visitor);
                 for (const auto &e : visitor.entries())
                     bindingManager.addToDeletionInMainThread(e);
            } else {
                Shiboken::DestructorEntry e{sotp->cpp_dtor, sbkObj->d->cptr[0]};
                bindingManager.addToDeletionInMainThread(e);
            }
            Py_AddPendingCall(mainThreadDeletionHandler, nullptr);
            canDelete = false;
        }
    }

    if (canDelete) {
        if (sotp->is_multicpp) {
            Shiboken::DtorAccumulatorVisitor visitor(sbkObj);
            Shiboken::walkThroughClassHierarchy(Py_TYPE(pyObj), &visitor);
            Shiboken::Object::deallocData(sbkObj, true);
            callDestructor(visitor.entries());
        } else {
            void *cptr = sbkObj->d->cptr[0];
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
    if (PepRuntime_38_flag) {
        // PYSIDE-939: Handling references correctly.
        // This was not needed before Python 3.8 (Python issue 35810)
        Py_DECREF(pyType);
    }
}

void SbkDeallocWrapper(PyObject *pyObj)
{
    SbkDeallocWrapperCommon(pyObj, true);
}

void SbkDeallocQAppWrapper(PyObject *pyObj)
{
    SbkDeallocWrapper(pyObj);
    // PYSIDE-571: make sure to create a singleton deleted qApp.
    Py_DECREF(MakeQAppWrapper(nullptr));
}

void SbkDeallocWrapperWithPrivateDtor(PyObject *self)
{
    SbkDeallocWrapperCommon(self, false);
}

void SbkObjectTypeDealloc(PyObject *pyObj)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(pyObj);
    auto type = reinterpret_cast<PyTypeObject *>(pyObj);

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
    if (PepRuntime_38_flag) {
        // PYSIDE-939: Handling references correctly.
        // This was not needed before Python 3.8 (Python issue 35810)
        Py_DECREF(Py_TYPE(pyObj));
    }
}

PyObject *SbkObjectTypeTpNew(PyTypeObject *metatype, PyObject *args, PyObject *kwds)
{
    // Check if all bases are new style before calling type.tp_new
    // Was causing gc assert errors in test_bug704.py when
    // this check happened after creating the type object.
    // Argument parsing take from type.tp_new code.

    // PYSIDE-595: Also check if all bases allow inheritance.
    // Before we changed to heap types, it was sufficient to remove the
    // Py_TPFLAGS_BASETYPE flag. That does not work, because PySide does
    // not respect this flag itself!
    PyObject *name;
    PyObject *pyBases;
    PyObject *dict;
    static const char *kwlist[] = { "name", "bases", "dict", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO!O!:sbktype", const_cast<char **>(kwlist),
                                     &name,
                                     &PyTuple_Type, &pyBases,
                                     &PyDict_Type, &dict))
        return nullptr;

    for (int i=0, i_max=PyTuple_GET_SIZE(pyBases); i < i_max; i++) {
        PyObject *baseType = PyTuple_GET_ITEM(pyBases, i);
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
    auto type_new = reinterpret_cast<newfunc>(PyType_Type.tp_new);

    // PYSIDE-939: This is a temporary patch that circumvents the problem
    // with Py_TPFLAGS_METHOD_DESCRIPTOR until this is finally solved.
    // PyType_Ready uses mro(). We need to temporarily remove the flag from it's type.
    // We cannot use PyMethodDescr_Type since it is not exported by Python 2.7 .
    static PyTypeObject *PyMethodDescr_TypePtr = Py_TYPE(
        PyObject_GetAttr(reinterpret_cast<PyObject *>(&PyType_Type), Shiboken::PyName::mro()));
    auto hold = PyMethodDescr_TypePtr->tp_flags;
    PyMethodDescr_TypePtr->tp_flags &= ~Py_TPFLAGS_METHOD_DESCRIPTOR;
    auto *newType = reinterpret_cast<SbkObjectType *>(type_new(metatype, args, kwds));
    PyMethodDescr_TypePtr->tp_flags = hold;

    if (!newType)
        return nullptr;
#if PY_VERSION_HEX < 0x03000000
    // PYSIDE-1051: The newly created metatype needs the PYSIDE-816 wrapper, too.
    if (patch_tp_new_wrapper(&newType->type) < 0)
        return nullptr;
#endif

    Shiboken::ObjectType::initPrivateData(newType);
    SbkObjectTypePrivate *sotp = PepType_SOTP(newType);

    const auto bases = Shiboken::getCppBaseClasses(reinterpret_cast<PyTypeObject *>(newType));
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

    for (SbkObjectType *base : bases) {
        if (PepType_SOTP(base)->subtype_init)
            PepType_SOTP(base)->subtype_init(newType, args, kwds);
    }

    return reinterpret_cast<PyObject *>(newType);
}

static PyObject *_setupNew(SbkObject *self, PyTypeObject *subtype)
{
    Py_INCREF(reinterpret_cast<PyObject *>(subtype));
    auto d = new SbkObjectPrivate;

    SbkObjectTypePrivate *sotp = PepType_SOTP(subtype);
    int numBases = ((sotp && sotp->is_multicpp) ?
        Shiboken::getNumberOfCppBaseClasses(subtype) : 1);
    d->cptr = new void *[numBases];
    std::memset(d->cptr, 0, sizeof(void *) *size_t(numBases));
    d->hasOwnership = 1;
    d->containsCppWrapper = 0;
    d->validCppObject = 0;
    d->parentInfo = nullptr;
    d->referredObjects = nullptr;
    d->cppObjectCreated = 0;
    self->ob_dict = nullptr;
    self->weakreflist = nullptr;
    self->d = d;
    return reinterpret_cast<PyObject *>(self);
}

PyObject *SbkObjectTpNew(PyTypeObject *subtype, PyObject *, PyObject *)
{
    SbkObject *self = PyObject_GC_New(SbkObject, subtype);
    PyObject *res = _setupNew(self, subtype);
    PyObject_GC_Track(reinterpret_cast<PyObject *>(self));
    return res;
}

PyObject *SbkQAppTpNew(PyTypeObject *subtype, PyObject *, PyObject *)
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
    auto self = reinterpret_cast<SbkObject *>(MakeQAppWrapper(subtype));
    return self == nullptr ? nullptr : _setupNew(self, subtype);
}

PyObject *SbkDummyNew(PyTypeObject *type, PyObject *, PyObject *)
{
    // PYSIDE-595: Give the same error as type_call does when tp_new is NULL.
    PyErr_Format(PyExc_TypeError,
                 "cannot create '%.100s' instances ¯\\_(ツ)_/¯",
                 type->tp_name);
    return nullptr;
}

PyObject *SbkType_FromSpec(PyType_Spec *spec)
{
    return SbkType_FromSpecWithBases(spec, nullptr);
}

PyObject *SbkType_FromSpecWithBases(PyType_Spec *spec, PyObject *bases)
{
    // PYSIDE-1286: Generate correct __module__ and __qualname__
    // The name field can now be extended by an "n:" prefix which is
    // the number of modules in the name. The default is 1.
    //
    // Example:
    //    "2:mainmod.submod.mainclass.subclass"
    // results in
    //    __module__   : "mainmod.submod"
    //    __qualname__ : "mainclass.subclass"
    //    __name__     : "subclass"

    PyType_Spec new_spec = *spec;
    const char *colon = strchr(spec->name, ':');
    assert(colon);
    int package_level = atoi(spec->name);
    const char *mod = new_spec.name = colon + 1;

    PyObject *type = PyType_FromSpecWithBases(&new_spec, bases);
    if (type == nullptr)
        return nullptr;

    const char *qual = mod;
    for (int idx = package_level; idx > 0; --idx) {
        const char *dot = strchr(qual, '.');
        if (!dot)
            break;
        qual = dot + 1;
    }
    int mlen = qual - mod - 1;
    Shiboken::AutoDecRef module(Shiboken::String::fromCString(mod, mlen));
    Shiboken::AutoDecRef qualname(Shiboken::String::fromCString(qual));
    if (PyObject_SetAttr(type, Shiboken::PyMagicName::module(), module) < 0)
        return nullptr;
    if (PyObject_SetAttr(type, Shiboken::PyMagicName::qualname(), qualname) < 0)
        return nullptr;
    return type;
}

} //extern "C"


namespace
{

void _destroyParentInfo(SbkObject *obj, bool keepReference)
{
    Shiboken::ParentInfo *pInfo = obj->d->parentInfo;
    if (pInfo) {
        while(!pInfo->children.empty()) {
            SbkObject *first = *pInfo->children.begin();
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
bool walkThroughClassHierarchy(PyTypeObject *currentType, HierarchyVisitor *visitor)
{
    PyObject *bases = currentType->tp_bases;
    Py_ssize_t numBases = PyTuple_GET_SIZE(bases);
    bool result = false;
    for (int i = 0; !result && i < numBases; ++i) {
        auto type = reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(bases, i));
        if (PyType_IsSubtype(type, reinterpret_cast<PyTypeObject *>(SbkObject_TypeF()))) {
            auto sbkType = reinterpret_cast<SbkObjectType *>(type);
            result = PepType_SOTP(sbkType)->is_user_type
                     ? walkThroughClassHierarchy(type, visitor) : visitor->visit(sbkType);
        }
    }
    return result;
}

// Wrapper metatype and base type ----------------------------------------------------------

HierarchyVisitor::HierarchyVisitor() = default;
HierarchyVisitor::~HierarchyVisitor() = default;

bool BaseCountVisitor::visit(SbkObjectType *)
{
    m_count++;
    return false;
}

bool BaseAccumulatorVisitor::visit(SbkObjectType *node)
{
    m_bases.push_back(node);
    return false;
}

bool GetIndexVisitor::visit(SbkObjectType *node)
{
    m_index++;
    return PyType_IsSubtype(reinterpret_cast<PyTypeObject *>(node), m_desiredType);
}

bool DtorAccumulatorVisitor::visit(SbkObjectType *node)
{
    m_entries.push_back(DestructorEntry{PepType_SOTP(node)->cpp_dtor,
                                        m_pyObject->d->cptr[m_entries.size()]});
    return false;
}

void _initMainThreadId(); // helper.cpp

namespace Conversions { void init(); }

void init()
{
    static bool shibokenAlreadInitialised = false;
    if (shibokenAlreadInitialised)
        return;

    _initMainThreadId();

    Conversions::init();

    PyEval_InitThreads();

    //Init private data
    Pep384_Init();

    Shiboken::ObjectType::initPrivateData(SbkObject_TypeF());

    if (PyType_Ready(SbkEnumType_TypeF()) < 0)
        Py_FatalError("[libshiboken] Failed to initialize Shiboken.SbkEnumType metatype.");

    if (PyType_Ready(SbkObjectType_TypeF()) < 0)
        Py_FatalError("[libshiboken] Failed to initialize Shiboken.BaseWrapperType metatype.");

    if (PyType_Ready(reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())) < 0)
        Py_FatalError("[libshiboken] Failed to initialize Shiboken.BaseWrapper type.");

    VoidPtr::init();

    shibokenAlreadInitialised = true;
}

// setErrorAboutWrongArguments now gets overload info from the signature module.
void setErrorAboutWrongArguments(PyObject *args, const char *funcName)
{
    SetError_Argument(args, funcName);
}

class FindBaseTypeVisitor : public HierarchyVisitor
{
public:
    explicit FindBaseTypeVisitor(PyTypeObject *typeToFind) : m_typeToFind(typeToFind) {}

    bool visit(SbkObjectType *node) override
    {
        return reinterpret_cast<PyTypeObject *>(node) == m_typeToFind;
    }

private:
    PyTypeObject *m_typeToFind;
};

std::vector<SbkObject *> splitPyObject(PyObject *pyObj)
{
    std::vector<SbkObject *> result;
    if (PySequence_Check(pyObj)) {
        AutoDecRef lst(PySequence_Fast(pyObj, "Invalid keep reference object."));
        if (!lst.isNull()) {
            for (Py_ssize_t i = 0, i_max = PySequence_Fast_GET_SIZE(lst.object()); i < i_max; ++i) {
                PyObject *item = PySequence_Fast_GET_ITEM(lst.object(), i);
                if (Object::checkType(item))
                    result.push_back(reinterpret_cast<SbkObject *>(item));
            }
        }
    } else {
        result.push_back(reinterpret_cast<SbkObject *>(pyObj));
    }
    return result;
}

template <class Iterator>
inline void decRefPyObjectList(Iterator i1, Iterator i2)
{
    for (; i1 != i2; ++i1)
        Py_DECREF(i1->second);
}

namespace ObjectType
{

bool checkType(PyTypeObject *type)
{
    return PyType_IsSubtype(type, reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())) != 0;
}

bool isUserType(PyTypeObject *type)
{
    return checkType(type) && PepType_SOTP(type)->is_user_type;
}

bool canCallConstructor(PyTypeObject *myType, PyTypeObject *ctorType)
{
    FindBaseTypeVisitor visitor(ctorType);
    if (!walkThroughClassHierarchy(myType, &visitor)) {
        PyErr_Format(PyExc_TypeError, "%s isn't a direct base class of %s", ctorType->tp_name, myType->tp_name);
        return false;
    }
    return true;
}

bool hasCast(SbkObjectType *type)
{
    return PepType_SOTP(type)->mi_specialcast != nullptr;
}

void *cast(SbkObjectType *sourceType, SbkObject *obj, PyTypeObject *targetType)
{
    return PepType_SOTP(sourceType)->mi_specialcast(Object::cppPointer(obj, targetType),
        reinterpret_cast<SbkObjectType *>(targetType));
}

void setCastFunction(SbkObjectType *type, SpecialCastFunction func)
{
    PepType_SOTP(type)->mi_specialcast = func;
}

void setOriginalName(SbkObjectType *type, const char *name)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(type);
    if (sotp->original_name)
        free(sotp->original_name);
    sotp->original_name = strdup(name);
}

const char *getOriginalName(SbkObjectType *type)
{
    return PepType_SOTP(type)->original_name;
}

void setTypeDiscoveryFunctionV2(SbkObjectType *type, TypeDiscoveryFuncV2 func)
{
    PepType_SOTP(type)->type_discovery = func;
}

void copyMultipleInheritance(SbkObjectType *type, SbkObjectType *other)
{
    PepType_SOTP(type)->mi_init = PepType_SOTP(other)->mi_init;
    PepType_SOTP(type)->mi_offsets = PepType_SOTP(other)->mi_offsets;
    PepType_SOTP(type)->mi_specialcast = PepType_SOTP(other)->mi_specialcast;
}

void setMultipleInheritanceFunction(SbkObjectType *type, MultipleInheritanceInitFunction function)
{
    PepType_SOTP(type)->mi_init = function;
}

MultipleInheritanceInitFunction getMultipleInheritanceFunction(SbkObjectType *type)
{
    return PepType_SOTP(type)->mi_init;
}

void setDestructorFunction(SbkObjectType *type, ObjectDestructor func)
{
    PepType_SOTP(type)->cpp_dtor = func;
}

void initPrivateData(SbkObjectType *type)
{
    PepType_SOTP(type) = new SbkObjectTypePrivate;
    memset(PepType_SOTP(type), 0, sizeof(SbkObjectTypePrivate));
}

SbkObjectType *
introduceWrapperType(PyObject *enclosingObject,
                     const char *typeName,
                     const char *originalName,
                     PyType_Spec *typeSpec,
                     const char *signatureStrings[],
                     ObjectDestructor cppObjDtor,
                     SbkObjectType *baseType,
                     PyObject *baseTypes,
                     unsigned wrapperFlags)
{
    typeSpec->slots[0].pfunc = reinterpret_cast<void *>(baseType ? baseType : SbkObject_TypeF());

    PyObject *heaptype = SbkType_FromSpecWithBases(typeSpec, baseTypes);
    Py_TYPE(heaptype) = SbkObjectType_TypeF();
    Py_INCREF(Py_TYPE(heaptype));
    auto *type = reinterpret_cast<SbkObjectType *>(heaptype);
#if PY_VERSION_HEX < 0x03000000
    // PYSIDE-1051: The newly created type needs the PYSIDE-816 wrapper, too.
    if (patch_tp_new_wrapper(&type->type) < 0)
        return nullptr;
#endif
    if (baseType) {
        if (baseTypes) {
            for (int i = 0; i < PySequence_Fast_GET_SIZE(baseTypes); ++i)
                BindingManager::instance().addClassInheritance(reinterpret_cast<SbkObjectType *>(PySequence_Fast_GET_ITEM(baseTypes, i)), type);
        } else {
            BindingManager::instance().addClassInheritance(baseType, type);
        }
    }
    // PYSIDE-510: Here is the single change to support signatures.
    if (SbkSpecial_Type_Ready(enclosingObject, reinterpret_cast<PyTypeObject *>(type), signatureStrings) < 0) {
        std::cerr << "Warning: " << __FUNCTION__ << " returns nullptr for "
            << typeName << '/' << originalName << " due to SbkSpecial_Type_Ready() failing\n";
        return nullptr;
    }

    initPrivateData(type);
    auto sotp = PepType_SOTP(type);
    if (wrapperFlags & DeleteInMainThread)
        sotp->delete_in_main_thread = 1;

    setOriginalName(type, originalName);
    setDestructorFunction(type, cppObjDtor);
    auto *ob_type = reinterpret_cast<PyObject *>(type);

    if (wrapperFlags & InnerClass)
        return PyDict_SetItemString(enclosingObject, typeName, ob_type) == 0 ? type : nullptr;

    // PyModule_AddObject steals type's reference.
    Py_INCREF(ob_type);
    if (PyModule_AddObject(enclosingObject, typeName, ob_type) != 0) {
        std::cerr << "Warning: " << __FUNCTION__ << " returns nullptr for "
            << typeName << '/' << originalName << " due to PyModule_AddObject(enclosingObject="
            << enclosingObject << ",ob_type=" << ob_type << ") failing\n";
        return nullptr;
    }
    return type;
}

void setSubTypeInitHook(SbkObjectType *type, SubTypeInitHook func)
{
    PepType_SOTP(type)->subtype_init = func;
}

void *getTypeUserData(SbkObjectType *type)
{
    return PepType_SOTP(type)->user_data;
}

void setTypeUserData(SbkObjectType *type, void *userData, DeleteUserDataFunc d_func)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(type);
    sotp->user_data = userData;
    sotp->d_func = d_func;
}

// Try to find the exact type of cptr.
SbkObjectType *typeForTypeName(const char *typeName)
{
    SbkObjectType *result{};
    if (typeName) {
        if (PyTypeObject *pyType = Shiboken::Conversions::getPythonTypeObject(typeName))
            result = reinterpret_cast<SbkObjectType *>(pyType);
    }
    return result;
}

bool hasSpecialCastFunction(SbkObjectType *sbkType)
{
    const SbkObjectTypePrivate *d = PepType_SOTP(sbkType);
    return d != nullptr && d->mi_specialcast != nullptr;
}

} // namespace ObjectType


namespace Object
{

static void recursive_invalidate(SbkObject *self, std::set<SbkObject *>& seen);

bool checkType(PyObject *pyObj)
{
    return ObjectType::checkType(Py_TYPE(pyObj));
}

bool isUserType(PyObject *pyObj)
{
    return ObjectType::isUserType(Py_TYPE(pyObj));
}

Py_hash_t hash(PyObject *pyObj)
{
    assert(Shiboken::Object::checkType(pyObj));
    return reinterpret_cast<Py_hash_t>(pyObj);
}

static void setSequenceOwnership(PyObject *pyObj, bool owner)
{

    bool has_length = true;

    if (!pyObj)
        return;

    if (PySequence_Size(pyObj) < 0) {
        PyErr_Clear();
        has_length = false;
    }

    if (PySequence_Check(pyObj) && has_length) {
        Py_ssize_t size = PySequence_Size(pyObj);
        if (size > 0) {
            const auto objs = splitPyObject(pyObj);
            if (owner) {
                for (SbkObject *o : objs)
                    getOwnership(o);
            } else {
                for (SbkObject *o : objs)
                    releaseOwnership(o);
            }
        }
    } else if (Object::checkType(pyObj)) {
        if (owner)
            getOwnership(reinterpret_cast<SbkObject *>(pyObj));
        else
            releaseOwnership(reinterpret_cast<SbkObject *>(pyObj));
    }
}

void setValidCpp(SbkObject *pyObj, bool value)
{
    pyObj->d->validCppObject = value;
}

void setHasCppWrapper(SbkObject *pyObj, bool value)
{
    pyObj->d->containsCppWrapper = value;
}

bool hasCppWrapper(SbkObject *pyObj)
{
    return pyObj->d->containsCppWrapper;
}

bool wasCreatedByPython(SbkObject *pyObj)
{
    return pyObj->d->cppObjectCreated;
}

void callCppDestructors(SbkObject *pyObj)
{
    PyTypeObject *type = Py_TYPE(pyObj);
    SbkObjectTypePrivate *sotp = PepType_SOTP(type);
    if (sotp->is_multicpp) {
        Shiboken::DtorAccumulatorVisitor visitor(pyObj);
        Shiboken::walkThroughClassHierarchy(type, &visitor);
        callDestructor(visitor.entries());
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
    pyObj->d->cptr = nullptr;
    pyObj->d->validCppObject = false;
}

bool hasOwnership(SbkObject *pyObj)
{
    return pyObj->d->hasOwnership;
}

void getOwnership(SbkObject *self)
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

void getOwnership(PyObject *pyObj)
{
    if (pyObj)
        setSequenceOwnership(pyObj, true);
}

void releaseOwnership(SbkObject *self)
{
    // skip if the ownership have already moved to c++
    auto *selfType = reinterpret_cast<SbkObjectType *>(Py_TYPE(self));
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

void releaseOwnership(PyObject *self)
{
    setSequenceOwnership(self, false);
}

/* Needed forward declarations */
static void recursive_invalidate(PyObject *pyobj, std::set<SbkObject *>& seen);
static void recursive_invalidate(SbkObject *self, std::set<SbkObject *> &seen);

void invalidate(PyObject *pyobj)
{
    std::set<SbkObject *> seen;
    recursive_invalidate(pyobj, seen);
}

void invalidate(SbkObject *self)
{
    std::set<SbkObject *> seen;
    recursive_invalidate(self, seen);
}

static void recursive_invalidate(PyObject *pyobj, std::set<SbkObject *> &seen)
{
    const auto objs = splitPyObject(pyobj);
    for (SbkObject *o : objs)
        recursive_invalidate(o, seen);
}

static void recursive_invalidate(SbkObject *self, std::set<SbkObject *> &seen)
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

        for (SbkObject *child : copy) {
            // invalidate the child
            recursive_invalidate(child, seen);

            // if the parent not is a wrapper class, then remove children from him, because We do not know when this object will be destroyed
            if (!self->d->validCppObject)
                removeParent(child, true, true);
        }
    }

    // If has ref to other objects invalidate all
    if (self->d->referredObjects) {
        RefCountMap &refCountMap = *(self->d->referredObjects);
        for (auto it = refCountMap.begin(), end = refCountMap.end(); it != end; ++it)
            recursive_invalidate(it->second, seen);
    }
}

void makeValid(SbkObject *self)
{
    // Skip if this object not is a valid object
    if (!self || reinterpret_cast<PyObject *>(self) == Py_None || self->d->validCppObject)
        return;

    // Mark object as invalid only if this is not a wrapper class
    self->d->validCppObject = true;

    // If it is a parent make  all children valid
    if (self->d->parentInfo) {
        for (SbkObject *child : self->d->parentInfo->children)
            makeValid(child);
    }

    // If has ref to other objects make all valid again
    if (self->d->referredObjects) {
        RefCountMap &refCountMap = *(self->d->referredObjects);
        RefCountMap::iterator iter;
        for (auto it = refCountMap.begin(), end = refCountMap.end(); it != end; ++it) {
            if (Shiboken::Object::checkType(it->second))
                makeValid(reinterpret_cast<SbkObject *>(it->second));
        }
    }
}

void *cppPointer(SbkObject *pyObj, PyTypeObject *desiredType)
{
    PyTypeObject *type = Py_TYPE(pyObj);
    int idx = 0;
    if (PepType_SOTP(reinterpret_cast<SbkObjectType *>(type))->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);
    if (pyObj->d->cptr)
        return pyObj->d->cptr[idx];
    return nullptr;
}

std::vector<void *> cppPointers(SbkObject *pyObj)
{
    int n = getNumberOfCppBaseClasses(Py_TYPE(pyObj));
    std::vector<void *> ptrs(n);
    for (int i = 0; i < n; ++i)
        ptrs[i] = pyObj->d->cptr[i];
    return ptrs;
}


bool setCppPointer(SbkObject *sbkObj, PyTypeObject *desiredType, void *cptr)
{
    int idx = 0;
    PyTypeObject *type = Py_TYPE(sbkObj);
    if (PepType_SOTP(type)->is_multicpp)
        idx = getTypeIndexOnHierarchy(type, desiredType);

    const bool alreadyInitialized = sbkObj->d->cptr[idx] != nullptr;
    if (alreadyInitialized)
        PyErr_SetString(PyExc_RuntimeError, "You can't initialize an object twice!");
    else
        sbkObj->d->cptr[idx] = cptr;

    sbkObj->d->cppObjectCreated = true;
    return !alreadyInitialized;
}

bool isValid(PyObject *pyObj)
{
    if (!pyObj || pyObj == Py_None
        || Py_TYPE(Py_TYPE(pyObj)) != SbkObjectType_TypeF()) {
        return true;
    }

    auto priv = reinterpret_cast<SbkObject *>(pyObj)->d;

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

bool isValid(SbkObject *pyObj, bool throwPyError)
{
    if (!pyObj)
        return false;

    SbkObjectPrivate *priv = pyObj->d;
    if (!priv->cppObjectCreated && isUserType(reinterpret_cast<PyObject *>(pyObj))) {
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

bool isValid(PyObject *pyObj, bool throwPyError)
{
    if (!pyObj || pyObj == Py_None ||
        !PyType_IsSubtype(Py_TYPE(pyObj), reinterpret_cast<PyTypeObject *>(SbkObject_TypeF()))) {
        return true;
    }
    return isValid(reinterpret_cast<SbkObject *>(pyObj), throwPyError);
}

SbkObject *findColocatedChild(SbkObject *wrapper,
                              const SbkObjectType *instanceType)
{
    // Degenerate case, wrapper is the correct wrapper.
    if (reinterpret_cast<const void *>(Py_TYPE(wrapper)) == reinterpret_cast<const void *>(instanceType))
        return wrapper;

    if (!(wrapper->d && wrapper->d->cptr))
        return nullptr;

    ParentInfo *pInfo = wrapper->d->parentInfo;
    if (!pInfo)
        return nullptr;

    ChildrenList &children = pInfo->children;

    for (SbkObject *child : children) {
        if (!(child->d && child->d->cptr))
            continue;
        if (child->d->cptr[0] == wrapper->d->cptr[0]) {
            return reinterpret_cast<const void *>(Py_TYPE(child)) == reinterpret_cast<const void *>(instanceType)
                ? child : findColocatedChild(child, instanceType);
        }
    }
    return nullptr;
}

PyObject *newObject(SbkObjectType *instanceType,
                    void *cptr,
                    bool hasOwnership,
                    bool isExactType,
                    const char *typeName)
{
    // Try to find the exact type of cptr.
    if (!isExactType) {
        if (SbkObjectType *exactType = ObjectType::typeForTypeName(typeName))
            instanceType = exactType;
        else
            instanceType = BindingManager::instance().resolveType(&cptr, instanceType);
    }

    bool shouldCreate = true;
    bool shouldRegister = true;
    SbkObject *self = nullptr;

    // Some logic to ensure that colocated child field does not overwrite the parent
    if (BindingManager::instance().hasWrapper(cptr)) {
        SbkObject *existingWrapper = BindingManager::instance().retrieveWrapper(cptr);

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
        self = reinterpret_cast<SbkObject *>(SbkObjectTpNew(reinterpret_cast<PyTypeObject *>(instanceType), nullptr, nullptr));
        self->d->cptr[0] = cptr;
        self->d->hasOwnership = hasOwnership;
        self->d->validCppObject = 1;
        if (shouldRegister) {
            BindingManager::instance().registerWrapper(self, cptr);
        }
    } else {
        Py_IncRef(reinterpret_cast<PyObject *>(self));
    }
    return reinterpret_cast<PyObject *>(self);
}

void destroy(SbkObject *self, void *cppData)
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
        self->d->cptr = nullptr;
    }

    // After this point the object can be death do not use the self pointer bellow
}

void removeParent(SbkObject *child, bool giveOwnershipBack, bool keepReference)
{
    ParentInfo *pInfo = child->d->parentInfo;
    if (!pInfo || !pInfo->parent) {
        if (pInfo && pInfo->hasWrapperRef) {
            pInfo->hasWrapperRef = false;
        }
        return;
    }

    ChildrenList &oldBrothers = pInfo->parent->d->parentInfo->children;
    // Verify if this child is part of parent list
    auto iChild = oldBrothers.find(child);
    if (iChild == oldBrothers.end())
        return;

    oldBrothers.erase(iChild);

    pInfo->parent = nullptr;

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

void setParent(PyObject *parent, PyObject *child)
{
    if (!child || child == Py_None || child == parent)
        return;

    /*
     * setParent is recursive when the child is a native Python sequence, i.e. objects not binded by Shiboken
     * like tuple and list.
     *
     * This "limitation" exists to fix the following problem: A class multiple inherits QObject and QString,
     * so if you pass this class to someone that takes the ownership, we CAN'T enter in this if, but hey! QString
     * follows the sequence protocol.
     */
    if (PySequence_Check(child) && !Object::checkType(child)) {
        Shiboken::AutoDecRef seq(PySequence_Fast(child, nullptr));
        for (Py_ssize_t i = 0, max = PySequence_Size(seq); i < max; ++i)
            setParent(parent, PySequence_Fast_GET_ITEM(seq.object(), i));
        return;
    }

    bool parentIsNull = !parent || parent == Py_None;
    auto parent_ = reinterpret_cast<SbkObject *>(parent);
    auto child_ = reinterpret_cast<SbkObject *>(child);

    if (!parentIsNull) {
        if (!parent_->d->parentInfo)
            parent_->d->parentInfo = new ParentInfo;

        // do not re-add a child
        if (child_->d->parentInfo && (child_->d->parentInfo->parent == parent_))
            return;
    }

    ParentInfo *pInfo = child_->d->parentInfo;
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

void deallocData(SbkObject *self, bool cleanup)
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
        self->d->cptr = nullptr;
        // delete self->d; PYSIDE-205: wrong!
    }
    delete self->d; // PYSIDE-205: always delete d.
    Py_XDECREF(self->ob_dict);

    // PYSIDE-571: qApp is no longer allocated.
    if (PyObject_IS_GC(reinterpret_cast<PyObject *>(self)))
        Py_TYPE(self)->tp_free(self);
}

void setTypeUserData(SbkObject *wrapper, void *userData, DeleteUserDataFunc d_func)
{
    SbkObjectTypePrivate *sotp = PepType_SOTP(Py_TYPE(wrapper));
    if (sotp->user_data)
        sotp->d_func(sotp->user_data);

    sotp->d_func = d_func;
    sotp->user_data = userData;
}

void *getTypeUserData(SbkObject *wrapper)
{
    return PepType_SOTP(Py_TYPE(wrapper))->user_data;
}

static inline bool isNone(const PyObject *o)
{
    return o == nullptr || o == Py_None;
}

static void removeRefCountKey(SbkObject *self, const char *key)
{
    if (self->d->referredObjects) {
        const auto iterPair = self->d->referredObjects->equal_range(key);
        if (iterPair.first != iterPair.second) {
            decRefPyObjectList(iterPair.first, iterPair.second);
            self->d->referredObjects->erase(iterPair.first, iterPair.second);
        }
    }
}

void keepReference(SbkObject *self, const char *key, PyObject *referredObject, bool append)
{
    if (isNone(referredObject)) {
        removeRefCountKey(self, key);
        return;
    }

    if (!self->d->referredObjects) {
        self->d->referredObjects =
            new Shiboken::RefCountMap{RefCountMap::value_type{key, referredObject}};
        Py_INCREF(referredObject);
        return;
    }

    RefCountMap &refCountMap = *(self->d->referredObjects);
    const auto iterPair = refCountMap.equal_range(key);
    if (std::any_of(iterPair.first, iterPair.second,
                    [referredObject](const RefCountMap::value_type &v) { return v.second == referredObject; })) {
        return;
    }

    if (!append && iterPair.first != iterPair.second) {
        decRefPyObjectList(iterPair.first, iterPair.second);
        refCountMap.erase(iterPair.first, iterPair.second);
    }

    refCountMap.insert(RefCountMap::value_type{key, referredObject});
    Py_INCREF(referredObject);
}

void removeReference(SbkObject *self, const char *key, PyObject *referredObject)
{
    if (!isNone(referredObject))
        removeRefCountKey(self, key);
}

void clearReferences(SbkObject *self)
{
    if (!self->d->referredObjects)
        return;

    RefCountMap &refCountMap = *(self->d->referredObjects);
    for (auto it = refCountMap.begin(), end = refCountMap.end(); it != end; ++it)
        Py_DECREF(it->second);
    self->d->referredObjects->clear();
}

std::string info(SbkObject *self)
{
    std::ostringstream s;

    if (self->d && self->d->cptr) {
        std::vector<SbkObjectType *> bases;
        if (ObjectType::isUserType(Py_TYPE(self)))
            bases = getCppBaseClasses(Py_TYPE(self));
        else
            bases.push_back(reinterpret_cast<SbkObjectType *>(Py_TYPE(self)));

        s << "C++ address....... ";
        for (size_t i = 0, size = bases.size(); i < size; ++i) {
            auto base = reinterpret_cast<PyTypeObject *>(bases[i]);
            s << base->tp_name << '/' << self->d->cptr[i] << ' ';
        }
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
        for (SbkObject *sbkChild : self->d->parentInfo->children) {
            Shiboken::AutoDecRef child(PyObject_Str(reinterpret_cast<PyObject *>(sbkChild)));
            s << String::toCString(child) << ' ';
        }
        s << '\n';
    }

    if (self->d->referredObjects && !self->d->referredObjects->empty()) {
        Shiboken::RefCountMap &map = *self->d->referredObjects;
        s << "referred objects.. ";
        std::string lastKey;
        for (auto it = map.begin(), end = map.end(); it != end; ++it) {
            if (it->first != lastKey) {
                if (!lastKey.empty())
                    s << "                   ";
                s << '"' << it->first << "\" => ";
                lastKey = it->first;
            }
            Shiboken::AutoDecRef obj(PyObject_Str(it->second));
            s << String::toCString(obj) << ' ';
        }
        s << '\n';
    }
    return s.str();
}

} // namespace Object

} // namespace Shiboken
