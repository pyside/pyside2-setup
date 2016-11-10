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

#include "testconversionoperator.h"
#include <QtTest/QTest>
#include "testutil.h"


void TestConversionOperator::testConversionOperator()
{
    const char cppCode[] = "\
    struct A {\
    };\
    struct B {\
        operator A() const;\
    };\
    struct C {\
        operator A() const;\
    };";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <value-type name='A' />\
        <value-type name='B' />\
        <value-type name='C' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    AbstractMetaClass* classC = classes.findClass(QLatin1String("C"));
    QVERIFY(classA);
    QVERIFY(classB);
    QVERIFY(classC);
    QCOMPARE(classA->functions().count(), 2);
    QCOMPARE(classB->functions().count(), 3);
    QCOMPARE(classC->functions().count(), 3);
    QCOMPARE(classA->externalConversionOperators().count(), 2);

    AbstractMetaFunction* convOp = 0;
    foreach(AbstractMetaFunction* func, classB->functions()) {
        if (func->isConversionOperator()) {
            convOp = func;
            break;
        }
    }
    QVERIFY(convOp);
    QVERIFY(classA->externalConversionOperators().contains(convOp));
}

void TestConversionOperator::testConversionOperatorOfDiscardedClass()
{
    const char cppCode[] = "\
    struct A {\
    };\
    struct B {\
        operator A() const;\
    };";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <value-type name='A' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->externalConversionOperators().count(), 0);
}

void TestConversionOperator::testRemovedConversionOperator()
{
    const char cppCode[] = "\
    struct A {\
    };\
    struct B {\
        operator A() const;\
    };";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <value-type name='A' />\
        <value-type name='B'>\
            <modify-function signature='operator A() const' remove='all' />\
        </value-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classA);
    QVERIFY(classB);
    QCOMPARE(classA->functions().count(), 2);
    QCOMPARE(classB->functions().count(), 3);
    QCOMPARE(classA->externalConversionOperators().count(), 0);
    QCOMPARE(classA->implicitConversions().count(), 0);
}

void TestConversionOperator::testConversionOperatorReturningReference()
{
    const char cppCode[] = "\
    struct A {};\
    struct B {\
        operator A&() const;\
    };";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\
        <value-type name='A' />\
        <value-type name='B' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classA);
    QVERIFY(classB);
    QCOMPARE(classA->functions().count(), 2);
    QCOMPARE(classB->functions().count(), 3);
    QCOMPARE(classA->externalConversionOperators().count(), 1);
    QCOMPARE(classA->externalConversionOperators().first()->type()->cppSignature(), QLatin1String("A"));
    QCOMPARE(classA->externalConversionOperators().first()->ownerClass()->name(), QLatin1String("B"));
    QCOMPARE(classA->implicitConversions().count(), 1);
    QCOMPARE(classA->implicitConversions().first()->type()->cppSignature(), QLatin1String("A"));
    QCOMPARE(classA->implicitConversions().first()->ownerClass()->name(), QLatin1String("B"));
}

void TestConversionOperator::testConversionOperatorReturningConstReference()
{
    const char cppCode[] = "\
    struct A {};\
    struct B {\
        operator const A&() const;\
    };";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\
        <value-type name='A' />\
        <value-type name='B' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classA);
    QVERIFY(classB);
    QCOMPARE(classA->functions().count(), 2);
    QCOMPARE(classB->functions().count(), 3);
    QCOMPARE(classA->externalConversionOperators().count(), 1);
    QCOMPARE(classA->externalConversionOperators().first()->type()->cppSignature(), QLatin1String("A"));
    QCOMPARE(classA->externalConversionOperators().first()->ownerClass()->name(), QLatin1String("B"));
    QCOMPARE(classA->implicitConversions().count(), 1);
    QCOMPARE(classA->implicitConversions().first()->type()->cppSignature(), QLatin1String("A"));
    QCOMPARE(classA->implicitConversions().first()->ownerClass()->name(), QLatin1String("B"));
}

QTEST_APPLESS_MAIN(TestConversionOperator)
