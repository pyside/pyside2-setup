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

#include "testremoveimplconv.h"
#include "testutil.h"
#include <QtTest/QTest>

// When a constructor able to trigger implicity conversions is removed
// it should not appear in the implicity conversion list.
void TestRemoveImplConv::testRemoveImplConv()
{
    const char* cppCode ="\
    struct A {};\
    struct B {};\
    struct C {\
        C(const A&);\
        C(const B&);\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A' /> \
        <value-type name='B' /> \
        <value-type name='C'> \
            <modify-function signature='C(const A&amp;)' remove='all' />\
        </value-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 3);
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classB);
    AbstractMetaClass* classC = classes.findClass(QLatin1String("C"));
    QVERIFY(classC);
    AbstractMetaFunctionList implConv = classC->implicitConversions();
    QCOMPARE(implConv.count(), 1);
    QCOMPARE(implConv.first()->arguments().first()->type()->typeEntry(), classB->typeEntry());
}

QTEST_APPLESS_MAIN(TestRemoveImplConv)
