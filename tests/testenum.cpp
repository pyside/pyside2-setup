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
    QCOMPARE(globalEnums.first()->name(), QString("GlobalEnum"));

    // enum as parameter of a function
    AbstractMetaFunctionList functions = t.builder()->globalFunctions();
    QCOMPARE(functions.count(), 1);
    QCOMPARE(functions.first()->arguments().count(), 1);
    QCOMPARE(functions.first()->arguments().first()->type()->cppSignature(), QString("A::ClassEnum"));

    // enum as parameter of a method
    AbstractMetaClass* classA = classes.findClass("A");
    QCOMPARE(classA->enums().count(), 1);
    AbstractMetaFunction* method = classA->queryFunctionsByName("method").first();
    QVERIFY(method);
    AbstractMetaArgument* arg = method->arguments().first();
    QCOMPARE(arg->type()->name(), QString("ClassEnum"));
    QCOMPARE(arg->type()->cppSignature(), QString("A::ClassEnum"));
    QCOMPARE(functions.first()->arguments().count(), 1);
    arg = functions.first()->arguments().first();
    QCOMPARE(arg->type()->name(), QString("ClassEnum"));
    QCOMPARE(arg->type()->cppSignature(), QString("A::ClassEnum"));

    AbstractMetaEnumList classEnums = classA->enums();
    QCOMPARE(classEnums.first()->name(), QString("ClassEnum"));
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

    TestUtil t(cppCode, xmlCode, true, 0.1);
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
    QCOMPARE(globalEnums.first()->typeEntry()->qualifiedCppName(), QString("Global0"));
    QVERIFY(globalEnums.first()->isAnonymous());

    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 2);

    AbstractMetaEnum* anonEnumA1 = classes[0]->findEnum("A1");
    QVERIFY(anonEnumA1);
    QVERIFY(anonEnumA1->isAnonymous());
    QCOMPARE(anonEnumA1->typeEntry()->qualifiedCppName(), QString("A::A1"));

    AbstractMetaEnumValue* enumValueA0 = anonEnumA1->values().first();
    QCOMPARE(enumValueA0->name(), QString("A0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString(""));

    AbstractMetaEnumValue* enumValueA1 = anonEnumA1->values().last();
    QCOMPARE(enumValueA1->name(), QString("A1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString(""));

    AbstractMetaEnum* anonEnumIsThis = classes[0]->findEnum("isThis");
    QVERIFY(anonEnumIsThis);
    QVERIFY(anonEnumIsThis->isAnonymous());
    QCOMPARE(anonEnumIsThis->typeEntry()->qualifiedCppName(), QString("A::isThis"));

    AbstractMetaEnumValue* enumValueIsThis = anonEnumIsThis->values().first();
    QCOMPARE(enumValueIsThis->name(), QString("isThis"));
    QCOMPARE(enumValueIsThis->value(), static_cast<int>(true));
    QCOMPARE(enumValueIsThis->stringValue(), QString("true"));

    AbstractMetaEnumValue* enumValueIsThat = anonEnumIsThis->values().last();
    QCOMPARE(enumValueIsThat->name(), QString("isThat"));
    QCOMPARE(enumValueIsThat->value(), static_cast<int>(false));
    QCOMPARE(enumValueIsThat->stringValue(), QString("false"));
}

void TestEnum::testGlobalEnums()
{
    const char* cppCode ="\
    enum EnumA { A0, A1 }; \
    enum EnumB { B0 = 2, B1 = 4 }; \
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
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QString("EnumA"));

    AbstractMetaEnumValue* enumValueA0 = enumA->values().first();
    QCOMPARE(enumValueA0->name(), QString("A0"));
    QCOMPARE(enumValueA0->value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString(""));

    AbstractMetaEnumValue* enumValueA1 = enumA->values().last();
    QCOMPARE(enumValueA1->name(), QString("A1"));
    QCOMPARE(enumValueA1->value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString(""));

    AbstractMetaEnum* enumB = globalEnums.last();
    QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QString("EnumB"));

    AbstractMetaEnumValue* enumValueB0 = enumB->values().first();
    QCOMPARE(enumValueB0->name(), QString("B0"));
    QCOMPARE(enumValueB0->value(), 2);
    QCOMPARE(enumValueB0->stringValue(), QString("2"));

    AbstractMetaEnumValue* enumValueB1 = enumB->values().last();
    QCOMPARE(enumValueB1->name(), QString("B1"));
    QCOMPARE(enumValueB1->value(), 4);
    QCOMPARE(enumValueB1->stringValue(), QString("4"));
}

QTEST_APPLESS_MAIN(TestEnum)

#include "testenum.moc"
