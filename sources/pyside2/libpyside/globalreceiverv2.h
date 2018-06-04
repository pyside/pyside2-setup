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

#ifndef GLOBALRECEIVER_V2_H
#define GLOBALRECEIVER_V2_H

#include <sbkpython.h>
#include <QObject>
#include <QHash>
#include <QSet>
#include <QSharedPointer>
#include <QLinkedList>
#include <QByteArray>

#include "dynamicqmetaobject.h"

namespace PySide
{

class DynamicSlotDataV2;
class GlobalReceiverV2;

typedef QMap<QByteArray, GlobalReceiverV2 *> GlobalReceiverV2Map;
typedef QSharedPointer<GlobalReceiverV2Map> SharedMap;

/**
 * A class used to make the link between the C++ Signal/Slot and Python callback
 * This class is used internally by SignalManager
 **/

class GlobalReceiverV2 : public QObject
{
public:
    /**
     * Create a GlobalReceiver object that will call 'callback' argumentent
     *
     * @param   callback    A Python callable object (can be a method or not)
     * @param   ma          A SharedPointer used on Signal manager that contains all instaces of GlobalReceiver
     **/
    GlobalReceiverV2(PyObject *callback, SharedMap map);

    /**
     * Destructor
     **/
    ~GlobalReceiverV2();

    /**
     * Reimplemented function from QObject
     **/
    int qt_metacall(QMetaObject::Call call, int id, void** args);
    const QMetaObject* metaObject() const;

    /**
     * Add a extra slot to this object
     *
     * @param   signature   The signature of the slot to be added
     * @return  The index of this slot on metaobject
     **/
    int addSlot(const char* signature);

    /**
     * Notify to GlobalReceiver about when a new connection was made
     **/
    void notify();

    /**
     * Used to increment the reference of the GlobalReceiver object
     *
     * @param   link    This is a optional paramenter used to link the ref to some QObject life
     **/
    void incRef(const QObject* link = 0);

    /**
     * Used to decrement the reference of the GlobalReceiver object
     *
     * @param   link    This is a optional paramenter used to dismiss the link ref to some QObject
     **/
    void decRef(const QObject* link = 0);

    /*
     * Return the count of refs which the GlobalReceiver has
     *
     * @param   link    If any QObject was passed, the function return the number of references relative to this 'link' object
     * @return  The number of references
     **/
    int refCount(const QObject* link) const;

    /**
     * Use to retrive the unique hash of this GlobalReceiver object
     *
     * @return  a string with a unique id based on GlobalReceiver contents
     **/
    QByteArray hash() const;

    /**
     * Use to retrive the unique hash of the PyObject based on GlobalReceiver rules
     *
     * @param   callback The Python callable object used to calculate the id
     * @return  a string with a unique id based on GlobalReceiver contents
     **/
    static QByteArray hash(PyObject* callback);

private:
    DynamicQMetaObject m_metaObject;
    DynamicSlotDataV2 *m_data;
    QList<const QObject*> m_refs;
    SharedMap m_sharedMap;
};

}

#endif
