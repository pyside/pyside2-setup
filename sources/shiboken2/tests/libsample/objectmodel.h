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

#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include "objecttype.h"
#include "libsamplemacros.h"

class LIBSAMPLE_API ObjectModel : public ObjectType
{
public:
    explicit ObjectModel(ObjectType* parent = 0)
        : ObjectType(parent), m_data(0)
    {}

    void setData(ObjectType* data);
    virtual ObjectType* data() const;

    // The MethodCalled enum and related static methods were created to
    // test bug #630 [http://bugs.openbossa.org/show_bug.cgi?id=630]
    enum MethodCalled { ObjectTypeCalled, ObjectModelCalled };
    static MethodCalled receivesObjectTypeFamily(const ObjectType& object) { return ObjectModel::ObjectTypeCalled; }
    static MethodCalled receivesObjectTypeFamily(const ObjectModel& object) { return ObjectModel::ObjectModelCalled; }

private:
    // The model holds only one piece of data.
    // (This is just a test after all.)
    ObjectType* m_data;
};

#endif // OBJECTMODEL_H

