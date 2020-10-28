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

#include "testtemplates.h"
#include <QtTest/QTest>
#include <QtCore/QTextStream>
#include <QTemporaryFile>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestTemplates::testTemplateWithNamespace()
{
    const char cppCode[] = R"CPP(
template<typename T> struct QList {};
struct Url {
  void name();
};
namespace Internet {
    struct Url{};
    struct Bookmarks {
        QList<Url> list();
    };
};
)CPP";

    const char xmlCode0[] = R"XML(
<typesystem package='Package.Network'>
    <value-type name='Url'/>
</typesystem>)XML";

    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(xmlCode0);
    file.close();

    QString xmlCode1 = QString::fromLatin1(R"XML(
<typesystem package='Package.Internet'>
    <load-typesystem name='%1' generate='no'/>
    <container-type name='QList' type='list'/>
    <namespace-type name='Internet' generate='no'>
        <value-type name='Url'/>
        <value-type name='Bookmarks'/>
    </namespace-type>
</typesystem>)XML").arg(file.fileName());

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, qPrintable(xmlCode1), false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("Bookmarks"));
    QVERIFY(classB);
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("list"));
    AbstractMetaType funcType = func->type();
    QVERIFY(funcType);
    QCOMPARE(funcType.cppSignature(), QLatin1String("QList<Internet::Url >"));
}

void TestTemplates::testTemplateOnContainers()
{
    const char cppCode[] = R"CPP(
struct Base {};
template<typename T> struct QList {};
namespace Namespace {
    enum SomeEnum { E1, E2 };
    template<SomeEnum type> struct A {
        A<type> foo(const QList<A<type> >& a);
    };
    typedef A<E1> B;
}
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package="Package">
    <container-type name='QList' type='list'/>
    <namespace-type name='Namespace'>
       <enum-type name='SomeEnum'/>
       <object-type name='A' generate='no'/>
       <object-type name='B'/>
    </namespace-type>
    <object-type name='Base'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isEmpty());
    const AbstractMetaFunction* func = classB->findFunction(QLatin1String("foo"));
    AbstractMetaType argType = func->arguments().constFirst().type();
    QCOMPARE(argType.instantiations().count(), 1);
    QCOMPARE(argType.typeEntry()->qualifiedCppName(), QLatin1String("QList"));

    const AbstractMetaType &instance1 = argType.instantiations().constFirst();
    QCOMPARE(instance1.instantiations().count(), 1);
    QCOMPARE(instance1.typeEntry()->qualifiedCppName(), QLatin1String("Namespace::A"));

    const AbstractMetaType &instance2 = instance1.instantiations().constFirst();
    QCOMPARE(instance2.instantiations().count(), 0);
    QCOMPARE(instance2.typeEntry()->qualifiedCppName(), QLatin1String("Namespace::E1"));
}

void TestTemplates::testTemplateValueAsArgument()
{
    const char cppCode[] = R"CPP(
template<typename T> struct List {};
void func(List<int> arg) {}
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <primitive-type name='int'/>
    <container-type name='List' type='list'/>
    <function signature='func(List&lt;int&gt;)'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction *func = globalFuncs.constFirst();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>)"));
    QCOMPARE(func->arguments().constFirst().type().cppSignature(),
             QLatin1String("List<int >"));
}

void TestTemplates::testTemplatePointerAsArgument()
{
    const char cppCode[] = R"CPP(
template<typename T> struct List {};
void func(List<int>* arg) {}
)CPP";

    const char xmlCode[] = R"XML(
 <typesystem package='Package'>
     <primitive-type name='int'/>
     <container-type name='List' type='list'/>
     <function signature='func(List&lt;int&gt;*)'/>
 </typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.constFirst();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>*)"));
    QCOMPARE(func->arguments().constFirst().type().cppSignature(),
             QLatin1String("List<int > *"));
}

void TestTemplates::testTemplateReferenceAsArgument()
{
    const char cppCode[] = R"CPP(
template<typename T> struct List {};
void func(List<int>& arg) {}
    )CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <primitive-type name='int'/>
    <container-type name='List' type='list'/>
    <function signature='func(List&lt;int&gt;&amp;)'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaFunctionList globalFuncs = builder->globalFunctions();
    QCOMPARE(globalFuncs.count(), 1);

    AbstractMetaFunction* func = globalFuncs.constFirst();
    QCOMPARE(func->minimalSignature(), QLatin1String("func(List<int>&)"));
    QCOMPARE(func->arguments().constFirst().type().cppSignature(),
             QLatin1String("List<int > &"));
}

void TestTemplates::testTemplateParameterFixup()
{
    const char cppCode[] = R"CPP(
template<typename T>
struct List {
    struct Iterator {};
    void append(List l);
    void erase(List::Iterator it);
};
)CPP";

    const char xmlCode[] = R"XML(
 <typesystem package='Package'>
     <container-type name='List' type='list'>
         <value-type name='Iterator'/>
     </container-type>
 </typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    const AbstractMetaClassList templates = builder->templates();

    QCOMPARE(templates.count(), 1);
    const AbstractMetaClass *list = templates.constFirst();
    // Verify that the parameter of "void append(List l)" gets fixed to "List<T >"
    const AbstractMetaFunction *append = list->findFunction(QStringLiteral("append"));
    QVERIFY(append);
    QCOMPARE(append->arguments().size(), 1);
    QCOMPARE(append->arguments().at(0).type().cppSignature(), QLatin1String("List<T >"));
    // Verify that the parameter of "void erase(Iterator)" is not modified
    const AbstractMetaFunction *erase = list->findFunction(QStringLiteral("erase"));
    QVERIFY(erase);
    QCOMPARE(erase->arguments().size(), 1);
    QEXPECT_FAIL("", "Clang: Some other code changes the parameter type", Abort);
    QCOMPARE(erase->arguments().at(0).type().cppSignature(), QLatin1String("List::Iterator"));
}

void TestTemplates::testInheritanceFromContainterTemplate()
{
    const char cppCode[] = R"CPP(
template<typename T>
struct ListContainer {
    inline void push_front(const T& t);
    inline T& front();
};
struct FooBar {};
struct FooBars : public ListContainer<FooBar> {};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <container-type name='ListContainer' type='list'/>
    <value-type name='FooBar'/>
    <value-type name='FooBars'>
        <modify-function signature='push_front(FooBar)' remove='all'/>
        <modify-function signature='front()' remove='all'/>
    </value-type>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClassList templates = builder->templates();
    QCOMPARE(classes.count(), 2);
    QCOMPARE(templates.count(), 1);

    const AbstractMetaClass* foobars = AbstractMetaClass::findClass(classes, QLatin1String("FooBars"));
    QCOMPARE(foobars->functions().count(), 4);

    const AbstractMetaClass *lc = templates.constFirst();
    QCOMPARE(lc->functions().count(), 2);
}

void TestTemplates::testTemplateInheritanceMixedWithForwardDeclaration()
{
    const char cppCode[] = R"CPP(
enum SomeEnum { E1, E2 };
template<SomeEnum type> struct Future;
template<SomeEnum type>
struct A {
    A();
    void method();
    friend struct Future<type>;
};
typedef A<E1> B;
template<SomeEnum type> struct Future {};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <enum-type name='SomeEnum'/>
    <value-type name='A' generate='no'/>
    <value-type name='B'/>
    <value-type name='Future' generate='no'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isEmpty());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTemplateInheritanceMixedWithNamespaceAndForwardDeclaration()
{
    const char cppCode[] = R"CPP(
namespace Namespace {
enum SomeEnum { E1, E2 };
template<SomeEnum type> struct Future;
template<SomeEnum type>
struct A {
    A();
    void method();
    friend struct Future<type>;
};
typedef A<E1> B;
template<SomeEnum type> struct Future {};
};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <namespace-type name='Namespace'>
        <enum-type name='SomeEnum'/>
        <value-type name='A' generate='no'/>
        <value-type name='B'/>
        <value-type name='Future' generate='no'/>
    </namespace-type>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("Namespace::B"));
    QVERIFY(classB);
    QVERIFY(!classB->baseClass());
    QVERIFY(classB->baseClassName().isEmpty());
    // 3 functions: simple constructor, copy constructor and "method()".
    QCOMPARE(classB->functions().count(), 3);
}

void TestTemplates::testTypedefOfInstantiationOfTemplateClass()
{
    const char cppCode[] = R"CPP(
namespace NSpace {
enum ClassType {
    TypeOne
};
template<ClassType CLASS_TYPE>
struct BaseTemplateClass {
    inline ClassType getClassType() const { CLASS_TYPE; }
};
typedef BaseTemplateClass<TypeOne> TypeOneClass;
}
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Package'>
    <namespace-type name='NSpace'>
        <enum-type name='ClassType'/>
        <object-type name='BaseTemplateClass' generate='no'/>
        <object-type name='TypeOneClass'/>
    </namespace-type>
</typesystem>)XML";

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
    const AbstractMetaType &inst = instantiations.constFirst();
    QVERIFY(inst);
    QVERIFY(!inst.isEnum());
    QVERIFY(!inst.typeEntry()->isEnum());
    QVERIFY(inst.typeEntry()->isEnumValue());
    QCOMPARE(inst.cppSignature(), QLatin1String("NSpace::TypeOne"));
}

void TestTemplates::testContainerTypeIncompleteArgument()
{
    const char cppCode[] = R"CPP(
template<typename T>
class Vector {
    void method(const Vector& vector);
    Vector otherMethod();
};
template <typename T>
void Vector<T>::method(const Vector<T>& vector) {}
template <typename T>
Vector<T> Vector<T>::otherMethod() { return Vector<T>(); }
typedef Vector<int> IntVector;
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Foo'>
    <primitive-type name='int'/>
    <container-type name='Vector' type='vector'/>
    <value-type name='IntVector'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);

    AbstractMetaClass* vector = AbstractMetaClass::findClass(classes, QLatin1String("IntVector"));
    QVERIFY(vector);
    auto baseContainer = vector->typeEntry()->baseContainerType();
    QVERIFY(baseContainer);
    QCOMPARE(reinterpret_cast<const ContainerTypeEntry*>(baseContainer)->containerKind(),
             ContainerTypeEntry::VectorContainer);
    QCOMPARE(vector->functions().count(), 4);

    const AbstractMetaFunction* method = vector->findFunction(QLatin1String("method"));
    QVERIFY(method);
    QCOMPARE(method->signature(), QLatin1String("method(const Vector<int > & vector)"));

    const AbstractMetaFunction* otherMethod = vector->findFunction(QLatin1String("otherMethod"));
    QVERIFY(otherMethod);
    QCOMPARE(otherMethod->signature(), QLatin1String("otherMethod()"));
    QVERIFY(otherMethod->type());
    QCOMPARE(otherMethod->type().cppSignature(), QLatin1String("Vector<int >"));
}

void TestTemplates::testNonTypeTemplates()
{
    // PYSIDe-1296, functions with non type templates parameters.
    const char cppCode[] = R"CPP(
template <class T, int Size>
class Array {
    T array[Size];
};

Array<int, 2> foo();

)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Foo'>
    <primitive-type name='int'/>
    <container-type name='Array' type='vector'/>
    <function signature="foo()"/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());
    auto functions = builder->globalFunctions();
    QCOMPARE(functions.count(), 1);
    auto foo = functions.constFirst();
    QCOMPARE(foo->name(), QLatin1String("foo"));
    QCOMPARE(foo->type().name(), QLatin1String("Array"));
}

// Perform checks on template inheritance; a typedef of a template class
// should result in rewritten types.
void TestTemplates::testTemplateTypeDefs_data()
{
    QTest::addColumn<QString>("cpp");
    QTest::addColumn<QString>("xml");

    const char optionalClassDef[] = R"CPP(
template<class T> // Some value type similar to std::optional
class Optional {
public:
    T value() const { return m_value; }
    operator bool() const { return m_success; }

    T m_value;
    bool m_success  = false;
};
)CPP";

    const char xmlPrefix[] = R"XML(
<typesystem package='Foo'>
    <primitive-type name='int'/>
    <primitive-type name='bool'/>
)XML";

    const char xmlOptionalDecl[] = "<value-type name='Optional' generate='no'/>\n";
    const char xmlOptionalIntDecl[] = "<value-type name='IntOptional'/>\n";
    const char xmlPostFix[] = "</typesystem>\n";

    // Flat, global namespace
    QString cpp;
    QTextStream(&cpp) << optionalClassDef
        << "typedef Optional<int> IntOptional;\n";
    QString xml;
    QTextStream(&xml) << xmlPrefix << xmlOptionalDecl << xmlOptionalIntDecl
        << "<typedef-type name='XmlIntOptional' source='Optional&lt;int&gt;'/>"
        << xmlPostFix;
    QTest::newRow("global-namespace")
        << cpp << xml;

    // Typedef from namespace Std
    cpp.clear();
    QTextStream(&cpp) << "namespace Std {\n" << optionalClassDef << "}\n"
        << "typedef Std::Optional<int> IntOptional;\n";
    xml.clear();
    QTextStream(&xml) << xmlPrefix
        << "<namespace-type name='Std'>\n" << xmlOptionalDecl
        << "</namespace-type>\n" << xmlOptionalIntDecl
        << "<typedef-type name='XmlIntOptional' source='Std::Optional&lt;int&gt;'/>"
        << xmlPostFix;
    QTest::newRow("namespace-Std")
        << cpp << xml;

    // Typedef from nested class
    cpp.clear();
    QTextStream(&cpp) << "class Outer {\npublic:\n" << optionalClassDef << "\n};\n"
        << "typedef Outer::Optional<int> IntOptional;\n";
    xml.clear();
    QTextStream(&xml) << xmlPrefix
        << "<object-type name='Outer'>\n" << xmlOptionalDecl
        << "</object-type>\n" << xmlOptionalIntDecl
        << "<typedef-type name='XmlIntOptional' source='Outer::Optional&lt;int&gt;'/>"
        << xmlPostFix;
    QTest::newRow("nested-class")
        << cpp << xml;
}

void TestTemplates::testTemplateTypeDefs()
{
    QFETCH(QString, cpp);
    QFETCH(QString, xml);

    const QByteArray cppBa = cpp.toLocal8Bit();
    const QByteArray xmlBa = xml.toLocal8Bit();
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppBa.constData(), xmlBa.constData(), true));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    const AbstractMetaClass *optional = AbstractMetaClass::findClass(classes, QLatin1String("Optional"));
    QVERIFY(optional);

    // Find the typedef'ed class
    const AbstractMetaClass *optionalInt =
        AbstractMetaClass::findClass(classes, QLatin1String("IntOptional"));
    QVERIFY(optionalInt);
    QCOMPARE(optionalInt->templateBaseClass(), optional);

    // Find the class typedef'ed in the typesystem XML
    const AbstractMetaClass *xmlOptionalInt =
        AbstractMetaClass::findClass(classes, QLatin1String("XmlIntOptional"));
    QVERIFY(xmlOptionalInt);
    QCOMPARE(xmlOptionalInt->templateBaseClass(), optional);

    // Check whether the value() method now has an 'int' return
    const AbstractMetaFunction *valueMethod =
        optionalInt->findFunction(QLatin1String("value"));
    QVERIFY(valueMethod);
    QCOMPARE(valueMethod->type().cppSignature(), QLatin1String("int"));

    // ditto for typesystem XML
    const AbstractMetaFunction *xmlValueMethod =
        xmlOptionalInt->findFunction(QLatin1String("value"));
    QVERIFY(xmlValueMethod);
    QCOMPARE(xmlValueMethod->type().cppSignature(), QLatin1String("int"));

    // Check whether the m_value field is of type 'int'
    const AbstractMetaField *valueField =
        optionalInt->findField(QLatin1String("m_value"));
    QVERIFY(valueField);
    QCOMPARE(valueField->type().cppSignature(), QLatin1String("int"));

    // ditto for typesystem XML
    const AbstractMetaField *xmlValueField =
        xmlOptionalInt->findField(QLatin1String("m_value"));
    QVERIFY(xmlValueField);
    QCOMPARE(xmlValueField->type().cppSignature(), QLatin1String("int"));
}

void TestTemplates::testTemplateTypeAliases()
{
    // Model Qt 6's "template<typename T> using QList = QVector<T>"
    const char cppCode[] = R"CPP(
template<typename T>
class Container1 { };

template<typename T>
using Container2 = Container1<T>;

class Test
{
public:
    Container2<int> m_intContainer;
};

class Derived : public Container2<int>
{
public:
};
)CPP";

    const char xmlCode[] = R"XML(
<typesystem package='Foo'>
    <primitive-type name='int'/>
    <value-type name='Container1'/>
    <value-type name='Derived'/>
    <object-type name='Test'/>
</typesystem>)XML";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    auto testClass = AbstractMetaClass::findClass(classes, QLatin1String("Test"));
    QVERIFY(testClass);

    auto fields = testClass->fields();
    QCOMPARE(fields.count(), 1);
    auto fieldType = testClass->fields().at(0)->type();
    QCOMPARE(fieldType.name(), QLatin1String("Container1"));
    QCOMPARE(fieldType.instantiations().size(), 1);

    auto derived = AbstractMetaClass::findClass(classes, QLatin1String("Derived"));
    QVERIFY(derived);
    auto base = derived->templateBaseClass();
    QCOMPARE(base->name(), QLatin1String("Container1"));
}

QTEST_APPLESS_MAIN(TestTemplates)
