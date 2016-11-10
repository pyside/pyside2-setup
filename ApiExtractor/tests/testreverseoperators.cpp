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

#include "testreverseoperators.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestReverseOperators::testReverseSum()
{
    const char cppCode[] = "struct A {\
            A& operator+(int);\
        };\
        A& operator+(int, const A&);";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <primitive-type name='int' />\
        <value-type name='A' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 4);

    const AbstractMetaFunction* reverseOp = 0;
    const AbstractMetaFunction* normalOp = 0;
    foreach(const AbstractMetaFunction* func, classA->functions()) {
        if (func->name() == QLatin1String("operator+")) {
            if (func->isReverseOperator())
                reverseOp = func;
            else
                normalOp = func;
        }
    }

    QVERIFY(normalOp);
    QVERIFY(!normalOp->isReverseOperator());
    QCOMPARE(normalOp->arguments().count(), 1);
    QVERIFY(reverseOp);
    QVERIFY(reverseOp->isReverseOperator());
    QCOMPARE(reverseOp->arguments().count(), 1);
}

void TestReverseOperators::testReverseSumWithAmbiguity()
{
    const char cppCode[] = "\
    struct A { A operator+(int); };\
    A operator+(int, const A&);\
    struct B {};\
    B operator+(const A&, const B&);\
    B operator+(const B&, const A&);\
    int operator-(int, const A*);\
    int operator/(const A*, int);\
    ";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <primitive-type name='int' />\
        <value-type name='A' />\
        <value-type name='B' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 6);

    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classB);
    QCOMPARE(classB->functions().count(), 4);

    const AbstractMetaFunction* reverseOp = 0;
    const AbstractMetaFunction* normalOp = 0;
    foreach(const AbstractMetaFunction* func, classB->functions()) {
        if (func->name() == QLatin1String("operator+")) {
            if (func->isReverseOperator())
                reverseOp = func;
            else
                normalOp = func;
        }
    }
    QVERIFY(normalOp);
    QVERIFY(!normalOp->isReverseOperator());
    QCOMPARE(normalOp->arguments().count(), 1);
    QCOMPARE(normalOp->minimalSignature(), QLatin1String("operator+(B,A)"));
    QVERIFY(reverseOp);
    QVERIFY(reverseOp->isReverseOperator());
    QCOMPARE(reverseOp->arguments().count(), 1);
    QCOMPARE(reverseOp->minimalSignature(), QLatin1String("operator+(A,B)"));

    reverseOp = classA->findFunction(QLatin1String("operator-"));
    QVERIFY(reverseOp);
    QCOMPARE(reverseOp->arguments().count(), 1);
    QVERIFY(reverseOp->isPointerOperator());
    QVERIFY(reverseOp->isReverseOperator());

    normalOp = classA->findFunction(QLatin1String("operator/"));
    QVERIFY(normalOp);
    QCOMPARE(normalOp->arguments().count(), 1);
    QVERIFY(normalOp->isPointerOperator());
    QVERIFY(!normalOp->isReverseOperator());

}



QTEST_APPLESS_MAIN(TestReverseOperators)

