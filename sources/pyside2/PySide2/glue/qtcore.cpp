/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

/*********************************************************************
 * INJECT CODE
 ********************************************************************/

// @snippet include-pyside
#include <pyside.h>
#include <limits>
// @snippet include-pyside

// @snippet pystring-check
bool py2kStrCheck(PyObject *obj)
{
#ifdef IS_PY3K
    return false;
#else
    return PyString_Check(obj);
#endif
}
// @snippet pystring-check

// @snippet qsettings-value
// If we enter the kwds, means that we have a defaultValue or
// at least a type.
// This avoids that we are passing '0' as defaultValue.
// defaultValue can also be passed as positional argument,
// not only as keyword.
QVariant out;
if (kwds || numArgs > 1) {
    Py_BEGIN_ALLOW_THREADS
    out = %CPPSELF.value(%1, %2);
    Py_END_ALLOW_THREADS
} else {
    Py_BEGIN_ALLOW_THREADS
    out = %CPPSELF.value(%1);
    Py_END_ALLOW_THREADS
}

PyTypeObject *typeObj = reinterpret_cast<PyTypeObject*>(%PYARG_3);

if (typeObj) {
    if (typeObj == &PyList_Type) {
        QByteArray out_ba = out.toByteArray();
        if (!out_ba.isEmpty()) {
            QByteArrayList valuesList = out_ba.split(',');
            const int valuesSize = valuesList.size();
            if (valuesSize > 0) {
                PyObject *list = PyList_New(valuesSize);
                for (int i = 0; i < valuesSize; i++) {
                    PyObject *item = PyUnicode_FromString(valuesList[i].data());
                    PyList_SET_ITEM(list, i, item);
                }
                %PYARG_0 = list;

            } else {
                %PYARG_0 = %CONVERTTOPYTHON[QVariant](out);
            }
        } else {
            %PYARG_0 = PyList_New(0);
        }
    } else if (typeObj == &PyBytes_Type) {
        QByteArray asByteArray = out.toByteArray();
        %PYARG_0 = PyBytes_FromString(asByteArray.data());
    } else if (typeObj == &PyUnicode_Type) {
        QByteArray asByteArray = out.toByteArray();
        %PYARG_0 = PyUnicode_FromString(asByteArray.data());
#ifdef IS_PY3K
    } else if (typeObj == &PyLong_Type) {
        float asFloat = out.toFloat();
        pyResult = PyLong_FromDouble(asFloat);
#else
    } else if (typeObj == &PyInt_Type) {
        float asFloat = out.toFloat();
        pyResult = PyInt_FromLong(long(asFloat));
#endif
    } else if (typeObj == &PyFloat_Type) {
        float asFloat = out.toFloat();
        %PYARG_0 = PyFloat_FromDouble(asFloat);
    } else if (typeObj == &PyBool_Type) {
        if (out.toBool()) {
            Py_INCREF(Py_True);
            %PYARG_0 = Py_True;
        } else {
            Py_INCREF(Py_False);
            %PYARG_0 = Py_False;
        }
    }
    // TODO: PyDict_Type and PyTuple_Type
}
else {
    if (!out.isValid()) {
        Py_INCREF(Py_None);
        %PYARG_0 = Py_None;
    } else {
        %PYARG_0 = %CONVERTTOPYTHON[QVariant](out);
    }
}

// @snippet qsettings-value

// @snippet qvariant-conversion
static const char *QVariant_resolveMetaType(PyTypeObject *type, int *typeId)
{
    if (PyObject_TypeCheck(type, SbkObjectType_TypeF())) {
        auto sbkType = reinterpret_cast<SbkObjectType *>(type);
        const char *typeName = Shiboken::ObjectType::getOriginalName(sbkType);
        if (!typeName)
            return nullptr;
        const bool valueType = '*' != typeName[qstrlen(typeName) - 1];
        // Do not convert user type of value
        if (valueType && Shiboken::ObjectType::isUserType(type))
            return nullptr;
        int obTypeId = QMetaType::type(typeName);
        if (obTypeId) {
            *typeId = obTypeId;
            return typeName;
        }
        // Do not resolve types to value type
        if (valueType)
            return nullptr;
        // Find in base types. First check tp_bases, and only after check tp_base, because
        // tp_base does not always point to the first base class, but rather to the first
        // that has added any python fields or slots to its object layout.
        // See https://mail.python.org/pipermail/python-list/2009-January/520733.html
        if (type->tp_bases) {
            for (int i = 0, size = PyTuple_GET_SIZE(type->tp_bases); i < size; ++i) {
                const char *derivedName = QVariant_resolveMetaType(reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(
                                          type->tp_bases, i)), typeId);
                if (derivedName)
                    return derivedName;
            }
        }
        else if (type->tp_base) {
            return QVariant_resolveMetaType(type->tp_base, typeId);
        }
    }
    *typeId = 0;
    return nullptr;
}
static QVariant QVariant_convertToValueList(PyObject *list)
{
    if (PySequence_Size(list) < 0) {
         // clear the error if < 0 which means no length at all
         PyErr_Clear();
         return QVariant();
    }

    Shiboken::AutoDecRef element(PySequence_GetItem(list, 0));
    int typeId;
    const char *typeName = QVariant_resolveMetaType(element.cast<PyTypeObject *>(), &typeId);
    if (typeName) {
        QByteArray listTypeName("QList<");
        listTypeName += typeName;
        listTypeName += '>';
        typeId = QMetaType::type(listTypeName);
        if (typeId > 0) {
            Shiboken::Conversions::SpecificConverter converter(listTypeName);
            if (converter) {
                QVariant var(typeId, nullptr);
                converter.toCpp(list, &var);
                return var;
            }
            qWarning() << "Type converter for :" << listTypeName << "not registered.";
        }
    }
    return QVariant();
}
static bool QVariant_isStringList(PyObject *list)
{
    if (!PySequence_Check(list)) {
        // If it is not a list or a derived list class
        // we assume that will not be a String list neither.
        return false;
    }

    if (PySequence_Size(list) < 0) {
        // clear the error if < 0 which means no length at all
        PyErr_Clear();
        return false;
    }

    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PySequence_Fast_GET_ITEM(fast.object(), i);
        if (!%CHECKTYPE[QString](item))
            return false;
    }
    return true;
}
static QVariant QVariant_convertToVariantMap(PyObject *map)
{
    Py_ssize_t pos = 0;
    Shiboken::AutoDecRef keys(PyDict_Keys(map));
    if (!QVariant_isStringList(keys))
        return QVariant();
    PyObject *key;
    PyObject *value;
    QMap<QString,QVariant> ret;
    while (PyDict_Next(map, &pos, &key, &value)) {
        QString cppKey = %CONVERTTOCPP[QString](key);
        QVariant cppValue = %CONVERTTOCPP[QVariant](value);
        ret.insert(cppKey, cppValue);
    }
    return QVariant(ret);
}
static QVariant QVariant_convertToVariantList(PyObject *list)
{
    if (QVariant_isStringList(list)) {
        QList<QString > lst = %CONVERTTOCPP[QList<QString>](list);
        return QVariant(QStringList(lst));
    }
    QVariant valueList = QVariant_convertToValueList(list);
    if (valueList.isValid())
        return valueList;

    if (PySequence_Size(list) < 0) {
        // clear the error if < 0 which means no length at all
        PyErr_Clear();
        return QVariant();
    }

    QList<QVariant> lst;
    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *pyItem = PySequence_Fast_GET_ITEM(fast.object(), i);
        QVariant item = %CONVERTTOCPP[QVariant](pyItem);
        lst.append(item);
    }
    return QVariant(lst);
}
// @snippet qvariant-conversion

// @snippet qvariantmap-check
static bool QVariantType_isStringList(PyObject *list)
{
    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i=0; i < size; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(fast.object(), i);
        if (!%CHECKTYPE[QString](item))
            return false;
    }
    return true;
}
static bool QVariantType_checkAllStringKeys(PyObject *dict)
{
    Shiboken::AutoDecRef keys(PyDict_Keys(dict));
    return QVariantType_isStringList(keys);
}
// @snippet qvariantmap-check

// @snippet qt-qabs
double _abs = qAbs(%1);
%PYARG_0 = %CONVERTTOPYTHON[double](_abs);
// @snippet qt-qabs

// @snippet qt-postroutine
namespace PySide {
static QStack<PyObject *> globalPostRoutineFunctions;
void globalPostRoutineCallback()
{
    Shiboken::GilState state;
    for (auto *callback : globalPostRoutineFunctions) {
        Shiboken::AutoDecRef result(PyObject_CallObject(callback, nullptr));
        Py_DECREF(callback);
    }
    globalPostRoutineFunctions.clear();
}
void addPostRoutine(PyObject *callback)
{
    if (PyCallable_Check(callback)) {
        globalPostRoutineFunctions << callback;
        Py_INCREF(callback);
    } else {
        PyErr_SetString(PyExc_TypeError, "qAddPostRoutine: The argument must be a callable object.");
    }
}
} // namespace
// @snippet qt-postroutine

// @snippet qt-addpostroutine
PySide::addPostRoutine(%1);
// @snippet qt-addpostroutine

// @snippet qt-qaddpostroutine
qAddPostRoutine(PySide::globalPostRoutineCallback);
// @snippet qt-qaddpostroutine

// @snippet qt-version
QList<QByteArray> version = QByteArray(qVersion()).split('.');
PyObject *pyQtVersion = PyTuple_New(3);
for (int i = 0; i < 3; ++i)
    PyTuple_SET_ITEM(pyQtVersion, i, PyInt_FromLong(version[i].toInt()));
PyModule_AddObject(module, "__version_info__", pyQtVersion);
PyModule_AddStringConstant(module, "__version__", qVersion());
// @snippet qt-version

// @snippet qobject-connect
static bool isDecorator(PyObject *method, PyObject *self)
{
    Shiboken::AutoDecRef methodName(PyObject_GetAttr(method, Shiboken::PyMagicName::name()));
    if (!PyObject_HasAttr(self, methodName))
        return true;
    Shiboken::AutoDecRef otherMethod(PyObject_GetAttr(self, methodName));
    return PyMethod_GET_FUNCTION(otherMethod.object()) != PyMethod_GET_FUNCTION(method);
}

static bool getReceiver(QObject *source, const char *signal, PyObject *callback, QObject **receiver, PyObject **self, QByteArray *callbackSig)
{
    bool forceGlobalReceiver = false;
    if (PyMethod_Check(callback)) {
        *self = PyMethod_GET_SELF(callback);
        if (%CHECKTYPE[QObject *](*self))
            *receiver = %CONVERTTOCPP[QObject *](*self);
        forceGlobalReceiver = isDecorator(callback, *self);
    } else if (PyCFunction_Check(callback)) {
        *self = PyCFunction_GET_SELF(callback);
        if (*self && %CHECKTYPE[QObject *](*self))
            *receiver = %CONVERTTOCPP[QObject *](*self);
    } else if (PyCallable_Check(callback)) {
        // Ok, just a callable object
        *receiver = nullptr;
        *self = nullptr;
    }

    bool usingGlobalReceiver = !*receiver || forceGlobalReceiver;

    // Check if this callback is a overwrite of a non-virtual Qt slot.
    if (!usingGlobalReceiver && receiver && self) {
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toLatin1();
        const QMetaObject *metaObject = (*receiver)->metaObject();
        int slotIndex = metaObject->indexOfSlot(callbackSig->constData());
        if (slotIndex != -1 && slotIndex < metaObject->methodOffset() && PyMethod_Check(callback))
            usingGlobalReceiver = true;
    }

    if (usingGlobalReceiver) {
        PySide::SignalManager &signalManager = PySide::SignalManager::instance();
        *receiver = signalManager.globalReceiver(source, callback);
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toLatin1();
    }

    return usingGlobalReceiver;
}

static bool qobjectConnect(QObject *source, const char *signal, QObject *receiver, const char *slot, Qt::ConnectionType type)
{
    if (!signal || !slot)
        return false;

    if (!PySide::Signal::checkQtSignal(signal))
        return false;
    signal++;

    if (!PySide::SignalManager::registerMetaMethod(source, signal, QMetaMethod::Signal))
        return false;

    bool isSignal = PySide::Signal::isQtSignal(slot);
    slot++;
    PySide::SignalManager::registerMetaMethod(receiver, slot, isSignal ? QMetaMethod::Signal : QMetaMethod::Slot);
    bool connection;
    connection = QObject::connect(source, signal - 1, receiver, slot - 1, type);
    return connection;
}

static bool qobjectConnect(QObject *source, QMetaMethod signal, QObject *receiver, QMetaMethod slot, Qt::ConnectionType type)
{
   return qobjectConnect(source, signal.methodSignature(), receiver, slot.methodSignature(), type);
}

static bool qobjectConnectCallback(QObject *source, const char *signal, PyObject *callback, Qt::ConnectionType type)
{
    if (!signal || !PySide::Signal::checkQtSignal(signal))
        return false;
    signal++;

    int signalIndex = PySide::SignalManager::registerMetaMethodGetIndex(source, signal, QMetaMethod::Signal);
    if (signalIndex == -1)
        return false;

    PySide::SignalManager &signalManager = PySide::SignalManager::instance();

    // Extract receiver from callback
    QObject *receiver = nullptr;
    PyObject *self = nullptr;
    QByteArray callbackSig;
    bool usingGlobalReceiver = getReceiver(source, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == nullptr && self == nullptr)
        return false;

    const QMetaObject *metaObject = receiver->metaObject();
    const char *slot = callbackSig.constData();
    int slotIndex = metaObject->indexOfSlot(slot);
    QMetaMethod signalMethod = metaObject->method(signalIndex);

    if (slotIndex == -1) {
        if (!usingGlobalReceiver && self && !Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject *>(self))) {
            qWarning("You can't add dynamic slots on an object originated from C++.");
            if (usingGlobalReceiver)
                signalManager.releaseGlobalReceiver(source, receiver);

            return false;
        }

        if (usingGlobalReceiver)
            slotIndex = signalManager.globalReceiverSlotIndex(receiver, slot);
        else
            slotIndex = PySide::SignalManager::registerMetaMethodGetIndex(receiver, slot, QMetaMethod::Slot);

        if (slotIndex == -1) {
            if (usingGlobalReceiver)
                signalManager.releaseGlobalReceiver(source, receiver);

            return false;
        }
    }
    bool connection;
    connection = QMetaObject::connect(source, signalIndex, receiver, slotIndex, type);
    if (connection) {
        if (usingGlobalReceiver)
            signalManager.notifyGlobalReceiver(receiver);
        #ifndef AVOID_PROTECTED_HACK
            source->connectNotify(signalMethod); //Qt5: QMetaMethod instead of char *
        #else
            // Need to cast to QObjectWrapper * and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper *>(source)->connectNotify(signalMethod); //Qt5: QMetaMethod instead of char *
        #endif

        return connection;
    }

    if (usingGlobalReceiver)
        signalManager.releaseGlobalReceiver(source, receiver);

    return false;
}


static bool qobjectDisconnectCallback(QObject *source, const char *signal, PyObject *callback)
{
    if (!PySide::Signal::checkQtSignal(signal))
        return false;

    PySide::SignalManager &signalManager = PySide::SignalManager::instance();

    // Extract receiver from callback
    QObject *receiver = nullptr;
    PyObject *self = nullptr;
    QByteArray callbackSig;
    QMetaMethod slotMethod;
    bool usingGlobalReceiver = getReceiver(nullptr, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == nullptr && self == nullptr)
        return false;

    const QMetaObject *metaObject = receiver->metaObject();
    int signalIndex = source->metaObject()->indexOfSignal(++signal);
    int slotIndex = -1;

    slotIndex = metaObject->indexOfSlot(callbackSig);
    slotMethod = metaObject->method(slotIndex);

    bool disconnected;
    disconnected = QMetaObject::disconnectOne(source, signalIndex, receiver, slotIndex);

    if (disconnected) {
        if (usingGlobalReceiver)
            signalManager.releaseGlobalReceiver(source, receiver);

        #ifndef AVOID_PROTECTED_HACK
            source->disconnectNotify(slotMethod); //Qt5: QMetaMethod instead of char *
        #else
            // Need to cast to QObjectWrapper * and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper *>(source)->disconnectNotify(slotMethod); //Qt5: QMetaMethod instead of char *
        #endif
        return true;
    }
    return false;
}
// @snippet qobject-connect

// @snippet qobject-connect-1
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %CPPSELF, %3, %4);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-1

// @snippet qobject-connect-2
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %3, %4, %5);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-2

// @snippet qobject-connect-3
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %3, %4, %5);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-3

// @snippet qobject-connect-4
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnectCallback(%1, %2, %PYARG_3, %4);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-4

// @snippet qobject-connect-5
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnectCallback(%CPPSELF, %1, %PYARG_2, %3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-5

// @snippet qobject-connect-6
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnect(%CPPSELF, %1, %2, %3, %4);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-6

// @snippet qobject-emit
%RETURN_TYPE %0 = PySide::SignalManager::instance().emitSignal(%CPPSELF, %1, %PYARG_2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-emit

// @snippet qobject-disconnect-1
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectDisconnectCallback(%CPPSELF, %1, %2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-disconnect-1

// @snippet qobject-disconnect-2
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectDisconnectCallback(%1, %2, %3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-disconnect-2

// @snippet qfatal
// qFatal doesn't have a stream version, so we do a
// qWarning call followed by a qFatal() call using a
// literal.
Py_BEGIN_ALLOW_THREADS
qWarning() << %1;
qFatal("[A qFatal() call was made from Python code]");
Py_END_ALLOW_THREADS
// @snippet qfatal

// @snippet moduleshutdown
PySide::runCleanupFunctions();
// @snippet moduleshutdown

// @snippet qt-pysideinit
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QSTRING_IDX], "unicode");
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QSTRING_IDX], "str");
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QTCORE_QLIST_QVARIANT_IDX], "QVariantList");
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QTCORE_QMAP_QSTRING_QVARIANT_IDX], "QVariantMap");

PySide::registerInternalQtConf();
PySide::init(module);
Py_AtExit(QtCoreModuleExit);
// @snippet qt-pysideinit

// @snippet qt-messagehandler
// Define a global variable to handle qInstallMessageHandler callback
static PyObject *qtmsghandler = nullptr;

static void msgHandlerCallback(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(3));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QtMsgType](type));
    PyTuple_SET_ITEM(arglist, 1, %CONVERTTOPYTHON[QMessageLogContext &](ctx));
    QByteArray array = msg.toUtf8();  // Python handler requires UTF-8
    char *data = array.data();
    PyTuple_SET_ITEM(arglist, 2, %CONVERTTOPYTHON[char *](data));
    Shiboken::AutoDecRef ret(PyObject_CallObject(qtmsghandler, arglist));
}
static void QtCoreModuleExit()
{
    PySide::SignalManager::instance().clear();
}
// @snippet qt-messagehandler

// @snippet qt-installmessagehandler
if (%PYARG_1 == Py_None) {
  qInstallMessageHandler(0);
  %PYARG_0 = qtmsghandler ? qtmsghandler : Py_None;
  qtmsghandler = 0;
} else if (!PyCallable_Check(%PYARG_1)) {
  PyErr_SetString(PyExc_TypeError, "parameter must be callable");
} else {
  %PYARG_0 = qtmsghandler ? qtmsghandler : Py_None;
  Py_INCREF(%PYARG_1);
  qtmsghandler = %PYARG_1;
  qInstallMessageHandler(msgHandlerCallback);
}

if (%PYARG_0 == Py_None)
    Py_INCREF(%PYARG_0);
// @snippet qt-installmessagehandler

// @snippet qline-hash
namespace PySide {
    template<> inline uint hash(const QLine &v) {
        return qHash(qMakePair(qMakePair(v.x1(), v.y1()), qMakePair(v.x2(), v.y2())));
    }
};
// @snippet qline-hash

// @snippet qlinef-intersect
QPointF p;
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%ARGUMENT_NAMES, &p);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QPointF](p));
// @snippet qlinef-intersect

// @snippet qresource-data
const void *d = %CPPSELF.%FUNCTION_NAME();
if (d) {
    %PYARG_0 = Shiboken::Buffer::newObject(d, %CPPSELF.size());
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qresource-data

// @snippet qdate-topython
if (!PyDateTimeAPI)
    PySideDateTime_IMPORT;
%PYARG_0 = PyDate_FromDate(%CPPSELF.year(), %CPPSELF.month(), %CPPSELF.day());
// @snippet qdate-topython

// @snippet qdate-getdate
int year, month, day;
%CPPSELF.%FUNCTION_NAME(&year, &month, &day);
%PYARG_0 = PyTuple_New(3);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](year));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[int](month));
PyTuple_SET_ITEM(%PYARG_0, 2, %CONVERTTOPYTHON[int](day));
// @snippet qdate-getdate

// @snippet qdate-weeknumber
int yearNumber;
int week = %CPPSELF.%FUNCTION_NAME(&yearNumber);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](week));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[int](yearNumber));
// @snippet qdate-weeknumber

// @snippet qdatetime-1
QDate date(%1, %2, %3);
QTime time(%4, %5, %6, %7);
%0 = new %TYPE(date, time, Qt::TimeSpec(%8));
// @snippet qdatetime-1

// @snippet qdatetime-2
QDate date(%1, %2, %3);
QTime time(%4, %5, %6);
%0 = new %TYPE(date, time);
// @snippet qdatetime-2

// @snippet qdatetime-topython
QDate date = %CPPSELF.date();
QTime time = %CPPSELF.time();
if (!PyDateTimeAPI) PySideDateTime_IMPORT;
%PYARG_0 = PyDateTime_FromDateAndTime(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec()*1000);
// @snippet qdatetime-topython

// @snippet qpoint
namespace PySide {
    template<> inline uint hash(const QPoint &v) {
        return qHash(qMakePair(v.x(), v.y()));
    }
};
// @snippet qpoint

// @snippet qrect
namespace PySide {
    template<> inline uint hash(const QRect &v) {
        return qHash(qMakePair(qMakePair(v.x(), v.y()), qMakePair(v.width(), v.height())));
    }
};
// @snippet qrect

// @snippet qsize
namespace PySide {
    template<> inline uint hash(const QSize &v) {
        return qHash(qMakePair(v.width(), v.height()));
    }
};
// @snippet qsize

// @snippet qtime-topython
if (!PyDateTimeAPI)
    PySideDateTime_IMPORT;
%PYARG_0 = PyTime_FromTime(%CPPSELF.hour(), %CPPSELF.minute(), %CPPSELF.second(), %CPPSELF.msec()*1000);
// @snippet qtime-topython

// @snippet qbitarray-len
return %CPPSELF.size();
// @snippet qbitarray-len

// @snippet qbitarray-getitem
if (_i < 0 || _i >= %CPPSELF.size()) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
}
bool ret = %CPPSELF.at(_i);
return %CONVERTTOPYTHON[bool](ret);
// @snippet qbitarray-getitem

// @snippet qbitarray-setitem
PyObject *args = Py_BuildValue("(iiO)", _i, 1, _value);
PyObject *result = Sbk_QBitArrayFunc_setBit(self, args);
Py_DECREF(args);
Py_XDECREF(result);
return !result ? -1 : 0;
// @snippet qbitarray-setitem

// @snippet unlock
%CPPSELF.unlock();
// @snippet unlock

// @snippet qabstractitemmodel-createindex
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(%1, %2, %PYARG_3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qabstractitemmodel-createindex

// @snippet qabstractitemmodel
qRegisterMetaType<QVector<int> >("QVector<int>");
// @snippet qabstractitemmodel

// @snippet qobject-metaobject
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME();
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-metaobject

// @snippet qobject-findchild-1
static QObject *_findChildHelper(const QObject *parent, const QString &name, PyTypeObject *desiredType)
{
    for (auto *child : parent->children()) {
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject *](child));
        if (PyType_IsSubtype(Py_TYPE(pyChild), desiredType)
            && (name.isNull() || name == child->objectName())) {
            return child;
        }
    }

    for (auto *child : parent->children()) {
        QObject *obj = _findChildHelper(child, name, desiredType);
        if (obj)
            return obj;
    }
    return nullptr;
}

static inline bool _findChildrenComparator(const QObject *&child, const QRegExp &name)
{
    return name.indexIn(child->objectName()) != -1;
}

static inline bool _findChildrenComparator(const QObject *&child, const QString &name)
{
    return name.isNull() || name == child->objectName();
}

template<typename T>
static void _findChildrenHelper(const QObject *parent, const T& name, PyTypeObject *desiredType, PyObject *result)
{
    for (const auto *child : parent->children()) {
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject *](child));
        if (PyType_IsSubtype(Py_TYPE(pyChild), desiredType) && _findChildrenComparator(child, name))
            PyList_Append(result, pyChild);
        _findChildrenHelper(child, name, desiredType, result);
    }
}
// @snippet qobject-findchild-1

// @snippet qobject-findchild-2
QObject *child = _findChildHelper(%CPPSELF, %2, reinterpret_cast<PyTypeObject *>(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[QObject *](child);
// @snippet qobject-findchild-2

// @snippet qobject-findchildren-1
%PYARG_0 = PyList_New(0);
_findChildrenHelper(%CPPSELF, %2, reinterpret_cast<PyTypeObject *>(%PYARG_1), %PYARG_0);
// @snippet qobject-findchildren-1

// @snippet qobject-findchildren-2
%PYARG_0 = PyList_New(0);
_findChildrenHelper(%CPPSELF, %2, reinterpret_cast<PyTypeObject *>(%PYARG_1), %PYARG_0);
// @snippet qobject-findchildren-2

// @snippet qobject-tr
QString result;
if (QCoreApplication::instance()) {
    PyObject *klass = PyObject_GetAttr(%PYSELF, Shiboken::PyMagicName::class_());
    PyObject *cname = PyObject_GetAttr(klass, Shiboken::PyMagicName::name());
    result = QString(QCoreApplication::instance()->translate(Shiboken::String::toCString(cname),
                                                        /*   %1, %2, QCoreApplication::CodecForTr, %3)); */
                                                             %1, %2, %3));

    Py_DECREF(klass);
    Py_DECREF(cname);
} else {
    result = QString(QString::fromLatin1(%1));
}
%PYARG_0 = %CONVERTTOPYTHON[QString](result);
// @snippet qobject-tr

// @snippet qobject-receivers
// Avoid return +1 because SignalManager connect to "destroyed()" signal to control object timelife
int ret = %CPPSELF.%FUNCTION_NAME(%1);
if (ret > 0 && ((strcmp(%1, SIGNAL(destroyed())) == 0) || (strcmp(%1, SIGNAL(destroyed(QObject*))) == 0)))
    ret -= PySide::SignalManager::instance().countConnectionsWith(%CPPSELF);

%PYARG_0 = %CONVERTTOPYTHON[int](ret);
// @snippet qobject-receivers

// @snippet qregexp-replace
%1.replace(*%CPPSELF, %2);
%PYARG_0 = %CONVERTTOPYTHON[QString](%1);
// @snippet qregexp-replace

// @snippet qbytearray-mgetitem
if (PyIndex_Check(_key)) {
    Py_ssize_t _i;
    _i = PyNumber_AsSsize_t(_key, PyExc_IndexError);
    if (_i < 0 || _i >= %CPPSELF.size()) {
        PyErr_SetString(PyExc_IndexError, "index out of bounds");
        return 0;
    } else {
        char res[2];
        res[0] = %CPPSELF.at(_i);
        res[1] = 0;
        return PyBytes_FromStringAndSize(res, 1);
    }
} else if (PySlice_Check(_key)) {
    Py_ssize_t start, stop, step, slicelength, cur;

#ifdef IS_PY3K
    PyObject *key = _key;
#else
    PySliceObject *key = reinterpret_cast<PySliceObject *>(_key);
#endif
    if (PySlice_GetIndicesEx(key, %CPPSELF.count(), &start, &stop, &step, &slicelength) < 0) {
        return nullptr;
    }

    QByteArray ba;
    if (slicelength <= 0) {
        return %CONVERTTOPYTHON[QByteArray](ba);
    } else if (step == 1) {
        Py_ssize_t max = %CPPSELF.count();
        start = qBound(Py_ssize_t(0), start, max);
        stop = qBound(Py_ssize_t(0), stop, max);
        QByteArray ba;
        if (start < stop)
            ba = %CPPSELF.mid(start, stop - start);
        return %CONVERTTOPYTHON[QByteArray](ba);
    } else {
        QByteArray ba;
        for (cur = start; slicelength > 0; cur += static_cast<size_t>(step), slicelength--) {
            ba.append(%CPPSELF.at(cur));
        }
        return %CONVERTTOPYTHON[QByteArray](ba);
    }
} else {
    PyErr_Format(PyExc_TypeError,
                 "list indices must be integers or slices, not %.200s",
                 Py_TYPE(_key)->tp_name);
    return nullptr;
}
// @snippet qbytearray-mgetitem

// @snippet qbytearray-msetitem
if (PyIndex_Check(_key)) {
    Py_ssize_t _i = PyNumber_AsSsize_t(_key, PyExc_IndexError);
    if (_i == -1 && PyErr_Occurred())
        return -1;

    if (_i < 0)
        _i += %CPPSELF.count();

    if (_i < 0 || _i >= %CPPSELF.size()) {
        PyErr_SetString(PyExc_IndexError, "QByteArray index out of range");
        return -1;
    }

    // Provide more specific error message for bytes/str, bytearray, QByteArray respectively
#ifdef IS_PY3K
    if (PyBytes_Check(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "bytes must be of size 1");
#else
    if (PyString_CheckExact(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "str must be of size 1");
#endif
            return -1;
        }
    } else if (PyByteArray_Check(_value)) {
        if (Py_SIZE(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "bytearray must be of size 1");
            return -1;
        }
    } else if (reinterpret_cast<PyTypeObject *>(Py_TYPE(_value)) == reinterpret_cast<PyTypeObject *>(SbkPySide2_QtCoreTypes[SBK_QBYTEARRAY_IDX])) {
        if (PyObject_Length(_value) != 1) {
            PyErr_SetString(PyExc_ValueError, "QByteArray must be of size 1");
            return -1;
        }
    } else {
#ifdef IS_PY3K
        PyErr_SetString(PyExc_ValueError, "a bytes, bytearray, QByteArray of size 1 is required");
#else
        PyErr_SetString(PyExc_ValueError, "a str, bytearray, QByteArray of size 1 is required");
#endif
        return -1;
    }

    // Not support int or long.
    %CPPSELF.remove(_i, 1);
    PyObject *args = Py_BuildValue("(nO)", _i, _value);
    PyObject *result = Sbk_QByteArrayFunc_insert(self, args);
    Py_DECREF(args);
    Py_XDECREF(result);
    return !result ? -1 : 0;
} else if (PySlice_Check(_key)) {
    Py_ssize_t start, stop, step, slicelength, value_length;

#ifdef IS_PY3K
    PyObject *key = _key;
#else
    PySliceObject *key = reinterpret_cast<PySliceObject *>(_key);
#endif
    if (PySlice_GetIndicesEx(key, %CPPSELF.count(), &start, &stop, &step, &slicelength) < 0) {
        return -1;
    }
    // The parameter candidates are: bytes/str, bytearray, QByteArray itself.
    // Not support iterable which contains ints between 0~255

    // case 1: value is nullpre, means delete the items within the range
    // case 2: step is 1, means shrink or expanse
    // case 3: step is not 1, then the number of slots have to equal the number of items in _value
    QByteArray ba;
    if (_value == nullptr || _value == Py_None) {
        ba = QByteArray();
        value_length = 0;
    } else if (!(PyBytes_Check(_value) || PyByteArray_Check(_value) || reinterpret_cast<PyTypeObject *>(Py_TYPE(_value)) == reinterpret_cast<PyTypeObject *>(SbkPySide2_QtCoreTypes[SBK_QBYTEARRAY_IDX]))) {
        PyErr_Format(PyExc_TypeError, "bytes, bytearray or QByteArray is required, not %.200s", Py_TYPE(_value)->tp_name);
        return -1;
    } else {
        value_length = PyObject_Length(_value);
    }

    if (step != 1 && value_length != slicelength) {
        PyErr_Format(PyExc_ValueError, "attempt to assign %s of size %d to extended slice of size %d",
                     Py_TYPE(_value)->tp_name, int(value_length), int(slicelength));
        return -1;
    }

    if (step != 1) {
        int i = start;
        for (int j = 0; j < slicelength; j++) {
            PyObject *item = PyObject_GetItem(_value, PyLong_FromLong(j));
            QByteArray temp;
#ifdef IS_PY3K
            if (PyLong_Check(item)) {
#else
            if (PyLong_Check(item) || PyInt_Check(item)) {
#endif
                int overflow;
                long ival = PyLong_AsLongAndOverflow(item, &overflow);
                // Not suppose to bigger than 255 because only bytes, bytearray, QByteArray were accept
                temp = QByteArray(reinterpret_cast<const char *>(&ival));
            } else {
                temp = %CONVERTTOCPP[QByteArray](item);
            }

            %CPPSELF.replace(i, 1, temp);
            i += step;
        }
        return 0;
    } else {
        ba = %CONVERTTOCPP[QByteArray](_value);
        %CPPSELF.replace(start, slicelength, ba);
        return 0;
    }
} else {
    PyErr_Format(PyExc_TypeError, "QBytearray indices must be integers or slices, not %.200s",
                  Py_TYPE(_key)->tp_name);
    return -1;
}
// @snippet qbytearray-msetitem

// @snippet qbytearray-bufferprotocol
extern "C" {
// QByteArray buffer protocol functions
// see: http://www.python.org/dev/peps/pep-3118/

static int SbkQByteArray_getbufferproc(PyObject *obj, Py_buffer *view, int flags)
{
    if (!view || !Shiboken::Object::isValid(obj))
        return -1;

    QByteArray * cppSelf = %CONVERTTOCPP[QByteArray *](obj);
    //XXX      /|\ omitting this space crashes shiboken!
 #ifdef Py_LIMITED_API
    view->obj = obj;
    view->buf = reinterpret_cast<void *>(cppSelf->data());
    view->len = cppSelf->size();
    view->readonly = 0;
    view->itemsize = 1;
    view->format = const_cast<char *>("c");
    view->ndim = 1;
    view->shape = (flags & PyBUF_ND) == PyBUF_ND ? &(view->len) : nullptr;
    view->strides = &view->itemsize;
    view->suboffsets = NULL;
    view->internal = NULL;

    Py_XINCREF(obj);
    return 0;
#else // Py_LIMITED_API
    const int result = PyBuffer_FillInfo(view, obj, reinterpret_cast<void *>(cppSelf->data()),
                                         cppSelf->size(), 0, flags);
    if (result == 0)
        Py_XINCREF(obj);
    return result;
#endif
}

#if PY_VERSION_HEX < 0x03000000
static Py_ssize_t SbkQByteArray_segcountproc(PyObject *self, Py_ssize_t *lenp)
{
    if (lenp)
        *lenp = Py_TYPE(self)->tp_as_sequence->sq_length(self);
    return 1;
}

static Py_ssize_t SbkQByteArray_readbufferproc(PyObject *self, Py_ssize_t segment, void **ptrptr)
{
    if (segment || !Shiboken::Object::isValid(self))
        return -1;

    QByteArray * cppSelf = %CONVERTTOCPP[QByteArray *](self);
    //XXX      /|\ omitting this space crashes shiboken!
    *ptrptr = reinterpret_cast<void *>(cppSelf->data());
    return cppSelf->size();
}

PyBufferProcs SbkQByteArrayBufferProc = {
    /*bf_getreadbuffer*/  &SbkQByteArray_readbufferproc,
    /*bf_getwritebuffer*/ (writebufferproc) &SbkQByteArray_readbufferproc,
    /*bf_getsegcount*/    &SbkQByteArray_segcountproc,
    /*bf_getcharbuffer*/  (charbufferproc) &SbkQByteArray_readbufferproc,
    /*bf_getbuffer*/  (getbufferproc)SbkQByteArray_getbufferproc,
};
#else

static PyBufferProcs SbkQByteArrayBufferProc = {
    /*bf_getbuffer*/  (getbufferproc)SbkQByteArray_getbufferproc,
    /*bf_releasebuffer*/ (releasebufferproc)0,
};

#endif
}
// @snippet qbytearray-bufferprotocol

// @snippet qbytearray-operatorplus-1
QByteArray ba = QByteArray(PyBytes_AS_STRING(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1)) + *%CPPSELF;
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-1

// @snippet qbytearray-operatorplus-2
QByteArray ba = QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1)) + *%CPPSELF;
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-2

// @snippet qbytearray-operatorplus-3
QByteArray ba = *%CPPSELF + QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-3

// @snippet qbytearray-operatorplusequal
*%CPPSELF += QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
// @snippet qbytearray-operatorplusequal

// @snippet qbytearray-operatorequalequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF == ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorequalequal

// @snippet qbytearray-operatornotequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF != ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatornotequal

// @snippet qbytearray-operatorgreater
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF > ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorgreater

// @snippet qbytearray-operatorgreaterequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF >= ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorgreaterequal

// @snippet qbytearray-operatorlower
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF < ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorlower

// @snippet qbytearray-operatorlowerequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF <= ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorlowerequal

// @snippet qbytearray-repr
PyObject *aux = PyBytes_FromStringAndSize(%CPPSELF.constData(), %CPPSELF.size());
if (aux == nullptr) {
    return nullptr;
}
QByteArray b(Py_TYPE(%PYSELF)->tp_name);
#ifdef IS_PY3K
    %PYARG_0 = PyUnicode_FromFormat("%s(%R)", b.constData(), aux);
#else
    aux = PyObject_Repr(aux);
    b += '(';
    b += QByteArray(PyBytes_AS_STRING(aux), PyBytes_GET_SIZE(aux));
    b += ')';
    %PYARG_0 = Shiboken::String::fromStringAndSize(b.constData(), b.size());
#endif
Py_DECREF(aux);
// @snippet qbytearray-repr

// @snippet qbytearray-1
if (PyBytes_Check(%PYARG_1)) {
    %0 = new QByteArray(PyBytes_AsString(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
} else if (Shiboken::String::check(%PYARG_1)) {
    %0 = new QByteArray(Shiboken::String::toCString(%PYARG_1), Shiboken::String::len(%PYARG_1));
}
// @snippet qbytearray-1

// @snippet qbytearray-2
%0 = new QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
// @snippet qbytearray-2

// @snippet qbytearray-3
%0 = new QByteArray(PyBytes_AS_STRING(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
// @snippet qbytearray-3

// @snippet qbytearray-py3
#if PY_VERSION_HEX < 0x03000000
Shiboken::SbkType<QByteArray>()->tp_as_buffer = &SbkQByteArrayBufferProc;
Shiboken::SbkType<QByteArray>()->tp_flags |= Py_TPFLAGS_HAVE_NEWBUFFER;
#else
PepType_AS_BUFFER(Shiboken::SbkType<QByteArray>()) = &SbkQByteArrayBufferProc;
#endif
// @snippet qbytearray-py3

// @snippet qbytearray-data
%PYARG_0 = PyBytes_FromStringAndSize(%CPPSELF.%FUNCTION_NAME(), %CPPSELF.size());
// @snippet qbytearray-data

// @snippet qbytearray-fromrawdata
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(PyBytes_AsString(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qbytearray-fromrawdata

// @snippet qbytearray-str
PyObject *aux = PyBytes_FromStringAndSize(%CPPSELF.constData(), %CPPSELF.size());
if (aux == nullptr) {
    return nullptr;
}
#ifdef IS_PY3K
    %PYARG_0 = PyObject_Repr(aux);
    Py_DECREF(aux);
#else
    %PYARG_0 = aux;
#endif
// @snippet qbytearray-str

// @snippet qbytearray-len
return %CPPSELF.count();
// @snippet qbytearray-len

// @snippet qbytearray-getitem
if (_i < 0 || _i >= %CPPSELF.size()) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
} else {
    char res[2];
    res[0] = %CPPSELF.at(_i);
    res[1] = 0;
    return PyBytes_FromStringAndSize(res, 1);
}
// @snippet qbytearray-getitem

// @snippet qbytearray-setitem
%CPPSELF.remove(_i, 1);
PyObject *args = Py_BuildValue("(nO)", _i, _value);
PyObject *result = Sbk_QByteArrayFunc_insert(self, args);
Py_DECREF(args);
Py_XDECREF(result);
return !result ? -1 : 0;
// @snippet qbytearray-setitem

// @snippet qfiledevice-unmap
uchar *ptr = reinterpret_cast<uchar *>(Shiboken::Buffer::getPointer(%PYARG_1));
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(ptr);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qfiledevice-unmap

// @snippet qfiledevice-map
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(%1, %2, %3), %2, Shiboken::Buffer::ReadWrite);
// @snippet qfiledevice-map

// @snippet qiodevice-readdata
QByteArray ba(1 + int(%2), char(0));
%CPPSELF.%FUNCTION_NAME(ba.data(), int(%2));
%PYARG_0 = Shiboken::String::fromCString(ba.constData());
// @snippet qiodevice-readdata

// @snippet qcryptographichash-adddata
%CPPSELF.%FUNCTION_NAME(Shiboken::String::toCString(%PYARG_1), Shiboken::String::len(%PYARG_1));
// @snippet qcryptographichash-adddata

// @snippet qsocketnotifier
PyObject *socket = %PYARG_1;
if (socket != nullptr) {
    // We use qintptr as PyLong, but we check for int
    // since it is currently an alias to be Python2 compatible.
    // Internally, ints are qlonglongs.
    if (%CHECKTYPE[int](socket)) {
        int cppSocket = %CONVERTTOCPP[int](socket);
        qintptr socket = (qintptr)cppSocket;
        %0 = new %TYPE(socket, %2, %3);
    } else {
        PyErr_SetString(PyExc_TypeError,
            "QSocketNotifier: first argument (socket) must be an int.");
    }
}
// @snippet qsocketnotifier

// @snippet qtranslator-load
Py_ssize_t size;
uchar *ptr = reinterpret_cast<uchar *>(Shiboken::Buffer::getPointer(%PYARG_1, &size));
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(const_cast<const uchar *>(ptr), size);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qtranslator-load

// @snippet qtimer-singleshot-1
// %FUNCTION_NAME() - disable generation of c++ function call
(void) %2; // remove warning about unused variable
Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
PyObject *pyTimer = reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_new(Shiboken::SbkType<QTimer>(), emptyTuple, 0);
reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_init(pyTimer, emptyTuple, 0);

auto timer = %CONVERTTOCPP[QTimer *](pyTimer);
//XXX  /|\ omitting this space crashes shiboken!
Shiboken::AutoDecRef result(
    PyObject_CallMethod(pyTimer,
                        const_cast<char *>("connect"),
                        const_cast<char *>("OsOs"),
                        pyTimer,
                        SIGNAL(timeout()),
                        %PYARG_2,
                        %3)
);
Shiboken::Object::releaseOwnership((SbkObject *)pyTimer);
Py_XDECREF(pyTimer);
timer->setSingleShot(true);
timer->connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
timer->start(%1);
// @snippet qtimer-singleshot-1

// @snippet qtimer-singleshot-2
// %FUNCTION_NAME() - disable generation of c++ function call
Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
PyObject *pyTimer = reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_new(Shiboken::SbkType<QTimer>(), emptyTuple, 0);
reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_init(pyTimer, emptyTuple, 0);
QTimer * timer = %CONVERTTOCPP[QTimer *](pyTimer);
timer->setSingleShot(true);

if (PyObject_TypeCheck(%2, PySideSignalInstanceTypeF())) {
    PySideSignalInstance *signalInstance = reinterpret_cast<PySideSignalInstance *>(%2);
    Shiboken::AutoDecRef signalSignature(Shiboken::String::fromFormat("2%s", PySide::Signal::getSignature(signalInstance)));
    Shiboken::AutoDecRef result(
        PyObject_CallMethod(pyTimer,
                            const_cast<char *>("connect"),
                            const_cast<char *>("OsOO"),
                            pyTimer,
                            SIGNAL(timeout()),
                            PySide::Signal::getObject(signalInstance),
                            signalSignature.object())
    );
} else {
    Shiboken::AutoDecRef result(
        PyObject_CallMethod(pyTimer,
                            const_cast<char *>("connect"),
                            const_cast<char *>("OsO"),
                            pyTimer,
                            SIGNAL(timeout()),
                            %PYARG_2)
    );
}

timer->connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()), Qt::DirectConnection);
Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject *>(pyTimer));
Py_XDECREF(pyTimer);
timer->start(%1);
// @snippet qtimer-singleshot-2

// @snippet qprocess-startdetached
qint64 pid;
%RETURN_TYPE retval = %TYPE::%FUNCTION_NAME(%1, %2, %3, &pid);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[qint64](pid));
// @snippet qprocess-startdetached

// @snippet qprocess-pid
long result;
#ifdef WIN32
    _PROCESS_INFORMATION *procInfo = %CPPSELF.%FUNCTION_NAME();
    result = procInfo ? procInfo->dwProcessId : 0;
#else
    result = %CPPSELF.%FUNCTION_NAME();
#endif
%PYARG_0 = %CONVERTTOPYTHON[long](result);
// @snippet qprocess-pid

// @snippet qcoreapplication-init
static void QCoreApplicationConstructor(PyObject *self, PyObject *pyargv, QCoreApplicationWrapper **cptr)
{
    static int argc;
    static char **argv;
    PyObject *stringlist = PyTuple_GET_ITEM(pyargv, 0);
    if (Shiboken::listToArgcArgv(stringlist, &argc, &argv, "PySideApp")) {
        *cptr = new QCoreApplicationWrapper(argc, argv);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject *>(self));
        PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    }
}
// @snippet qcoreapplication-init

// @snippet qcoreapplication-1
QCoreApplicationConstructor(%PYSELF, args, &%0);
// @snippet qcoreapplication-1

// @snippet qcoreapplication-2
PyObject *empty = PyTuple_New(2);
if (!PyTuple_SetItem(empty, 0, PyList_New(0))) {
    QCoreApplicationConstructor(%PYSELF, empty, &%0);
}
// @snippet qcoreapplication-2

// @snippet qcoreapplication-instance
PyObject *pyApp = Py_None;
if (qApp) {
    pyApp = reinterpret_cast<PyObject *>(
        Shiboken::BindingManager::instance().retrieveWrapper(qApp));
    if (!pyApp)
        pyApp = %CONVERTTOPYTHON[QCoreApplication *](qApp);
        // this will keep app live after python exit (extra ref)
}
// PYSIDE-571: make sure that we return the singleton "None"
if (Py_TYPE(pyApp) == Py_TYPE(Py_None))
    Py_DECREF(MakeQAppWrapper(nullptr));
%PYARG_0 = pyApp;
Py_XINCREF(%PYARG_0);
// @snippet qcoreapplication-instance

// @snippet qdatastream-readrawdata
QByteArray data;
data.resize(%2);
int result = 0;
Py_BEGIN_ALLOW_THREADS
result = %CPPSELF.%FUNCTION_NAME(data.data(), data.size());
Py_END_ALLOW_THREADS
if (result == -1) {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
} else {
    %PYARG_0 = PyBytes_FromStringAndSize(data.data(), result);
}
// @snippet qdatastream-readrawdata

// @snippet qdatastream-writerawdata
int r = 0;
Py_BEGIN_ALLOW_THREADS
r = %CPPSELF.%FUNCTION_NAME(%1, Shiboken::String::len(%PYARG_1));
Py_END_ALLOW_THREADS
%PYARG_0 = %CONVERTTOPYTHON[int](r);
// @snippet qdatastream-writerawdata

// @snippet releaseownership
Shiboken::Object::releaseOwnership(%PYARG_0);
// @snippet releaseownership

// @snippet qanimationgroup-clear
for (int counter = 0, count = %CPPSELF.animationCount(); counter < count; ++counter ) {
    QAbstractAnimation *animation = %CPPSELF.animationAt(counter);
    PyObject *obj = %CONVERTTOPYTHON[QAbstractAnimation *](animation);
    Shiboken::Object::setParent(nullptr, obj);
    Py_DECREF(obj);
}
%CPPSELF.clear();
// @snippet qanimationgroup-clear

// @snippet qeasingcurve
PySideEasingCurveFunctor::init();
// @snippet qeasingcurve

// @snippet qeasingcurve-setcustomtype
QEasingCurve::EasingFunction func = PySideEasingCurveFunctor::createCustomFuntion(%PYSELF, %PYARG_1);
if (func)
    %CPPSELF.%FUNCTION_NAME(func);
// @snippet qeasingcurve-setcustomtype

// @snippet qeasingcurve-customtype
//%FUNCTION_NAME()
%PYARG_0 = PySideEasingCurveFunctor::callable(%PYSELF);
// @snippet qeasingcurve-customtype

// @snippet qsignaltransition
if (PyObject_TypeCheck(%1, PySideSignalInstanceTypeF())) {
    PyObject *dataSource = PySide::Signal::getObject((PySideSignalInstance *)%PYARG_1);
    Shiboken::AutoDecRef obType(PyObject_Type(dataSource));
    QObject * sender = %CONVERTTOCPP[QObject *](dataSource);
    //XXX   /|\ omitting this space crashes shiboken!
    if (sender) {
        const char *dataSignature = PySide::Signal::getSignature((PySideSignalInstance *)%PYARG_1);
        QByteArray signature(dataSignature); // Append SIGNAL flag (2)
        signature.prepend('2');
        %0 = new QSignalTransitionWrapper(sender, signature, %2);
    }
}
// @snippet qsignaltransition

// @snippet qstate-addtransition-1
QString signalName(%2);
if (PySide::SignalManager::registerMetaMethod(%1, signalName.mid(1).toLatin1().data(), QMetaMethod::Signal)) {
    QSignalTransition *%0 = %CPPSELF->addTransition(%1, %2, %3);
    %PYARG_0 = %CONVERTTOPYTHON[QSignalTransition *](%0);
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qstate-addtransition-1

// @snippet qstate-addtransition-2
// Obviously the label used by the following goto is a very awkward solution,
// since it refers to a name very tied to the generator implementation.
// Check bug #362 for more information on this
// http://bugs.openbossa.org/show_bug.cgi?id=362
if (!PyObject_TypeCheck(%1, PySideSignalInstanceTypeF()))
    goto Sbk_%TYPEFunc_%FUNCTION_NAME_TypeError;
PySideSignalInstance *signalInstance = reinterpret_cast<PySideSignalInstance *>(%1);
auto sender = %CONVERTTOCPP[QObject *](PySide::Signal::getObject(signalInstance));
QSignalTransition *%0 = %CPPSELF->%FUNCTION_NAME(sender, PySide::Signal::getSignature(signalInstance),%2);
%PYARG_0 = %CONVERTTOPYTHON[QSignalTransition *](%0);
// @snippet qstate-addtransition-2

// @snippet qstatemachine-configuration
%PYARG_0 = PySet_New(0);
for (auto *abs_state : %CPPSELF.configuration()) {
        Shiboken::AutoDecRef obj(%CONVERTTOPYTHON[QAbstractState *](abs_state));
        Shiboken::Object::setParent(self, obj);
        PySet_Add(%PYARG_0, obj);
}
// @snippet qstatemachine-configuration

// @snippet qstatemachine-defaultanimations
%PYARG_0 = PyList_New(0);
for (auto *abs_anim : %CPPSELF.defaultAnimations()) {
        Shiboken::AutoDecRef obj(%CONVERTTOPYTHON[QAbstractAnimation *](abs_anim));
        Shiboken::Object::setParent(self, obj);
        PyList_Append(%PYARG_0, obj);
}
// @snippet qstatemachine-defaultanimations

// @snippet qt-signal
%PYARG_0 = Shiboken::String::fromFormat("2%s",QMetaObject::normalizedSignature(%1).constData());
// @snippet qt-signal

// @snippet qt-slot
%PYARG_0 = Shiboken::String::fromFormat("1%s",QMetaObject::normalizedSignature(%1).constData());
// @snippet qt-slot

// @snippet qt-registerresourcedata
QT_BEGIN_NAMESPACE
extern bool
qRegisterResourceData(int,
                      const unsigned char *,
                      const unsigned char *,
                      const unsigned char *);

extern bool
qUnregisterResourceData(int,
                        const unsigned char *,
                        const unsigned char *,
                        const unsigned char *);
QT_END_NAMESPACE
// @snippet qt-registerresourcedata

// @snippet qt-qregisterresourcedata
%RETURN_TYPE %0 = %FUNCTION_NAME(%1, reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_2)),
                                     reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_3)),
                                     reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_4)));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qt-qregisterresourcedata

// @snippet qt-qunregisterresourcedata
%RETURN_TYPE %0 = %FUNCTION_NAME(%1, reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_2)),
                                     reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_3)),
                                     reinterpret_cast<uchar *>(PyBytes_AS_STRING(%PYARG_4)));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qt-qunregisterresourcedata

// @snippet use-stream-for-format-security
// Uses the stream version for security reasons
// see gcc man page at -Wformat-security
Py_BEGIN_ALLOW_THREADS
%FUNCTION_NAME() << %1;
Py_END_ALLOW_THREADS
// @snippet use-stream-for-format-security

// @snippet qresource-registerResource
 auto ptr = reinterpret_cast<uchar *>(Shiboken::Buffer::getPointer(%PYARG_1));
 %RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(const_cast<const uchar *>(ptr), %2);
 %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qresource-registerResource

// @snippet qstring-return
%PYARG_0 = %CONVERTTOPYTHON[QString](%1);
// @snippet qstring-return

// @snippet stream-write-method
Py_BEGIN_ALLOW_THREADS
(*%CPPSELF) << %1;
Py_END_ALLOW_THREADS
// @snippet stream-write-method

// @snippet stream-read-method
%RETURN_TYPE _cpp_result;
Py_BEGIN_ALLOW_THREADS
(*%CPPSELF) >> _cpp_result;
Py_END_ALLOW_THREADS
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](_cpp_result);
// @snippet stream-read-method

// @snippet return-qstring-ref
QString &res = *%0;
%PYARG_0 = %CONVERTTOPYTHON[QString](res);
// @snippet return-qstring-ref

// @snippet return-readData
%RETURN_TYPE %0 = 0;
if (PyBytes_Check(%PYARG_0)) {
    %0 = PyBytes_GET_SIZE((PyObject *)%PYARG_0);
    memcpy(%1, PyBytes_AS_STRING((PyObject *)%PYARG_0), %0);
} else if (Shiboken::String::check(%PYARG_0)) {
    %0 = Shiboken::String::len((PyObject *)%PYARG_0);
    memcpy(%1, Shiboken::String::toCString((PyObject *)%PYARG_0), %0);
}
// @snippet return-readData

// @snippet qiodevice-readData
QByteArray ba(1 + int(%2), char(0));
Py_BEGIN_ALLOW_THREADS
%CPPSELF.%FUNCTION_NAME(ba.data(), int(%2));
Py_END_ALLOW_THREADS
%PYARG_0 = Shiboken::String::fromCString(ba.constData());
// @snippet qiodevice-readData

// @snippet qt-module-shutdown
{ // Avoid name clash
    Shiboken::AutoDecRef regFunc(static_cast<PyObject *>(nullptr));
    Shiboken::AutoDecRef atexit(Shiboken::Module::import("atexit"));
    if (atexit.isNull()) {
        qWarning("Module atexit not found for registering __moduleShutdown");
        PyErr_Clear();
    }else{
        regFunc.reset(PyObject_GetAttrString(atexit, "register"));
        if (regFunc.isNull()) {
            qWarning("Function atexit.register not found for registering __moduleShutdown");
            PyErr_Clear();
        }
    }
    if (!atexit.isNull() && !regFunc.isNull()){
        PyObject *shutDownFunc = PyObject_GetAttrString(module, "__moduleShutdown");
        Shiboken::AutoDecRef args(PyTuple_New(1));
        PyTuple_SET_ITEM(args, 0, shutDownFunc);
        Shiboken::AutoDecRef retval(PyObject_Call(regFunc, args, 0));
        Q_ASSERT(!retval.isNull());
    }
}
// @snippet qt-module-shutdown


/*********************************************************************
 * CONVERSIONS
 ********************************************************************/

// @snippet conversion-pybool
%out = %OUTTYPE(%in == Py_True);
// @snippet conversion-pybool

// @snippet conversion-pylong
%out = %OUTTYPE(PyLong_AsLong(%in));
// @snippet conversion-pylong

// @snippet conversion-pylong-unsigned
%out = %OUTTYPE(PyLong_AsUnsignedLong(%in));
// @snippet conversion-pylong-unsigned

// @snippet conversion-pylong-quintptr
#if defined(IS_PY3K) && QT_POINTER_SIZE == 8
%out = %OUTTYPE(PyLong_AsUnsignedLongLong(%in));
#else
%out = %OUTTYPE(PyLong_AsUnsignedLong(%in));
#endif
// @snippet conversion-pylong-quintptr

// @snippet conversion-pyunicode
#ifndef Py_LIMITED_API
Py_UNICODE *unicode = PyUnicode_AS_UNICODE(%in);
#  if defined(Py_UNICODE_WIDE)
// cast as Py_UNICODE can be a different type
#    if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
%out = QString::fromUcs4(reinterpret_cast<const char32_t *>(unicode));
#    else
%out = QString::fromUcs4(reinterpret_cast<const uint *>(unicode));
#    endif // Qt 6
#  else // Py_UNICODE_WIDE
#    if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
%out = QString::fromUtf16(reinterpret_cast<const char16_t *>(unicode), PepUnicode_GetLength(%in));
#    else
%out = QString::fromUtf16(reinterpret_cast<const ushort *>(unicode), PepUnicode_GetLength(%in));
#    endif // Qt 6
# endif
#else
wchar_t *temp = PyUnicode_AsWideCharString(%in, NULL);
%out = QString::fromWCharArray(temp);
PyMem_Free(temp);
#endif
// @snippet conversion-pyunicode

// @snippet conversion-pystring
#ifndef IS_PY3K
const char * str = %CONVERTTOCPP[const char *](%in);
//XXX      /|\ omitting this space crashes shiboken!
%out = %OUTTYPE(str);
#endif
// @snippet conversion-pystring

// @snippet conversion-pynone
%out = %OUTTYPE();
// @snippet conversion-pynone

// @snippet conversion-pystring-char
char c = %CONVERTTOCPP[char](%in);
%out = %OUTTYPE(c);
// @snippet conversion-pystring-char

// @snippet conversion-pyint
int i = %CONVERTTOCPP[int](%in);
%out = %OUTTYPE(i);
// @snippet conversion-pyint

// @snippet conversion-qlonglong
// PYSIDE-1250: For QVariant, if the type fits into an int; use int preferably.
qlonglong in = %CONVERTTOCPP[qlonglong](%in);
constexpr qlonglong intMax = qint64(std::numeric_limits<int>::max());
constexpr qlonglong intMin = qint64(std::numeric_limits<int>::min());
%out = in >= intMin && in <= intMax ? %OUTTYPE(int(in)) : %OUTTYPE(in);
// @snippet conversion-qlonglong

// @snippet conversion-qstring
QString in = %CONVERTTOCPP[QString](%in);
%out = %OUTTYPE(in);
// @snippet conversion-qstring

// @snippet conversion-qbytearray
QByteArray in = %CONVERTTOCPP[QByteArray](%in);
%out = %OUTTYPE(in);
// @snippet conversion-qbytearray

// @snippet conversion-pyfloat
double in = %CONVERTTOCPP[double](%in);
%out = %OUTTYPE(in);
// @snippet conversion-pyfloat

// @snippet conversion-sbkobject
// a class supported by QVariant?
int typeCode;
const char *typeName = QVariant_resolveMetaType(Py_TYPE(%in), &typeCode);
if (!typeCode || !typeName) {
    // If the type was not encountered, return a default PyObjectWrapper
    %out = QVariant::fromValue(PySide::PyObjectWrapper(%in));
}
else {
    QVariant var(typeCode, nullptr);
    Shiboken::Conversions::SpecificConverter converter(typeName);
    converter.toCpp(pyIn, var.data());
    %out = var;
}
// @snippet conversion-sbkobject

// @snippet conversion-pydict
QVariant ret = QVariant_convertToVariantMap(%in);
%out = ret.isValid() ? ret : QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pydict

// @snippet conversion-pylist
QVariant ret = QVariant_convertToVariantList(%in);
%out = ret.isValid() ? ret : QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pylist

// @snippet conversion-pyobject
// Is a shiboken type not known by Qt
%out = QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pyobject

// @snippet conversion-qvariant-invalid
%out = QVariant::Invalid;
// @snippet conversion-qvariant-invalid

// @snippet conversion-qvariant-pytypeobject
const char *typeName;
if (Shiboken::String::checkType(reinterpret_cast<PyTypeObject *>(%in)))
    typeName = "QString";
else if (%in == reinterpret_cast<PyObject *>(&PyFloat_Type))
    typeName = "double"; // float is a UserType in QVariant.
else if (%in == reinterpret_cast<PyObject *>(&PyLong_Type))
    typeName = "int";    // long is a UserType in QVariant.
else if (Py_TYPE(%in) == SbkObjectType_TypeF())
    typeName = Shiboken::ObjectType::getOriginalName((SbkObjectType *)%in);
else
    typeName = reinterpret_cast<PyTypeObject *>(%in)->tp_name;
%out = QVariant::nameToType(typeName);
// @snippet conversion-qvariant-pytypeobject

// @snippet conversion-qvariant-pystring
%out = QVariant::nameToType(Shiboken::String::toCString(%in));
// @snippet conversion-qvariant-pystring

// @snippet conversion-qvariant-pydict
%out = QVariant::nameToType("QVariantMap");
// @snippet conversion-qvariant-pydict

// @snippet conversion-qvariant-pysequence
%out = QVariantType_isStringList(%in) ? QVariant::StringList : QVariant::List;
// @snippet conversion-qvariant-pysequence

// @snippet conversion-qjsonobject-pydict
QVariant dict = QVariant_convertToVariantMap(%in);
QJsonValue val = QJsonValue::fromVariant(dict);
%out = val.toObject();
// @snippet conversion-qjsonobject-pydict

// @snippet conversion-qpair-pysequence
%out.first = %CONVERTTOCPP[%OUTTYPE_0](PySequence_Fast_GET_ITEM(%in, 0));
%out.second = %CONVERTTOCPP[%OUTTYPE_1](PySequence_Fast_GET_ITEM(%in, 1));
// @snippet conversion-qpair-pysequence

// @snippet conversion-qdate-pydate
int day = PyDateTime_GET_DAY(%in);
int month = PyDateTime_GET_MONTH(%in);
int year = PyDateTime_GET_YEAR(%in);
%out = %OUTTYPE(year, month, day);
// @snippet conversion-qdate-pydate

// @snippet conversion-qdatetime-pydatetime
int day = PyDateTime_GET_DAY(%in);
int month = PyDateTime_GET_MONTH(%in);
int year = PyDateTime_GET_YEAR(%in);
int hour = PyDateTime_DATE_GET_HOUR(%in);
int min = PyDateTime_DATE_GET_MINUTE(%in);
int sec = PyDateTime_DATE_GET_SECOND(%in);
int usec = PyDateTime_DATE_GET_MICROSECOND(%in);
%out = %OUTTYPE(QDate(year, month, day), QTime(hour, min, sec, usec/1000));
// @snippet conversion-qdatetime-pydatetime

// @snippet conversion-qtime-pytime
int hour = PyDateTime_TIME_GET_HOUR(%in);
int min = PyDateTime_TIME_GET_MINUTE(%in);
int sec = PyDateTime_TIME_GET_SECOND(%in);
int usec = PyDateTime_TIME_GET_MICROSECOND(%in);
%out = %OUTTYPE(hour, min, sec, usec/1000);
// @snippet conversion-qtime-pytime

// @snippet conversion-qbytearray-pybytes
#ifdef IS_PY3K
%out = %OUTTYPE(PyBytes_AS_STRING(%in), PyBytes_GET_SIZE(%in));
#else
%out = %OUTTYPE(Shiboken::String::toCString(%in), Shiboken::String::len(%in));
#endif
// @snippet conversion-qbytearray-pybytes

// @snippet conversion-qbytearray-pybytearray
%out = %OUTTYPE(PyByteArray_AsString(%in), PyByteArray_Size(%in));
// @snippet conversion-qbytearray-pybytearray

// @snippet conversion-qbytearray-pystring
%out = %OUTTYPE(Shiboken::String::toCString(%in), Shiboken::String::len(%in));
// @snippet conversion-qbytearray-pystring

/*********************************************************************
 * NATIVE TO TARGET CONVERSIONS
 ********************************************************************/

// @snippet return-pybool
return PyBool_FromLong((bool)%in);
// @snippet return-pybool

// @snippet return-pylong
return PyLong_FromLong(%in);
// @snippet return-pylong

// @snippet return-pylong-unsigned
return PyLong_FromUnsignedLong(%in);
// @snippet return-pylong-unsigned

// @snippet return-pylong-quintptr
#if defined(IS_PY3K) && QT_POINTER_SIZE == 8
return PyLong_FromUnsignedLongLong(%in);
#else
return PyLong_FromUnsignedLong(%in);
#endif
// @snippet return-pylong-quintptr

// @snippet return-pyunicode
QByteArray ba = %in.toUtf8();
return PyUnicode_FromStringAndSize(ba.constData(), ba.size());
// @snippet return-pyunicode

// @snippet return-pyunicode-qstringref
 const int N = %in.length();
 wchar_t *str = new wchar_t[N];
 %in.toString().toWCharArray(str);
 PyObject *%out = PyUnicode_FromWideChar(str, N);
 delete[] str;
 return %out;
// @snippet return-pyunicode-qstringref

// @snippet return-pyunicode-qchar
wchar_t c = (wchar_t)%in.unicode();
return PyUnicode_FromWideChar(&c, 1);
// @snippet return-pyunicode-qchar

// @snippet return-qvariant
if (!%in.isValid())
    Py_RETURN_NONE;

if (qstrcmp(%in.typeName(), "QVariantList") == 0) {
    QList<QVariant> var = %in.value<QVariantList>();
    return %CONVERTTOPYTHON[QList<QVariant>](var);
}

if (qstrcmp(%in.typeName(), "QStringList") == 0) {
    QStringList var = %in.value<QStringList>();
    return %CONVERTTOPYTHON[QList<QString>](var);
}

if (qstrcmp(%in.typeName(), "QVariantMap") == 0) {
    QMap<QString, QVariant> var = %in.value<QVariantMap>();
    return %CONVERTTOPYTHON[QMap<QString, QVariant>](var);
}

Shiboken::Conversions::SpecificConverter converter(cppInRef.typeName());
if (converter) {
   void *ptr = cppInRef.data();
   return converter.toPython(ptr);
}
PyErr_Format(PyExc_RuntimeError, "Can't find converter for '%s'.", %in.typeName());
return 0;
// @snippet return-qvariant

// @snippet return-qvariant-type
const char *typeName = QVariant::typeToName(%in);
PyObject *%out;
PyTypeObject *pyType = nullptr;
if (typeName)
    pyType = Shiboken::Conversions::getPythonTypeObject(typeName);
%out = pyType ? (reinterpret_cast<PyObject *>(pyType)) : Py_None;
Py_INCREF(%out);
return %out;
// @snippet return-qvariant-type

// @snippet return-qjsonobject
// The QVariantMap returned by QJsonObject seems to cause a segfault, so
// using QJsonObject.toVariantMap() won't work.
// Wrapping it in a QJsonValue first allows it to work
QJsonValue val(%in);
QVariant ret = val.toVariant();

return %CONVERTTOPYTHON[QVariant](ret);
// @snippet return-qjsonobject

// @snippet return-qpair
PyObject *%out = PyTuple_New(2);
PyTuple_SET_ITEM(%out, 0, %CONVERTTOPYTHON[%INTYPE_0](%in.first));
PyTuple_SET_ITEM(%out, 1, %CONVERTTOPYTHON[%INTYPE_1](%in.second));
return %out;
// @snippet return-qpair
