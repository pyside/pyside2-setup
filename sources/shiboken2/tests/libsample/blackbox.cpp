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
        ObjectType* second = it->second;
        m_objects.erase(it);
        return second;
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
        Point* second = it->second;
        m_points.erase(it);
        return second;
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

