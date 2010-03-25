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

#include "testabstractmetaclass.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestAbstractMetaClass::testClassName()
{
    const char* cppCode ="class ClassName {};";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"ClassName\"/></typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->name(), QString("ClassName"));
}

void TestAbstractMetaClass::testClassNameUnderNamespace()
{
    const char* cppCode ="namespace Namespace { class ClassName {}; }";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name=\"Namespace\"/> \
        <value-type name=\"Namespace::ClassName\"/> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2); // 1 namespace + 1 class
    if (classes.first()->name() != "ClassName")
        classes.swap(0, 1);

    QCOMPARE(classes[0]->name(), QString("ClassName"));
    QCOMPARE(classes[0]->qualifiedCppName(), QString("Namespace::ClassName"));
    QCOMPARE(classes[1]->name(), QString("Namespace"));
    QVERIFY(classes[1]->isNamespace());

    // Check ctors info
    QVERIFY(classes[0]->hasConstructors());
    QCOMPARE(classes[0]->functions().size(), 1);
    AbstractMetaFunctionList ctors = classes[0]->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors[0]->arguments().size(), 0);
    QVERIFY(!classes[0]->hasPrivateDestructor());

    QVERIFY(classes[0]->hasCloneOperator()); // implicity default copy ctor
    QVERIFY(!classes[0]->hasHashFunction());
    // This method is buggy and nobody wants to fix it or needs it fixed :-/
    // QVERIFY(classes[0]->hasNonPrivateConstructor());
}

void TestAbstractMetaClass::testVirtualMethods()
{
    const char* cppCode ="\
    class A {\
    public:\
        virtual int pureVirtual() const = 0;\
    };\
    class B : public A {};\
    class C : public B {\
    public:\
        int pureVirtual() const { return 0; }\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <primitive-type name='int' />\
        <object-type name='A'/> \
        <object-type name='B'/> \
        <object-type name='C'/> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 3);
    AbstractMetaClass* a = classes.findClass("A");
    AbstractMetaClass* b = classes.findClass("B");
    AbstractMetaClass* c = classes.findClass("C");

    AbstractMetaClass* no_class = 0;

    QCOMPARE(a->baseClass(), no_class);
    QCOMPARE(b->baseClass(), a);
    QCOMPARE(c->baseClass(), b);

    QCOMPARE(a->functions().size(), 2); // implicity ctor + the pv method
    QCOMPARE(b->functions().size(), 2);
    QCOMPARE(c->functions().size(), 2);

    // implementing class, ownclass, declaringclass
    AbstractMetaFunction* ctorA = a->queryFunctions(AbstractMetaClass::Constructors).first();
    AbstractMetaFunction* ctorB = b->queryFunctions(AbstractMetaClass::Constructors).first();
    AbstractMetaFunction* ctorC = c->queryFunctions(AbstractMetaClass::Constructors).first();
    QVERIFY(ctorA->isConstructor());
    QVERIFY(!ctorA->isVirtual());
    QVERIFY(ctorB->isConstructor());
    QVERIFY(!ctorB->isVirtual());
    QVERIFY(ctorC->isConstructor());
    QVERIFY(!ctorC->isVirtual());
    QCOMPARE(ctorA->implementingClass(), a);
    QCOMPARE(ctorA->ownerClass(), a);
    QCOMPARE(ctorA->declaringClass(), a);

    QCOMPARE(a->virtualFunctions().size(), 1); // Add a pureVirtualMethods method !?
    QCOMPARE(b->virtualFunctions().size(), 1);
    QCOMPARE(c->virtualFunctions().size(), 1);

    AbstractMetaFunction* funcA = a->virtualFunctions().first();
    AbstractMetaFunction* funcB = b->virtualFunctions().first();
    AbstractMetaFunction* funcC = c->virtualFunctions().first();

    QCOMPARE(funcA->ownerClass(), a);
    QCOMPARE(funcB->ownerClass(), b);
    QCOMPARE(funcC->ownerClass(), c);

    QCOMPARE(funcA->declaringClass(), a);
    QCOMPARE(funcB->declaringClass(), a);
    QCOMPARE(funcC->declaringClass(), a);

    // The next two tests could return null, because it makes more sense.
    // But we have too many code written relying on this behaviour where
    // implementingClass is equals to declaringClass on pure virtual functions
    QCOMPARE(funcA->implementingClass(), a);
    QCOMPARE(funcB->implementingClass(), a);
    QCOMPARE(funcC->implementingClass(), c);
}

void TestAbstractMetaClass::testDefaultValues()
{
    const char* cppCode ="\
    struct A {\
        class B {};\
        void method(B b = B());\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'/> \
        <value-type name='A::B'/> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = classes.findClass("A");
    QCOMPARE(classA->queryFunctionsByName("method").count(), 1);
    AbstractMetaFunction* method = classA->queryFunctionsByName("method").first();
    AbstractMetaArgument* arg = method->arguments().first();
    QCOMPARE(arg->defaultValueExpression(), arg->originalDefaultValueExpression());
}

void TestAbstractMetaClass::testModifiedDefaultValues()
{
    const char* cppCode ="\
    struct A {\
        class B {};\
        void method(B b = B());\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'> \
        <modify-function signature='method(A::B)'>\
            <modify-argument index='1'>\
                <replace-default-expression with='Hello'/>\
            </modify-argument>\
        </modify-function>\
        </value-type>\
        <value-type name='A::B'/> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = classes.findClass("A");
    QCOMPARE(classA->queryFunctionsByName("method").count(), 1);
    AbstractMetaFunction* method = classA->queryFunctionsByName("method").first();
    AbstractMetaArgument* arg = method->arguments().first();
    QCOMPARE(arg->defaultValueExpression(), QString("Hello"));
    QCOMPARE(arg->originalDefaultValueExpression(), QString("A::B()"));
}

void TestAbstractMetaClass::testInnerClassOfAPolymorphicOne()
{
    const char* cppCode ="\
    struct A {\
        class B {};\
        virtual void method();\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <object-type name='A' /> \
        <value-type name='A::B' /> \
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = classes.findClass("A");
    QVERIFY(classA);
    QVERIFY(classA->isPolymorphic());
    AbstractMetaClass* classB = classes.findClass("A::B");
    QVERIFY(classB);
    QVERIFY(!classB->isPolymorphic());
}

QTEST_APPLESS_MAIN(TestAbstractMetaClass)

#include "testabstractmetaclass.moc"
