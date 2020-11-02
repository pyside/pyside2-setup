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

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include "pysidemacros.h"

#include <sbkpython.h>
#include <shibokenmacros.h>

#include <QtCore/QMetaMethod>

QT_FORWARD_DECLARE_CLASS(QDataStream)

namespace PySide
{

/// Thin wrapper for PyObject which increases the reference count at the constructor but *NOT* at destructor.
class PYSIDE_API PyObjectWrapper
{
public:
    PyObjectWrapper(PyObjectWrapper&&) = delete;
    PyObjectWrapper& operator=(PyObjectWrapper &&) = delete;

    PyObjectWrapper();
    explicit PyObjectWrapper(PyObject* me);
    PyObjectWrapper(const PyObjectWrapper &other);
    PyObjectWrapper& operator=(const PyObjectWrapper &other);

    void reset(PyObject *o);

    ~PyObjectWrapper();
    operator PyObject*() const;

private:
    PyObject* m_me;
};

PYSIDE_API QDataStream &operator<<(QDataStream& out, const PyObjectWrapper& myObj);
PYSIDE_API QDataStream &operator>>(QDataStream& in, PyObjectWrapper& myObj);

class PYSIDE_API SignalManager
{
    Q_DISABLE_COPY(SignalManager)
public:
    static SignalManager& instance();

    QObject* globalReceiver(QObject* sender, PyObject* callback);
    void releaseGlobalReceiver(const QObject* sender, QObject* receiver);
    int globalReceiverSlotIndex(QObject* sender, const char* slotSignature) const;
    void notifyGlobalReceiver(QObject* receiver);

    bool emitSignal(QObject* source, const char* signal, PyObject* args);
    static int qt_metacall(QObject* object, QMetaObject::Call call, int id, void** args);

    // Used to register a new signal/slot on QMetaobject of source.
    static bool registerMetaMethod(QObject* source, const char* signature, QMetaMethod::MethodType type);
    static int registerMetaMethodGetIndex(QObject* source, const char* signature, QMetaMethod::MethodType type);

    // used to discovery metaobject
    static const QMetaObject* retrieveMetaObject(PyObject* self);

    // Used to discovery if SignalManager was connected with object "destroyed()" signal.
    int countConnectionsWith(const QObject *object);

    // Disconnect all signals managed by Globalreceiver
    void clear();

    // Utility function to call a python method usign args received in qt_metacall
    static int callPythonMetaMethod(const QMetaMethod& method, void** args, PyObject* obj, bool isShortCuit);

private:
    struct SignalManagerPrivate;
    SignalManagerPrivate* m_d;

    SignalManager();
    ~SignalManager();
};

}

Q_DECLARE_METATYPE(PySide::PyObjectWrapper)

#endif
