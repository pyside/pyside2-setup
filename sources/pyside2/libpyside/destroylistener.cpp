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
#include "destroylistener.h"

#include <QObject>
#include <shiboken.h>
#include <QDebug>
#include <QMutex>

PySide::DestroyListener* PySide::DestroyListener::m_instance = 0;

namespace PySide
{

struct DestroyListenerPrivate
{
    static bool m_destroyed;
};


DestroyListener* DestroyListener::instance()
{
    if (!m_instance)
        m_instance = new DestroyListener(0);
    return m_instance;
}

void DestroyListener::destroy()
{
    if (m_instance) {
        m_instance->disconnect();
        delete m_instance;
        m_instance = 0;
    }
}

void DestroyListener::listen(QObject *obj)
{
    SbkObject* wrapper = Shiboken::BindingManager::instance().retrieveWrapper(obj);
    if (!wrapper) // avoid problem with multiple inheritance
        return;

    if (Py_IsInitialized() == 0)
        onObjectDestroyed(obj);
    else
        QObject::connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onObjectDestroyed(QObject*)), Qt::DirectConnection);
}

void DestroyListener::onObjectDestroyed(QObject* obj)
{
    SbkObject* wrapper = Shiboken::BindingManager::instance().retrieveWrapper(obj);
    if (wrapper) //make sure the object exists before destroy
        Shiboken::Object::destroy(wrapper, obj);
}

DestroyListener::DestroyListener(QObject *parent)
    : QObject(parent)
{
    m_d = new DestroyListenerPrivate();
}

DestroyListener::~DestroyListener()
{
    delete m_d;
}

}//namespace

