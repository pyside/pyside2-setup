/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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

#include "testnumericaltypedef.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestNumericalTypedef::testNumericalTypedef()
{
    const char* cppCode ="\
    typedef double real;\
    void funcDouble(double);\
    void funcReal(real);\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='double' /> \
        <primitive-type name='real' /> \
        <function signature='funcDouble(double)' />\
        <function signature='funcReal(real)' />\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);

    QCOMPARE(t.builder()->globalFunctions().size(), 2);
    const AbstractMetaFunction* funcDouble = t.builder()->globalFunctions().first();
    QVERIFY(funcDouble);
    const AbstractMetaFunction* funcReal = t.builder()->globalFunctions().last();
    QVERIFY(funcReal);

    if (funcDouble->name() == QLatin1String("funcReal"))
        std::swap(funcDouble, funcReal);

    QCOMPARE(funcDouble->minimalSignature(), QLatin1String("funcDouble(double)"));
    QCOMPARE(funcReal->minimalSignature(), QLatin1String("funcReal(real)"));

    const AbstractMetaType* doubleType = funcDouble->arguments().first()->type();
    QVERIFY(doubleType);
    QCOMPARE(doubleType->cppSignature(), QLatin1String("double"));
    QVERIFY(doubleType->isPrimitive());
    QVERIFY(doubleType->typeEntry()->isCppPrimitive());

    const AbstractMetaType* realType = funcReal->arguments().first()->type();
    QVERIFY(realType);
    QCOMPARE(realType->cppSignature(), QLatin1String("real"));
    QVERIFY(realType->isPrimitive());
    QVERIFY(realType->typeEntry()->isCppPrimitive());
}

void TestNumericalTypedef::testUnsignedNumericalTypedef()
{
    const char* cppCode ="\
    typedef unsigned short ushort;\
    void funcUnsignedShort(unsigned short);\
    void funcUShort(ushort);\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='short' /> \
        <primitive-type name='unsigned short' /> \
        <primitive-type name='ushort' /> \
        <function signature='funcUnsignedShort(unsigned short)' />\
        <function signature='funcUShort(ushort)' />\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);

    QCOMPARE(t.builder()->globalFunctions().size(), 2);
    const AbstractMetaFunction* funcUnsignedShort = t.builder()->globalFunctions().first();
    QVERIFY(funcUnsignedShort);
    const AbstractMetaFunction* funcUShort = t.builder()->globalFunctions().last();
    QVERIFY(funcUShort);

    if (funcUnsignedShort->name() == QLatin1String("funcUShort"))
        std::swap(funcUnsignedShort, funcUShort);

    QCOMPARE(funcUnsignedShort->minimalSignature(), QLatin1String("funcUnsignedShort(unsigned short)"));
    QCOMPARE(funcUShort->minimalSignature(), QLatin1String("funcUShort(ushort)"));

    const AbstractMetaType* unsignedShortType = funcUnsignedShort->arguments().first()->type();
    QVERIFY(unsignedShortType);
    QCOMPARE(unsignedShortType->cppSignature(), QLatin1String("unsigned short"));
    QVERIFY(unsignedShortType->isPrimitive());
    QVERIFY(unsignedShortType->typeEntry()->isCppPrimitive());

    const AbstractMetaType* ushortType = funcUShort->arguments().first()->type();
    QVERIFY(ushortType);
    QCOMPARE(ushortType->cppSignature(), QLatin1String("ushort"));
    QVERIFY(ushortType->isPrimitive());
    QVERIFY(ushortType->typeEntry()->isCppPrimitive());
}

QTEST_APPLESS_MAIN(TestNumericalTypedef)

