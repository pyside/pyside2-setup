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

#include "testtemplates.h"
#include <QtTest/QTest>
#include <QTemporaryFile>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestTemplates::testTemplateWithNamespace()
{
    const char cppCode[] = "\n\
    template<typename T> struct QList {}; \n\
    struct Url {\n\
      void name();\n\
    };\n\
    namespace Internet {\n\
        struct Url{};\n\
        struct Bookmarks {\n\
            QList<Url> list();\n\
        };\n\
    }";
    const char xmlCode0[] = "\n\
    <typesystem package='Pakcage.Network'>\n\
        <value-type name='Url'/>\n\
    </typesystem>";

    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(xmlCode0);
    file.close();

    QString xmlCode1 = QString::fromLatin1("\n\
    <typesystem package='Package.Internet'>\n\
        <load-typesystem name='%1' generate='no'/>\n\
        <container-type name='QList' type='list'/>\n\
        <namespace-type name='Internet' generate='no'/>\n\
        <value-type name='Internet::Url'/>\n\
        <value-type name='Internet::Bookmarks'/>\n\
    </typesystem>").arg(file.fileName());

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, qPrintable(xmlCode1), false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("Bookmarks"));
    QVERIFY(classB);
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("list"));
    AbstractMetaType* funcType = func->type();
    QVERIFY(funcType);
    QCOMPARE(funcType->cppSignature(), QLatin1String("QList<Internet::Url >"));
}

void TestTemplates::testTemplateOnContainers()
{
    const char cppCode[] = "\n\
    struct Base {};\n\
    template<typename T> struct QList {}; \n\
    namespace Namespace {\n\
    enum SomeEnum { E1, E2 };\n\
    template<SomeEnum type> struct A {\n\
        A<type> foo(const QList<A<type> >& a);\n\
    };\n\
    typedef A<E1> B;\n\
    }\n\
    ";
    const char xmlCode[] = "\n\
    <typesystem package=\"Package\">\n\
        <container-type name='QList' type='list'/>\n\
        <namespace-type name='Namespace'/>\n\
        <enum-type name='Namespace::SomeEnum'/>\n\
        <object-type name='Base'/>\n\
        <object-type name='Namespace::A' generate='no'/>\n\
        <object-type name='Namespace::B'/>\n\
    </typesystem>";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("foo"));
    AbstractMetaType* argType = func->arguments().first()->type();
    QCOMPARE(argType->instantiations().count(), 1);
    QCOMPARE(argType->typeEntry()->qualifiedCppName(), QLatin1String("QList"));

    const AbstractMetaType* instance1 = argType->instantiations().first();
    QCOMPARE(instance1->instantiations().count(), 1);
    QCOMPARE(instance1->typeEntry()->qualifiedCppName(), QLatin1String("Namespace::A"));

    const AbstractMetaType* instance2 = instance1->instantiations().first();
    QCOMPARE(instance2->instantiations().count(), 0);
    QCOMPARE(instance2->typeEntry()->qualifiedCppName(), QLatin1String("Namespace::E1"));
}

void TestTemplates::testTemplateValueAsArgument()
{
    const char cppCode[] = "\n\
    template<typename T> struct List {};\n\
    void func(List<int> arg) {}\n\
    ";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <primitive-type name='int'/>\n\
        <container-type name='List' type='list'/>\n\
        <function signature='func(List&lt;int&gt;)'/>\n\
    </typesystem>\n\
    ";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int >"));
}

void TestTemplates::testTemplatePointerAsArgument()
{
    const char cppCode[] = "\n\
    template<typename T> struct List {};\n\
    void func(List<int>* arg) {}\n\
    ";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <primitive-type name='int'/>\n\
        <container-type name='List' type='list'/>\n\
        <function signature='func(List&lt;int&gt;*)'/>\n\
    </typesystem>\n\
    ";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>*)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int > *"));
}

void TestTemplates::testTemplateReferenceAsArgument()
{
    const char cppCode[] = "\n\
    template<typename T> struct List {};\n\
    void func(List<int>& arg) {}\n\
    ";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <primitive-type name='int'/>\n\
        <container-type name='List' type='list'/>\n\
        <function signature='func(List&lt;int&gt;&amp;)'/>\n\
    </typesystem>\n\
    ";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>&)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int > &"));
}

void TestTemplates::testTemplateParameterFixup()
{
    const char cppCode[] = "\n\
    template<typename T>\n\
    struct List {\n\
        struct Iterator {};\n\
        void append(List l);\n\
        void erase(List::Iterator it);\n\
    };\n";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <container-type name='List' type='list'/>\n\
        <value-type name='List::Iterator'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    const AbstractMetaClassList templates = builder->templates();

    QCOMPARE(templates.count(), 1);
    const AbstractMetaClass *list = templates.first();
    // Verify that the parameter of "void append(List l)" gets fixed to "List<T >"
    const AbstractMetaFunction *append = list->findFunction(QStringLiteral("append"));
    QVERIFY(append);
    QCOMPARE(append->arguments().size(), 1);
    QCOMPARE(append->arguments().at(0)->type()->cppSignature(), QLatin1String("List<T >"));
    // Verify that the parameter of "void erase(Iterator)" is not modified
    const AbstractMetaFunction *erase = list->findFunction(QStringLiteral("erase"));
    QVERIFY(erase);
    QCOMPARE(erase->arguments().size(), 1);
    QCOMPARE(erase->arguments().at(0)->type()->cppSignature(), QLatin1String("List::Iterator"));
}

void TestTemplates::testInheritanceFromContainterTemplate()
{
    const char cppCode[] = "\n\
    template<typename T>\n\
    struct ListContainer {\n\
        inline void push_front(const T& t);\n\
        inline T& front();\n\
    };\n\
    struct FooBar {};\n\
    struct FooBars : public ListContainer<FooBar> {};\n\
    ";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <container-type name='ListContainer' type='list'/>\n\
        <value-type name='FooBar'/>\n\
        <value-type name='FooBars'>\n\
            <modify-function signature='push_front(FooBar)' remove='all'/>\n\
            <modify-function signature='front()' remove='all'/>\n\
        </value-type>\n\
    </typesystem>\n\
    ";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClassList templates = builder->templates();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(templates.count(), 1);

    const AbstractMetaClass* foobars = AbstractMetaClass::findClass(classes, QLatin1String("FooBars"));
    QCOMPARE(foobars->functions().count(), 4);

    const AbstractMetaClass* lc = templates.first();
    QCOMPARE(lc->functions().count(), 2);
}

void TestTemplates::testTemplateInheritanceMixedWithForwardDeclaration()
{
    const char cppCode[] = "\n\
    enum SomeEnum { E1, E2 };\n\
    template<SomeEnum type> struct Future;\n\
    template<SomeEnum type>\n\
    struct A {\n\
        A();\n\
        void method();\n\
        friend struct Future<type>;\n\
    };\n\
    typedef A<E1> B;\n\
    template<SomeEnum type> struct Future {};\n\
    ";
    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <enum-type name='SomeEnum'/>\n\
        <value-type name='A' generate='no'/>\n\
        <value-type name='B'/>\n\
        <value-type name='Future' generate='no'/>\n\
    </typesystem>";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTemplateInheritanceMixedWithNamespaceAndForwardDeclaration()
{
    const char cppCode[] = "\n\
    namespace Namespace {\n\
    enum SomeEnum { E1, E2 };\n\
    template<SomeEnum type> struct Future;\n\
    template<SomeEnum type>\n\
    struct A {\n\
        A();\n\
        void method();\n\
        friend struct Future<type>;\n\
    };\n\
    typedef A<E1> B;\n\
    template<SomeEnum type> struct Future {};\n\
    };\n\
    ";
    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <namespace-type name='Namespace'/>\n\
        <enum-type name='Namespace::SomeEnum'/>\n\
        <value-type name='Namespace::A' generate='no'/>\n\
        <value-type name='Namespace::B'/>\n\
        <value-type name='Namespace::Future' generate='no'/>\n\
    </typesystem>";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("Namespace::B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTypedefOfInstantiationOfTemplateClass()
{
    const char cppCode[] = "\n\
    namespace NSpace {\n\
    enum ClassType {\n\
        TypeOne\n\
    };\n\
    template<ClassType CLASS_TYPE>\n\
    struct BaseTemplateClass {\n\
        inline ClassType getClassType() const { CLASS_TYPE; }\n\
    };\n\
    typedef BaseTemplateClass<TypeOne> TypeOneClass;\n\
    }\n\
    ";

    const char xmlCode[] = "\n\
    <typesystem package='Package'>\n\
        <namespace-type name='NSpace'>\n\
            <enum-type name='ClassType'/>\n\
            <object-type name='BaseTemplateClass' generate='no'/>\n\
            <object-type name='TypeOneClass'/>\n\
        </namespace-type>\n\
    </typesystem>\n\
    ";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);

    const AbstractMetaClass* base = AbstractMetaClass::findClass(classes, QLatin1String("BaseTemplateClass"));
    QVERIFY(base);
    const AbstractMetaClass* one = AbstractMetaClass::findClass(classes, QLatin1String("TypeOneClass"));
    QVERIFY(one);
    QCOMPARE(one->templateBaseClass(), base);
    QCOMPARE(one->functions().count(), base->functions().count());
    QVERIFY(one->isTypeDef());
    const ComplexTypeEntry* oneType = one->typeEntry();
    const ComplexTypeEntry* baseType = base->typeEntry();
    QCOMPARE(oneType->baseContainerType(), baseType);
    QCOMPARE(one->baseClassNames(), QStringList(QLatin1String("BaseTemplateClass<TypeOne>")));

    QVERIFY(one->hasTemplateBaseClassInstantiations());
    AbstractMetaTypeList instantiations = one->templateBaseClassInstantiations();
    QCOMPARE(instantiations.count(), 1);
    const AbstractMetaType* inst = instantiations.first();
    QVERIFY(inst);
    QVERIFY(!inst->isEnum());
    QVERIFY(!inst->typeEntry()->isEnum());
    QVERIFY(inst->typeEntry()->isEnumValue());
    QCOMPARE(inst->cppSignature(), QLatin1String("NSpace::TypeOne"));
}

void TestTemplates::testContainerTypeIncompleteArgument()
{
    const char* cppCode ="\n\
    template<typename T>\n\
    class Vector {\n\
        void method(const Vector& vector);\n\
        Vector otherMethod();\n\
    };\n\
    template <typename T>\n\
    void Vector<T>::method(const Vector<T>& vector) {}\n\
    template <typename T>\n\
    Vector<T> Vector<T>::otherMethod() { return Vector<T>(); }\n\
    typedef Vector<int> IntVector;\n\
    ";
    const char* xmlCode = "\n\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <container-type name='Vector' type='vector'/>\n\
        <value-type name='IntVector'/>\n\
    </typesystem>";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);

    AbstractMetaClass* vector = AbstractMetaClass::findClass(classes, QLatin1String("IntVector"));
    QVERIFY(vector);
    QVERIFY(vector->typeEntry()->baseContainerType());
    QCOMPARE(reinterpret_cast<const ContainerTypeEntry*>(vector->typeEntry()->baseContainerType())->type(), ContainerTypeEntry::VectorContainer);
    QCOMPARE(vector->functions().count(), 4);

    const AbstractMetaFunction* method = vector->findFunction(QLatin1String("method"));
    QVERIFY(method);
    QCOMPARE(method->signature(), QLatin1String("method(const Vector<int > & vector)"));

    const AbstractMetaFunction* otherMethod = vector->findFunction(QLatin1String("otherMethod"));
    QVERIFY(otherMethod);
    QCOMPARE(otherMethod->signature(), QLatin1String("otherMethod()"));
    QVERIFY(otherMethod->type());
    QCOMPARE(otherMethod->type()->cppSignature(), QLatin1String("Vector<int >"));
}

QTEST_APPLESS_MAIN(TestTemplates)
