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

#ifndef OBJECTTYPE_H
#define OBJECTTYPE_H

#include <list>
#include "str.h"
#include "null.h"

#include "libsamplemacros.h"

#include <stddef.h>

struct Event
{
    enum EventType {
        NO_EVENT,
        BASIC_EVENT,
        SOME_EVENT,
        ANY_EVENT
    };

    enum class EventTypeClass {
        Value1,
        Value2
    };

    Event(EventType eventType) : m_eventType(eventType) {}
    EventType eventType() { return m_eventType; }

    void setEventType(EventType et) { m_eventType = et; }
    void setEventTypeByConstRef(const EventType &et) { m_eventType = et; }

private:
    EventType m_eventType;
};

class ObjectTypeLayout;
class ObjectType;
using ObjectTypeList = std::list<ObjectType*>;

class LIBSAMPLE_API ObjectType
{
public:
    // ### Fixme: Use uintptr_t in C++ 11
    using Identifier = size_t;

    explicit ObjectType(ObjectType *parent = nullptr);
    virtual ~ObjectType();

    // factory method
    inline static ObjectType* create() { return new ObjectType(); }
    static ObjectType* createWithChild();

    void setParent(ObjectType* parent);
    inline ObjectType* parent() const { return m_parent; }
    inline const ObjectTypeList& children() const { return m_children; }
    void killChild(const Str& name);
    void removeChild(ObjectType* child);
    ObjectType* takeChild(ObjectType* child);
    virtual ObjectType* takeChild(const Str& name);
    ObjectType* findChild(const Str& name);

    Str objectName() const;
    void setObjectName(const Str& name);

    inline Identifier identifier() const { return reinterpret_cast<Identifier>(this); }

    bool causeEvent(Event::EventType eventType);

    // Returns true if the event is processed.
    virtual bool event(Event* event);
    static int processEvent(ObjectTypeList objects, Event *event);

    void callInvalidateEvent(Event* event);
    virtual void invalidateEvent(Event* event) {}

    // This nonsense method emulate QWidget.setLayout method
    // All layout objects will became children of this object.
    void setLayout(ObjectTypeLayout* layout);
    inline ObjectTypeLayout* layout() const { return m_layout; }

    // This method should be reimplemented by ObjectTypeLayout.
    virtual bool isLayoutType() { return false; }

    unsigned char callWithEnum(const Str& prefix, Event::EventType type, unsigned char value=80);
    unsigned char callWithEnum(const Str& prefix, unsigned char value=0);

    //Functions used in test with named arguments
    void setObjectSplittedName(const char*, const Str& prefix = Str("<unk"), const Str& suffix = Str("nown>"));
    void setObjectNameWithSize(const char*, int size=9, const Str& name = Str("<unknown>"));
    void setObjectNameWithSize(const Str& name = Str("<unknown>"), int size=9);

    //Function used to confuse the generator when two values accept Null as arg
    void setObject(ObjectType *);
    void setObject(const Null&);
    int callId() const;

    //Function used to create a parent from C++
    virtual bool isPython() { return false; }
    void callVirtualCreateChild();
    virtual ObjectType* createChild(ObjectType* parent);
    static std::size_t createObjectType();

    //return a parent from C++
    ObjectType* getCppParent() {
        if (!m_parent) {
            ObjectType* parent = new ObjectType();
            setParent(parent);
        }
        return m_parent;
    }

    void destroyCppParent() {
        delete m_parent;
    }

    //Deprecated test
    bool deprecatedFunction() { return true; }

    // nextInFocusChain simply returns the parent to test object cycles; the parent
    // may be returned by the QWidget's implementation but isn't always returned
    ObjectType* nextInFocusChain() { return m_parent; }

private:
    ObjectType(const ObjectType&);
    ObjectType& operator=(const ObjectType&);

    ObjectTypeLayout* takeLayout();

    Str m_objectName;
    ObjectType* m_parent;
    ObjectTypeList m_children;

    ObjectTypeLayout* m_layout;


    //used on overload null test
    int m_call_id;
};

LIBSAMPLE_API unsigned int objectTypeHash(const ObjectType* objectType);

class LIBSAMPLE_API OtherBase {
public:
    OtherBase() {};
    virtual ~OtherBase();
};

class LIBSAMPLE_API ObjectTypeDerived: public ObjectType, public OtherBase {
public:
    ObjectTypeDerived(): ObjectType(), OtherBase() {};

    bool event(Event* event) override;
    ~ObjectTypeDerived() override;
};

#endif // OBJECTTYPE_H

