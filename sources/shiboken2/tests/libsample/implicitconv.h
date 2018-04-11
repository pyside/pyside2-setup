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

#ifndef IMPLICITCONV_H
#define IMPLICITCONV_H

#include "libsamplemacros.h"
#include "null.h"

class ObjectType;

class LIBSAMPLE_API ImplicitConv
{
public:
    enum CtorEnum {
        CtorNone,
        CtorOne,
        CtorTwo,
        CtorThree,
        CtorObjectTypeReference,
        CtorPrimitiveType
    };

    enum ICOverloadedFuncEnum {
        OverFunc_Ii,
        OverFunc_Ib,
        OverFunc_i,
        OverFunc_C
    };

    ImplicitConv() : m_ctorEnum(CtorNone), m_objId(-1), m_value(-1.0) {}
    ImplicitConv(int objId) : m_ctorEnum(CtorOne), m_objId(objId), m_value(-1.0) {}
    ImplicitConv(CtorEnum ctorEnum) : m_ctorEnum(ctorEnum), m_objId(-1), m_value(-1.0) {}
    ImplicitConv(ObjectType&) : m_ctorEnum(CtorObjectTypeReference), m_objId(-1), m_value(-1.0) {}
    ImplicitConv(double value, bool=true) : m_ctorEnum(CtorNone), m_value(value) {}
    ImplicitConv(const Null& null) : m_ctorEnum(CtorPrimitiveType) {}
    ~ImplicitConv() {}

    inline CtorEnum ctorEnum() { return m_ctorEnum; }
    inline int objId() { return m_objId; }
    inline double value() { return m_value; }

    static ImplicitConv implicitConvCommon(ImplicitConv implicit);

    static ImplicitConv implicitConvDefault(ImplicitConv implicit = CtorTwo);

    static ICOverloadedFuncEnum implicitConvOverloading(ImplicitConv implicit, int dummyArg);
    static ICOverloadedFuncEnum implicitConvOverloading(ImplicitConv implicit, bool dummyArg);
    static ICOverloadedFuncEnum implicitConvOverloading(int dummyArg);
    static ICOverloadedFuncEnum implicitConvOverloading(CtorEnum dummyArg);

private:
    CtorEnum m_ctorEnum;
    int m_objId;
    double m_value;
};

#endif // IMPLICITCONV_H
