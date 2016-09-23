/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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
#include "reference.h"

using namespace std;

void
Reference::show() const
{
    cout << "Reference.objId: " << m_objId << ", address: " << this;
}

int
Reference::usesReferenceVirtual(Reference& r, int inc)
{
    return r.m_objId + inc;
}

int
Reference::usesConstReferenceVirtual(const Reference& r, int inc)
{
    return r.m_objId + inc;
}

int
Reference::callUsesReferenceVirtual(Reference& r, int inc)
{
    return usesReferenceVirtual(r, inc);
}

int
Reference::callUsesConstReferenceVirtual(const Reference& r, int inc)
{
    return usesConstReferenceVirtual(r, inc);
}

void
Reference::alterReferenceIdVirtual(Reference& r)
{
    r.setObjId(r.objId() * Reference::multiplier());
}

void
Reference::callAlterReferenceIdVirtual(Reference& r)
{
    alterReferenceIdVirtual(r);
}

ObjTypeReference::~ObjTypeReference()
{
}
