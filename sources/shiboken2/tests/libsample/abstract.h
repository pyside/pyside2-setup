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

#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "libsamplemacros.h"
#include "point.h"
#include "complex.h"

class ObjectType;

// this class is not exported to python
class HideType
{
};

class LIBSAMPLE_API Abstract
{
private:
    enum PrivateEnum {
        PrivValue0,
        PrivValue1,
        PrivValue2 = PrivValue1 + 2
    };
public:
    enum PrintFormat {
        Short,
        Verbose,
        OnlyId,
        ClassNameAndId,
        DummyItemToTestPrivateEnum1 = Abstract::PrivValue1,
        DummyItemToTestPrivateEnum2 = PrivValue2,
    };

    enum Type {
        TpAbstract, TpDerived
    };

    static const int staticPrimitiveField;
    int primitiveField;
    Complex userPrimitiveField;
    Point valueTypeField;
    ObjectType* objectTypeField;

    Abstract(int id = -1);
    virtual ~Abstract();

    inline int id() { return m_id; }

    // factory method
    inline static Abstract* createObject() { return 0; }

    // method that receives an Object Type
    inline static int getObjectId(Abstract* obj) { return obj->id(); }

    virtual void pureVirtual() = 0;
    virtual void* pureVirtualReturningVoidPtr() = 0;
    virtual void unpureVirtual();

    virtual PrintFormat returnAnEnum() = 0;
    void callVirtualGettingEnum(PrintFormat p);
    virtual void virtualGettingAEnum(PrintFormat p);

    void callPureVirtual();
    void callUnpureVirtual();

    void show(PrintFormat format = Verbose);
    virtual Type type() const { return TpAbstract; }

    virtual void hideFunction(HideType* arg) = 0;

protected:
    virtual const char* className() { return "Abstract"; }

    // Protected bit-field structure member.
    unsigned int bitField: 1;

private:
    virtual void pureVirtualPrivate() = 0;
    int m_id;
};
#endif // ABSTRACT_H
