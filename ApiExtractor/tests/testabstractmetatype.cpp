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

#include "testabstractmetatype.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestAbstractMetaType::testConstCharPtrType()
{
    const char* cppCode ="const char* justAtest();\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
        <primitive-type name='char'/>\n\
        <function signature='justAtest()' />\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    QCOMPARE(builder->globalFunctions().size(), 1);
    AbstractMetaFunction* func = builder->globalFunctions().first();
    AbstractMetaType* rtype = func->type();
    // Test properties of const char*
    QVERIFY(rtype);
    QCOMPARE(rtype->package(), QLatin1String("Foo"));
    QCOMPARE(rtype->name(), QLatin1String("char"));
    QVERIFY(rtype->isConstant());
    QVERIFY(!rtype->isArray());
    QVERIFY(!rtype->isContainer());
    QVERIFY(!rtype->isObject());
    QVERIFY(!rtype->isPrimitive()); // const char* differs from char, so it's not considered a primitive type by apiextractor
    QVERIFY(rtype->isNativePointer());
    QVERIFY(!rtype->isQObject());
    QCOMPARE(rtype->referenceType(), NoReference);
    QVERIFY(!rtype->isValue());
    QVERIFY(!rtype->isValuePointer());
}

void TestAbstractMetaType::testApiVersionSupported()
{
    const char* cppCode ="class foo {}; class foo2 {};\n\
                          void justAtest(); void justAtest3();\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
        <value-type name='foo' since='0.1'/>\n\
        <value-type name='foo2' since='1.0'/>\n\
        <value-type name='foo3' since='1.1'/>\n\
        <function signature='justAtest()' since='0.1'/>\n\
        <function signature='justAtest2()' since='1.1'/>\n\
        <function signature='justAtest3()'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false, "1.0"));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 2);


    AbstractMetaFunctionList functions = builder->globalFunctions();
    QCOMPARE(functions.size(), 2);
}


void TestAbstractMetaType::testApiVersionNotSupported()
{
    const char* cppCode ="class object {};\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
        <value-type name='object' since='0.1'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true, "0.1"));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
}

void TestAbstractMetaType::testCharType()
{
    const char* cppCode ="char justAtest(); class A {};\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <primitive-type name='char'/>\n\
    <value-type name='A'/>\n\
    <function signature='justAtest()'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    QCOMPARE(classes.first()->package(), QLatin1String("Foo"));

    AbstractMetaFunctionList functions = builder->globalFunctions();
    QCOMPARE(functions.size(), 1);
    AbstractMetaFunction* func = functions.first();
    AbstractMetaType* rtype = func->type();
    // Test properties of const char*
    QVERIFY(rtype);
    QCOMPARE(rtype->package(), QLatin1String("Foo"));
    QCOMPARE(rtype->name(), QLatin1String("char"));
    QVERIFY(!rtype->isConstant());
    QVERIFY(!rtype->isArray());
    QVERIFY(!rtype->isContainer());
    QVERIFY(!rtype->isObject());
    QVERIFY(rtype->isPrimitive());
    QVERIFY(!rtype->isNativePointer());
    QVERIFY(!rtype->isQObject());
    QCOMPARE(rtype->referenceType(), NoReference);
    QVERIFY(!rtype->isValue());
    QVERIFY(!rtype->isValuePointer());
}

void TestAbstractMetaType::testTypedef()
{
    const char* cppCode ="\
    struct A {\n\
        void someMethod();\n\
    };\n\
    typedef A B;\n\
    typedef B C;\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <value-type name='C' />\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    AbstractMetaClass* c = classes.findClass(QLatin1String("C"));
    QVERIFY(c);
    QVERIFY(c->isTypeDef());
}

void TestAbstractMetaType::testTypedefWithTemplates()
{
    const char* cppCode ="\
    template<typename T>\n\
    class A {};\n\
    \n\
    class B {};\n\
    typedef A<B> C;\n\
    \n\
    void func(C c);\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <container-type name='A' type='list'/>\n\
    <value-type name='B' />\n\
    <function signature='func(A&lt;B&gt;)'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    AbstractMetaFunctionList functions = builder->globalFunctions();
    QCOMPARE(functions.count(), 1);
    AbstractMetaFunction* function = functions.first();
    AbstractMetaArgumentList args = function->arguments();
    QCOMPARE(args.count(), 1);
    AbstractMetaArgument* arg = args.first();
    AbstractMetaType* metaType = arg->type();
    QCOMPARE(metaType->cppSignature(), QLatin1String("A<B >"));
}


void TestAbstractMetaType::testObjectTypeUsedAsValue()
{
    const char* cppCode ="\
    class A {\n\
        void method(A);\n\
    };\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
    <object-type name='A'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    AbstractMetaFunctionList overloads = classA->queryFunctionsByName(QLatin1String("method"));
    QCOMPARE(overloads.count(), 1);
    AbstractMetaFunction* method = overloads.first();
    QVERIFY(method);
    AbstractMetaArgumentList args = method->arguments();
    QCOMPARE(args.count(), 1);
    AbstractMetaArgument* arg = args.first();
    AbstractMetaType* metaType = arg->type();
    QCOMPARE(metaType->cppSignature(), QLatin1String("A"));
    QVERIFY(metaType->isValue());
    QVERIFY(metaType->typeEntry()->isObject());
}

QTEST_APPLESS_MAIN(TestAbstractMetaType)
