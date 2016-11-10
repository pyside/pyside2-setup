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

#include "testenum.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestEnum::testEnumCppSignature()
{
    const char* cppCode ="\
    enum GlobalEnum { A, B };\
    \
    struct A {\
        enum ClassEnum { A, B };\
        void method(ClassEnum);\
    };\
    void func(A::ClassEnum);\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <enum-type name='GlobalEnum' />\
        <value-type name='A'> \
            <enum-type name='ClassEnum' />\
        </value-type> \
        <function signature='func(A::ClassEnum)' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.first()->name(), QLatin1String("GlobalEnum"));

    // enum as parameter of a function
    AbstractMetaFunctionList functions = t.builder()->globalFunctions();
    QCOMPARE(functions.count(), 1);
    QCOMPARE(functions.first()->arguments().count(), 1);
    QCOMPARE(functions.first()->arguments().first()->type()->cppSignature(), QLatin1String("A::ClassEnum"));

    // enum as parameter of a method
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QCOMPARE(classA->enums().count(), 1);
    AbstractMetaFunctionList funcs = classA->queryFunctionsByName(QLatin1String("method"));
    QVERIFY(!funcs.isEmpty());
    AbstractMetaFunction* method = funcs.first();
    QVERIFY(method);
    AbstractMetaArgument* arg = method->arguments().first();
    QCOMPARE(arg->type()->name(), QLatin1String("ClassEnum"));
    QCOMPARE(arg->type()->cppSignature(), QLatin1String("A::ClassEnum"));
    QCOMPARE(functions.first()->arguments().count(), 1);
    arg = functions.first()->arguments().first();
    QCOMPARE(arg->type()->name(), QLatin1String("ClassEnum"));
    QCOMPARE(arg->type()->cppSignature(), QLatin1String("A::ClassEnum"));

    AbstractMetaEnumList classEnums = classA->enums();
    QCOMPARE(classEnums.first()->name(), QLatin1String("ClassEnum"));
}

void TestEnum::testEnumWithApiVersion()
{
    const char* cppCode ="\
    struct A {\
        enum ClassEnum { EnumA, EnumB };\
        enum ClassEnum2 { EnumC, EnumD };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'> \
            <enum-type name='ClassEnum' since='0.1'/>\
            <enum-type name='ClassEnum2' since='0.2'/>\
        </value-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, true, "0.1");
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 1);
}

void TestEnum::testAnonymousEnum()
{
    const char* cppCode ="\
    enum { Global0, Global1 }; \
    struct A {\
        enum { A0, A1 };\
        enum { isThis = true, isThat = false };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <!-- Uses the first value of the enum to identify it. -->\
        <enum-type identified-by-value='Global0'/>\
        <value-type name='A'> \
            <!-- Uses the second value of the enum to identify it. -->\
            <enum-type identified-by-value='A1'/>\
            <enum-type identified-by-value='isThis'/>\
        </value-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.first()->typeEntry()->qualifiedCppName(), QLatin1String("Global0"));
    QVERIFY(globalEnums.first()->isAnonymous());

    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 2);

    AbstractMetaEnum* anonEnumA1 = classes[0]->findEnum(QLatin1String("A1"));
    QVERIFY(anonEnumA1);
    QVERIFY(anonEnumA1->isAnonymous());
    QCOMPARE(anonEnumA1->typeEntry()->qualifiedCppName(), QLatin1String("A::A1"));

    AbstractMetaEnumValue* enumValueA0 = anonEnumA1->values().first();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue* enumValueA1 = anonEnumA1->values().last();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum* anonEnumIsThis = classes[0]->findEnum(QLatin1String("isThis"));
    QVERIFY(anonEnumIsThis);
    QVERIFY(anonEnumIsThis->isAnonymous());
    QCOMPARE(anonEnumIsThis->typeEntry()->qualifiedCppName(), QLatin1String("A::isThis"));

    AbstractMetaEnumValue* enumValueIsThis = anonEnumIsThis->values().first();
    QCOMPARE(enumValueIsThis->name(), QLatin1String("isThis"));
    QCOMPARE(enumValueIsThis->value(), static_cast<int>(true));
    QCOMPARE(enumValueIsThis->stringValue(), QLatin1String("true"));

    AbstractMetaEnumValue* enumValueIsThat = anonEnumIsThis->values().last();
    QCOMPARE(enumValueIsThat->name(), QLatin1String("isThat"));
    QCOMPARE(enumValueIsThat->value(), static_cast<int>(false));
    QCOMPARE(enumValueIsThat->stringValue(), QLatin1String("false"));
}

void TestEnum::testGlobalEnums()
{
    const char* cppCode ="\
    enum EnumA { A0, A1 }; \
    enum EnumB { B0 = 2, B1 = 0x4 }; \
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <enum-type name='EnumA'/>\
        <enum-type name='EnumB'/>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QCOMPARE(globalEnums.count(), 2);

    AbstractMetaEnum* enumA = globalEnums.first();
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("EnumA"));

    AbstractMetaEnumValue* enumValueA0 = enumA->values().first();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue* enumValueA1 = enumA->values().last();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum* enumB = globalEnums.last();
    QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QLatin1String("EnumB"));

    AbstractMetaEnumValue* enumValueB0 = enumB->values().first();
    QCOMPARE(enumValueB0->name(), QLatin1String("B0"));
    QCOMPARE(enumValueB0->value(), 2);
    QCOMPARE(enumValueB0->stringValue(), QLatin1String("2"));

    AbstractMetaEnumValue* enumValueB1 = enumB->values().last();
    QCOMPARE(enumValueB1->name(), QLatin1String("B1"));
    QCOMPARE(enumValueB1->value(), 4);
    QCOMPARE(enumValueB1->stringValue(), QLatin1String("0x4"));
}

void TestEnum::testEnumValueFromNeighbourEnum()
{
    const char* cppCode ="\
    namespace A {\
        enum EnumA { ValueA0, ValueA1 };\
        enum EnumB { ValueB0 = A::ValueA1, ValueB1 = ValueA0 };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name='A'> \
            <enum-type name='EnumA'/>\
            <enum-type name='EnumB'/>\
        </namespace-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 2);

    AbstractMetaEnum* enumA = classes[0]->findEnum(QLatin1String("EnumA"));
    QVERIFY(enumA);
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumA"));

    AbstractMetaEnumValue* enumValueA0 = enumA->values().first();
    QCOMPARE(enumValueA0->name(), QLatin1String("ValueA0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue* enumValueA1 = enumA->values().last();
    QCOMPARE(enumValueA1->name(), QLatin1String("ValueA1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum* enumB = classes[0]->findEnum(QLatin1String("EnumB"));
    QVERIFY(enumB);
    QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumB"));

    AbstractMetaEnumValue* enumValueB0 = enumB->values().first();
    QCOMPARE(enumValueB0->name(), QLatin1String("ValueB0"));
    QCOMPARE(enumValueB0->value(), 1);
    QCOMPARE(enumValueB0->stringValue(), QLatin1String("A::ValueA1"));

    AbstractMetaEnumValue* enumValueB1 = enumB->values().last();
    QCOMPARE(enumValueB1->name(), QLatin1String("ValueB1"));
    QCOMPARE(enumValueB1->value(), 0);
    QCOMPARE(enumValueB1->stringValue(), QLatin1String("ValueA0"));
}

void TestEnum::testEnumValueFromExpression()
{
    const char* cppCode ="\
    struct A {\
        enum EnumA {\
            ValueA0 = 3u,\
            ValueA1 = ~3u,\
            ValueA2 = ~3,\
            ValueA3 = 0xf0,\
            ValueA4 = 8 |ValueA3,\
            ValueA5 = ValueA3|32,\
            ValueA6 = ValueA3 >> 1,\
            ValueA7 = ValueA3 << 1\
        };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'> \
            <enum-type name='EnumA'/>\
        </value-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* enumA = classA->findEnum(QLatin1String("EnumA"));
    QVERIFY(enumA);
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumA"));

    AbstractMetaEnumValue* valueA0 = enumA->values().at(0);
    QCOMPARE(valueA0->name(), QLatin1String("ValueA0"));
    QCOMPARE(valueA0->stringValue(), QLatin1String("3u"));
    QCOMPARE(valueA0->value(), (int) 3u);

    AbstractMetaEnumValue* valueA1 = enumA->values().at(1);
    QCOMPARE(valueA1->name(), QLatin1String("ValueA1"));
    QCOMPARE(valueA1->stringValue(), QLatin1String("~3u"));
    QCOMPARE(valueA1->value(), (int) ~3u);

    AbstractMetaEnumValue* valueA2 = enumA->values().at(2);
    QCOMPARE(valueA2->name(), QLatin1String("ValueA2"));
    QCOMPARE(valueA2->stringValue(), QLatin1String("~3"));
    QCOMPARE(valueA2->value(), ~3);

    AbstractMetaEnumValue* valueA3 = enumA->values().at(3);
    QCOMPARE(valueA3->name(), QLatin1String("ValueA3"));
    QCOMPARE(valueA3->stringValue(), QLatin1String("0xf0"));
    QCOMPARE(valueA3->value(), 0xf0);

    AbstractMetaEnumValue* valueA4 = enumA->values().at(4);
    QCOMPARE(valueA4->name(), QLatin1String("ValueA4"));
    QCOMPARE(valueA4->stringValue(), QLatin1String("8|ValueA3"));
    QCOMPARE(valueA4->value(), 8|0xf0);

    AbstractMetaEnumValue* valueA5 = enumA->values().at(5);
    QCOMPARE(valueA5->name(), QLatin1String("ValueA5"));
    QCOMPARE(valueA5->stringValue(), QLatin1String("ValueA3|32"));
    QCOMPARE(valueA5->value(), 0xf0|32);

    AbstractMetaEnumValue* valueA6 = enumA->values().at(6);
    QCOMPARE(valueA6->name(), QLatin1String("ValueA6"));
    QCOMPARE(valueA6->stringValue(), QLatin1String("ValueA3>>1"));
    QCOMPARE(valueA6->value(), 0xf0 >> 1);

    AbstractMetaEnumValue* valueA7 = enumA->values().at(7);
    QCOMPARE(valueA7->name(), QLatin1String("ValueA7"));
    QCOMPARE(valueA7->stringValue(), QLatin1String("ValueA3<<1"));
    QCOMPARE(valueA7->value(), 0xf0 << 1);
}

void TestEnum::testPrivateEnum()
{
    const char* cppCode ="\
    class A {\
    private:\
        enum PrivateEnum { Priv0 = 0x0f, Priv1 = 0xf0 };\
    public:\
        enum PublicEnum { Pub0 = Priv0, Pub1 = A::Priv1 };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'> \
            <enum-type name='PublicEnum'/>\
        </value-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->enums().count(), 2);

    AbstractMetaEnum* privateEnum = classA->findEnum(QLatin1String("PrivateEnum"));
    QVERIFY(privateEnum);
    QVERIFY(privateEnum->isPrivate());
    QCOMPARE(privateEnum->typeEntry()->qualifiedCppName(), QLatin1String("A::PrivateEnum"));

    AbstractMetaEnum* publicEnum = classA->findEnum(QLatin1String("PublicEnum"));
    QVERIFY(publicEnum);
    QCOMPARE(publicEnum->typeEntry()->qualifiedCppName(), QLatin1String("A::PublicEnum"));

    AbstractMetaEnumValue* pub0 = publicEnum->values().first();
    QCOMPARE(pub0->name(), QLatin1String("Pub0"));
    QCOMPARE(pub0->value(), 0x0f);
    QCOMPARE(pub0->stringValue(), QLatin1String("Priv0"));

    AbstractMetaEnumValue* pub1 = publicEnum->values().last();
    QCOMPARE(pub1->name(), QLatin1String("Pub1"));
    QCOMPARE(pub1->value(), 0xf0);
    QCOMPARE(pub1->stringValue(), QLatin1String("A::Priv1"));
}

void TestEnum::testTypedefEnum()
{
    const char* cppCode ="\
    typedef enum EnumA { \
        A0, \
        A1, \
    } EnumA ; \
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <enum-type name='EnumA'/>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QEXPECT_FAIL("", "APIExtractor does not handle typedef enum correctly yet", Abort);
    QCOMPARE(globalEnums.count(), 1);

    AbstractMetaEnum* enumA = globalEnums.first();
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("EnumA"));

    AbstractMetaEnumValue* enumValueA0 = enumA->values().first();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QLatin1String(""));

    AbstractMetaEnumValue* enumValueA1 = enumA->values().last();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());
}

QTEST_APPLESS_MAIN(TestEnum)
