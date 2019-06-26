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

#ifndef PHOTON_H
#define PHOTON_H

#include <list>
#include "libsamplemacros.h"

// This namespace and classes simulate
// situations found in Qt's phonon module.

namespace Photon
{

enum ClassType {
    BaseType = 0,
    IdentityType = 1,
    DuplicatorType = 2
};

class LIBSAMPLE_API Base
{
public:
    explicit Base(int value) : m_value(value) {}
    virtual ~Base() {}
    inline void setValue(int value) { m_value = value; }
    inline int value() const { return m_value; }

    template <class T> bool isType() { return type() == T::staticType; }
    bool isType(ClassType t) { return type() == t; }

    virtual ClassType type() const { return BaseType; };
    static const ClassType staticType = BaseType;

protected:
    int m_value;
};

template<ClassType CLASS_TYPE>
class LIBSAMPLE_API TemplateBase : public Base
{
public:
    explicit TemplateBase(int value) : Base(value) {}
    inline int multiplicator() const { return (int)CLASS_TYPE; }
    inline int calculate() const { return m_value * ((int)CLASS_TYPE); }
    static inline ClassType classType() { return CLASS_TYPE; }

    inline int sumValueUsingPointer(TemplateBase<CLASS_TYPE>* other) const { return m_value + other->m_value; }
    inline int sumValueUsingReference(TemplateBase<CLASS_TYPE>& other) const { return m_value + other.m_value; }

    inline std::list<TemplateBase<CLASS_TYPE> > getListOfThisTemplateBase()
    {
        std::list<TemplateBase<CLASS_TYPE> > objs;
        objs.push_back(*this);
        objs.push_back(*this);
        return objs;
    }

    static inline TemplateBase<CLASS_TYPE>* passPointerThrough(TemplateBase<CLASS_TYPE>* obj) { return obj; }

    ClassType type() const override { return CLASS_TYPE; }
    static const ClassType staticType = CLASS_TYPE;
};

#if defined _WIN32 || defined __CYGWIN__
template class LIBSAMPLE_API TemplateBase<IdentityType>;
template class LIBSAMPLE_API TemplateBase<DuplicatorType>;
#endif

using ValueIdentity = TemplateBase<IdentityType>;
using ValueDuplicator = TemplateBase<DuplicatorType>;

LIBSAMPLE_API int callCalculateForValueDuplicatorPointer(ValueDuplicator* value);
LIBSAMPLE_API int callCalculateForValueDuplicatorReference(ValueDuplicator& value);
LIBSAMPLE_API int countValueIdentities(const std::list<ValueIdentity>& values);
LIBSAMPLE_API int countValueDuplicators(const std::list<TemplateBase<DuplicatorType> >& values);

// This simulates an internal error (SEGV) caused by 'noexcept' in
// boost::intrusive_ptr before support for 'noexcept' was added. The ENTIRE
// code below is needed to trigger the exception; it isn't seen with just a
// 'noexcept' following a declaration.
//
// NOTE: For reasons that should be fairly obvious, this test unfortunately can
//       only be "run" when building in C++11 mode.
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
#  define PHOTON_NOEXCEPT noexcept
#else
#  define PHOTON_NOEXCEPT
#endif
class Pointer
{
public:
    Pointer() PHOTON_NOEXCEPT : px(nullptr) {}
    Pointer(int* p) : px(p) {}

    void reset() PHOTON_NOEXCEPT { Pointer().swap(*this); }

    int* get() const PHOTON_NOEXCEPT { return px; }
    int& operator*() const { return *px; }

    void swap(Pointer& rhs) PHOTON_NOEXCEPT
    {
        int* tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:
    int* px;
};

} // namespace Photon

#endif // PHOTON_H
