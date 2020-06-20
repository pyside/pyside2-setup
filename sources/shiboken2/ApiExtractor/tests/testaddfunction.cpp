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

#include "testaddfunction.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestAddFunction::testParsingFuncNameAndConstness()
{
    // generic test...
    const char sig1[] = "func(type1, const type2, const type3* const)";
    AddedFunction f1(QLatin1String(sig1), QLatin1String("void"));
    QCOMPARE(f1.name(), QLatin1String("func"));
    QCOMPARE(f1.arguments().count(), 3);
    AddedFunction::TypeInfo retval = f1.returnType();
    QCOMPARE(retval.name, QLatin1String("void"));
    QCOMPARE(retval.indirections, 0);
    QCOMPARE(retval.isConstant, false);
    QCOMPARE(retval.isReference, false);

    // test with a ugly template as argument and other ugly stuff
    const char sig2[] = "    _fu__nc_       (  type1, const type2, const Abc<int& , C<char*> *   >  * *@my_name@, const type3* const    )   const ";
    AddedFunction f2(QLatin1String(sig2), QLatin1String("const Abc<int& , C<char*> *   >  * *"));
    QCOMPARE(f2.name(), QLatin1String("_fu__nc_"));
    const auto &args = f2.arguments();
    QCOMPARE(args.count(), 4);
    retval = f2.returnType();
    QCOMPARE(retval.name, QLatin1String("Abc<int& , C<char*> *   >"));
    QCOMPARE(retval.indirections, 2);
    QCOMPARE(retval.isConstant, true);
    QCOMPARE(retval.isReference, false);
    retval = args.at(2).typeInfo;
    QVERIFY(args.at(0).name.isEmpty());
    QVERIFY(args.at(1).name.isEmpty());
    QCOMPARE(args.at(2).name, QLatin1String("my_name"));
    QVERIFY(args.at(3).name.isEmpty());
    QCOMPARE(retval.name, QLatin1String("Abc<int& , C<char*> *   >"));
    QCOMPARE(retval.indirections, 2);
    QCOMPARE(retval.isConstant, true);
    QCOMPARE(retval.isReference, false);

    // function with no args.
    const char sig3[] = "func()";
    AddedFunction f3(QLatin1String(sig3), QLatin1String("void"));
    QCOMPARE(f3.name(), QLatin1String("func"));
    QCOMPARE(f3.arguments().count(), 0);
}

void TestAddFunction::testAddFunction()
{
    const char cppCode[] = R"CPP(
struct B {};
struct A {
    void a(int);
};)CPP";
    const char xmlCode[] = R"XML(
<typesystem package='Foo'>
    <primitive-type name='int'/>
    <primitive-type name='float'/>
    <value-type name='B'/>
    <value-type name='A'>
        <add-function signature='b(int, float = 4.6, const B&amp;)' return-type='int' access='protected'/>
        <add-function signature='operator()(int)' return-type='int' access='public'/>
    </value-type>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    TypeDatabase* typeDb = TypeDatabase::instance();
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 5); // default ctor, default copy ctor, func a() and the added functions

    auto addedFunc = classA->findFunction(QLatin1String("b"));
    QVERIFY(addedFunc);
    QCOMPARE(addedFunc->visibility(), AbstractMetaFunction::Protected);
    QCOMPARE(addedFunc->functionType(), AbstractMetaFunction::NormalFunction);
    QVERIFY(addedFunc->isUserAdded());
    QCOMPARE(addedFunc->ownerClass(), classA);
    QCOMPARE(addedFunc->implementingClass(), classA);
    QCOMPARE(addedFunc->declaringClass(), classA);
    QVERIFY(!addedFunc->isVirtual());
    QVERIFY(!addedFunc->isSignal());
    QVERIFY(!addedFunc->isSlot());
    QVERIFY(!addedFunc->isStatic());

    AbstractMetaType* returnType = addedFunc->type();
    QCOMPARE(returnType->typeEntry(), typeDb->findPrimitiveType(QLatin1String("int")));
    AbstractMetaArgumentList args = addedFunc->arguments();
    QCOMPARE(args.count(), 3);
    QCOMPARE(args[0]->type()->typeEntry(), returnType->typeEntry());
    QCOMPARE(args[1]->defaultValueExpression(), QLatin1String("4.6"));
    QCOMPARE(args[2]->type()->typeEntry(), typeDb->findType(QLatin1String("B")));

    auto addedCallOperator = classA->findFunction(QLatin1String("operator()"));
    QVERIFY(addedCallOperator);
}

void TestAddFunction::testAddFunctionConstructor()
{
    const char cppCode[] = "struct A { A() {} };\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'>\n\
            <add-function signature='A(int)'/>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 3); // default and added ctors
    AbstractMetaFunction* addedFunc = classA->functions().last();
    QCOMPARE(addedFunc->visibility(), AbstractMetaFunction::Public);
    QCOMPARE(addedFunc->functionType(), AbstractMetaFunction::ConstructorFunction);
    QCOMPARE(addedFunc->arguments().size(), 1);
    QVERIFY(addedFunc->isUserAdded());
    QVERIFY(!addedFunc->type());
}

void TestAddFunction::testAddFunctionTagDefaultValues()
{
    const char cppCode[] = "struct A {};\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <add-function signature='func()'/>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 3); // default ctor, default copy ctor and the added function
    AbstractMetaFunction* addedFunc = classA->functions().last();
    QCOMPARE(addedFunc->visibility(), AbstractMetaFunction::Public);
    QCOMPARE(addedFunc->functionType(), AbstractMetaFunction::NormalFunction);
    QVERIFY(addedFunc->isUserAdded());
    QVERIFY(!addedFunc->type());
}

void TestAddFunction::testAddFunctionCodeSnippets()
{
    const char cppCode[] = "struct A {};\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <add-function signature='func()'>\n\
                <inject-code class='target' position='end'>Hi!, I am the code.</inject-code>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    AbstractMetaFunction* addedFunc = classA->functions().last();
    QVERIFY(addedFunc->hasInjectedCode());
}

void TestAddFunction::testAddFunctionWithoutParenteses()
{
    const char sig1[] = "func";
    AddedFunction f1(QLatin1String(sig1), QLatin1String("void"));

    QCOMPARE(f1.name(), QLatin1String("func"));
    QCOMPARE(f1.arguments().count(), 0);
    QCOMPARE(f1.isConstant(), false);

    const char cppCode[] = "struct A {};\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <add-function signature='func'>\n\
                <inject-code class='target' position='end'>Hi!, I am the code.</inject-code>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("func"));
    QVERIFY(addedFunc);
    QVERIFY(addedFunc->hasInjectedCode());
    QCOMPARE(addedFunc->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode).count(), 1);
}

void TestAddFunction::testAddFunctionWithDefaultArgs()
{
    const char sig1[] = "func";
    AddedFunction f1(QLatin1String(sig1), QLatin1String("void"));

    QCOMPARE(f1.name(), QLatin1String("func"));
    QCOMPARE(f1.arguments().count(), 0);
    QCOMPARE(f1.isConstant(), false);

    const char cppCode[] = "struct A { };\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'>\n\
            <add-function signature='func(int, int)'>\n\
              <modify-argument index='2'>\n\
                <replace-default-expression with='2'/>\n\
              </modify-argument>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("func"));
    QVERIFY(addedFunc);
    AbstractMetaArgument *arg = addedFunc->arguments()[1];
    QCOMPARE(arg->defaultValueExpression(), QLatin1String("2"));
}

void TestAddFunction::testAddFunctionAtModuleLevel()
{
    const char cppCode[] = "struct A { };\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'/>\n\
        <add-function signature='func(int, int)'>\n\
            <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
        </add-function>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);

    TypeDatabase* typeDb = TypeDatabase::instance();

    AddedFunctionList addedFuncs = typeDb->findGlobalUserFunctions(QLatin1String("func"));

    QCOMPARE(addedFuncs.size(), 1);

    const FunctionModificationList mods = addedFuncs.constFirst()->modifications;

    QCOMPARE(mods.size(), 1);
    QVERIFY(mods.first().isCodeInjection());
    CodeSnip snip = mods.first().snips.first();
    QCOMPARE(snip.code().trimmed(), QLatin1String("custom_code();"));
}

void TestAddFunction::testAddFunctionWithVarargs()
{
    const char sig1[] = "func(int,char,...)";
    AddedFunction f1( QLatin1String(sig1), QLatin1String("void"));

    QCOMPARE(f1.name(), QLatin1String("func"));
    QCOMPARE(f1.arguments().count(), 3);
    QVERIFY(!f1.isConstant());

    const char cppCode[] = "struct A {};\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <primitive-type name='char'/>\n\
        <value-type name='A'>\n\
            <add-function signature='func(int,char,...)'/>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("func"));
    QVERIFY(addedFunc);
    const AbstractMetaArgument* arg = addedFunc->arguments().last();
    QVERIFY(arg->type()->isVarargs());
    QVERIFY(arg->type()->typeEntry()->isVarargs());
}

void TestAddFunction::testAddStaticFunction()
{
    const char cppCode[] = "struct A { };\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'>\n\
            <add-function signature='func(int, int)' static='yes'>\n\
                <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("func"));
    QVERIFY(addedFunc);
    QVERIFY(addedFunc->isStatic());
}

void TestAddFunction::testAddGlobalFunction()
{
    const char cppCode[] = "struct A { };struct B {};\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='A'/>\n\
        <add-function signature='globalFunc(int, int)' static='yes'>\n\
            <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
        </add-function>\n\
        <add-function signature='globalFunc2(int, int)' static='yes'>\n\
            <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
        </add-function>\n\
        <value-type name='B'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 2);
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(builder->classes(), QLatin1String("B"));
    QVERIFY(classB);
    QVERIFY(!classB->findFunction(QLatin1String("globalFunc")));
    QVERIFY(!classB->findFunction(QLatin1String("globalFunc2")));
    QVERIFY(!globalFuncs[0]->injectedCodeSnips().isEmpty());
    QVERIFY(!globalFuncs[1]->injectedCodeSnips().isEmpty());
}

void TestAddFunction::testAddFunctionWithApiVersion()
{
    const char cppCode[] = "";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <add-function signature='globalFunc(int, int)' static='yes' since='1.3'>\n\
            <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
        </add-function>\n\
        <add-function signature='globalFunc2(int, int)' static='yes' since='0.1'>\n\
            <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
        </add-function>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, QLatin1String("0.1")));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);
}

void TestAddFunction::testModifyAddedFunction()
{
    const char cppCode[] = "class Foo { };\n";
    const char xmlCode[] = "\
    <typesystem package='Package'>\n\
        <primitive-type name='float'/>\n\
        <primitive-type name='int'/>\n\
        <value-type name='Foo'>\n\
            <add-function signature='method(float, int)'>\n\
                <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
                <modify-argument index='2'>\n\
                    <replace-default-expression with='0'/>\n\
                    <rename to='varName'/>\n\
                </modify-argument>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClass* foo = AbstractMetaClass::findClass(classes, QLatin1String("Foo"));
    const AbstractMetaFunction* method = foo->findFunction(QLatin1String("method"));
    QCOMPARE(method->arguments().size(), 2);
    AbstractMetaArgument* arg = method->arguments().at(1);
    QCOMPARE(arg->defaultValueExpression(), QLatin1String("0"));
    QCOMPARE(arg->name(), QLatin1String("varName"));
    QCOMPARE(method->argumentName(2), QLatin1String("varName"));
}

void TestAddFunction::testAddFunctionOnTypedef()
{
    const char cppCode[] = "template<class T> class Foo { }; typedef Foo<int> FooInt;\n";
    const char xmlCode[] = "\
    <typesystem package='Package'>\n\
        <custom-type name='PySequence'/>\n\
        <primitive-type name='int'/>\n\
        <value-type name='FooInt'>\n\
            <add-function signature='FooInt(PySequence)'>\n\
                <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
            </add-function>\n\
            <add-function signature='method()'>\n\
                <inject-code class='target' position='beginning'>custom_code();</inject-code>\n\
            </add-function>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClass* foo = AbstractMetaClass::findClass(classes, QLatin1String("FooInt"));
    QVERIFY(foo);
    QVERIFY(foo->hasNonPrivateConstructor());
    const AbstractMetaFunctionList &lst = foo->queryFunctions(AbstractMetaClass::Constructors);
    for (const AbstractMetaFunction *f : lst)
        QVERIFY(f->signature().startsWith(f->name()));
    QCOMPARE(lst.size(), 2);
    const AbstractMetaFunction* method = foo->findFunction(QLatin1String("method"));
    QVERIFY(method);
}

void TestAddFunction::testAddFunctionWithTemplateArg()
{
    const char cppCode[] = "template<class T> class Foo { };\n";
    const char xmlCode[] = "\
    <typesystem package='Package'>\n\
        <primitive-type name='int'/>\n\
        <container-type name='Foo' type='list'/>\n\
        <add-function signature='func(Foo&lt;int>)'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    QCOMPARE(builder->globalFunctions().size(), 1);
    AbstractMetaFunction* func = builder->globalFunctions().first();
    AbstractMetaArgument* arg = func->arguments().first();
    QCOMPARE(arg->type()->instantiations().count(), 1);
}

QTEST_APPLESS_MAIN(TestAddFunction)

