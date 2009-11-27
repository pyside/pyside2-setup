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

#ifndef OBJECTTYPE_H
#define OBJECTTYPE_H

#include <list>
#include "str.h"

#include "libsamplemacros.h"

struct LIBSAMPLE_API Event
{
    enum EventType {
        NO_EVENT,
        BASIC_EVENT,
        SOME_EVENT,
        ANY_EVENT
    };
    Event(EventType eventType) : m_eventType(eventType) {}
    EventType eventType() { return m_eventType; }
private:
    EventType m_eventType;
};

class LIBSAMPLE_API ObjectType
{
public:
    typedef std::list<ObjectType*> ObjectTypeList;

    ObjectType(ObjectType* parent = 0);
    virtual ~ObjectType();

    // factory method
    static ObjectType* create() { return new ObjectType(); }

    void setParent(ObjectType* parent);
    ObjectType* parent() const { return m_parent; }
    const ObjectTypeList& children() const { return m_children; }

    Str objectName() const;
    void setObjectName(const Str& name);

    bool causeEvent(Event::EventType eventType);

    // Returns true if the event is processed.
    virtual bool event(Event* event);

private:
    ObjectType(const ObjectType&);
    ObjectType& operator=(const ObjectType&);

    Str* m_objectName;
    ObjectType* m_parent;
    ObjectTypeList m_children;
};
#endif // OBJECTTYPE_H

