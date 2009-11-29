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
    // Free all maps.
    while (!m_objects.empty()) {
        delete (*m_objects.begin()).second;
        m_objects.erase(m_objects.begin());
    }
    while (!m_points.empty()) {
        delete (*m_points.begin()).second;
        m_points.erase(m_points.begin());
    }
}

int
BlackBox::keepObjectType(ObjectType* object)
{
    m_ticket++;
    std::pair<int, ObjectType*> item(m_ticket, object);
    m_objects.insert(item);
    object->setParent(0);

    return m_ticket;
}

ObjectType*
BlackBox::retrieveObjectType(int ticket)
{
    map<int, ObjectType*>::iterator it = m_objects.find(ticket);
    if (it != m_objects.end()) {
        m_objects.erase(it);
        return it->second;
    }
    return 0;
}

void
BlackBox::disposeObjectType(int ticket)
{
    ObjectType* object = retrieveObjectType(ticket);
    if (object)
        delete object;
}

int
BlackBox::keepPoint(Point* point)
{
    m_ticket++;
    std::pair<int, Point*> item(m_ticket, point);
    m_points.insert(item);

    return m_ticket;
}

Point*
BlackBox::retrievePoint(int ticket)
{
    map<int, Point*>::iterator it = m_points.find(ticket);
    if (it != m_points.end()) {
        m_points.erase(it);
        return it->second;
    }
    return 0;
}

void
BlackBox::disposePoint(int ticket)
{
    Point* point = retrievePoint(ticket);
    if (point)
        delete point;
}


std::list<ObjectType*>
BlackBox::objects()
{
    std::list<ObjectType*> l;
    map<int, ObjectType*>::iterator it;

    for ( it = m_objects.begin() ; it != m_objects.end(); it++ )
        l.push_back((*it).second);

    return l;
}

std::list<Point*>
BlackBox::points()
{
    std::list<Point*> l;
    map<int, Point*>::iterator it;

    for ( it = m_points.begin() ; it != m_points.end(); it++ )
        l.push_back((*it).second);

    return l;
}

