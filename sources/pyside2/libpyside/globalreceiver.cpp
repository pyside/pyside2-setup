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

#include "globalreceiver.h"
#include "dynamicqmetaobject_p.h"
#include "pysideweakref.h"

#include <QMetaMethod>
#include <QDebug>
#include <QEvent>
#include <QVector>
#include <autodecref.h>
#include <sbkconverter.h>
#include <gilstate.h>

#include "signalmanager.h"

#define RECEIVER_DESTROYED_SLOT_NAME "__receiverDestroyed__(QObject*)"

namespace PySide
{
class DynamicSlotData
{
    Q_DISABLE_COPY(DynamicSlotData)
    public:
        DynamicSlotData(int id, PyObject* callback, GlobalReceiver* parent);
        void addRef(const QObject* o);
        void decRef(const QObject* o);
        void clear();
        int hasRefTo(const QObject* o) const;
        int refCount() const;
        int id() const;
        PyObject* call(PyObject* args);
        ~DynamicSlotData();
        static void onCallbackDestroyed(void* data);

    private:
        int m_id;
        bool m_isMethod;
        PyObject* m_callback;
        PyObject* m_pythonSelf;
        PyObject* m_pyClass;
        PyObject* m_weakRef;
        GlobalReceiver* m_parent;
        QVector<const QObject*> m_refs;
};

}

using namespace PySide;

DynamicSlotData::DynamicSlotData(int id, PyObject* callback, GlobalReceiver* parent)
    : m_id(id), m_pythonSelf(0), m_pyClass(0), m_weakRef(0), m_parent(parent)
{
    Shiboken::GilState gil;

    m_isMethod = PyMethod_Check(callback);
    if (m_isMethod) {
        //Can not store calback pointe because this will be destroyed at the end of the scope
        //To avoid increment intance reference keep the callback information
        m_callback = PyMethod_GET_FUNCTION(callback);
#ifdef IS_PY3K
        m_pyClass = 0;
#else
        m_pyClass = PyMethod_GET_CLASS(callback);
#endif

        m_pythonSelf = PyMethod_GET_SELF(callback);

        //monitor class from method lifetime
        m_weakRef = WeakRef::create(m_pythonSelf, DynamicSlotData::onCallbackDestroyed, this);
    } else {
        m_callback = callback;
        Py_INCREF(m_callback);
    }
}

PyObject* DynamicSlotData::call(PyObject* args)
{
    PyObject* callback = m_callback;

    //create a callback based on method data
    Shiboken::GilState gil;
    if (m_isMethod)
#ifdef IS_PY3K
        callback = PyMethod_New(callback, m_pythonSelf);
#else
        callback = PyMethod_New(callback, m_pythonSelf, m_pyClass);
#endif

    PyObject* result =  PyObject_CallObject(callback, args);

    if (m_isMethod)
        Py_DECREF(callback);

    return result;
}

void DynamicSlotData::addRef(const QObject *o)
{
    m_refs.append(o);
}

void DynamicSlotData::decRef(const QObject *o)
{
    m_refs.removeOne(o);
}

int DynamicSlotData::refCount() const
{
    return m_refs.size();
}

int DynamicSlotData::id() const
{
    return m_id;
}

int DynamicSlotData::hasRefTo(const QObject *o) const
{
    return m_refs.count(o);
}

void DynamicSlotData::clear()
{
    Shiboken::GilState gil;
    Py_XDECREF(m_weakRef);
    m_weakRef = 0;
    m_refs.clear();
}

DynamicSlotData::~DynamicSlotData()
{
    Shiboken::GilState gil;
    clear();
    if (!m_isMethod)
       Py_DECREF(m_callback);
}

void DynamicSlotData::onCallbackDestroyed(void *data)
{
    Shiboken::GilState gil;
    DynamicSlotData* self = reinterpret_cast<DynamicSlotData*>(data);

    //Disconnect all sources
    QMetaMethod m = self->m_parent->metaObject()->method(self->m_id);
    QByteArray methodName = QByteArray::number(m.methodType()).append(m.methodSignature());
    const QVector<const QObject*> sources = self->m_refs;
    for (const QObject* src : sources)
        const_cast<QObject*>(src)->disconnect(self->m_parent, methodName);
    self->m_weakRef = 0;
}

GlobalReceiver::GlobalReceiver()
    : m_metaObject(GLOBAL_RECEIVER_CLASS_NAME, &QObject::staticMetaObject)
{
    //slot used to be notifyed of object destrouction
    m_metaObject.addSlot(RECEIVER_DESTROYED_SLOT_NAME);
    m_metaObject.update();
    setObjectName(QLatin1String("GLOBAL RECEIVER"));
}

GlobalReceiver::~GlobalReceiver()
{
    while(!m_slotReceivers.empty()) {
        DynamicSlotData* data = m_slotReceivers.take(m_slotReceivers.begin().key());
        data->clear();
        delete data;
    }
}

void GlobalReceiver::connectNotify(QObject* source, int slotId)
{
    const auto it = m_slotReceivers.constFind(slotId);
    if (it != m_slotReceivers.cend()) {
        DynamicSlotData* data = it.value();
        if (!data->hasRefTo(source))
            QObject::connect(source, SIGNAL(destroyed(QObject*)), this, "1" RECEIVER_DESTROYED_SLOT_NAME);
        data->addRef(source);
    }
}

void GlobalReceiver::disconnectNotify(QObject* source, int slotId)
{
    const auto it = m_slotReceivers.constFind(slotId);
    if (it != m_slotReceivers.cend()) {
        DynamicSlotData* data = it.value();
        data->decRef(source);
        if (data->refCount() == 0)
            removeSlot(slotId);

        if (!hasConnectionWith(source))
            QObject::disconnect(source, SIGNAL(destroyed(QObject*)), this, "1" RECEIVER_DESTROYED_SLOT_NAME);
    }
}

const QMetaObject* GlobalReceiver::metaObject() const
{
    return m_metaObject.update();
}

int GlobalReceiver::addSlot(const char* slot, PyObject* callback)
{
    int slotId = m_metaObject.addSlot(slot);
    if (!m_slotReceivers.contains(slotId))
        m_slotReceivers.insert(slotId, new DynamicSlotData(slotId, callback, this));

    bool isShortCircuit = true;
    for (int i = 0; slot[i]; ++i) {
        if (slot[i] == '(') {
            isShortCircuit = false;
            break;
        }
    }

    if (isShortCircuit)
        m_shortCircuitSlots << slotId;

    Q_ASSERT(slotId >= QObject::staticMetaObject.methodCount());
    return slotId;
}

void GlobalReceiver::removeSlot(int slotId)
{
    auto it = m_slotReceivers.find(slotId);
    if (it != m_slotReceivers.end()) {
        delete it.value();
        m_slotReceivers.erase(it);
        m_metaObject.removeSlot(slotId);
        m_shortCircuitSlots.remove(slotId);
    }
}

bool GlobalReceiver::hasConnectionWith(const QObject *object)
{
    QHash<int, DynamicSlotData*>::iterator i = m_slotReceivers.begin();
    while(i != m_slotReceivers.end()) {
        if (i.value()->hasRefTo(object)) {
            return true;
        }
        i++;
    }
    return false;
}

int GlobalReceiver::qt_metacall(QMetaObject::Call call, int id, void** args)
{
    Q_ASSERT(call == QMetaObject::InvokeMetaMethod);
    Q_ASSERT(id >= QObject::staticMetaObject.methodCount());
    QMetaMethod slot = metaObject()->method(id);
    Q_ASSERT(slot.methodType() == QMetaMethod::Slot);

    if (strcmp(slot.methodSignature(), RECEIVER_DESTROYED_SLOT_NAME) == 0) {
        QObject *arg = *(QObject**)args[1];

        //avoid hash changes during the destruction
        QHash<int, DynamicSlotData*> copy = m_slotReceivers;
        QHash<int, DynamicSlotData*>::iterator i = copy.begin();
        while(i != copy.end()) {
            //Remove all refs
            int refs = i.value()->hasRefTo(arg);
            while(refs) {
                disconnectNotify(arg, i.key());
                refs--;
            }
            i++;
        }
        return -1;
    }

    DynamicSlotData* data = m_slotReceivers.value(id);
    if (!data) {
        qWarning() << "Unknown global slot, id:" << id;
        return -1;
    }

    Shiboken::GilState gil;
    PyObject* retval = 0;
    if (m_shortCircuitSlots.contains(id)) {
        retval = data->call(reinterpret_cast<PyObject*>(args[1]));
    } else {
        QList<QByteArray> paramTypes = slot.parameterTypes();
        Shiboken::AutoDecRef preparedArgs(PyTuple_New(paramTypes.count()));
        for (int i = 0, max = paramTypes.count(); i < max; ++i) {
            const QByteArray& paramType = paramTypes[i];
            Shiboken::Conversions::SpecificConverter converter(paramType.constData());
            PyTuple_SET_ITEM(preparedArgs.object(), i, converter.toPython(args[i+1]));
        }
        retval = data->call(preparedArgs);
    }

    if (!retval)
        PyErr_Print();
    else
        Py_DECREF(retval);

    return -1;
}
