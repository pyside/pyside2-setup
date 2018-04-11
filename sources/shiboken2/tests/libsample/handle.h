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

#ifndef HANDLE_H
#define HANDLE_H

#include "libsamplemacros.h"

/* See http://bugs.pyside.org/show_bug.cgi?id=1105. */
namespace Foo {
    typedef unsigned long HANDLE;
}

class LIBSAMPLE_API OBJ
{
};

typedef OBJ* HANDLE;

class LIBSAMPLE_API HandleHolder
{
public:
    explicit HandleHolder(HANDLE ptr = 0) : m_handle(ptr) {}
    explicit HandleHolder(Foo::HANDLE val): m_handle2(val) {}

    inline void set(HANDLE ptr) { HANDLE tmp; tmp = m_handle;  m_handle = tmp; }
    inline void set(const Foo::HANDLE& val) { m_handle2 = val; }
    inline HANDLE handle() { return m_handle; }
    inline Foo::HANDLE handle2() { return m_handle2; }

    static HANDLE createHandle();
    bool compare(HandleHolder* other);
    bool compare2(HandleHolder* other);

private:
    HANDLE m_handle;
    Foo::HANDLE m_handle2;
};

struct LIBSAMPLE_API PrimitiveStruct {};
typedef struct PrimitiveStruct* PrimitiveStructPtr;
struct LIBSAMPLE_API PrimitiveStructPointerHolder
{
    PrimitiveStructPtr primitiveStructPtr;
};

#endif // HANDLE_H
