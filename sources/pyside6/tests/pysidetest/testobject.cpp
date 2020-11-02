/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "testobject.h"

#include <QtCore/QDebug>

void TestObject::emitIdValueSignal()
{
    emit idValue(m_idValue);
}

void TestObject::emitStaticMethodDoubleSignal()
{
    emit staticMethodDouble();
}

void TestObject::emitSignalWithDefaultValue_void()
{
    emit signalWithDefaultValue();
}

void TestObject::emitSignalWithDefaultValue_bool()
{
    emit signalWithDefaultValue(true);
}

void TestObject::emitSignalWithTypedefValue(int value)
{
    emit signalWithTypedefValue(TypedefValue(value));
}

QDebug operator<<(QDebug dbg, TestObject& testObject)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "TestObject(id=" << testObject.idValue() << ") ";
    return dbg;
}

namespace PySideCPP {
    QDebug operator<<(QDebug dbg, TestObjectWithNamespace& testObject)
    {
        QDebugStateSaver saver(dbg);
        dbg.nospace() << "TestObjectWithNamespace(" << testObject.name() << ") ";
        return dbg;
    }
    QDebug operator<<(QDebug dbg, TestObject2WithNamespace& testObject)
    {
        QDebugStateSaver saver(dbg);
        dbg.nospace() << "TestObject2WithNamespace(" << testObject.name() << ") ";
        return dbg;
    }
}
