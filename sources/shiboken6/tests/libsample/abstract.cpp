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

#include <iostream>
#include "abstract.h"
#include "objecttype.h"

using namespace std;

const int Abstract::staticPrimitiveField = 0;

Abstract::Abstract(int id) : m_id(id)
{
    toBeRenamedField = readOnlyField = primitiveField = 123;
    valueTypeField = Point(12, 34);
    objectTypeField = nullptr;
    bitField = 0;
}

Abstract::~Abstract()
{
}

void
Abstract::unpureVirtual()
{
}

void
Abstract::callUnpureVirtual()
{
    this->unpureVirtual();
}


void
Abstract::callPureVirtual()
{
    this->pureVirtual();
}

void
Abstract::show(PrintFormat format)
{
    cout << '<';
    switch(format) {
        case Short:
            cout << this;
            break;
        case Verbose:
            cout << "class " << className() << " | cptr: " << this;
            cout << ", id: " << m_id;
            break;
        case OnlyId:
            cout << "id: " << m_id;
            break;
        case ClassNameAndId:
            cout << className() << " - id: " << m_id;
            break;
    }
    cout << '>';
}

void Abstract::callVirtualGettingEnum(PrintFormat p)
{
    virtualGettingAEnum(p);
}

void Abstract::virtualGettingAEnum(Abstract::PrintFormat p)
{
}

