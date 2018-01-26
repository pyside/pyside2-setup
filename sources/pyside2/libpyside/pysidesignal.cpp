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

#include <sbkpython.h>
#include "pysidesignal.h"
#include "pysidesignal_p.h"
#include "signalmanager.h"

#include <shiboken.h>
#include <QDebug>

#define SIGNAL_CLASS_NAME "Signal"
#define SIGNAL_INSTANCE_NAME "SignalInstance"
#define QT_SIGNAL_SENTINEL '2'

namespace PySide {
namespace Signal {
    //aux
    class SignalSignature {
    public:
        SignalSignature() : m_attributes(QMetaMethod::Compatibility) {}
        SignalSignature(QByteArray parameterTypes) : m_parameterTypes(parameterTypes),
            m_attributes(QMetaMethod::Compatibility) {}
        SignalSignature(QByteArray parameterTypes, QMetaMethod::Attributes attributes) :
            m_parameterTypes(parameterTypes),
            m_attributes(attributes) {}
        QByteArray m_parameterTypes;
        QMetaMethod::Attributes m_attributes;
    };

    static char*        buildSignature(const char*, const char*);
    static void         appendSignature(PySideSignal*, const SignalSignature &);
    static void         instanceInitialize(PySideSignalInstance*, PyObject*, PySideSignal*, PyObject*, int);
    static char*        parseSignature(PyObject*);
    static PyObject*    buildQtCompatible(const char*);
}
}

extern "C"
{

// Signal methods
static int signalTpInit(PyObject*, PyObject*, PyObject*);
static void signalFree(void*);
static void signalInstanceFree(void*);
static PyObject* signalGetItem(PyObject* self, PyObject* key);
static PyObject* signalToString(PyObject* self);

// Signal Instance methods
static PyObject* signalInstanceConnect(PyObject*, PyObject*, PyObject*);
static PyObject* signalInstanceDisconnect(PyObject*, PyObject*);
static PyObject* signalInstanceEmit(PyObject*, PyObject*);
static PyObject* signalInstanceGetItem(PyObject*, PyObject*);

static PyObject* signalInstanceCall(PyObject* self, PyObject* args, PyObject* kw);
static PyObject* signalCall(PyObject*, PyObject*, PyObject*);

static PyObject* metaSignalCheck(PyObject*, PyObject*);

static PyMappingMethods Signal_as_mapping = {
    0,
    signalGetItem,
    0
};

static PyMethodDef Signal_methods[] = {
    {"__instancecheck__", (PyCFunction)metaSignalCheck, METH_O, NULL},
    {0, 0, 0, 0}
};

PyTypeObject PySideSignalMetaType = {
    PyVarObject_HEAD_INIT(0, 0)
    /*tp_name*/             "PySide2.QtCore.MetaSignal",
    /*tp_basicsize*/        sizeof(PyTypeObject),
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
    /*tp_flags*/            Py_TPFLAGS_DEFAULT,
    /*tp_doc*/              0,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Signal_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             &PyType_Type,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              0,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0,
    /*tp_del*/              0,
    /*tp_version_tag*/      0
};

PyTypeObject PySideSignalType = {
    PyVarObject_HEAD_INIT(&PySideSignalMetaType, 0)
    /*tp_name*/             "PySide2.QtCore." SIGNAL_CLASS_NAME,
    /*tp_basicsize*/        sizeof(PySideSignal),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       &Signal_as_mapping,
    /*tp_hash*/             0,
    /*tp_call*/             signalCall,
    /*tp_str*/              signalToString,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT,
    /*tp_doc*/              SIGNAL_CLASS_NAME,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             signalTpInit,
    /*tp_alloc*/            0,
    /*tp_new*/              PyType_GenericNew,
    /*tp_free*/             signalFree,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0,
    /*tp_del*/              0,
    /*tp_version_tag*/      0
};

static PyMethodDef SignalInstance_methods[] = {
    {"connect", (PyCFunction)signalInstanceConnect, METH_VARARGS|METH_KEYWORDS, 0},
    {"disconnect", signalInstanceDisconnect, METH_VARARGS, 0},
    {"emit", signalInstanceEmit, METH_VARARGS, 0},
    {0, 0, 0, 0}  /* Sentinel */
};

static PyMappingMethods SignalInstance_as_mapping = {
    0,
    signalInstanceGetItem,
    0
};

PyTypeObject PySideSignalInstanceType = {
    PyVarObject_HEAD_INIT(0, 0)
    /*tp_name*/             "PySide2.QtCore." SIGNAL_INSTANCE_NAME,
    /*tp_basicsize*/        sizeof(PySideSignalInstance),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       &SignalInstance_as_mapping,
    /*tp_hash*/             0,
    /*tp_call*/             signalInstanceCall,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT,
    /*tp_doc*/              SIGNAL_INSTANCE_NAME,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          SignalInstance_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              PyType_GenericNew,
    /*tp_free*/             signalInstanceFree,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0,
    /*tp_del*/              0,
    /*tp_version_tag*/      0
};

int signalTpInit(PyObject* self, PyObject* args, PyObject* kwds)
{
    static PyObject* emptyTuple = 0;
    static const char* kwlist[] = {"name", 0};
    char* argName = 0;

    if (emptyTuple == 0)
        emptyTuple = PyTuple_New(0);

    if (!PyArg_ParseTupleAndKeywords(emptyTuple, kwds,
        "|s:QtCore." SIGNAL_CLASS_NAME, const_cast<char**>(kwlist), &argName))
        return 0;

    bool tupledArgs = false;
    PySideSignal* data = reinterpret_cast<PySideSignal*>(self);
    if (argName) {
        data->signalName = strdup(argName);
    }

    for (Py_ssize_t i = 0, i_max = PyTuple_Size(args); i < i_max; i++) {
        PyObject* arg = PyTuple_GET_ITEM(args, i);
        if (PySequence_Check(arg) && !Shiboken::String::check(arg)) {
            tupledArgs = true;
            char *sig = PySide::Signal::parseSignature(arg);
            PySide::Signal::appendSignature(
                        data,
                        PySide::Signal::SignalSignature(sig));
            free(sig);
        }
    }

    if (!tupledArgs) {
        char *sig = PySide::Signal::parseSignature(args);
        PySide::Signal::appendSignature(
                    data,
                    PySide::Signal::SignalSignature(sig));
        free(sig);
    }

    return 1;
}

void signalFree(void* self)
{
    PyObject* pySelf = reinterpret_cast<PyObject*>(self);
    PySideSignal* data = reinterpret_cast<PySideSignal*>(self);

    for (int i = 0, i_max = data->signaturesSize; i < i_max; i++) {
        if (data->signatures[i])
            free(data->signatures[i]);
    }

    free(data->signatures);
    free(data->signatureAttributes);
    free(data->signalName);
    data->initialized = 0;
    data->signaturesSize = 0;
    Py_XDECREF(data->homonymousMethod);
    data->homonymousMethod = 0;

    pySelf->ob_type->tp_base->tp_free(self);
}

PyObject* signalGetItem(PyObject* self, PyObject* key)
{
    PySideSignal* data = reinterpret_cast<PySideSignal*>(self);
    char* sigKey;
    if (key) {
        sigKey = PySide::Signal::parseSignature(key);
    } else {
        if (data->signatures[0])
            sigKey = strdup(data->signatures[0]);
        else
            sigKey = strdup("void");
    }
    char* sig = PySide::Signal::buildSignature(data->signalName, sigKey);
    free(sigKey);
    PyObject* pySignature = Shiboken::String::fromCString(sig);
    free(sig);
    return pySignature;
}


PyObject* signalToString(PyObject* self)
{
    return signalGetItem(self, 0);
}

void signalInstanceFree(void* self)
{
    PyObject* pySelf = reinterpret_cast<PyObject*>(self);
    PySideSignalInstance* data = reinterpret_cast<PySideSignalInstance*>(self);

    PySideSignalInstancePrivate* dataPvt = data->d;
    free(dataPvt->signalName);
    free(dataPvt->signature);

    Py_XDECREF(dataPvt->homonymousMethod);

    if (dataPvt->next) {
        Py_DECREF(dataPvt->next);
        dataPvt->next = 0;
    }
    delete dataPvt;
    data->d = 0;
    pySelf->ob_type->tp_base->tp_free(self);
}

PyObject* signalInstanceConnect(PyObject* self, PyObject* args, PyObject* kwds)
{
    PyObject* slot = 0;
    PyObject* type = 0;
    static const char* kwlist[] = {"slot", "type", 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwds,
        "O|O:" SIGNAL_INSTANCE_NAME, const_cast<char**>(kwlist), &slot, &type))
        return 0;

    PySideSignalInstance* source = reinterpret_cast<PySideSignalInstance*>(self);
    Shiboken::AutoDecRef pyArgs(PyList_New(0));

    bool match = false;
    if (slot->ob_type == &PySideSignalInstanceType) {
        PySideSignalInstance* sourceWalk = source;
        PySideSignalInstance* targetWalk;

        //find best match
        while (sourceWalk && !match) {
            targetWalk = reinterpret_cast<PySideSignalInstance*>(slot);
            while (targetWalk && !match) {
                if (QMetaObject::checkConnectArgs(sourceWalk->d->signature, targetWalk->d->signature)) {
                    PyList_Append(pyArgs, sourceWalk->d->source);
                    Shiboken::AutoDecRef sourceSignature(PySide::Signal::buildQtCompatible(sourceWalk->d->signature));
                    PyList_Append(pyArgs, sourceSignature);

                    PyList_Append(pyArgs, targetWalk->d->source);
                    Shiboken::AutoDecRef targetSignature(PySide::Signal::buildQtCompatible(targetWalk->d->signature));
                    PyList_Append(pyArgs, targetSignature);

                    match = true;
                }
                targetWalk = reinterpret_cast<PySideSignalInstance*>(targetWalk->d->next);
            }
            sourceWalk = reinterpret_cast<PySideSignalInstance*>(sourceWalk->d->next);
        }
    } else {
        // Check signature of the slot (method or function) to match signal
        int slotArgs = -1;
        bool useSelf = false;
        bool isMethod = PyMethod_Check(slot);
        bool isFunction = PyFunction_Check(slot);
        bool matchedSlot = false;

        QByteArray functionName;
        PySideSignalInstance *it = source;

        if (isMethod || isFunction) {
            PyObject *function = isMethod ? PyMethod_GET_FUNCTION(slot) : slot;
            PyCodeObject *objCode = reinterpret_cast<PyCodeObject *>(PyFunction_GET_CODE(function));
            PyFunctionObject *function_obj = reinterpret_cast<PyFunctionObject *>(function);
            functionName = Shiboken::String::toCString(Pep384Function_GetName(function_obj));
            useSelf = isMethod;
            slotArgs = Pep384Code_GET_FLAGS(objCode) & CO_VARARGS ? -1 : Pep384Code_GET_ARGCOUNT(objCode);
            if (useSelf)
                slotArgs -= 1;

            // Get signature args
            bool isShortCircuit = false;
            int signatureArgs = 0;
            QStringList argsSignature;

            argsSignature = PySide::Signal::getArgsFromSignature(it->d->signature,
                &isShortCircuit);
            signatureArgs = argsSignature.length();

            // Iterate the possible types of connection for this signal and compare
            // it with slot arguments
            if (signatureArgs != slotArgs) {
                while (it->d->next != nullptr) {
                    it = it->d->next;
                    argsSignature = PySide::Signal::getArgsFromSignature(it->d->signature,
                        &isShortCircuit);
                    signatureArgs = argsSignature.length();
                    if (signatureArgs == slotArgs) {
                        matchedSlot = true;
                        break;
                    }
                }
            }
        }

        // Adding references to pyArgs
        PyList_Append(pyArgs, source->d->source);

        if (matchedSlot) {
            // If a slot matching the same number of arguments was found,
            // include signature to the pyArgs
            Shiboken::AutoDecRef signature(PySide::Signal::buildQtCompatible(it->d->signature));
            PyList_Append(pyArgs, signature);
        } else {
            // Try the first by default if the slot was not found
            Shiboken::AutoDecRef signature(PySide::Signal::buildQtCompatible(source->d->signature));
            PyList_Append(pyArgs, signature);
        }
        PyList_Append(pyArgs, slot);
        match = true;
    }

    if (type)
        PyList_Append(pyArgs, type);

    if (match) {
        Shiboken::AutoDecRef tupleArgs(PyList_AsTuple(pyArgs));
        Shiboken::AutoDecRef pyMethod(PyObject_GetAttrString(source->d->source, "connect"));
        if (pyMethod.isNull()) { // PYSIDE-79: check if pyMethod exists.
            PyErr_SetString(PyExc_RuntimeError, "method 'connect' vanished!");
            return 0;
        }
        PyObject* result = PyObject_CallObject(pyMethod, tupleArgs);
        if (result == Py_True || result == Py_False)
            return result;
        else
            Py_XDECREF(result);
    }
    if (!PyErr_Occurred()) // PYSIDE-79: inverse the logic. A Null return needs an error.
        PyErr_Format(PyExc_RuntimeError, "Failed to connect signal %s.", source->d->signature);
    return 0;
}

int argCountInSignature(const char *signature)
{
    return QByteArray(signature).count(",") + 1;
}

PyObject* signalInstanceEmit(PyObject* self, PyObject* args)
{
    PySideSignalInstance* source = reinterpret_cast<PySideSignalInstance*>(self);

    Shiboken::AutoDecRef pyArgs(PyList_New(0));
    int numArgsGiven = PySequence_Fast_GET_SIZE(args);
    int numArgsInSignature = argCountInSignature(source->d->signature);

    // If number of arguments given to emit is smaller than the first source signature expects,
    // it is possible it's a case of emitting a signal with default parameters.
    // Search through all the overloaded signals with the same name, and try to find a signature
    // with the same number of arguments as given to emit, and is also marked as a cloned method
    // (which in metaobject parlance means a signal with default parameters).
    // @TODO: This should be improved to take into account argument types as well. The current
    // assumption is there are no signals which are both overloaded on argument types and happen to
    // have signatures with default parameters.
    if (numArgsGiven < numArgsInSignature) {
        PySideSignalInstance *possibleDefaultInstance = source;
        while ((possibleDefaultInstance = possibleDefaultInstance->d->next)) {
            if (possibleDefaultInstance->d->attributes & QMetaMethod::Cloned
                    && argCountInSignature(possibleDefaultInstance->d->signature) == numArgsGiven) {
                source = possibleDefaultInstance;
                break;
            }
        }
    }
    Shiboken::AutoDecRef sourceSignature(PySide::Signal::buildQtCompatible(source->d->signature));

    PyList_Append(pyArgs, sourceSignature);
    for (Py_ssize_t i = 0, max = PyTuple_Size(args); i < max; i++)
        PyList_Append(pyArgs, PyTuple_GetItem(args, i));

    Shiboken::AutoDecRef pyMethod(PyObject_GetAttrString(source->d->source, "emit"));

    Shiboken::AutoDecRef tupleArgs(PyList_AsTuple(pyArgs));
    return PyObject_CallObject(pyMethod, tupleArgs);
}

PyObject* signalInstanceGetItem(PyObject* self, PyObject* key)
{
    PySideSignalInstance* data = reinterpret_cast<PySideSignalInstance*>(self);
    char* sigKey = PySide::Signal::parseSignature(key);
    char* sig = PySide::Signal::buildSignature(data->d->signalName, sigKey);
    free(sigKey);
    const char* sigName = data->d->signalName;

    while (data) {
        if (strcmp(data->d->signature, sig) == 0) {
            free(sig);
            PyObject* result = reinterpret_cast<PyObject*>(data);
            Py_INCREF(result);
            return result;
        }
        data = reinterpret_cast<PySideSignalInstance*>(data->d->next);
    }

    PyErr_Format(PyExc_IndexError, "Signature %s not found for signal: %s", sig, sigName);
    free(sig);
    return 0;
}

PyObject* signalInstanceDisconnect(PyObject* self, PyObject* args)
{
    PySideSignalInstance* source = reinterpret_cast<PySideSignalInstance*>(self);
    Shiboken::AutoDecRef pyArgs(PyList_New(0));

    PyObject* slot;
    if (PyTuple_Check(args) && PyTuple_GET_SIZE(args))
        slot = PyTuple_GET_ITEM(args, 0);
    else
        slot = Py_None;

    bool match = false;
    if (slot->ob_type == &PySideSignalInstanceType) {
        PySideSignalInstance* target = reinterpret_cast<PySideSignalInstance*>(slot);
        if (QMetaObject::checkConnectArgs(source->d->signature, target->d->signature)) {
            PyList_Append(pyArgs, source->d->source);
            Shiboken::AutoDecRef source_signature(PySide::Signal::buildQtCompatible(source->d->signature));
            PyList_Append(pyArgs, source_signature);

            PyList_Append(pyArgs, target->d->source);
            Shiboken::AutoDecRef target_signature(PySide::Signal::buildQtCompatible(target->d->signature));
            PyList_Append(pyArgs, target_signature);
            match = true;
        }
    } else {
        //try the first signature
        PyList_Append(pyArgs, source->d->source);
        Shiboken::AutoDecRef signature(PySide::Signal::buildQtCompatible(source->d->signature));
        PyList_Append(pyArgs, signature);

        // disconnect all, so we need to use the c++ signature disconnect(qobj, signal, 0, 0)
        if (slot == Py_None)
            PyList_Append(pyArgs, slot);
        PyList_Append(pyArgs, slot);
        match = true;
    }

    if (match) {
        Shiboken::AutoDecRef tupleArgs(PyList_AsTuple(pyArgs));
        Shiboken::AutoDecRef pyMethod(PyObject_GetAttrString(source->d->source, "disconnect"));
        PyObject* result = PyObject_CallObject(pyMethod, tupleArgs);
        if (!result || result == Py_True)
            return result;
        else
            Py_DECREF(result);
    }

    PyErr_Format(PyExc_RuntimeError, "Failed to disconnect signal %s.", source->d->signature);
    return 0;
}

PyObject* signalCall(PyObject* self, PyObject* args, PyObject* kw)
{
    PySideSignal* signal = reinterpret_cast<PySideSignal*>(self);

    // Native C++ signals can't be called like functions, thus we throw an exception.
    // The only way calling a signal can succeed (the Python equivalent of C++'s operator() )
    // is when a method with the same name as the signal is attached to an object.
    // An example is QProcess::error() (don't check the docs, but the source code of qprocess.h).
    if (!signal->homonymousMethod) {
        PyErr_SetString(PyExc_TypeError, "native Qt signal is not callable");
        return 0;
    }

    descrgetfunc getDescriptor = signal->homonymousMethod->ob_type->tp_descr_get;

    // Check if there exists a method with the same name as the signal, which is also a static
    // method in C++ land.
    Shiboken::AutoDecRef homonymousMethod(getDescriptor(signal->homonymousMethod, 0, 0));
    if (PyCFunction_Check(homonymousMethod)
            && (PyCFunction_GET_FLAGS(homonymousMethod.object()) & METH_STATIC)) {
        return PyCFunction_Call(homonymousMethod, args, kw);
    }

    // Assumes homonymousMethod is not a static method.
    ternaryfunc callFunc = signal->homonymousMethod->ob_type->tp_call;
    return callFunc(homonymousMethod, args, kw);
}

PyObject* signalInstanceCall(PyObject* self, PyObject* args, PyObject* kw)
{
    PySideSignalInstance* PySideSignal = reinterpret_cast<PySideSignalInstance*>(self);
    if (!PySideSignal->d->homonymousMethod) {
        PyErr_SetString(PyExc_TypeError, "native Qt signal is not callable");
        return 0;
    }

    descrgetfunc getDescriptor = PySideSignal->d->homonymousMethod->ob_type->tp_descr_get;
    Shiboken::AutoDecRef homonymousMethod(getDescriptor(PySideSignal->d->homonymousMethod, PySideSignal->d->source, 0));
    return PyCFunction_Call(homonymousMethod, args, kw);
}

static PyObject *metaSignalCheck(PyObject * /* klass */, PyObject* args)
{
    if (PyType_IsSubtype(args->ob_type, &PySideSignalInstanceType))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

} // extern "C"

namespace PySide {
namespace Signal {

void init(PyObject* module)
{
    if (PyType_Ready(&PySideSignalMetaType) < 0)
        return;

    if (PyType_Ready(&PySideSignalType) < 0)
        return;

    Py_INCREF(&PySideSignalType);
    PyModule_AddObject(module, SIGNAL_CLASS_NAME, reinterpret_cast<PyObject *>(&PySideSignalType));

    if (PyType_Ready(&PySideSignalInstanceType) < 0)
        return;

    Py_INCREF(&PySideSignalInstanceType);
}

bool checkType(PyObject* pyObj)
{
    if (pyObj)
        return PyType_IsSubtype(pyObj->ob_type, &PySideSignalType);
    return false;
}

void updateSourceObject(PyObject* source)
{
    PyTypeObject* objType = reinterpret_cast<PyTypeObject*>(PyObject_Type(source));

    Py_ssize_t pos = 0;
    PyObject* value;
    PyObject* key;

    while (PyDict_Next(objType->tp_dict, &pos, &key, &value)) {
        if (PyObject_TypeCheck(value, &PySideSignalType)) {
            Shiboken::AutoDecRef signalInstance(reinterpret_cast<PyObject *>(PyObject_New(PySideSignalInstance, &PySideSignalInstanceType)));
            instanceInitialize(signalInstance.cast<PySideSignalInstance*>(), key, reinterpret_cast<PySideSignal*>(value), source, 0);
            PyObject_SetAttr(source, key, signalInstance);
        }
    }

    Py_XDECREF(objType);
}

char* getTypeName(PyObject* type)
{
    if (PyType_Check(type)) {
        char* typeName = NULL;
        if (PyType_IsSubtype(reinterpret_cast<PyTypeObject*>(type), reinterpret_cast<PyTypeObject*>(&SbkObject_Type))) {
            SbkObjectType* objType = reinterpret_cast<SbkObjectType*>(type);
            typeName = strdup(Shiboken::ObjectType::getOriginalName(objType));
        } else {
            // Translate python types to Qt names
            PyTypeObject* objType = reinterpret_cast<PyTypeObject*>(type);
            if (Shiboken::String::checkType(objType))
                typeName = strdup("QString");
            else if (objType == &PyInt_Type)
                typeName = strdup("int");
            else if (objType == &PyLong_Type)
                typeName = strdup("long");
            else if (objType == &PyFloat_Type)
                typeName = strdup("double");
            else if (objType == &PyBool_Type)
                typeName = strdup("bool");
            else if (Py_TYPE(objType) == &SbkEnumType_Type)
                typeName = strdup(Shiboken::Enum::getCppName(objType));
            else
                typeName = strdup("PyObject");
        }
        return typeName;
    } else if (type == Py_None) { // Must be checked before as Shiboken::String::check accepts Py_None
        return strdup("void");
    } else if (Shiboken::String::check(type)) {
        const char *result = Shiboken::String::toCString(type);
        if (!strcmp(result, "qreal"))
            result = sizeof(qreal) == sizeof(double) ? "double" : "float";
        return strdup(result);
    }
    return 0;
}

char* buildSignature(const char* name, const char* signature)
{
    QByteArray signal(name);
    signal += '(';
    signal += signature;
    signal += ')';
    return strdup(QMetaObject::normalizedSignature(signal));
}

char* parseSignature(PyObject* args)
{
    char* signature = 0;
    if (args && (Shiboken::String::check(args) || !PySequence_Check(args)))
        return getTypeName(args);

    for (Py_ssize_t i = 0, i_max = PySequence_Size(args); i < i_max; i++) {
        Shiboken::AutoDecRef arg(PySequence_ITEM(args, i));
        char* typeName = getTypeName(arg);
        if (typeName) {
            if (signature) {
                signature = reinterpret_cast<char*>(realloc(signature, (strlen(signature) + 1 + strlen(typeName)) * sizeof(char*)));
                signature = strcat(signature, ",");
                signature = strcat(signature, typeName);
                free(typeName);
            } else {
                signature = typeName;
            }
        }
    }
    return signature;
}

void appendSignature(PySideSignal* self, const SignalSignature &signature)
{
    self->signaturesSize++;

    if (self->signaturesSize > 1) {
        self->signatures = reinterpret_cast<char **>(realloc(self->signatures, sizeof(char *) * self->signaturesSize));
        self->signatureAttributes = reinterpret_cast<int *>(realloc(self->signatureAttributes, sizeof(int) * self->signaturesSize));
    } else {
        self->signatures = reinterpret_cast<char **>(malloc(sizeof(char *)));
        self->signatureAttributes = reinterpret_cast<int *>(malloc(sizeof(int)));
    }
    self->signatures[self->signaturesSize - 1] = strdup(signature.m_parameterTypes);
    self->signatureAttributes[self->signaturesSize - 1] = signature.m_attributes;
}

PySideSignalInstance* initialize(PySideSignal* self, PyObject* name, PyObject* object)
{
    PySideSignalInstance* instance = PyObject_New(PySideSignalInstance, &PySideSignalInstanceType);
    SbkObject* sbkObj = reinterpret_cast<SbkObject*>(object);
    if (!Shiboken::Object::wasCreatedByPython(sbkObj))
        Py_INCREF(object); // PYSIDE-79: this flag was crucial for a wrapper call.
    instanceInitialize(instance, name, self, object, 0);
    return instance;
}

void instanceInitialize(PySideSignalInstance* self, PyObject* name, PySideSignal* data, PyObject* source, int index)
{
    self->d = new PySideSignalInstancePrivate;
    PySideSignalInstancePrivate* selfPvt = self->d;
    selfPvt->next = 0;
    if (data->signalName)
        selfPvt->signalName = strdup(data->signalName);
    else {
        selfPvt->signalName = strdup(Shiboken::String::toCString(name));
        data->signalName = strdup(selfPvt->signalName);
    }

    selfPvt->source = source;
    selfPvt->signature = buildSignature(self->d->signalName, data->signatures[index]);
    selfPvt->attributes = data->signatureAttributes[index];
    selfPvt->homonymousMethod = 0;
    if (data->homonymousMethod) {
        selfPvt->homonymousMethod = data->homonymousMethod;
        Py_INCREF(selfPvt->homonymousMethod);
    }
    index++;

    if (index < data->signaturesSize) {
        selfPvt->next = PyObject_New(PySideSignalInstance, &PySideSignalInstanceType);
        instanceInitialize(selfPvt->next, name, data, source, index);
    }
}

bool connect(PyObject* source, const char* signal, PyObject* callback)
{
    Shiboken::AutoDecRef pyMethod(PyObject_GetAttrString(source, "connect"));
    if (pyMethod.isNull())
        return false;

    Shiboken::AutoDecRef pySignature(Shiboken::String::fromCString(signal));
    Shiboken::AutoDecRef pyArgs(PyTuple_Pack(3, source, pySignature.object(), callback));
    PyObject* result =  PyObject_CallObject(pyMethod, pyArgs);
    if (result == Py_False) {
        PyErr_Format(PyExc_RuntimeError, "Failed to connect signal %s, to python callable object.", signal);
        Py_DECREF(result);
        result = 0;
    }
    return result;
}

PySideSignalInstance* newObjectFromMethod(PyObject* source, const QList<QMetaMethod>& methodList)
{
    PySideSignalInstance* root = 0;
    PySideSignalInstance* previous = 0;
    foreach (const QMetaMethod &m, methodList) {
        PySideSignalInstance* item = PyObject_New(PySideSignalInstance, &PySideSignalInstanceType);
        if (!root)
            root = item;

        if (previous)
            previous->d->next = item;

        item->d = new PySideSignalInstancePrivate;
        PySideSignalInstancePrivate* selfPvt = item->d;
        selfPvt->source = source;
        Py_INCREF(selfPvt->source); // PYSIDE-79: an INCREF is missing.
        QByteArray cppName(m.methodSignature());
        cppName.truncate(cppName.indexOf('('));
        // separe SignalName
        selfPvt->signalName = strdup(cppName.data());
        selfPvt->signature = strdup(m.methodSignature());
        selfPvt->attributes = m.attributes();
        selfPvt->homonymousMethod = 0;
        selfPvt->next = 0;
    }
    return root;
}

PySideSignal* newObject(const char* name, ...)
{
    va_list listSignatures;
    char* sig = 0;
    PySideSignal* self = PyObject_New(PySideSignal, &PySideSignalType);
    self->signalName = strdup(name);
    self->signaturesSize = 0;
    self->signatures = 0;
    self->signatureAttributes = 0;
    self->initialized = 0;
    self->homonymousMethod = 0;

    va_start(listSignatures, name);
    sig = va_arg(listSignatures, char*);

    while (sig != NULL) {
        if (strcmp(sig, "void") == 0)
            appendSignature(self, SignalSignature(""));
        else
            appendSignature(self, SignalSignature(sig));

        sig = va_arg(listSignatures, char*);
    }

    va_end(listSignatures);

    return self;
}

template<typename T>
static typename T::value_type join(T t, const char* sep)
{
    typename T::value_type res;
    if (!t.size())
        return res;

    typename T::const_iterator it = t.begin();
    typename T::const_iterator end = t.end();
    res += *it;
    ++it;

    while (it != end) {
        res += sep;
        res += *it;
        ++it;
    }
    return res;
}

static void _addSignalToWrapper(SbkObjectType* wrapperType, const char* signalName, PySideSignal* signal)
{
    PyObject* typeDict = wrapperType->super.ht_type.tp_dict;
    PyObject* homonymousMethod;
    if ((homonymousMethod = PyDict_GetItemString(typeDict, signalName))) {
        Py_INCREF(homonymousMethod);
        signal->homonymousMethod = homonymousMethod;
    }
    PyDict_SetItemString(typeDict, signalName, reinterpret_cast<PyObject*>(signal));
}

// This function is used by qStableSort to promote empty signatures
static bool compareSignals(const SignalSignature &sig1, const SignalSignature &)
{
    return sig1.m_parameterTypes.isEmpty();
}

void registerSignals(SbkObjectType* pyObj, const QMetaObject* metaObject)
{
    typedef QHash<QByteArray, QList<SignalSignature> > SignalSigMap;
    SignalSigMap signalsFound;
    for (int i = metaObject->methodOffset(), max = metaObject->methodCount(); i < max; ++i) {
        QMetaMethod method = metaObject->method(i);

        if (method.methodType() == QMetaMethod::Signal) {
            QByteArray methodName(method.methodSignature());
            methodName.chop(methodName.size() - methodName.indexOf('('));
            SignalSignature signature;
            signature.m_parameterTypes = join(method.parameterTypes(), ",");
            if (method.attributes() & QMetaMethod::Cloned)
                signature.m_attributes = QMetaMethod::Cloned;
            signalsFound[methodName] << signature;
        }
    }

    SignalSigMap::Iterator it = signalsFound.begin();
    SignalSigMap::Iterator end = signalsFound.end();
    for (; it != end; ++it) {
        PySideSignal* self = PyObject_New(PySideSignal, &PySideSignalType);
        self->signalName = strdup(it.key().constData());
        self->signaturesSize = 0;
        self->signatures = 0;
        self->signatureAttributes = 0;
        self->initialized = 0;
        self->homonymousMethod = 0;

        // Empty signatures comes first! So they will be the default signal signature
        qStableSort(it.value().begin(), it.value().end(), &compareSignals);
        SignalSigMap::mapped_type::const_iterator j = it.value().begin();
        SignalSigMap::mapped_type::const_iterator endJ = it.value().end();
        for (; j != endJ; ++j) {
            const SignalSignature &sig = *j;
            appendSignature(self, sig);
        }

        _addSignalToWrapper(pyObj, it.key(), self);
        Py_DECREF(reinterpret_cast<PyObject *>(self));
    }
}

PyObject* buildQtCompatible(const char* signature)
{
    QByteArray ba;
    ba.append(QT_SIGNAL_SENTINEL);
    ba.append(signature);
    return Shiboken::String::fromStringAndSize(ba, ba.size());
}

void addSignalToWrapper(SbkObjectType* wrapperType, const char* signalName, PySideSignal* signal)
{
    _addSignalToWrapper(wrapperType, signalName, signal);
}

PyObject* getObject(PySideSignalInstance* signal)
{
    return signal->d->source;
}

const char* getSignature(PySideSignalInstance* signal)
{
    return signal->d->signature;
}

const char** getSignatures(PyObject* signal, int* size)
{
    PySideSignal* self = reinterpret_cast<PySideSignal*>(signal);
    *size = self->signaturesSize;
    return const_cast<const char**>(self->signatures);
}

QStringList getArgsFromSignature(const char* signature, bool* isShortCircuit)
{
    const QString qsignature = QLatin1String(signature);
    QStringList result;
    QRegExp splitRegex(QLatin1String("\\s*,\\s*"));

    if (isShortCircuit)
        *isShortCircuit = !qsignature.contains(QLatin1Char('('));
    if (qsignature.contains(QLatin1String("()")) || qsignature.contains(QLatin1String("(void)"))) {
        return result;
    } else if (qsignature.contains(QLatin1Char('('))) {
        static QRegExp regex(QLatin1String(".+\\((.*)\\)"));
        //get args types
        QString types = qsignature;
        types.replace(regex, QLatin1String("\\1"));
        result = types.split(splitRegex);
    }
    return result;
}

QString getCallbackSignature(const char* signal, QObject* receiver, PyObject* callback, bool encodeName)
{
    QByteArray functionName;
    int numArgs = -1;
    bool useSelf = false;
    bool isMethod = PyMethod_Check(callback);
    bool isFunction = PyFunction_Check(callback);

    if (isMethod || isFunction) {
        PyObject* function = isMethod ? PyMethod_GET_FUNCTION(callback) : callback;
        PyCodeObject* objCode = reinterpret_cast<PyCodeObject*>(PyFunction_GET_CODE(function));
        functionName = Shiboken::String::toCString(Pep384Function_GetName(function));
        useSelf = isMethod;
        numArgs = Pep384Code_GET_FLAGS(objCode) & CO_VARARGS ? -1 : Pep384Code_GET_ARGCOUNT(objCode);
    } else if (PyCFunction_Check(callback)) {
        const PyCFunctionObject *funcObj = reinterpret_cast<const PyCFunctionObject *>(callback);
        functionName = Pep384CFunction_GET_NAMESTR(funcObj);
        useSelf = PyCFunction_GET_SELF(funcObj);
        const int flags = PyCFunction_GET_FLAGS(funcObj);

        if (receiver) {
            //Search for signature on metaobject
            const QMetaObject* mo = receiver->metaObject();
            QByteArray prefix(functionName);
            prefix += '(';
            for (int i = 0; i < mo->methodCount(); i++) {
                QMetaMethod me = mo->method(i);
                if ((strncmp(me.methodSignature(), prefix, prefix.size()) == 0) &&
                    QMetaObject::checkConnectArgs(signal, me.methodSignature())) {
                    numArgs = me.parameterTypes().size() + useSelf;
                    break;
                }
           }
        }

        if (numArgs == -1) {
            if (flags & METH_VARARGS)
                numArgs = -1;
            else if (flags & METH_NOARGS)
                numArgs = 0;
        }
    } else if (PyCallable_Check(callback)) {
        functionName = "__callback" + QByteArray::number((qlonglong)callback);
    }

    Q_ASSERT(!functionName.isEmpty());

    bool isShortCircuit = false;

    const QString functionNameS = QLatin1String(functionName);
    QString signature = encodeName ? codeCallbackName(callback, functionNameS) : functionNameS;
    QStringList args = getArgsFromSignature(signal, &isShortCircuit);

    if (!isShortCircuit) {
        signature.append(QLatin1Char('('));
        if (numArgs == -1)
            numArgs = std::numeric_limits<int>::max();
        while (args.count() && (args.count() > (numArgs - useSelf))) {
            args.removeLast();
        }
        signature.append(args.join(QLatin1Char(',')));
        signature.append(QLatin1Char(')'));
    }
    return signature;
}

bool isQtSignal(const char* signal)
{
    return (signal && signal[0] == QT_SIGNAL_SENTINEL);
}

bool checkQtSignal(const char* signal)
{
    if (!isQtSignal(signal)) {
        PyErr_SetString(PyExc_TypeError, "Use the function PySide2.QtCore.SIGNAL on signals");
        return false;
    }
    return true;
}

QString codeCallbackName(PyObject* callback, const QString& funcName)
{
    if (PyMethod_Check(callback)) {
        PyObject* self = PyMethod_GET_SELF(callback);
        PyObject* func = PyMethod_GET_FUNCTION(callback);
        return funcName + QString::number(quint64(self), 16) + QString::number(quint64(func), 16);
    } else {
        return funcName + QString::number(quint64(callback), 16);
    }
}

} //namespace Signal
} //namespace PySide

