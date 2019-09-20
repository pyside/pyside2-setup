// -*- mode: cpp;-*-
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

#include "signalmanager.h"
#include "pysidesignal.h"
#include "pysideproperty.h"
#include "pysideproperty_p.h"
#include "pyside.h"
#include "pyside_p.h"
#include "dynamicqmetaobject.h"
#include "pysidemetafunction_p.h"

#include <autodecref.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <gilstate.h>
#include <sbkconverter.h>
#include <sbkstring.h>

#include <QtCore/QDebug>
#include <QtCore/QHash>

#include <algorithm>
#include <limits>

// These private headers are needed to throw JavaScript exceptions
#if PYSIDE_QML_PRIVATE_API_SUPPORT
    #include <private/qv4engine_p.h>
    #include <private/qv4context_p.h>
    #include <private/qqmldata_p.h>
#if QT_VERSION < 0x050700
        #include <private/qqmlcontextwrapper_p.h>
#endif
#endif

#if QSLOT_CODE != 1 || QSIGNAL_CODE != 2
#error QSLOT_CODE and/or QSIGNAL_CODE changed! change the hardcoded stuff to the correct value!
#endif
#define PYSIDE_SLOT '1'
#define PYSIDE_SIGNAL '2'
#include "globalreceiverv2.h"

namespace {
    static PyObject *metaObjectAttr = 0;

    static int callMethod(QObject *object, int id, void **args);
    static PyObject *parseArguments(const QList< QByteArray >& paramTypes, void **args);
    static bool emitShortCircuitSignal(QObject *source, int signalIndex, PyObject *args);

#ifdef IS_PY3K
    static void destroyMetaObject(PyObject *obj)
    {
        void *ptr = PyCapsule_GetPointer(obj, 0);
        auto meta = reinterpret_cast<PySide::MetaObjectBuilder *>(ptr);
        SbkObject *wrapper = Shiboken::BindingManager::instance().retrieveWrapper(meta);
        if (wrapper)
            Shiboken::BindingManager::instance().releaseWrapper(wrapper);
        delete meta;
    }

#else
    static void destroyMetaObject(void *obj)
    {
        auto meta = reinterpret_cast<PySide::MetaObjectBuilder *>(obj);
        SbkObject *wrapper = Shiboken::BindingManager::instance().retrieveWrapper(meta);
        if (wrapper)
            Shiboken::BindingManager::instance().releaseWrapper(wrapper);
        delete meta;
    }
#endif
}

namespace PySide {


PyObjectWrapper::PyObjectWrapper()
    :m_me(Py_None)
{
    Py_XINCREF(m_me);
}

PyObjectWrapper::PyObjectWrapper(PyObject *me)
    : m_me(me)
{
    Py_XINCREF(m_me);
}

PyObjectWrapper::PyObjectWrapper(const PyObjectWrapper &other)
    : m_me(other.m_me)
{
    Py_XINCREF(m_me);
}

PyObjectWrapper::~PyObjectWrapper()
{
    // Check that Python is still initialized as sometimes this is called by a static destructor
    // after Python interpeter is shutdown.
    if (!Py_IsInitialized())
        return;

    Shiboken::GilState gil;
    Py_XDECREF(m_me);
}

void PyObjectWrapper::reset(PyObject *o)
{
    Py_XINCREF(o);
    Py_XDECREF(m_me);
    m_me = o;
}

PyObjectWrapper &PyObjectWrapper::operator=(const PySide::PyObjectWrapper &other)
{
    reset(other.m_me);
    return *this;
}

PyObjectWrapper::operator PyObject *() const
{
    return m_me;
}

QDataStream &operator<<(QDataStream &out, const PyObjectWrapper &myObj)
{
    if (Py_IsInitialized() == 0) {
        qWarning() << "Stream operator for PyObject called without python interpreter.";
        return out;
    }

    static PyObject *reduce_func  = 0;

    Shiboken::GilState gil;
    if (!reduce_func) {
        Shiboken::AutoDecRef pickleModule(PyImport_ImportModule("pickle"));
        reduce_func = PyObject_GetAttrString(pickleModule, "dumps");
    }
    Shiboken::AutoDecRef repr(PyObject_CallFunctionObjArgs(reduce_func, (PyObject *)myObj, NULL));
    if (repr.object()) {
        const char *buff = nullptr;
        Py_ssize_t size  = 0;
        if (PyBytes_Check(repr.object())) {
            buff = PyBytes_AS_STRING(repr.object());
            size = PyBytes_GET_SIZE(repr.object());
        } else if (Shiboken::String::check(repr.object())) {
            buff = Shiboken::String::toCString(repr.object());
            size = Shiboken::String::len(repr.object());
        }
        QByteArray data(buff, size);
        out << data;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, PyObjectWrapper &myObj)
{
    if (Py_IsInitialized() == 0) {
        qWarning() << "Stream operator for PyObject called without python interpreter.";
        return in;
    }

    static PyObject *eval_func  = 0;

    Shiboken::GilState gil;
    if (!eval_func) {
        Shiboken::AutoDecRef pickleModule(PyImport_ImportModule("pickle"));
        eval_func = PyObject_GetAttrString(pickleModule, "loads");
    }

    QByteArray repr;
    in >> repr;
    Shiboken::AutoDecRef pyCode(PyBytes_FromStringAndSize(repr.data(), repr.size()));
    Shiboken::AutoDecRef value(PyObject_CallFunctionObjArgs(eval_func, pyCode.object(), 0));
    if (!value.object())
        value.reset(Py_None);
    myObj.reset(value);
    return in;
}

};

using namespace PySide;

struct SignalManager::SignalManagerPrivate
{
    SharedMap m_globalReceivers;

    SignalManagerPrivate()
    {
        m_globalReceivers = SharedMap( new QMap<QByteArray, GlobalReceiverV2 *>() );
    }

    ~SignalManagerPrivate()
    {
        if (!m_globalReceivers.isNull()) {
            // Delete receivers by always retrieving the current first element, because deleting a
            // receiver can indirectly delete another one, and if we use qDeleteAll, that could
            // cause either a double delete, or iterator invalidation, and thus undefined behavior.
            while (!m_globalReceivers->isEmpty())
                delete *m_globalReceivers->cbegin();
            Q_ASSERT(m_globalReceivers->isEmpty());
        }
    }
};

static void clearSignalManager()
{
    PySide::SignalManager::instance().clear();
}

static void PyObject_PythonToCpp_PyObject_PTR(PyObject *pyIn, void *cppOut)
{
    *reinterpret_cast<PyObject **>(cppOut) = pyIn;
}
static PythonToCppFunc is_PyObject_PythonToCpp_PyObject_PTR_Convertible(PyObject *pyIn)
{
    return PyObject_PythonToCpp_PyObject_PTR;
}
static PyObject *PyObject_PTR_CppToPython_PyObject(const void *cppIn)
{
    auto pyOut = reinterpret_cast<PyObject *>(const_cast<void *>(cppIn));
    if (pyOut)
        Py_INCREF(pyOut);
    return pyOut;
}

SignalManager::SignalManager() : m_d(new SignalManagerPrivate)
{
    // Register Qt primitive typedefs used on signals.
    using namespace Shiboken;

    // Register PyObject type to use in queued signal and slot connections
    qRegisterMetaType<PyObjectWrapper>("PyObject");
    qRegisterMetaTypeStreamOperators<PyObjectWrapper>("PyObject");
    qRegisterMetaTypeStreamOperators<PyObjectWrapper>("PyObjectWrapper");
    qRegisterMetaTypeStreamOperators<PyObjectWrapper>("PySide::PyObjectWrapper");

    SbkConverter *converter = Shiboken::Conversions::createConverter(&PyBaseObject_Type, nullptr);
    Shiboken::Conversions::setCppPointerToPythonFunction(converter, PyObject_PTR_CppToPython_PyObject);
    Shiboken::Conversions::setPythonToCppPointerFunctions(converter, PyObject_PythonToCpp_PyObject_PTR, is_PyObject_PythonToCpp_PyObject_PTR_Convertible);
    Shiboken::Conversions::registerConverterName(converter, "PyObject");
    Shiboken::Conversions::registerConverterName(converter, "object");
    Shiboken::Conversions::registerConverterName(converter, "PyObjectWrapper");
    Shiboken::Conversions::registerConverterName(converter, "PySide::PyObjectWrapper");

    PySide::registerCleanupFunction(clearSignalManager);

    if (!metaObjectAttr)
        metaObjectAttr = Shiboken::String::fromCString("__METAOBJECT__");
}

void SignalManager::clear()
{
    delete m_d;
    m_d = new SignalManagerPrivate();
}

SignalManager::~SignalManager()
{
    delete m_d;
}

SignalManager &SignalManager::instance()
{
    static SignalManager me;
    return me;
}

QObject *SignalManager::globalReceiver(QObject *sender, PyObject *callback)
{
    SharedMap globalReceivers = m_d->m_globalReceivers;
    QByteArray hash = GlobalReceiverV2::hash(callback);
    GlobalReceiverV2 *gr = nullptr;
    auto it = globalReceivers->find(hash);
    if (it == globalReceivers->end()) {
        gr = new GlobalReceiverV2(callback, globalReceivers);
        globalReceivers->insert(hash, gr);
        if (sender) {
            gr->incRef(sender); // create a link reference
            gr->decRef(); // remove extra reference
        }
    } else {
        gr = it.value();
        if (sender)
            gr->incRef(sender);
    }

    return reinterpret_cast<QObject *>(gr);
}

int SignalManager::countConnectionsWith(const QObject *object)
{
    int count = 0;
    for (GlobalReceiverV2Map::const_iterator it = m_d->m_globalReceivers->cbegin(), end = m_d->m_globalReceivers->cend(); it != end; ++it) {
        if (it.value()->refCount(object))
            count++;
    }
    return count;
}

void SignalManager::notifyGlobalReceiver(QObject *receiver)
{
    reinterpret_cast<GlobalReceiverV2 *>(receiver)->notify();
}

void SignalManager::releaseGlobalReceiver(const QObject *source, QObject *receiver)
{
    auto gr = reinterpret_cast<GlobalReceiverV2 *>(receiver);
    gr->decRef(source);
}

int SignalManager::globalReceiverSlotIndex(QObject *receiver, const char *signature) const
{
    return reinterpret_cast<GlobalReceiverV2 *>(receiver)->addSlot(signature);
}

bool SignalManager::emitSignal(QObject *source, const char *signal, PyObject *args)
{
    if (!Signal::checkQtSignal(signal))
        return false;
    signal++;

    int signalIndex = source->metaObject()->indexOfSignal(signal);
    if (signalIndex != -1) {
        // cryptic but works!
        // if the signature doesn't have a '(' it's a shor circuited signal, i.e. std::find
        // returned the string null terminator.
        bool isShortCircuit = !*std::find(signal, signal + std::strlen(signal), '(');
        if (isShortCircuit)
            return emitShortCircuitSignal(source, signalIndex, args);
        else
            return MetaFunction::call(source, signalIndex, args);
    }
    return false;
}

int SignalManager::qt_metacall(QObject *object, QMetaObject::Call call, int id, void **args)
{
    const QMetaObject *metaObject = object->metaObject();
    PySideProperty *pp = nullptr;
    PyObject *pp_name = nullptr;
    QMetaProperty mp;
    PyObject *pySelf = nullptr;
    int methodCount = metaObject->methodCount();
    int propertyCount = metaObject->propertyCount();

    if (call != QMetaObject::InvokeMetaMethod) {
        mp = metaObject->property(id);
        if (!mp.isValid()) {
            return id - methodCount;
        }

        Shiboken::GilState gil;
        pySelf = reinterpret_cast<PyObject *>(Shiboken::BindingManager::instance().retrieveWrapper(object));
        Q_ASSERT(pySelf);
        pp_name = Shiboken::String::fromCString(mp.name());
        pp = Property::getObject(pySelf, pp_name);
        if (!pp) {
            qWarning("Invalid property: %s.", mp.name());
            Py_XDECREF(pp_name);
            return id - methodCount;
        }
    }

    switch(call) {
#ifndef QT_NO_PROPERTIES
        case QMetaObject::ReadProperty:
        case QMetaObject::WriteProperty:
        case QMetaObject::ResetProperty:
        case QMetaObject::QueryPropertyDesignable:
        case QMetaObject::QueryPropertyScriptable:
        case QMetaObject::QueryPropertyStored:
        case QMetaObject::QueryPropertyEditable:
        case QMetaObject::QueryPropertyUser:
            pp->d->metaCallHandler(pp, pySelf, call, args);
            break;
#endif
        case QMetaObject::InvokeMetaMethod:
            id = callMethod(object, id, args);
            break;

        default:
            qWarning("Unsupported meta invocation type.");
    }

    // WARNING Isn't safe to call any metaObject and/or object methods beyond this point
    //         because the object can be deleted inside the called slot.

    if (call == QMetaObject::InvokeMetaMethod) {
        id = id - methodCount;
    } else {
        id = id - propertyCount;
    }

    if (pp || pp_name) {
        Shiboken::GilState gil;
        Py_XDECREF(pp);
        Py_XDECREF(pp_name);
    }

    // Bubbles Python exceptions up to the Javascript engine, if called from one
    {
        Shiboken::GilState gil;

        if (PyErr_Occurred()) {

#if PYSIDE_QML_PRIVATE_API_SUPPORT
            // This JS engine grabber based off of Qt 5.5's `qjsEngine` function
            QQmlData *data = QQmlData::get(object, false);

            if (data && !data->jsWrapper.isNullOrUndefined()) {
                QV4::ExecutionEngine *engine = data->jsWrapper.engine();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                if (engine->currentStackFrame != nullptr) {
#elif QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
                if (engine->currentContext->d() != engine->rootContext()->d()) {
#else
                QV4::ExecutionContext *ctx = engine->currentContext();
                if (ctx->type == QV4::Heap::ExecutionContext::Type_CallContext ||
                    ctx->type == QV4::Heap::ExecutionContext::Type_SimpleCallContext) {
#endif
                    PyObject *errType, *errValue, *errTraceback;
                    PyErr_Fetch(&errType, &errValue, &errTraceback);
                    // PYSIDE-464: The error is only valid before PyErr_Restore,
                    // PYSIDE-464: therefore we take local copies.
                    Shiboken::AutoDecRef objStr(PyObject_Str(errValue));
                    const QString errString = QLatin1String(Shiboken::String::toCString(objStr));
                    const bool isSyntaxError = errType == PyExc_SyntaxError;
                    const bool isTypeError = errType == PyExc_TypeError;
                    PyErr_Restore(errType, errValue, errTraceback);

                    PyErr_Print();    // Note: PyErr_Print clears the error.

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
                    if (isSyntaxError) {
                        return engine->throwSyntaxError(errString);
                    } else if (isTypeError) {
                        return engine->throwTypeError(errString);
                    } else {
                        return engine->throwError(errString);
                    }
#else
                    if (isSyntaxError) {
                        return ctx->throwSyntaxError(errString);
                    } else if (isTypeError) {
                        return ctx->throwTypeError(errString);
                    } else {
                        return ctx->throwError(errString);
                    }
#endif
                }
            }
#endif

            int reclimit = Py_GetRecursionLimit();
            // Inspired by Python's errors.c: PyErr_GivenExceptionMatches() function.
            // Temporarily bump the recursion limit, so that PyErr_Print will not raise a recursion
            // error again. Don't do it when the limit is already insanely high, to avoid overflow.
            if (reclimit < (1 << 30))
                Py_SetRecursionLimit(reclimit + 5);
            PyErr_Print();
            Py_SetRecursionLimit(reclimit);
        }
    }

    return id;
}

int SignalManager::callPythonMetaMethod(const QMetaMethod &method, void **args, PyObject *pyMethod, bool isShortCuit)
{
    Q_ASSERT(pyMethod);

    Shiboken::GilState gil;
    PyObject *pyArguments = nullptr;

    if (isShortCuit){
        pyArguments = reinterpret_cast<PyObject *>(args[1]);
    } else {
        pyArguments = parseArguments(method.parameterTypes(), args);
    }

    if (pyArguments) {
        Shiboken::Conversions::SpecificConverter *retConverter = nullptr;
        const char *returnType = method.typeName();
        if (returnType && std::strcmp("", returnType) && std::strcmp("void", returnType)) {
            retConverter = new Shiboken::Conversions::SpecificConverter(returnType);
            if (!retConverter || !*retConverter) {
                PyErr_Format(PyExc_RuntimeError, "Can't find converter for '%s' to call Python meta method.", returnType);
                return -1;
            }
        }

        Shiboken::AutoDecRef retval(PyObject_CallObject(pyMethod, pyArguments));

        if (!isShortCuit && pyArguments){
            Py_DECREF(pyArguments);
        }

        if (!retval.isNull() && retval != Py_None && !PyErr_Occurred() && retConverter) {
            retConverter->toCpp(retval, args[0]);
        }
        delete retConverter;
    }

    return -1;
}

bool SignalManager::registerMetaMethod(QObject *source, const char *signature, QMetaMethod::MethodType type)
{
    int ret = registerMetaMethodGetIndex(source, signature, type);
    return (ret != -1);
}

static MetaObjectBuilder *metaBuilderFromDict(PyObject *dict)
{
    if (!dict || !PyDict_Contains(dict, metaObjectAttr))
        return nullptr;

    PyObject *pyBuilder = PyDict_GetItem(dict, metaObjectAttr);
#ifdef IS_PY3K
    return reinterpret_cast<MetaObjectBuilder *>(PyCapsule_GetPointer(pyBuilder, nullptr));
#else
    return reinterpret_cast<MetaObjectBuilder *>(PyCObject_AsVoidPtr(pyBuilder));
#endif
}

int SignalManager::registerMetaMethodGetIndex(QObject *source, const char *signature, QMetaMethod::MethodType type)
{
    if (!source) {
        qWarning("SignalManager::registerMetaMethodGetIndex(\"%s\") called with source=nullptr.",
                 signature);
        return -1;
    }
    const QMetaObject *metaObject = source->metaObject();
    int methodIndex = metaObject->indexOfMethod(signature);
    // Create the dynamic signal is needed
    if (methodIndex == -1) {
        SbkObject *self = Shiboken::BindingManager::instance().retrieveWrapper(source);
        if (!Shiboken::Object::hasCppWrapper(self)) {
            qWarning() << "Invalid Signal signature:" << signature;
            return -1;
        } else {
            auto pySelf = reinterpret_cast<PyObject *>(self);
            PyObject *dict = self->ob_dict;
            MetaObjectBuilder *dmo = metaBuilderFromDict(dict);

            // Create a instance meta object
            if (!dmo) {
                dmo = new MetaObjectBuilder(Py_TYPE(pySelf), metaObject);
#ifdef IS_PY3K
                PyObject *pyDmo = PyCapsule_New(dmo, 0, destroyMetaObject);
#else
                PyObject *pyDmo = PyCObject_FromVoidPtr(dmo, destroyMetaObject);
#endif

                PyObject_SetAttr(pySelf, metaObjectAttr, pyDmo);
                Py_DECREF(pyDmo);
            }

            if (type == QMetaMethod::Signal)
                return dmo->addSignal(signature);
            else
                return dmo->addSlot(signature);
        }
    }
    return methodIndex;
}

const QMetaObject *SignalManager::retrieveMetaObject(PyObject *self)
{
    Shiboken::GilState gil;
    Q_ASSERT(self);

    MetaObjectBuilder *builder = metaBuilderFromDict(reinterpret_cast<SbkObject *>(self)->ob_dict);
    if (!builder)
        builder = &(retrieveTypeUserData(self)->mo);

    return builder->update();
}

namespace {

static int callMethod(QObject *object, int id, void **args)
{
    const QMetaObject *metaObject = object->metaObject();
    QMetaMethod method = metaObject->method(id);

    if (method.methodType() == QMetaMethod::Signal) {
        // emit python signal
        QMetaObject::activate(object, id, args);
    } else {
        Shiboken::GilState gil;
        auto self = reinterpret_cast<PyObject *>(Shiboken::BindingManager::instance().retrieveWrapper(object));
        QByteArray methodName = method.methodSignature();
        methodName.truncate(methodName.indexOf('('));
        Shiboken::AutoDecRef pyMethod(PyObject_GetAttrString(self, methodName));
        return SignalManager::callPythonMetaMethod(method, args, pyMethod, false);
    }
    return -1;
}


static PyObject *parseArguments(const QList<QByteArray>& paramTypes, void **args)
{
    int argsSize = paramTypes.count();
    PyObject *preparedArgs = PyTuple_New(argsSize);

    for (int i = 0, max = argsSize; i < max; ++i) {
        void *data = args[i+1];
        const char *dataType = paramTypes[i].constData();
        Shiboken::Conversions::SpecificConverter converter(dataType);
        if (converter) {
            PyTuple_SET_ITEM(preparedArgs, i, converter.toPython(data));
        } else {
            PyErr_Format(PyExc_TypeError, "Can't call meta function because I have no idea how to handle %s", dataType);
            Py_DECREF(preparedArgs);
            return 0;
        }
    }
    return preparedArgs;
}

static bool emitShortCircuitSignal(QObject *source, int signalIndex, PyObject *args)
{
    void *signalArgs[2] = {nullptr, args};
    source->qt_metacall(QMetaObject::InvokeMetaMethod, signalIndex, signalArgs);
    return true;
}

} //namespace
