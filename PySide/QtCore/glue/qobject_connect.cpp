static bool isDecorator(PyObject* method, PyObject* self)
{
    Shiboken::AutoDecRef methodName(PyObject_GetAttrString(method, "__name__"));
    if (!PyObject_HasAttr(self, methodName))
        return true;
    Shiboken::AutoDecRef otherMethod(PyObject_GetAttr(self, methodName));
    return reinterpret_cast<PyMethodObject*>(otherMethod.object())->im_func != \
           reinterpret_cast<PyMethodObject*>(method)->im_func;
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
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toUtf8();
        const QMetaObject* metaObject = (*receiver)->metaObject();
        int slotIndex = metaObject->indexOfSlot(callbackSig->constData());
        if (slotIndex != -1 && slotIndex < metaObject->methodOffset() && PyMethod_Check(callback))
            usingGlobalReceiver = true;
    }

    if (usingGlobalReceiver) {
        PySide::SignalManager& signalManager = PySide::SignalManager::instance();
        *receiver = signalManager.globalReceiver(source, callback);
        *callbackSig = PySide::Signal::getCallbackSignature(signal, *receiver, callback, usingGlobalReceiver).toUtf8();
    }

    return usingGlobalReceiver;
}

static QMetaObject::Connection qobjectConnect(QObject* source, const char* signal, QObject* receiver, const char* slot, Qt::ConnectionType type)
{
    if (!signal || !slot)
        return QMetaObject::Connection();

    if (!PySide::Signal::checkQtSignal(signal))
        return QMetaObject::Connection();
    signal++;

    if (!PySide::SignalManager::registerMetaMethod(source, signal, QMetaMethod::Signal))
        return QMetaObject::Connection();

    bool isSignal = PySide::Signal::isQtSignal(slot);
    slot++;
    PySide::SignalManager::registerMetaMethod(receiver, slot, isSignal ? QMetaMethod::Signal : QMetaMethod::Slot);
    QMetaObject::Connection connection;
    Py_BEGIN_ALLOW_THREADS
    connection = QObject::connect(source, signal - 1, receiver, slot - 1, type);
    Py_END_ALLOW_THREADS
    return connection;
}

static QMetaObject::Connection qobjectConnectCallback(QObject* source, const char* signal, PyObject* callback, Qt::ConnectionType type)
{
    if (!signal || !PySide::Signal::checkQtSignal(signal))
        return QMetaObject::Connection();
    signal++;

    int signalIndex = PySide::SignalManager::registerMetaMethodGetIndex(source, signal, QMetaMethod::Signal);
    if (signalIndex == -1)
        return QMetaObject::Connection();

    PySide::SignalManager& signalManager = PySide::SignalManager::instance();

    // Extract receiver from callback
    QObject* receiver = 0;
    PyObject* self = 0;
    QByteArray callbackSig;
    bool usingGlobalReceiver = getReceiver(source, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == 0 && self == 0)
        return QMetaObject::Connection();

    const QMetaObject* metaObject = receiver->metaObject();
    const char* slot = callbackSig.constData();
    int slotIndex = metaObject->indexOfSlot(slot);

    if (slotIndex == -1) {
        if (!usingGlobalReceiver && self && !Shiboken::Object::hasCppWrapper((SbkObject*)self)) {
            qWarning() << "You can't add dynamic slots on an object originated from C++.";
            if (usingGlobalReceiver)
                signalManager.releaseGlobalReceiver(source, receiver);

            return QMetaObject::Connection();
        }

        if (usingGlobalReceiver)
            slotIndex = signalManager.globalReceiverSlotIndex(receiver, slot);
        else
            slotIndex = PySide::SignalManager::registerMetaMethodGetIndex(receiver, slot, QMetaMethod::Slot);

        if (slotIndex == -1) {
            if (usingGlobalReceiver)
                signalManager.releaseGlobalReceiver(source, receiver);

            return QMetaObject::Connection();
        }
    }
    QMetaObject::Connection connection;
    Py_BEGIN_ALLOW_THREADS
    connection = QMetaObject::connect(source, signalIndex, receiver, slotIndex, type);
    Py_END_ALLOW_THREADS
    if (connection) {
        if (usingGlobalReceiver)
            signalManager.notifyGlobalReceiver(receiver);
        #ifndef AVOID_PROTECTED_HACK
            source->connectNotify(signal - 1);
        #else
            // Need to cast to QObjectWrapper* and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper*>(source)->connectNotify(signal - 1);
        #endif

        return connection;
    }

    if (usingGlobalReceiver)
        signalManager.releaseGlobalReceiver(source, receiver);

    return QMetaObject::Connection();
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
    bool usingGlobalReceiver = getReceiver(NULL, signal, callback, &receiver, &self, &callbackSig);
    if (receiver == 0 && self == 0)
        return false;

    const QMetaObject* metaObject = receiver->metaObject();
    int signalIndex = source->metaObject()->indexOfSignal(++signal);
    int slotIndex = -1;

    slotIndex = metaObject->indexOfSlot(callbackSig);

    bool disconnected;
    Py_BEGIN_ALLOW_THREADS
    disconnected = QMetaObject::disconnectOne(source, signalIndex, receiver, slotIndex);
    Py_END_ALLOW_THREADS

    if (disconnected) {
        if (usingGlobalReceiver)
            signalManager.releaseGlobalReceiver(source, receiver);

        #ifndef AVOID_PROTECTED_HACK
            source->disconnectNotify(callbackSig);
        #else
            // Need to cast to QObjectWrapper* and call the public version of
            // connectNotify when avoiding the protected hack.
            reinterpret_cast<QObjectWrapper*>(source)->disconnectNotify(callbackSig);
        #endif
        return true;
    }
    return false;
}
