/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef SMART_SHARED_PTR_H
#define SMART_SHARED_PTR_H

#include "libsmartmacros.h"

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

struct SharedPtrBase
{
    LIB_SMART_API static void logDefaultConstructor(const void *t);
    LIB_SMART_API static void logConstructor(const void *t, const void *pointee);
    LIB_SMART_API static void logCopyConstructor(const void *t, const void *refData);
    LIB_SMART_API static void logAssignment(const void *t, const void *refData);
    LIB_SMART_API static void logDestructor(const void *t, int remainingRefCount);
};

template <class T>
class SharedPtr : public SharedPtrBase {
public:
    SharedPtr() { logDefaultConstructor(this); }

    SharedPtr(T *v)
    {
        logConstructor(this, v);
        if (v)
            m_refData = new RefData<T>(v);
    }

    SharedPtr(const SharedPtr<T> &other) : m_refData(other.m_refData)
    {
        logCopyConstructor(this, other.m_refData);
        if (m_refData)
            m_refData->inc();
    }

    SharedPtr<T> &operator=(const SharedPtr<T>& other)
    {
        if (this != &other) {
            logAssignment(this, other.m_refData);
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
        return m_refData ? m_refData->m_heldPtr : nullptr;
    }

    int useCount() const
    {
        return m_refData ? m_refData->useCount() : 0;
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
        return m_refData ? m_refData->m_heldPtr : nullptr;
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
        if (m_refData)
            logDestructor(this, m_refData->useCount() - 1);
        if (m_refData && m_refData->dec() == 0)
           delete m_refData;
    }

private:
    RefData<T> *m_refData = nullptr;
};

#endif // SMART_SHARED_PTR_H
