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

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <list>
#include "libsamplemacros.h"

#include "objecttype.h"

class LIBSAMPLE_API Collector
{
public:
    Collector() {}
    virtual ~Collector() {}

    void clear();

    Collector& operator<<(ObjectType::Identifier item);

    Collector& operator<<(const ObjectType *);

    std::list<ObjectType::Identifier> items();
    int size();

private:
    std::list<ObjectType::Identifier> m_items;

    Collector(const Collector&);
    Collector& operator=(const Collector&);
};

/* Helper for testing external operators */
class IntWrapper
{
public:
    IntWrapper(int x=0):value(x){}

    int value;
};

LIBSAMPLE_API Collector &operator<<(Collector&, const IntWrapper&);

#endif // COLLECTOR_H

