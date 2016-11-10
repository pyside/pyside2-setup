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
    QCOMPARE(classes[0]->name(), QLatin1String("ClassName"));
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
    if (classes.first()->name() != QLatin1String("ClassName"))
        classes.swap(0, 1);

    QCOMPARE(classes[0]->name(), QLatin1String("ClassName"));
    QCOMPARE(classes[0]->qualifiedCppName(), QLatin1String("Namespace::ClassName"));
    QCOMPARE(classes[1]->name(), QLatin1String("Namespace"));
    QVERIFY(classes[1]->isNamespace());

    // Check ctors info
    QVERIFY(classes[0]->hasConstructors());
    QCOMPARE(classes[0]->functions().size(), 2); // default ctor + copy ctor

    AbstractMetaFunctionList ctors = classes[0]->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.first()->minimalSignature() != QLatin1String("ClassName()"))
        ctors.swap(0, 1);

    QCOMPARE(ctors[0]->arguments().size(), 0);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("ClassName()"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("ClassName(Namespace::ClassName)"));

    QVERIFY(!classes[0]->hasPrivateDestructor());
    QVERIFY(classes[0]->hasCloneOperator()); // implicit default copy ctor
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
    AbstractMetaClass* a = classes.findClass(QLatin1String("A"));
    AbstractMetaClass* b = classes.findClass(QLatin1String("B"));
    AbstractMetaClass* c = classes.findClass(QLatin1String("C"));

    AbstractMetaClass* no_class = 0;

    QCOMPARE(a->baseClass(), no_class);
    QCOMPARE(b->baseClass(), a);
    QCOMPARE(c->baseClass(), b);

    QCOMPARE(a->functions().size(), 2); // default ctor + the pure virtual method
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
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QCOMPARE(classA->queryFunctionsByName(QLatin1String("method")).count(), 1);
    AbstractMetaFunction* method = classA->queryFunctionsByName(QLatin1String("method")).first();
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
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QCOMPARE(classA->queryFunctionsByName(QLatin1String("method")).count(), 1);
    AbstractMetaFunction* method = classA->queryFunctionsByName(QLatin1String("method")).first();
    AbstractMetaArgument* arg = method->arguments().first();
    QCOMPARE(arg->defaultValueExpression(), QLatin1String("Hello"));
    QCOMPARE(arg->originalDefaultValueExpression(), QLatin1String("A::B()"));
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
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QVERIFY(classA->isPolymorphic());
    AbstractMetaClass* classB = classes.findClass(QLatin1String("A::B"));
    QVERIFY(classB);
    QVERIFY(!classB->isPolymorphic());
}

void TestAbstractMetaClass::testClassDefaultConstructors()
{
    const char* cppCode ="\
    struct A {};\
    \
    struct B {\
        B();\
    private: \
        B(const B&);\
    };\
    \
    struct C {\
        C(const C&);\
    };\
    \
    struct D {\
    private: \
        D(const D&);\
    };\
    \
    struct E {\
    private: \
        ~E();\
    };\
    \
    struct F {\
        F(int, int);\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <primitive-type name='int' />\
        <value-type name='A' /> \
        <object-type name='B' /> \
        <value-type name='C' /> \
        <object-type name='D' /> \
        <object-type name='E' /> \
        <value-type name='F' /> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 6);

    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().size(), 2);

    AbstractMetaFunctionList ctors = classA->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.first()->minimalSignature() != QLatin1String("A()"))
        ctors.swap(0, 1);

    QCOMPARE(ctors[0]->arguments().size(), 0);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("A()"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("A(A)"));

    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classB);
    QCOMPARE(classB->functions().size(), 2);
    QCOMPARE(classB->functions().first()->minimalSignature(), QLatin1String("B()"));

    AbstractMetaClass* classC = classes.findClass(QLatin1String("C"));
    QVERIFY(classC);
    QCOMPARE(classC->functions().size(), 1);
    QCOMPARE(classC->functions().first()->minimalSignature(), QLatin1String("C(C)"));

    AbstractMetaClass* classD = classes.findClass(QLatin1String("D"));
    QVERIFY(classD);
    QCOMPARE(classD->functions().size(), 1);
    QCOMPARE(classD->functions().first()->minimalSignature(), QLatin1String("D(D)"));
    QVERIFY(classD->functions().first()->isPrivate());

    AbstractMetaClass* classE = classes.findClass(QLatin1String("E"));
    QVERIFY(classE);
    QVERIFY(classE->hasPrivateDestructor());
    QCOMPARE(classE->functions().size(), 0);

    AbstractMetaClass* classF = classes.findClass(QLatin1String("F"));
    QVERIFY(classF);

    ctors = classF->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.first()->minimalSignature() != QLatin1String("F(int,int)"))
        ctors.swap(0, 1);

    QCOMPARE(ctors[0]->arguments().size(), 2);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("F(int,int)"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("F(F)"));
}

void TestAbstractMetaClass::testClassInheritedDefaultConstructors()
{
    const char* cppCode ="\
    struct A {\
        A();\
    private: \
        A(const A&);\
    };\
    struct B : public A {};\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <object-type name='A' /> \
        <object-type name='B' /> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaFunctionList ctors = classA->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.first()->minimalSignature() != QLatin1String("A()"))
        ctors.swap(0, 1);

    QCOMPARE(ctors[0]->arguments().size(), 0);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("A()"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("A(A)"));
    QVERIFY(ctors[1]->isPrivate());

    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(classB);

    ctors = classB->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.first()->arguments().size(), 0);
    QCOMPARE(ctors.first()->minimalSignature(), QLatin1String("B()"));
}

void TestAbstractMetaClass::testAbstractClassDefaultConstructors()
{
    const char* cppCode ="\
    struct A {\
        virtual method() = 0;\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <object-type name='A' /> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaFunctionList ctors = classA->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.first()->arguments().size(), 0);
    QCOMPARE(ctors.first()->minimalSignature(), QLatin1String("A()"));
}

void TestAbstractMetaClass::testObjectTypesMustNotHaveCopyConstructors()
{
    const char* cppCode ="struct A {};";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <object-type name='A' /> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaFunctionList ctors = classA->queryFunctions(AbstractMetaClass::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.first()->arguments().size(), 0);
    QCOMPARE(ctors.first()->minimalSignature(), QLatin1String("A()"));
}

void TestAbstractMetaClass::testIsPolymorphic()
{
    const char* cppCode = "\
    class A\
    {\
    public:\
        A();\
        inline bool abc() const {}\
    };\
    \
    class B : public A\
    {\
    public:\
        B();\
        inline bool abc() const {}\
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='bool' />\
        <value-type name='A' />\
        <value-type name='B' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* b = classes.findClass(QLatin1String("A"));

    QVERIFY(!b->isPolymorphic());
    AbstractMetaClass* a = classes.findClass(QLatin1String("B"));
    QVERIFY(!a->isPolymorphic());
}

QTEST_APPLESS_MAIN(TestAbstractMetaClass)
