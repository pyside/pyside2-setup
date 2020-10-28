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

#include <memory>

#include "libsmartmacros.h"

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

    SharedPtr(T *v) : mPtr(v)
    {
        logConstructor(this, v);
    }

    SharedPtr(const SharedPtr<T> &other) : mPtr(other.mPtr)
    {
        logCopyConstructor(this, data());
    }

    template<class X>
    SharedPtr(const SharedPtr<X> &other) : mPtr(other.mPtr)
    {
        logCopyConstructor(this, data());
    }

    SharedPtr& operator=(const SharedPtr& other)
    {
        mPtr = other.mPtr;
        return *this;
    }

    T *data() const
    {
        return mPtr.get();
    }

    int useCount() const
    {
        return mPtr.use_count();
    }

    void dummyMethod1()
    {
    }

    bool isNull() const
    {
        return mPtr.get() == nullptr;
    }

    T& operator*() const
    {
        // Crashes if smart pointer is empty (just like std::shared_ptr).
        return *mPtr;
    }

    T *operator->() const
    {
        return mPtr.get();
    }

    bool operator!() const
    {
        return !mPtr;
    }

    ~SharedPtr()
    {
        if (mPtr.use_count() >= 1)
            logDestructor(this, mPtr.use_count() - 1);
    }

    std::shared_ptr<T> mPtr;
};

#endif // SMART_SHARED_PTR_H
