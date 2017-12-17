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

static bool isDecorator(PyObject* method, PyObject* self)
{
    Shiboken::AutoDecRef methodName(PyObject_GetAttrString(method, "__name__"));
    if (!PyObject_HasAttr(self, methodName))
        return true;
    Shiboken::AutoDecRef otherMethod(PyObject_GetAttr(self, methodName));
    return PyMethod_GET_FUNCTION(otherMethod.object()) != PyMethod_GET_FUNCTION(method);
}

static bool getReceiver(QObject *source, const char* signal, PyObject* callback, QObject** receiver, PyObject** self, QByteArray* callbackSig)
{
    bool forceGlobalReceiver = false;
    if (PyMethod_Check(callback)) {
        *self = PyMethod_GET_SELF(callback);
        if (%CHECKTYPE[QObject*](*self))
            *receiver = %CONVERTTOCPP[QObject*](*self);
        forceGlobalReceiver = isDecorator(callback, *self);
    } else if (PyCFunction_Check(callback)) {
        *self = PyCFunction_GET_SELF(callback);
        if (*self && %CHECKTYPE[QObject*](*self))
            *receiver = %CONVERTTOCPP[QObject*](*self);
    } else if (PyCallable_Check(callback)) {
        // Ok, just a callable object
        *receiver = 0;
        *self = 0;
    }

    bool usingGlobalReceiver = !*receiver || forceGlobalReceiver;

    // Check if this callback is a overwrite of a non-virtual Qt slot.
    if (!usingGlobalReceiver && receiver && self) {
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toLatin1();
        const QMetaObject* metaObject = (*receiver)->metaObject();
        int slotIndex = metaObject->indexOfSlot(callbackSig->constData());
        if (slotIndex != -1 && slotIndex < metaObject->methodOffset() && PyMethod_Check(callback))
            usingGlobalReceiver = true;
    }

    if (usingGlobalReceiver) {
        PySide::SignalManager& signalManager = PySide::SignalManager::instance();
        *receiver = signalManager.globalReceiver(source, callback);
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toLatin1();
    }

    return usingGlobalReceiver;
}

static bool qobjectConnect(QObject* source, const char* signal, QObject* receiver, const char* slot, Qt::ConnectionType type)
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
    Py_BEGIN_ALLOW_THREADS
    connection = QObject::connect(source, signal - 1, receiver, slot - 1, type);
    Py_END_ALLOW_THREADS
    return connection;
}

static bool qobjectConnect(QObject* source, QMetaMethod signal, QObject* receiver, QMetaMethod slot, Qt::ConnectionType type)
{
   return qobjectConnect(source, signal.methodSignature(), receiver, slot.methodSignature(), type);
}

static bool qobjectConnectCallback(QObject* source, const char* signal, PyObject* callback, Qt::ConnectionType type)
{
    if (!signal || !PySide::Signal::checkQtSignal(signal))
        return false;
    signal++;

    int signalIndex = PySide::SignalManager::registerMetaMethodGetIndex(source, signal, QMetaMethod::Signal);
    if (signalIndex == -1)
        return false;

    PySide::SignalManager& signalManager = PySide::SignalManager::instance();

    // Extract receiver from callback
    QObject* receiver = 0;
    PyObject* self = 0;
    QByteArray callbackSig;
    bool usingGlobalReceiver = getReceiver(source, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == 0 && self == 0)
        return false;

    const QMetaObject* metaObject = receiver->metaObject();
    const char* slot = callbackSig.constData();
    int slotIndex = metaObject->indexOfSlot(slot);
    QMetaMethod signalMethod = metaObject->method(signalIndex);

    if (slotIndex == -1) {
        if (!usingGlobalReceiver && self && !Shiboken::Object::hasCppWrapper((SbkObject*)self)) {
            qWarning() << "You can't add dynamic slots on an object originated from C++.";
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
    Py_BEGIN_ALLOW_THREADS
    connection = QMetaObject::connect(source, signalIndex, receiver, slotIndex, type);
    Py_END_ALLOW_THREADS
    if (connection) {
        if (usingGlobalReceiver)
            signalManager.notifyGlobalReceiver(receiver);
        #ifndef AVOID_PROTECTED_HACK
            source->connectNotify(signalMethod); //Qt5: QMetaMethod instead of char*
        #else
            // Need to cast to QObjectWrapper* and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper*>(source)->connectNotify(signalMethod); //Qt5: QMetaMethod instead of char*
        #endif

        return connection;
    }

    if (usingGlobalReceiver)
        signalManager.releaseGlobalReceiver(source, receiver);

    return false;
}


static bool qobjectDisconnectCallback(QObject* source, const char* signal, PyObject* callback)
{
    if (!PySide::Signal::checkQtSignal(signal))
        return false;

    PySide::SignalManager& signalManager = PySide::SignalManager::instance();

    // Extract receiver from callback
    QObject* receiver = 0;
    PyObject* self = 0;
    QByteArray callbackSig;
    QMetaMethod slotMethod;
    bool usingGlobalReceiver = getReceiver(NULL, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == 0 && self == 0)
        return false;

    const QMetaObject* metaObject = receiver->metaObject();
    int signalIndex = source->metaObject()->indexOfSignal(++signal);
    int slotIndex = -1;

    slotIndex = metaObject->indexOfSlot(callbackSig);
    slotMethod = metaObject->method(slotIndex);

    bool disconnected;
    Py_BEGIN_ALLOW_THREADS
    disconnected = QMetaObject::disconnectOne(source, signalIndex, receiver, slotIndex);
    Py_END_ALLOW_THREADS

    if (disconnected) {
        if (usingGlobalReceiver)
            signalManager.releaseGlobalReceiver(source, receiver);

        #ifndef AVOID_PROTECTED_HACK
            source->disconnectNotify(slotMethod); //Qt5: QMetaMethod instead of char*
        #else
            // Need to cast to QObjectWrapper* and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper*>(source)->disconnectNotify(slotMethod); //Qt5: QMetaMethod instead of char*
        #endif
        return true;
    }
    return false;
}
