/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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
#include <abstractmetafunction.h>
#include <abstractmetalang.h>
#include <typesystem.h>

void TestAbstractMetaClass::testClassName()
{
    const char* cppCode ="class ClassName {};";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"ClassName\"/></typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->name(), QLatin1String("ClassName"));
}

void TestAbstractMetaClass::testClassNameUnderNamespace()
{
    const char* cppCode ="namespace Namespace { class ClassName {}; }\n";
    const char* xmlCode = R"XML(
    <typesystem package="Foo">
        <namespace-type name="Namespace">
            <value-type name="ClassName"/>
        </namespace-type>
    </typesystem>)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2); // 1 namespace + 1 class
    if (classes.constFirst()->name() != QLatin1String("ClassName"))
        qSwap(classes[0], classes[1]);

    QCOMPARE(classes[0]->name(), QLatin1String("ClassName"));
    QCOMPARE(classes[0]->qualifiedCppName(), QLatin1String("Namespace::ClassName"));
    QCOMPARE(classes[1]->name(), QLatin1String("Namespace"));
    QVERIFY(classes[1]->isNamespace());

    // Check ctors info
    QVERIFY(classes[0]->hasConstructors());
    QCOMPARE(classes[0]->functions().size(), 2); // default ctor + copy ctor

    auto ctors = classes[0]->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.constFirst()->minimalSignature() != QLatin1String("ClassName()"))
        qSwap(ctors[0], ctors[1]);

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

static AbstractMetaFunctionCList virtualFunctions(const AbstractMetaClass *c)
{
    AbstractMetaFunctionCList result;
    const auto &functions = c->functions();
    for (const auto &f : functions) {
        if (f->isVirtual())
            result.append(f);
    }
    return result;
}

void TestAbstractMetaClass::testVirtualMethods()
{
    const char cppCode[] =R"CPP(
class A {
public:
    virtual int pureVirtual() const = 0;
};
class B : public A {};
class C : public B {
public:
    int pureVirtual() const override { return 0; }
};
class F final : public C {
public:
    int pureVirtual() const final { return 1; }
};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package="Foo">
    <primitive-type name='int'/>
    <object-type name='A'/>
    <object-type name='B'/>
    <object-type name='C'/>
    <object-type name='F'/>
</typesystem>
)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 4);
    AbstractMetaClass* a = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    AbstractMetaClass* b = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    AbstractMetaClass* c = AbstractMetaClass::findClass(classes, QLatin1String("C"));
    const AbstractMetaClass *f = AbstractMetaClass::findClass(classes, QLatin1String("F"));
    QVERIFY(f);

    AbstractMetaClass* no_class = nullptr;

    QCOMPARE(a->baseClass(), no_class);
    QCOMPARE(b->baseClass(), a);
    QCOMPARE(c->baseClass(), b);
    QCOMPARE(f->baseClass(), c);

    QCOMPARE(a->functions().size(), 2); // default ctor + the pure virtual method
    QCOMPARE(b->functions().size(), 2);
    QCOMPARE(c->functions().size(), 2);
    QCOMPARE(f->functions().size(), 2);
    QVERIFY(f->attributes() & AbstractMetaAttributes::FinalCppClass);

    // implementing class, ownclass, declaringclass
    const auto ctorA = a->queryFunctions(FunctionQueryOption::Constructors).constFirst();
    const auto ctorB = b->queryFunctions(FunctionQueryOption::Constructors).constFirst();
    const auto ctorC = c->queryFunctions(FunctionQueryOption::Constructors).constFirst();
    QVERIFY(ctorA->isConstructor());
    QVERIFY(!ctorA->isVirtual());
    QVERIFY(ctorB->isConstructor());
    QVERIFY(!ctorB->isVirtual());
    QVERIFY(ctorC->isConstructor());
    QVERIFY(!ctorC->isVirtual());
    QCOMPARE(ctorA->implementingClass(), a);
    QCOMPARE(ctorA->ownerClass(), a);
    QCOMPARE(ctorA->declaringClass(), a);

    const auto virtualFunctionsA = virtualFunctions(a);
    const auto virtualFunctionsB = virtualFunctions(b);
    const auto virtualFunctionsC = virtualFunctions(c);
    const auto virtualFunctionsF = virtualFunctions(f);
    QCOMPARE(virtualFunctionsA.size(), 1); // Add a pureVirtualMethods method !?
    QCOMPARE(virtualFunctionsB.size(), 1);
    QCOMPARE(virtualFunctionsC.size(), 1);
    QCOMPARE(virtualFunctionsF.size(), 1);

    const auto funcA = virtualFunctionsA.constFirst();
    const auto funcB = virtualFunctionsB.constFirst();
    const auto funcC = virtualFunctionsC.constFirst();
    const auto funcF = virtualFunctionsF.constFirst();

    QCOMPARE(funcA->ownerClass(), a);
    QVERIFY(funcC->attributes() & AbstractMetaAttributes::VirtualCppMethod);
    QCOMPARE(funcB->ownerClass(), b);
    QCOMPARE(funcC->ownerClass(), c);
    QVERIFY(funcC->attributes() & AbstractMetaAttributes::OverriddenCppMethod);
    QVERIFY(funcF->attributes() & AbstractMetaAttributes::FinalCppMethod);

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

void TestAbstractMetaClass::testVirtualBase()
{
    const char cppCode[] =R"CPP(
class Base {
public:
    virtual ~Base() = default;
};
class Derived : public Base {};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package="Foo">
    <object-type name='Base'/>
    <object-type name='Derived'/>
</typesystem>
)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    auto base = AbstractMetaClass::findClass(classes, QLatin1String("Base"));
    QVERIFY(base);
    QVERIFY(base->isPolymorphic());
    auto derived = AbstractMetaClass::findClass(classes, QLatin1String("Derived"));
    QVERIFY(derived);
    QVERIFY(derived->isPolymorphic());
}

void TestAbstractMetaClass::testDefaultValues()
{
    const char* cppCode ="\
    struct A {\n\
        class B {};\n\
        void method(B b = B());\n\
    };\n";
    const char* xmlCode = R"XML(
    <typesystem package="Foo">
        <value-type name='A'>
            <value-type name='B'/>
        </value-type>
    </typesystem>)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QCOMPARE(classA->queryFunctionsByName(QLatin1String("method")).count(), 1);
    const auto method = classA->queryFunctionsByName(QLatin1String("method")).constFirst();
    const AbstractMetaArgument &arg = method->arguments().constFirst();
    QCOMPARE(arg.defaultValueExpression(), arg.originalDefaultValueExpression());
}

void TestAbstractMetaClass::testModifiedDefaultValues()
{
    const char* cppCode ="\
    struct A {\n\
        class B {};\n\
        void method(B b = B());\n\
    };\n";
    const char* xmlCode = R"XML(
    <typesystem package="Foo">
        <value-type name='A'>
            <modify-function signature='method(A::B)'>
                <modify-argument index='1'>
                    <replace-default-expression with='Hello'/>
                </modify-argument>
            </modify-function>
            <value-type name='B'/>
        </value-type>
    </typesystem>)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    const auto methodMatches = classA->queryFunctionsByName(QLatin1String("method"));
    QCOMPARE(methodMatches.count(), 1);
    const auto method = methodMatches.constFirst();
    const AbstractMetaArgument &arg = method->arguments().constFirst();
    QCOMPARE(arg.defaultValueExpression(), QLatin1String("Hello"));
    QCOMPARE(arg.originalDefaultValueExpression(), QLatin1String("A::B()"));
}

void TestAbstractMetaClass::testInnerClassOfAPolymorphicOne()
{
    const char* cppCode ="\
    struct A {\n\
        class B {};\n\
        virtual void method();\n\
    };\n";
    const char* xmlCode = R"XML(
    <typesystem package="Foo">
        <object-type name='A'>
            <value-type name='B'/>
        </object-type>
    </typesystem>)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QVERIFY(classA->isPolymorphic());
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("A::B"));
    QVERIFY(classB);
    QVERIFY(!classB->isPolymorphic());
}

void TestAbstractMetaClass::testForwardDeclaredInnerClass()
{
    const char cppCode[] ="\
    class A {\n\
        class B;\n\
    };\n\
    class A::B {\n\
    public:\n\
        void foo();\n\
    };\n";
    const char xmlCode[] = R"XML(
    <typesystem package="Foo">
        <value-type name='A'>
            <value-type name='B'/>
        </value-type>
    </typesystem>)XML";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("A::B"));
    QVERIFY(classB);
    const auto fooF = classB->findFunction(QLatin1String("foo"));
    QVERIFY(!fooF.isNull());
}

void TestAbstractMetaClass::testSpecialFunctions()
{
    const char cppCode[] ="\
    struct A {\n\
        A();\n\
        A(const A&);\n\
        A &operator=(const A&);\n\
    };\n\
    struct B {\n\
        B();\n\
        B(const B &);\n\
        B &operator=(B);\n\
    };\n";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\n\
        <object-type name='A'/>\n\
        <object-type name='B'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);

    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    auto ctors = classA->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    QCOMPARE(ctors.constFirst()->functionType(), AbstractMetaFunction::ConstructorFunction);
    QCOMPARE(ctors.at(1)->functionType(), AbstractMetaFunction::CopyConstructorFunction);
    auto assigmentOps = classA->queryFunctionsByName(QLatin1String("operator="));
    QCOMPARE(assigmentOps.size(), 1);
    QCOMPARE(assigmentOps.constFirst()->functionType(),
             AbstractMetaFunction::AssignmentOperatorFunction);

    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    ctors = classB->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    QCOMPARE(ctors.constFirst()->functionType(), AbstractMetaFunction::ConstructorFunction);
    QCOMPARE(ctors.at(1)->functionType(), AbstractMetaFunction::CopyConstructorFunction);
    assigmentOps = classA->queryFunctionsByName(QLatin1String("operator="));
    QCOMPARE(assigmentOps.size(), 1);
    QCOMPARE(assigmentOps.constFirst()->functionType(), AbstractMetaFunction::AssignmentOperatorFunction);
}

void TestAbstractMetaClass::testClassDefaultConstructors()
{
    const char* cppCode ="\
    struct A {};\n\
    \n\
    struct B {\n\
        B();\n\
    private: \n\
        B(const B&);\n\
    };\n\
    \n\
    struct C {\n\
        C(const C&);\n\
    };\n\
    \n\
    struct D {\n\
    private: \n\
        D(const D&);\n\
    };\n\
    \n\
    struct E {\n\
    private: \n\
        ~E();\n\
    };\n\
    \n\
    struct F {\n\
        F(int, int);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'/>\n\
        <object-type name='B'/>\n\
        <value-type name='C'/>\n\
        <object-type name='D'/>\n\
        <object-type name='E'/>\n\
        <value-type name='F'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 6);

    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().size(), 2);

    auto ctors = classA->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.constFirst()->minimalSignature() != QLatin1String("A()"))
        qSwap(ctors[0], ctors[1]);

    QCOMPARE(ctors[0]->arguments().size(), 0);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("A()"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("A(A)"));

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    QCOMPARE(classB->functions().size(), 2);
    QCOMPARE(classB->functions().constFirst()->minimalSignature(), QLatin1String("B()"));

    AbstractMetaClass* classC = AbstractMetaClass::findClass(classes, QLatin1String("C"));
    QVERIFY(classC);
    QCOMPARE(classC->functions().size(), 1);
    QCOMPARE(classC->functions().constFirst()->minimalSignature(), QLatin1String("C(C)"));

    AbstractMetaClass* classD = AbstractMetaClass::findClass(classes, QLatin1String("D"));
    QVERIFY(classD);
    QCOMPARE(classD->functions().size(), 1);
    QCOMPARE(classD->functions().constFirst()->minimalSignature(), QLatin1String("D(D)"));
    QVERIFY(classD->functions().constFirst()->isPrivate());

    AbstractMetaClass* classE = AbstractMetaClass::findClass(classes, QLatin1String("E"));
    QVERIFY(classE);
    QVERIFY(classE->hasPrivateDestructor());
    QCOMPARE(classE->functions().size(), 0);

    AbstractMetaClass* classF = AbstractMetaClass::findClass(classes, QLatin1String("F"));
    QVERIFY(classF);

    ctors = classF->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.constFirst()->minimalSignature() != QLatin1String("F(int,int)"))
        qSwap(ctors[0], ctors[1]);

    QCOMPARE(ctors[0]->arguments().size(), 2);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("F(int,int)"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("F(F)"));
}

void TestAbstractMetaClass::testClassInheritedDefaultConstructors()
{
    const char* cppCode ="\
    struct A {\n\
        A();\n\
    private: \n\
        A(const A&);\n\
    };\n\
    struct B : public A {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <object-type name='A'/>\n\
        <object-type name='B'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);

    auto ctors = classA->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 2);
    if (ctors.constFirst()->minimalSignature() != QLatin1String("A()"))
        qSwap(ctors[0], ctors[1]);

    QCOMPARE(ctors[0]->arguments().size(), 0);
    QCOMPARE(ctors[0]->minimalSignature(), QLatin1String("A()"));
    QCOMPARE(ctors[1]->arguments().size(), 1);
    QCOMPARE(ctors[1]->minimalSignature(), QLatin1String("A(A)"));
    QVERIFY(ctors[1]->isPrivate());

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);

    ctors = classB->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.constFirst()->arguments().size(), 0);
    QCOMPARE(ctors.constFirst()->minimalSignature(), QLatin1String("B()"));
}

void TestAbstractMetaClass::testAbstractClassDefaultConstructors()
{
    const char* cppCode ="\
    struct A {\n\
        virtual void method() = 0;\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <object-type name='A'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);

    const auto ctors = classA->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.constFirst()->arguments().size(), 0);
    QCOMPARE(ctors.constFirst()->minimalSignature(), QLatin1String("A()"));
}

void TestAbstractMetaClass::testObjectTypesMustNotHaveCopyConstructors()
{
    const char* cppCode ="struct A {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <object-type name='A'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);

    const auto ctors = classA->queryFunctions(FunctionQueryOption::Constructors);
    QCOMPARE(ctors.size(), 1);
    QCOMPARE(ctors.constFirst()->arguments().size(), 0);
    QCOMPARE(ctors.constFirst()->minimalSignature(), QLatin1String("A()"));
}

void TestAbstractMetaClass::testIsPolymorphic()
{
    const char* cppCode = "\
    class A\n\
    {\n\
    public:\n\
        A();\n\
        inline bool abc() const {}\n\
    };\n\
    \n\
    class B : public A\n\
    {\n\
    public:\n\
        B();\n\
        inline bool abc() const {}\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='bool'/>\n\
        <value-type name='A'/>\n\
        <value-type name='B'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* b = AbstractMetaClass::findClass(classes, QLatin1String("A"));

    QVERIFY(!b->isPolymorphic());
    AbstractMetaClass* a = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(!a->isPolymorphic());
}

void TestAbstractMetaClass::testClassTypedefedBaseClass()
{
    const char cppCode[] =R"CPP(
class Base {
};

using BaseAlias1 = Base;
using BaseAlias2 = BaseAlias1;

class Derived : public BaseAlias2 {
};
)CPP";
    const char xmlCode[] = R"XML(
<typesystem package='Foo'>
    <object-type name='Base'/>
    <object-type name='Derived'/>
</typesystem>
)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    auto base = AbstractMetaClass::findClass(classes, QLatin1String("Base"));
    QVERIFY(base);
    auto derived = AbstractMetaClass::findClass(classes, QLatin1String("Derived"));
    QVERIFY(derived);
    QCOMPARE(derived->baseClasses().value(0), base);
}

void TestAbstractMetaClass::testFreeOperators_data()
{
    QTest::addColumn<QByteArray>("code");

    const QByteArray classHeader(R"CPP(
class Value
{
public:
    Value(int v) : m_value(v) {}
    int value() const { return m_value; }
)CPP");

    const QByteArray classFooter(R"CPP(
private:
    int m_value;
};
)CPP");

    const QByteArray multOperatorSignature("Value operator*(const Value &v1, const Value &v2)");
    const QByteArray multOperatorBody("{ return Value(v1.value() * v2.value()); }");
    const QByteArray multOperator =  multOperatorSignature + '\n' + multOperatorBody;

    QTest::newRow("free")
        << (classHeader + classFooter + "\ninline " + multOperator);
    QTest::newRow("free-friend-declared")
        << (classHeader + "\n    friend " + multOperatorSignature + ";\n" + classFooter
            + "\ninline " + multOperator);
    QTest::newRow("hidden friend")
        << (classHeader + "    friend inline " + multOperator + classFooter);
};

void TestAbstractMetaClass::testFreeOperators()
{
    QFETCH(QByteArray, code);
    const char  xmlCode[] = R"XML(
    <typesystem package="Foo">
        <primitive-type name="int"/>
        <value-type name="Value"/>
    </typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(code.constData(), xmlCode));
    QVERIFY(!builder.isNull());
    const auto classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    QVERIFY(classes.constFirst()->hasArithmeticOperatorOverload());
    FunctionQueryOptions opts(FunctionQueryOption::OperatorOverloads);
    QCOMPARE(classes.constFirst()->queryFunctions(opts).size(), 1);
}

QTEST_APPLESS_MAIN(TestAbstractMetaClass)
