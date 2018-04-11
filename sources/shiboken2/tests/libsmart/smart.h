/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef SMART_H
#define SMART_H

#include <algorithm>
#include <iostream>
#include <vector>

#include "libsmartmacros.h"

// Forward declarations.
template <class T>
class SharedPtr;
class Integer;
class Obj;

LIB_SMART_API bool shouldPrint();

// Used to track which C++ objects are alive.
class LIB_SMART_API Registry {
public:
    static Registry *getInstance();

    void add(Obj *p);
    void add(Integer *p);
    void remove(Obj *p);
    void remove(Integer *p);
    int countObjects() const;
    int countIntegers() const;
    bool shouldPrint() const;
    void setShouldPrint(bool flag);

protected:
    Registry();

private:
    bool m_printStuff;
    std::vector<Obj *> m_objects;
    std::vector<Integer *> m_integers;
};

template <class T>
class RefData {
public:
    RefData(T *ptr) :  m_refCount(1), m_heldPtr(ptr) {}
    ~RefData() { delete m_heldPtr; }
    int inc() { return ++m_refCount; }
    int dec() { return --m_refCount; }
    int useCount() { return m_refCount; }
    int m_refCount;
    T *m_heldPtr;
};

template <class T>
class SharedPtr {
public:
    SharedPtr() : m_refData(0) {
        if (shouldPrint())
            std::cout << "shared_ptr default constructor " << this << "\n";
    }

    SharedPtr(T *v)
    {
        if (shouldPrint())
            std::cout << "shared_ptr constructor " << this << " with pointer " << v << "\n";
        if (v)
            m_refData = new RefData<T>(v);
    }

    SharedPtr(const SharedPtr<T> &other) : m_refData(other.m_refData)
    {
        if (shouldPrint())
            std::cout << "shared_ptr copy constructor " << this << " with pointer "
                      << other.m_refData << "\n";
        if (m_refData)
            m_refData->inc();
    }

    SharedPtr<T> &operator=(const SharedPtr<T>& other)
    {
        if (this != &other) {
            if (shouldPrint())
                std::cout << "shared_ptr assignment operator " << this << " with pointer "
                          << other.m_refData << "\n";
            if (m_refData && m_refData->dec() == 0)
               delete m_refData;
            m_refData = other.m_refData;
            if (m_refData)
                m_refData->inc();
        }
        return *this;
    }

    T *data() const
    {
        if (m_refData)
            return m_refData->m_heldPtr;
        return 0;
    }

    int useCount() const
    {
        if (m_refData)
            return m_refData->useCount();
        return 0;
    }

    void dummyMethod1()
    {

    }

    T& operator*() const
    {
        // Crashes if smart pointer is empty (just like std::shared_ptr).
        return *(m_refData->m_heldPtr);
    }

    T *operator->() const
    {
        if (m_refData)
            return m_refData->m_heldPtr;
        return 0;
    }

    bool operator!() const
    {
        return !m_refData || !m_refData->m_heldPtr;
    }

    bool isNull() const
    {
        return !m_refData || !m_refData->m_heldPtr;
    }

    operator bool() const
    {
        return m_refData && m_refData->m_heldPtr;
    }

    ~SharedPtr()
    {
        if (m_refData) {
            if (shouldPrint())
                std::cout << "shared_ptr destructor " << this << " remaining refcount "
                          << m_refData->useCount() - 1 <<  "\n";
        }
        if (m_refData && m_refData->dec() == 0)
           delete m_refData;
    }

    RefData<T> *m_refData;
};

class LIB_SMART_API Integer {
public:
    Integer();
    Integer(const Integer &other);
    Integer &operator=(const Integer &other);
    ~Integer();
    void printInteger();
    int m_int;
};

// Couldn't name it Object because it caused some namespace clashes.
class LIB_SMART_API Obj {
public:
    Obj();
    virtual ~Obj();

    void printObj();
    Integer takeInteger(Integer val);
    SharedPtr<Obj> giveSharedPtrToObj();
    SharedPtr<Integer> giveSharedPtrToInteger();
    int takeSharedPtrToObj(SharedPtr<Obj> pObj);
    int takeSharedPtrToInteger(SharedPtr<Integer> pInt);

    int m_integer;
    Integer *m_internalInteger;
};

#endif // SMART_H

