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

#include "testmodifyfunction.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestModifyFunction::testRenameArgument()
{
    const char* cppCode ="\
    struct A {\
        void method(int=0);\
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
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    const AbstractMetaFunction* func = classA->findFunction(QLatin1String("method"));
    Q_ASSERT(func);

    QCOMPARE(func->argumentName(1), QLatin1String("otherArg"));
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
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("method"));

    QCOMPARE(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0), TypeSystem::CppOwnership);
}


void TestModifyFunction::invalidateAfterUse()
{
    const char* cppCode ="\
    struct A {\
        virtual void call(int *a);\
    };\
    struct B : A {\
    };\
    struct C : B {\
        virtual void call2(int *a);\
    };\
    struct D : C {\
        virtual void call2(int *a);\
    };\
    struct E : D {\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='int'/>\
        <object-type name='A'> \
        <modify-function signature='call(int*)'>\
          <modify-argument index='1' invalidate-after-use='true'/>\
        </modify-function>\
        </object-type>\
        <object-type name='B' /> \
        <object-type name='C'> \
        <modify-function signature='call2(int*)'>\
          <modify-argument index='1' invalidate-after-use='true'/>\
        </modify-function>\
        </object-type>\
        <object-type name='D'> \
        <modify-function signature='call2(int*)'>\
          <modify-argument index='1' invalidate-after-use='true'/>\
        </modify-function>\
        </object-type>\
        <object-type name='E' /> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false, "0.1");
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("call"));
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    AbstractMetaClass* classC = classes.findClass(QLatin1String("C"));
    QVERIFY(classC);
    func = classC->findFunction(QLatin1String("call"));
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    func = classC->findFunction(QLatin1String("call2"));
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    AbstractMetaClass* classD =  classes.findClass(QLatin1String("D"));
    QVERIFY(classD);
    func = classD->findFunction(QLatin1String("call"));
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    func = classD->findFunction(QLatin1String("call2"));
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    AbstractMetaClass* classE = classes.findClass(QLatin1String("E"));
    QVERIFY(classE);
    func = classE->findFunction(QLatin1String("call"));
    QVERIFY(func);
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);

    func = classE->findFunction(QLatin1String("call2"));
    QVERIFY(func);
    QCOMPARE(func->modifications().size(), 1);
    QCOMPARE(func->modifications().at(0).argument_mods.size(), 1);
    QVERIFY(func->modifications().at(0).argument_mods.at(0).resetAfterUse);
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
    TestUtil t(cppCode, xmlCode, false, "0.1");
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("method"));

    QCOMPARE(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0), TypeSystem::CppOwnership);

    func = classB->findFunction(QLatin1String("methodB"));
    QVERIFY(func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0) != TypeSystem::CppOwnership);
}

void TestModifyFunction::testGlobalFunctionModification()
{
    const char* cppCode ="\
    struct A {};\
    void function(A* a = 0);\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='A'/>\
        <function signature='function(A*)'>\
            <modify-function signature='function(A*)'>\
                <modify-argument index='1'>\
                    <replace-type modified-type='A'/>\
                    <replace-default-expression with='A()'/>\
                </modify-argument>\
            </modify-function>\
        </function>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    QCOMPARE(t.builder()->globalFunctions().size(), 1);

    FunctionModificationList mods = TypeDatabase::instance()->functionModifications(QLatin1String("function(A*)"));
    QCOMPARE(mods.count(), 1);
    QList<ArgumentModification> argMods = mods.first().argument_mods;
    QCOMPARE(argMods.count(), 1);
    ArgumentModification argMod = argMods.first();
    QCOMPARE(argMod.replacedDefaultExpression, QLatin1String("A()"));

    const AbstractMetaFunction* func = t.builder()->globalFunctions().first();
    QVERIFY(func);
    QCOMPARE(func->arguments().count(), 1);
    const AbstractMetaArgument* arg = func->arguments().first();
    QCOMPARE(arg->type()->cppSignature(), QLatin1String("A *"));
    QCOMPARE(arg->originalDefaultValueExpression(), QLatin1String("0"));
    QCOMPARE(arg->defaultValueExpression(), QLatin1String("A()"));
}

QTEST_APPLESS_MAIN(TestModifyFunction)
