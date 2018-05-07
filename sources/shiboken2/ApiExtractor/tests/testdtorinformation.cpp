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

#include "testdtorinformation.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestDtorInformation::testDtorIsPrivate()
{
    const char* cppCode ="class Control { public: ~Control() {} }; class Subject { private: ~Subject() {} };";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"Control\"/><value-type name=\"Subject\"/></typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Control"))->hasPrivateDestructor(), false);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Subject"))->hasPrivateDestructor(), true);
}

void TestDtorInformation::testDtorIsProtected()
{
    const char* cppCode ="class Control { public: ~Control() {} }; class Subject { protected: ~Subject() {} };";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"Control\"/><value-type name=\"Subject\"/></typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Control"))->hasProtectedDestructor(), false);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Subject"))->hasProtectedDestructor(), true);
}

void TestDtorInformation::testDtorIsVirtual()
{
    const char* cppCode ="class Control { public: ~Control() {} }; class Subject { protected: virtual ~Subject() {} };";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"Control\"/><value-type name=\"Subject\"/></typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Control"))->hasVirtualDestructor(), false);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Subject"))->hasVirtualDestructor(), true);
}

void TestDtorInformation::testClassWithVirtualDtorIsPolymorphic()
{
    const char* cppCode ="class Control { public: virtual ~Control() {} }; class Subject { protected: virtual ~Subject() {} };";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"Control\"/><value-type name=\"Subject\"/></typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Control"))->isPolymorphic(), true);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Subject"))->isPolymorphic(), true);
}

QTEST_APPLESS_MAIN(TestDtorInformation)


