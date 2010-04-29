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

#include "testtemplates.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestTemplates::testTemplateOnContainers()
{
    const char cppCode[] = "\
    struct Base {};\
    namespace Namespace {\
    enum SomeEnum { E1, E2 };\
    template<SomeEnum type> struct A {\
        A<type> foo(const QList<A<type> >& a);\
    };\
    typedef A<E1> B;\
    }\
    ";
    const char xmlCode[] = "\
    <typesystem package=\"Package\">\
        <container-type name='QList' type='list'/> \
        <namespace-type name='Namespace' />\
        <enum-type name='Namespace::SomeEnum'/>\
        <object-type name='Base' />\
        <object-type name='Namespace::A' generate='no'/> \
        <object-type name='Namespace::B'/> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();

    AbstractMetaClass* classB = classes.findClass("B");
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    const AbstractMetaFunction* func = classB->findFunction("foo");
    AbstractMetaType* argType = func->arguments().first()->type();
    QCOMPARE(argType->instantiations().count(), 1);
    QCOMPARE(argType->typeEntry()->qualifiedCppName(), QString("QList"));

    AbstractMetaType* instance1 = argType->instantiations().first();
    QCOMPARE(instance1->instantiations().count(), 1);
    QCOMPARE(instance1->typeEntry()->qualifiedCppName(), QString("Namespace::A"));

    AbstractMetaType* instance2 = instance1->instantiations().first();
    QCOMPARE(instance2->instantiations().count(), 0);
    QCOMPARE(instance2->typeEntry()->qualifiedCppName(), QString("Namespace::E1"));
}

QTEST_APPLESS_MAIN(TestTemplates)

#include "testtemplates.moc"

