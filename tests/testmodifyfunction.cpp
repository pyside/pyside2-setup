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

#include "testmodifyfunction.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestModifyFunction::testRenameArgument()
{
    const char* cppCode ="\
    struct A {\
        void method(int myarg);\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='int'/>\
        <object-type name='A'> \
        <modify-function signature='method(int)'>\
            <modify-argument index='1'>\
                <rename to='otherArg' />\
            </modify-argument>\
        </modify-function>\
        </object-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass("A");
    const AbstractMetaFunction* func = classA->findFunction("method");
    Q_ASSERT(func);

    FunctionModificationList modList = func->modifications(classA);
    QVERIFY(modList.size() == 1);
    FunctionModification mod = modList.at(0);
    QVERIFY(mod.argument_mods.size() == 1);

    QCOMPARE(mod.argument_mods.at(0).renamed_to, QString("otherArg"));
}

void TestModifyFunction::testOwnershipTransfer()
{
    const char* cppCode ="\
    struct A {};\
    struct B {\
        virtual A* method();\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <object-type name='A' /> \
        <object-type name='B'> \
        <modify-function signature='method()'>\
            <modify-argument index='return'>\
                <define-ownership owner='c++' /> \
            </modify-argument>\
        </modify-function>\
        </object-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classB = classes.findClass("B");
    const AbstractMetaFunction* func = classB->findFunction("method");

    QCOMPARE(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0), TypeSystem::CppOwnership);
}

void TestModifyFunction::testWithApiVersion()
{
    const char* cppCode ="\
    struct A {};\
    struct B {\
        virtual A* method();\
        virtual B* methodB();\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <object-type name='A' /> \
        <object-type name='B'> \
        <modify-function signature='method()' since='0.1'>\
            <modify-argument index='return'>\
                <define-ownership owner='c++' /> \
            </modify-argument>\
        </modify-function>\
        <modify-function signature='methodB()' since='0.2'>\
            <modify-argument index='return'>\
                <define-ownership owner='c++' /> \
            </modify-argument>\
        </modify-function>\
        </object-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false, 0.1);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classB = classes.findClass("B");
    const AbstractMetaFunction* func = classB->findFunction("method");

    QCOMPARE(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0), TypeSystem::CppOwnership);

    func = classB->findFunction("methodB");
    QVERIFY(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0) != TypeSystem::CppOwnership);
}


QTEST_APPLESS_MAIN(TestModifyFunction)

#include "testmodifyfunction.moc"
