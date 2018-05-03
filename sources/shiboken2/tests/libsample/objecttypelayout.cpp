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

#include "objecttypelayout.h"
#include <iostream>

using namespace std;

void ObjectTypeLayout::addObject(ObjectType* obj)
{
    if (obj->isLayoutType()) {
        ObjectTypeLayout* l = reinterpret_cast<ObjectTypeLayout*>(obj);
        if (l->parent()) {
            cerr << "[WARNING] ObjectTypeLayout::addObject: layout '" << l->objectName().cstring();
            cerr << "' already has a parent." << endl;
            return;
        }

        l->setParent(this);

        if (parent() && !parent()->isLayoutType())
            l->reparentChildren(parent());
    }

    m_objects.push_back(obj);
}

std::list< ObjectType* > ObjectTypeLayout::objects() const
{
    return m_objects;
}

void ObjectTypeLayout::reparentChildren(ObjectType* parent)
{
    std::list<ObjectType*>::const_iterator it = m_objects.begin();
    for (; it != m_objects.end(); ++it) {
        if ((*it)->isLayoutType())
            reinterpret_cast<ObjectTypeLayout*>(*it)->reparentChildren(parent);
        else
            (*it)->setParent(parent);
    }
}

