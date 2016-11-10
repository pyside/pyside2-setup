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

void TestTemplates::testTemplateWithNamespace()
{
    const char cppCode[] = "\
    struct Url {\
      void name();\
    };\
    namespace Internet {\
        struct Url{};\
        struct Bookmarks {\
            QList<Url> list();\
        };\
    }";
    const char xmlCode0[] = "\
    <typesystem package='Pakcage.Network'>\
        <value-type name='Url' />\
    </typesystem>";

    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(xmlCode0);
    file.close();

    QString xmlCode1 = QString::fromLatin1("\
    <typesystem package='Package.Internet'>\
        <load-typesystem name='%1' generate='no'/>\
        <container-type name='QList' type='list'/> \
        <namespace-type name='Internet' generate='no' />\
        <value-type name='Internet::Url'/>\
        <value-type name='Internet::Bookmarks'/>\
    </typesystem>").arg(file.fileName());

    TestUtil t(cppCode, qPrintable(xmlCode1), false);
    AbstractMetaClassList classes = t.builder()->classes();

    AbstractMetaClass* classB = classes.findClass(QLatin1String("Bookmarks"));
    QVERIFY(classB);
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("list"));
    AbstractMetaType* funcType = func->type();
    QVERIFY(funcType);
    QCOMPARE(funcType->cppSignature(), QLatin1String("QList<Internet::Url >"));
}

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

    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
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
    const char cppCode[] = "\
    template<typename T> struct List() {};\
    void func(List<int> arg) {}\
    ";

    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <primitive-type name='int' />\
        <container-type name='List' type='list' />\
        <function signature='func(List&lt;int&gt;)' />\
    </typesystem>\
    ";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaFunctionList globalFuncs = t.builder()->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int >"));
}

void TestTemplates::testTemplatePointerAsArgument()
{
    const char cppCode[] = "\
    template<typename T> struct List() {};\
    void func(List<int>* arg) {}\
    ";

    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <primitive-type name='int' />\
        <container-type name='List' type='list' />\
        <function signature='func(List&lt;int&gt;*)' />\
    </typesystem>\
    ";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaFunctionList globalFuncs = t.builder()->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>*)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int > *"));
}

void TestTemplates::testTemplateReferenceAsArgument()
{
    const char cppCode[] = "\
    template<typename T> struct List() {};\
    void func(List<int>& arg) {}\
    ";

    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <primitive-type name='int' />\
        <container-type name='List' type='list' />\
        <function signature='func(List&lt;int&gt;&amp;)' />\
    </typesystem>\
    ";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaFunctionList globalFuncs = t.builder()->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.first();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>&)"));
    QCOMPARE(func->arguments().first()->type()->cppSignature(), QLatin1String("List<int > &"));
}

void TestTemplates::testInheritanceFromContainterTemplate()
{
    const char cppCode[] = "\
    template<typename T>\
    struct ListContainer {\
        inline void push_front(const T& t);\
        inline T& front();\
    };\
    struct FooBar {};\
    struct FooBars : public ListContainer<FooBar> {};\
    ";

    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <container-type name='ListContainer' type='list' /> \
        <value-type name='FooBar' />\
        <value-type name='FooBars'>\
            <modify-function signature='push_front(FooBar)' remove='all' />\
            <modify-function signature='front()' remove='all' />\
        </value-type>\
    </typesystem>\
    ";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClassList templates = t.builder()->templates();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(templates.count(), 1);

    const AbstractMetaClass* foobars = classes.findClass(QLatin1String("FooBars"));
    QCOMPARE(foobars->functions().count(), 4);

    const AbstractMetaClass* lc = templates.first();
    QCOMPARE(lc->functions().count(), 2);
}

void TestTemplates::testTemplateInheritanceMixedWithForwardDeclaration()
{
    const char cppCode[] = "\
    enum SomeEnum { E1, E2 };\
    template<SomeEnum type> struct Future;\
    template<SomeEnum type>\
    struct A {\
        A();\
        void method();\
        friend struct Future<type>;\
    };\
    typedef A<E1> B;\
    template<SomeEnum type> struct Future {};\
    ";
    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <enum-type name='SomeEnum' />\
        <value-type name='A' generate='no' />\
        <value-type name='B' />\
        <value-type name='Future' generate='no' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();

    AbstractMetaClass* classB = classes.findClass(QLatin1String("B"));
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTemplateInheritanceMixedWithNamespaceAndForwardDeclaration()
{
    const char cppCode[] = "\
    namespace Namespace {\
    enum SomeEnum { E1, E2 };\
    template<SomeEnum type> struct Future;\
    template<SomeEnum type>\
    struct A {\
        A();\
        void method();\
        friend struct Future<type>;\
    };\
    typedef A<E1> B;\
    template<SomeEnum type> struct Future {};\
    };\
    ";
    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <namespace-type name='Namespace' />\
        <enum-type name='Namespace::SomeEnum' />\
        <value-type name='Namespace::A' generate='no' />\
        <value-type name='Namespace::B' />\
        <value-type name='Namespace::Future' generate='no' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();

    AbstractMetaClass* classB = classes.findClass(QLatin1String("Namespace::B"));
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isNull());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTypedefOfInstantiationOfTemplateClass()
{
    const char cppCode[] = "\
    namespace NSpace {\
    enum ClassType {\
        TypeOne\
    };\
    template<ClassType CLASS_TYPE>\
    struct BaseTemplateClass {\
        inline ClassType getClassType() const { CLASS_TYPE; }\
    };\
    typedef BaseTemplateClass<TypeOne> TypeOneClass;\
    }\
    ";

    const char xmlCode[] = "\
    <typesystem package='Package'>\
        <namespace-type name='NSpace'>\
            <enum-type name='ClassType'/>\
            <object-type name='BaseTemplateClass' generate='no'/>\
            <object-type name='TypeOneClass'/>\
        </namespace-type>\
    </typesystem>\
    ";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 3);

    const AbstractMetaClass* base = classes.findClass(QLatin1String("BaseTemplateClass"));
    QVERIFY(base);
    const AbstractMetaClass* one = classes.findClass(QLatin1String("TypeOneClass"));
    QVERIFY(one);
    QCOMPARE(one->templateBaseClass(), base);
    QCOMPARE(one->functions().count(), base->functions().count());
    QVERIFY(one->isTypeAlias());
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
    const char* cppCode ="\
    template<typename T>\
    class Vector {\
        void method(const Vector& vector);\
        Vector otherMethod();\
    };\
    template <typename T>\
    void Vector<T>::method(const Vector<T>& vector) {}\
    Vector Vector<T>::otherMethod() { return Vector<T>(); }\
    typedef Vector<int> IntVector;\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='int'/>\
        <container-type name='Vector' type='vector'/>\
        <value-type name='IntVector'/>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, true);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 1);

    AbstractMetaClass* vector = classes.findClass(QLatin1String("IntVector"));
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
