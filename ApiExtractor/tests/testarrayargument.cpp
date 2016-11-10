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

#include "testarrayargument.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestArrayArgument::testArrayArgumentWithSizeDefinedByInteger()
{
    const char* cppCode ="\
    struct A { \
        enum SomeEnum { Value0, Value1, NValues }; \
        void method(double[3]); \
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='double'/>\
        <object-type name='A'>\
            <enum-type name='SomeEnum'/>\
        </object-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("A"));
    QVERIFY(classA);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), 3);
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
}

void TestArrayArgument::testArrayArgumentWithSizeDefinedByEnumValue()
{
    const char* cppCode ="\
    struct A { \
        enum SomeEnum { Value0, Value1, NValues }; \
        void method(double[NValues]); \
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='double'/>\
        <object-type name='A'>\
            <enum-type name='SomeEnum'/>\
        </object-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* someEnum = classA->findEnum(QLatin1String("SomeEnum"));
    QVERIFY(someEnum);
    AbstractMetaEnumValue* nvalues = classA->findEnumValue(QLatin1String("NValues"), someEnum);
    QVERIFY(nvalues);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), nvalues->value());
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
};

void TestArrayArgument::testArrayArgumentWithSizeDefinedByEnumValueFromGlobalEnum()
{
    const char* cppCode ="\
    enum SomeEnum { Value0, Value1, NValues }; \
    struct A { \
        void method(double[NValues]); \
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='double'/>\
        <enum-type name='SomeEnum'/>\
        <object-type name='A'>\
        </object-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* someEnum = t.builder()->globalEnums().first();
    QVERIFY(someEnum);
    AbstractMetaEnumValue* nvalues = 0;
    foreach (AbstractMetaEnumValue* enumValue, someEnum->values()) {
        if (enumValue->name() == QLatin1String("NValues")) {
            nvalues = enumValue;
            break;
        }
    }
    QVERIFY(nvalues);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), nvalues->value());
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
};

QTEST_APPLESS_MAIN(TestArrayArgument)
