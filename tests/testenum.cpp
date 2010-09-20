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
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <!-- Uses the first value of the enum to identify it. -->\
        <enum-type identified-by-value='Global0'/>\
        <value-type name='A'> \
            <!-- Uses the second value of the enum to identify it. -->\
            <enum-type identified-by-value='A1'/>\
        </value-type> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.first()->typeEntry()->qualifiedCppName(), QString("Global0"));
    QVERIFY(globalEnums.first()->isAnonymous());

    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 1);
    QCOMPARE(classes[0]->enums().first()->typeEntry()->qualifiedCppName(), QString("A::A1"));
    QVERIFY(classes[0]->enums().first()->isAnonymous());
}

QTEST_APPLESS_MAIN(TestEnum)

#include "testenum.moc"
