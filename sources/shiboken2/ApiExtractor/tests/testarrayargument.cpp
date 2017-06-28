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

#include "testarrayargument.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestArrayArgument::testArrayArgumentWithSizeDefinedByInteger()
{
    const char* cppCode ="\
    struct A {\n\
        enum SomeEnum { Value0, Value1, NValues };\n\
        void method(double[3]);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='double'/>\n\
        <object-type name='A'>\n\
            <enum-type name='SomeEnum'/>\n\
        </object-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), 3);
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
}

static QString functionMinimalSignature(const AbstractMetaClass *c, const QString &name)
{
    const AbstractMetaFunction *f = c->findFunction(name);
    return f ? f->minimalSignature() : QString();
}

void TestArrayArgument::testArraySignature()
{
    const char cppCode[] ="\
    struct A {\n\
        void mi1(int arg[5]);\n\
        void mi1c(const int arg[5]);\n\
        void muc2(unsigned char *arg[2][3]);\n\
        void mc2c(const char *arg[5][6]);\n\
    };\n";
    const char xmlCode[] = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='char'/>\n\
        <primitive-type name='unsigned char'/>\n\
        <primitive-type name='int'/>\n\
        <object-type name='A'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QCOMPARE(functionMinimalSignature(classA, QLatin1String("mi1")),
             QLatin1String("mi1(int[5])"));
    QCOMPARE(functionMinimalSignature(classA, QLatin1String("mi1c")),
             QLatin1String("mi1c(const int[5])"));
    QCOMPARE(functionMinimalSignature(classA, QLatin1String("muc2")),
             QLatin1String("muc2(unsigned char*[2][3])"));
    QCOMPARE(functionMinimalSignature(classA, QLatin1String("mc2c")),
             QLatin1String("mc2c(const char*[5][6])"));
}

void TestArrayArgument::testArrayArgumentWithSizeDefinedByEnumValue()
{
    const char* cppCode ="\
    struct A {\n\
        enum SomeEnum { Value0, Value1, NValues };\n\
        void method(double[NValues]);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='double'/>\n\
        <object-type name='A'>\n\
            <enum-type name='SomeEnum'/>\n\
        </object-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* someEnum = classA->findEnum(QLatin1String("SomeEnum"));
    QVERIFY(someEnum);
    AbstractMetaEnumValue* nvalues = classA->findEnumValue(QLatin1String("NValues"), someEnum);
    QVERIFY(nvalues);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), nvalues->value());
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
};

void TestArrayArgument::testArrayArgumentWithSizeDefinedByEnumValueFromGlobalEnum()
{
    const char* cppCode ="\
    enum SomeEnum { Value0, Value1, NValues };\n\
    struct A {\n\
        void method(double[NValues]);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='double'/>\n\
        <enum-type name='SomeEnum'/>\n\
        <object-type name='A'>\n\
        </object-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);

    AbstractMetaEnum* someEnum = builder->globalEnums().first();
    QVERIFY(someEnum);
    AbstractMetaEnumValue* nvalues = 0;
    const AbstractMetaEnumValueList &values = someEnum->values();
    for (AbstractMetaEnumValue *enumValue : values) {
        if (enumValue->name() == QLatin1String("NValues")) {
            nvalues = enumValue;
            break;
        }
    }
    QVERIFY(nvalues);

    const AbstractMetaArgument* arg = classA->functions().last()->arguments().first();
    QVERIFY(arg->type()->isArray());
    QCOMPARE(arg->type()->arrayElementCount(), nvalues->value());
    QCOMPARE(arg->type()->arrayElementType()->name(), QLatin1String("double"));
};

QTEST_APPLESS_MAIN(TestArrayArgument)
