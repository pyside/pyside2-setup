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

#ifndef BUCKET_H
#define BUCKET_H

#include "libsamplemacros.h"
#include "objecttype.h"
#include <list>

class ObjectType;

class LIBSAMPLE_API Bucket : public ObjectType
{
public:
    Bucket();
    void push(int);
    int pop();
    bool empty();
    void lock();
    inline bool locked() { return m_locked; }
    void unlock();

    virtual bool virtualBlockerMethod();
    inline bool callVirtualBlockerMethodButYouDontKnowThis() { return virtualBlockerMethod(); }

private:
    std::list<int> m_data;

    volatile bool m_locked;
};

#endif // BUCKET_H

