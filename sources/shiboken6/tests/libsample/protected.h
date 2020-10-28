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

#ifndef PROTECTED_H
#define PROTECTED_H

#include "libsamplemacros.h"
#include "objecttype.h"
#include "point.h"
#include <string>
#include <list>

class LIBSAMPLE_API ProtectedNonPolymorphic
{
public:
    explicit ProtectedNonPolymorphic(const char *name) : m_name(name) {}
    ~ProtectedNonPolymorphic() {}

    inline const char* publicName() { return m_name.c_str(); }

    inline static ProtectedNonPolymorphic* create() { return new ProtectedNonPolymorphic("created"); }

protected:
    inline const char* protectedName() { return m_name.c_str(); }
    inline int protectedSum(int a0, int a1) { return a0 + a1; }
    inline int modifiedProtectedSum(int a0, int a1) { return a0 + a1; }
    inline static const char* protectedStatic() { return "protectedStatic"; }
    inline const char* dataTypeName(void *data = nullptr) const { return "pointer"; }
    inline const char* dataTypeName(int data) const { return "integer"; }

private:
    std::string m_name;
};

class LIBSAMPLE_API ProtectedPolymorphic
{
public:
    explicit ProtectedPolymorphic(const char *name) : m_name(name) {}
    virtual ~ProtectedPolymorphic() {}

    inline static ProtectedPolymorphic* create() { return new ProtectedPolymorphic("created"); }
    inline const char* publicName() { return m_name.c_str(); }
    inline const char* callProtectedName() { return protectedName(); }

protected:
    virtual const char* protectedName() { return m_name.c_str(); }

private:
    std::string m_name;
};

class LIBSAMPLE_API ProtectedPolymorphicDaughter : public ProtectedPolymorphic
{
public:
    explicit ProtectedPolymorphicDaughter(const char *name) : ProtectedPolymorphic(name) {}
    inline static ProtectedPolymorphicDaughter* create() { return new ProtectedPolymorphicDaughter("created"); }
};

class LIBSAMPLE_API ProtectedPolymorphicGrandDaughter: public ProtectedPolymorphicDaughter
{
public:
    explicit ProtectedPolymorphicGrandDaughter(const char *name) : ProtectedPolymorphicDaughter(name) {}
    inline static ProtectedPolymorphicGrandDaughter* create() { return new ProtectedPolymorphicGrandDaughter("created"); }
};

class LIBSAMPLE_API ProtectedVirtualDestructor
{
public:
    ProtectedVirtualDestructor() {}
    inline static ProtectedVirtualDestructor* create() { return new ProtectedVirtualDestructor(); }
    inline static int dtorCalled() { return dtor_called; }
    inline static void resetDtorCounter() { dtor_called = 0; }
protected:
    virtual ~ProtectedVirtualDestructor() { dtor_called++; }
private:
    static int dtor_called;
};

class LIBSAMPLE_API ProtectedEnumClass
{
public:
    ProtectedEnumClass() {}
    virtual ~ProtectedEnumClass() {}
    enum PublicEnum {
        PublicItem0,
        PublicItem1
    };
protected:
    enum ProtectedEnum {
        ProtectedItem0,
        ProtectedItem1
    };
    ProtectedEnum callProtectedEnumMethod(ProtectedEnum in) { return protectedEnumMethod(in); }
    inline PublicEnum callPublicEnumMethod(PublicEnum in) { return publicEnumMethod(in); }

    virtual ProtectedEnum protectedEnumMethod(ProtectedEnum in) { return in; }
    virtual PublicEnum publicEnumMethod(PublicEnum in) { return in; }
};


class LIBSAMPLE_API ProtectedProperty
{
public:
    ProtectedProperty()
        : protectedValueTypeProperty(Point(0, 0)),
          protectedProperty(0),
          protectedEnumProperty(Event::NO_EVENT),
          protectedValueTypePointerProperty(nullptr),
          protectedObjectTypeProperty(nullptr)
    {}
protected:
    // This is deliberately the first member to test wrapper registration
    // for value type members sharing the same memory address.
    Point protectedValueTypeProperty;
    int protectedProperty;
    std::list<int> protectedContainerProperty;
    Event::EventType protectedEnumProperty;
    Point* protectedValueTypePointerProperty;
    ObjectType* protectedObjectTypeProperty;
};

LIBSAMPLE_API inline ProtectedProperty* createProtectedProperty() {
    return new ProtectedProperty;
}

#endif // PROTECTED_H
