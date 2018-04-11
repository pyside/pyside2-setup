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

#ifndef CONSTVALUELIST_H
#define CONSTVALUELIST_H

#include <list>
#include "libsamplemacros.h"

class CVValueType
{
    CVValueType();
};

typedef std::list<const CVValueType*> const_ptr_value_list;

// This tests binding generation for a container of a const value type. The
// class doesn't need to do anything; this is just to verify that the generated
// binding code (the container conversion in particular) is const-valid.

class CVListUser
{
public:
    static const_ptr_value_list produce() { return const_ptr_value_list(); }
    static void consume(const const_ptr_value_list& l) { (void)l; }
};

#endif // LIST_H
