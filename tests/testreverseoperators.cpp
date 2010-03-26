/*
* This file is part of the API Extractor project.
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: PySide team <contact@pyside.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

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
    AbstractMetaClass* classA = classes.findClass("A");
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 4);

    const AbstractMetaFunction* reverseOp = 0;
    const AbstractMetaFunction* normalOp = 0;
    foreach(const AbstractMetaFunction* func, classA->functions()) {
        if (func->name() == "operator+") {
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

QTEST_APPLESS_MAIN(TestReverseOperators)

#include "testreverseoperators.moc"
