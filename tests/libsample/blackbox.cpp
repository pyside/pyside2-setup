/*
 * This file is part of the Shiboken Python Binding Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "blackbox.h"

using namespace std;

BlackBox::~BlackBox()
{
    // Free all lists.
    while (!m_objects.empty()) {
        delete m_objects.back();
        m_objects.pop_back();
    }
    while (!m_points.empty()) {
        delete m_points.back();
        m_points.pop_back();
    }
}

void
BlackBox::keepObjectType(ObjectType* object)
{
    m_objects.push_back(object);
}

ObjectType*
BlackBox::retrieveObjectType(ObjectType* object)
{
    for(ObjectTypeList::iterator objecttype_iter = m_objects.begin();
        objecttype_iter != m_objects.end(); objecttype_iter++) {
        if (object == *objecttype_iter) {
            m_objects.erase(objecttype_iter);
            return object;
        }
    }
    return 0;
}

void
BlackBox::disposeObjectType(ObjectType* object)
{
//TODO: implement + describe inside typesystem file.
}

void
BlackBox::keepPoint(Point* point)
{
    m_points.push_back(point);
}

Point*
BlackBox::retrievePoint(Point* point)
{
    for(PointList::iterator point_iter = m_points.begin();
        point_iter != m_points.end(); point_iter++) {
        if (point == *point_iter) {
            m_points.erase(point_iter);
            return point;
        }
    }
    return 0;
}

void
BlackBox::disposePoint(Point* point)
{
//TODO: implement + describe inside typesystem file.
}

