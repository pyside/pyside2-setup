/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef REMOVEDNAMESPACE_H
#define REMOVEDNAMESPACE_H

#include "libsamplemacros.h"

namespace RemovedNamespace1
{

enum RemovedNamespace1_Enum { RemovedNamespace1_Enum_Value0 = 0,
                              RemovedNamespace1_Enum_Value1 = 1 };

enum { RemovedNamespace1_AnonymousEnum_Value0 };

inline int mathSum(int x, int y) { return x + y; }

struct ObjectOnInvisibleNamespace
{
    bool exists() const { return true; }
    static int toInt(RemovedNamespace1_Enum e) { return static_cast<int>(e); }
    static ObjectOnInvisibleNamespace consume(const ObjectOnInvisibleNamespace &other) { return other; }
};

namespace RemovedNamespace2
{

enum RemovedNamespace2_Enum { RemovedNamespace2_Enum_Value0 };

} // namespace RemovedNamespace2
} // namespace RemovedNamespace1

namespace UnremovedNamespace
{

namespace RemovedNamespace3
{
    enum RemovedNamespace3_Enum { RemovedNamespace3_Enum_Value0 };

    enum { RemovedNamespace3_AnonymousEnum_Value0 };

    inline int nestedMathSum(int x, int y) { return x + y; }

} // namespace RemovedNamespace3
} // namespace UnremovedNamespace

#endif // REMOVEDNAMESPACE_H

