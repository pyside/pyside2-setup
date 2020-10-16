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

#include "testenum.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestEnum::testEnumCppSignature()
{
    const char* cppCode ="\
    enum GlobalEnum { A, B };\n\
    \n\
    struct A {\n\
        enum ClassEnum { CA, CB };\n\
        void method(ClassEnum);\n\
    };\n\
    void func(A::ClassEnum);\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <enum-type name='GlobalEnum'/>\n\
        <value-type name='A'>\n\
            <enum-type name='ClassEnum'/>\n\
        </value-type>\n\
        <function signature='func(A::ClassEnum)'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);

    AbstractMetaEnumList globalEnums = builder->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.constFirst()->name(), QLatin1String("GlobalEnum"));

    // enum as parameter of a function
    AbstractMetaFunctionList functions = builder->globalFunctions();
    QCOMPARE(functions.count(), 1);
    QCOMPARE(functions.constFirst()->arguments().count(), 1);
    QCOMPARE(functions.constFirst()->arguments().constFirst()->type().cppSignature(),
             QLatin1String("A::ClassEnum"));

    // enum as parameter of a method
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QCOMPARE(classA->enums().count(), 1);
    AbstractMetaFunctionList funcs = classA->queryFunctionsByName(QLatin1String("method"));
    QVERIFY(!funcs.isEmpty());
    AbstractMetaFunction *method = funcs.constFirst();
    QVERIFY(method);
    AbstractMetaArgument *arg = method->arguments().constFirst();
    QCOMPARE(arg->type().name(), QLatin1String("ClassEnum"));
    QCOMPARE(arg->type().cppSignature(), QLatin1String("A::ClassEnum"));
    QCOMPARE(functions.constFirst()->arguments().count(), 1);
    arg = functions.constFirst()->arguments().constFirst();
    QCOMPARE(arg->type().name(), QLatin1String("ClassEnum"));
    QCOMPARE(arg->type().cppSignature(), QLatin1String("A::ClassEnum"));

    AbstractMetaEnumList classEnums = classA->enums();
    QCOMPARE(classEnums.constFirst()->name(), QLatin1String("ClassEnum"));
    AbstractMetaEnumValue *e = AbstractMetaClass::findEnumValue(classes, QLatin1String("CA"));
    QVERIFY(e);
    e = AbstractMetaClass::findEnumValue(classes, QLatin1String("ClassEnum::CA"));
    QVERIFY(e);
}

void TestEnum::testEnumWithApiVersion()
{
    const char* cppCode ="\
    struct A {\n\
        enum ClassEnum { EnumA, EnumB };\n\
        enum ClassEnum2 { EnumC, EnumD };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <enum-type name='ClassEnum' since='0.1'/>\n\
            <enum-type name='ClassEnum2' since='0.2'/>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, QLatin1String("0.1")));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 1);
}

void TestEnum::testAnonymousEnum()
{
    const char* cppCode ="\
    enum { Global0, Global1 };\n\
    struct A {\n\
        enum { A0, A1 };\n\
        enum { isThis = true, isThat = false };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <!-- Uses the first value of the enum to identify it. -->\n\
        <enum-type identified-by-value='Global0'/>\n\
        <value-type name='A'>\n\
            <!-- Uses the second value of the enum to identify it. -->\n\
            <enum-type identified-by-value='A1'/>\n\
            <enum-type identified-by-value='isThis'/>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaEnumList globalEnums = builder->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.constFirst()->typeEntry()->qualifiedCppName(),
             QLatin1String("Global0"));
    QVERIFY(globalEnums.constFirst()->isAnonymous());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 2);

    AbstractMetaEnum* anonEnumA1 = classes[0]->findEnum(QLatin1String("A1"));
    QVERIFY(anonEnumA1);
    QVERIFY(anonEnumA1->isAnonymous());
    QCOMPARE(anonEnumA1->typeEntry()->qualifiedCppName(), QLatin1String("A::A1"));

    AbstractMetaEnumValue* enumValueA0 = anonEnumA1->values().constFirst();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value().value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue *enumValueA1 = anonEnumA1->values().constLast();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value().value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum* anonEnumIsThis = classes[0]->findEnum(QLatin1String("isThis"));
    QVERIFY(anonEnumIsThis);
    QVERIFY(anonEnumIsThis->isAnonymous());
    QCOMPARE(anonEnumIsThis->typeEntry()->qualifiedCppName(), QLatin1String("A::isThis"));

    AbstractMetaEnumValue* enumValueIsThis = anonEnumIsThis->values().constFirst();
    QCOMPARE(enumValueIsThis->name(), QLatin1String("isThis"));
    QCOMPARE(enumValueIsThis->value().value(), static_cast<int>(true));
    QCOMPARE(enumValueIsThis->stringValue(), QLatin1String("true"));

    AbstractMetaEnumValue *enumValueIsThat = anonEnumIsThis->values().constLast();
    QCOMPARE(enumValueIsThat->name(), QLatin1String("isThat"));
    QCOMPARE(enumValueIsThat->value().value(), static_cast<int>(false));
    QCOMPARE(enumValueIsThat->stringValue(), QLatin1String("false"));
}

void TestEnum::testGlobalEnums()
{
    const char* cppCode ="\
    enum EnumA { A0, A1 };\n\
    enum EnumB { B0 = 2, B1 = 0x4 };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <enum-type name='EnumA'/>\n\
        <enum-type name='EnumB'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaEnumList globalEnums = builder->globalEnums();
    QCOMPARE(globalEnums.count(), 2);

    AbstractMetaEnum *enumA = globalEnums.constFirst();
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("EnumA"));

    AbstractMetaEnumValue *enumValueA0 = enumA->values().constFirst();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value().value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue *enumValueA1 = enumA->values().constLast();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value().value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum *enumB = globalEnums.constLast();
    QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QLatin1String("EnumB"));

    AbstractMetaEnumValue* enumValueB0 = enumB->values().constFirst();
    QCOMPARE(enumValueB0->name(), QLatin1String("B0"));
    QCOMPARE(enumValueB0->value().value(), 2);
    QCOMPARE(enumValueB0->stringValue(), QLatin1String("2"));

    AbstractMetaEnumValue *enumValueB1 = enumB->values().constLast();
    QCOMPARE(enumValueB1->name(), QLatin1String("B1"));
    QCOMPARE(enumValueB1->value().value(), 4);
    QCOMPARE(enumValueB1->stringValue(), QLatin1String("0x4"));
}

void TestEnum::testEnumValueFromNeighbourEnum()
{
    const char* cppCode ="\
    namespace A {\n\
        enum EnumA { ValueA0, ValueA1 };\n\
        enum EnumB { ValueB0 = A::ValueA1, ValueB1 = ValueA0 };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <namespace-type name='A'>\n\
            <enum-type name='EnumA'/>\n\
            <enum-type name='EnumB'/>\n\
        </namespace-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->enums().count(), 2);

    AbstractMetaEnum* enumA = classes[0]->findEnum(QLatin1String("EnumA"));
    QVERIFY(enumA);
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumA"));

    AbstractMetaEnumValue* enumValueA0 = enumA->values().constFirst();
    QCOMPARE(enumValueA0->name(), QLatin1String("ValueA0"));
    QCOMPARE(enumValueA0->value().value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QString());

    AbstractMetaEnumValue* enumValueA1 = enumA->values().constLast();
    QCOMPARE(enumValueA1->name(), QLatin1String("ValueA1"));
    QCOMPARE(enumValueA1->value().value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());

    AbstractMetaEnum* enumB = classes[0]->findEnum(QLatin1String("EnumB"));
    QVERIFY(enumB);
    QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumB"));

    AbstractMetaEnumValue *enumValueB0 = enumB->values().constFirst();
    QCOMPARE(enumValueB0->name(), QLatin1String("ValueB0"));
    QCOMPARE(enumValueB0->value().value(), 1);
    QCOMPARE(enumValueB0->stringValue(), QLatin1String("A::ValueA1"));

    AbstractMetaEnumValue *enumValueB1 = enumB->values().constLast();
    QCOMPARE(enumValueB1->name(), QLatin1String("ValueB1"));
    QCOMPARE(enumValueB1->value().value(), 0);
    QCOMPARE(enumValueB1->stringValue(), QLatin1String("ValueA0"));
}

void TestEnum::testEnumValueFromExpression()
{
    const char* cppCode ="\
    struct A {\n\
        enum EnumA : unsigned {\n\
            ValueA0 = 3u,\n\
            ValueA1 = ~3u,\n\
            ValueA2 = 0xffffffff,\n\
            ValueA3 = 0xf0,\n\
            ValueA4 = 8 |ValueA3,\n\
            ValueA5 = ValueA3|32,\n\
            ValueA6 = ValueA3 >> 1,\n\
            ValueA7 = ValueA3 << 1\n\
        };\n\
        enum EnumB : int {\n\
            ValueB0 = ~3,\n\
        };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <enum-type name='EnumA'/>\n\
            <enum-type name='EnumB'/>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* enumA = classA->findEnum(QLatin1String("EnumA"));
    QVERIFY(enumA);
    QVERIFY(!enumA->isSigned());
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumA"));

    AbstractMetaEnumValue* valueA0 = enumA->values().at(0);
    QCOMPARE(valueA0->name(), QLatin1String("ValueA0"));
    QCOMPARE(valueA0->stringValue(), QLatin1String("3u"));
    QCOMPARE(valueA0->value().unsignedValue(), 3u);

    AbstractMetaEnumValue* valueA1 = enumA->values().at(1);
    QCOMPARE(valueA1->name(), QLatin1String("ValueA1"));
    QCOMPARE(valueA1->stringValue(), QLatin1String("~3u"));
    QCOMPARE(valueA1->value().unsignedValue(), ~3u);

    AbstractMetaEnumValue* valueA2 = enumA->values().at(2);
    QCOMPARE(valueA2->name(), QLatin1String("ValueA2"));
    QCOMPARE(valueA2->stringValue(), QLatin1String("0xffffffff"));
    QCOMPARE(valueA2->value().unsignedValue(), 0xffffffffu);

    AbstractMetaEnumValue* valueA3 = enumA->values().at(3);
    QCOMPARE(valueA3->name(), QLatin1String("ValueA3"));
    QCOMPARE(valueA3->stringValue(), QLatin1String("0xf0"));
    QCOMPARE(valueA3->value().unsignedValue(), 0xf0u);

    AbstractMetaEnumValue* valueA4 = enumA->values().at(4);
    QCOMPARE(valueA4->name(), QLatin1String("ValueA4"));
    QCOMPARE(valueA4->stringValue(), QLatin1String("8 |ValueA3"));
    QCOMPARE(valueA4->value().unsignedValue(), 8|0xf0u);

    AbstractMetaEnumValue* valueA5 = enumA->values().at(5);
    QCOMPARE(valueA5->name(), QLatin1String("ValueA5"));
    QCOMPARE(valueA5->stringValue(), QLatin1String("ValueA3|32"));
    QCOMPARE(valueA5->value().unsignedValue(), 0xf0u|32);

    AbstractMetaEnumValue* valueA6 = enumA->values().at(6);
    QCOMPARE(valueA6->name(), QLatin1String("ValueA6"));
    QCOMPARE(valueA6->stringValue(), QLatin1String("ValueA3 >> 1"));
    QCOMPARE(valueA6->value().unsignedValue(), 0xf0u >> 1);

    AbstractMetaEnumValue* valueA7 = enumA->values().at(7);
    QCOMPARE(valueA7->name(), QLatin1String("ValueA7"));
    QCOMPARE(valueA7->stringValue(), QLatin1String("ValueA3 << 1"));
    QCOMPARE(valueA7->value().unsignedValue(), 0xf0u << 1);

   const AbstractMetaEnum *enumB = classA->findEnum(QLatin1String("EnumB"));
   QVERIFY(enumB);
   QVERIFY(enumB->isSigned());
   QCOMPARE(enumB->typeEntry()->qualifiedCppName(), QLatin1String("A::EnumB"));
   QCOMPARE(enumB->values().size(), 1);
   const AbstractMetaEnumValue *valueB0 = enumB->values().at(0);
   QCOMPARE(valueB0->name(), QLatin1String("ValueB0"));
   QCOMPARE(valueB0->stringValue(), QLatin1String("~3"));
   QCOMPARE(valueB0->value().value(), ~3);
}

void TestEnum::testPrivateEnum()
{
    const char* cppCode ="\
    class A {\n\
    private:\n\
        enum PrivateEnum { Priv0 = 0x0f, Priv1 = 0xf0 };\n\
    public:\n\
        enum PublicEnum { Pub0 = Priv0, Pub1 = A::Priv1 };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <enum-type name='PublicEnum'/>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->enums().count(), 2);

    AbstractMetaEnum* privateEnum = classA->findEnum(QLatin1String("PrivateEnum"));
    QVERIFY(privateEnum);
    QVERIFY(privateEnum->isPrivate());
    QCOMPARE(privateEnum->typeEntry()->qualifiedCppName(), QLatin1String("A::PrivateEnum"));

    AbstractMetaEnum* publicEnum = classA->findEnum(QLatin1String("PublicEnum"));
    QVERIFY(publicEnum);
    QCOMPARE(publicEnum->typeEntry()->qualifiedCppName(), QLatin1String("A::PublicEnum"));

    AbstractMetaEnumValue *pub0 = publicEnum->values().constFirst();
    QCOMPARE(pub0->name(), QLatin1String("Pub0"));
    QCOMPARE(pub0->value().value(), 0x0f);
    QCOMPARE(pub0->stringValue(), QLatin1String("Priv0"));

    AbstractMetaEnumValue *pub1 = publicEnum->values().constLast();
    QCOMPARE(pub1->name(), QLatin1String("Pub1"));
    QCOMPARE(pub1->value().value(), 0xf0);
    QCOMPARE(pub1->stringValue(), QLatin1String("A::Priv1"));
}

void TestEnum::testTypedefEnum()
{
    const char* cppCode ="\
    typedef enum EnumA {\n\
        A0,\n\
        A1,\n\
    } EnumA;\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <enum-type name='EnumA'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaEnumList globalEnums = builder->globalEnums();
    QCOMPARE(globalEnums.count(), 1);

    AbstractMetaEnum *enumA = globalEnums.constFirst();
    QCOMPARE(enumA->typeEntry()->qualifiedCppName(), QLatin1String("EnumA"));

    AbstractMetaEnumValue *enumValueA0 = enumA->values().constFirst();
    QCOMPARE(enumValueA0->name(), QLatin1String("A0"));
    QCOMPARE(enumValueA0->value().value(), 0);
    QCOMPARE(enumValueA0->stringValue(), QLatin1String(""));

    AbstractMetaEnumValue *enumValueA1 = enumA->values().constLast();
    QCOMPARE(enumValueA1->name(), QLatin1String("A1"));
    QCOMPARE(enumValueA1->value().value(), 1);
    QCOMPARE(enumValueA1->stringValue(), QString());
}

QTEST_APPLESS_MAIN(TestEnum)
