/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#ifndef BLACKBOX_H
#define BLACKBOX_H

#include "libsamplemacros.h"
#include <map>
#include "objecttype.h"
#include "point.h"

class LIBSAMPLE_API BlackBox
{
public:
    typedef std::map<int, ObjectType*> ObjectTypeMap;
    typedef std::map<int, Point*> PointMap;

    BlackBox() { m_ticket = -1;}
    ~BlackBox();

    int keepObjectType(ObjectType* object);
    ObjectType* retrieveObjectType(int ticket);
    void disposeObjectType(int ticket);

    int keepPoint(Point* point);
    Point* retrievePoint(int ticket);
    void disposePoint(int ticket);

    std::list<ObjectType*> objects();
    std::list<Point*> points();

    inline void referenceToValuePointer(Point*&) {}
    inline void referenceToObjectPointer(ObjectType*&) {}

private:
    ObjectTypeMap m_objects;
    PointMap m_points;
    int m_ticket;
};

#endif // BLACKBOX_H

