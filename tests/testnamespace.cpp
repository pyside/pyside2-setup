/*
* This file is part of the API Extractor project.
*
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "testnamespace.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestNamespace::testNamespaceMembers()
{
    const char* cppCode = "\
    namespace Namespace\
    {\
        enum Option {\
            OpZero,\
            OpOne\
        };\
        void foo(Option opt);\
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <namespace-type name='Namespace' />\
        <enum-type name='Namespace::Option' /> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* ns = classes.findClass("Namespace");
    QVERIFY(ns);
    const AbstractMetaEnum* metaEnum = ns->findEnum("Option");
    QVERIFY(metaEnum);
    const AbstractMetaFunction* func = ns->findFunction("foo");
    QVERIFY(func);
}

QTEST_APPLESS_MAIN(TestNamespace)

#include "testnamespace.moc"

