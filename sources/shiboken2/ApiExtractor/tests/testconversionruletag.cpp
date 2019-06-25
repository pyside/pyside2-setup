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

#include "testconversionruletag.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>
#include <QFile>
#include <QTemporaryFile>

void TestConversionRuleTag::testConversionRuleTagWithFile()
{
    // temp file used later
    const char conversionData[] = "Hi! I'm a conversion rule.";
    QTemporaryFile file;
    file.open();
    QCOMPARE(file.write(conversionData), qint64(sizeof(conversionData)-1));
    file.close();

    const char cppCode[] = "struct A {};\n";
    QString xmlCode = QLatin1String("\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <conversion-rule file='") + file.fileName() + QLatin1String("'/>\n\
        </value-type>\n\
    </typesystem>\n");
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode.toLocal8Bit().data()));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const ComplexTypeEntry* typeEntry = classA->typeEntry();
    QVERIFY(typeEntry->hasConversionRule());
    QCOMPARE(typeEntry->conversionRule(), QLatin1String(conversionData));
}

void TestConversionRuleTag::testConversionRuleTagReplace()
{
    const char cppCode[] = "\
    struct A {\n\
        A();\n\
        A(const char*, int);\n\
    };\n\
    struct B {\n\
        A createA();\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <primitive-type name='char'/>\n\
        <primitive-type name='A'>\n\
            <conversion-rule>\n\
                <native-to-target>\n\
                DoThis();\n\
                return ConvertFromCppToPython(%IN);\n\
                </native-to-target>\n\
                <target-to-native>\n\
                    <add-conversion type='TargetNone' check='%IN == Target_None'>\n\
                    DoThat();\n\
                    DoSomething();\n\
                    %OUT = A();\n\
                    </add-conversion>\n\
                    <add-conversion type='B' check='CheckIfInputObjectIsB(%IN)'>\n\
                    %OUT = %IN.createA();\n\
                    </add-conversion>\n\
                    <add-conversion type='String' check='String_Check(%IN)'>\n\
                    %OUT = new A(String_AsString(%IN), String_GetSize(%IN));\n\
                    </add-conversion>\n\
                </target-to-native>\n\
            </conversion-rule>\n\
        </primitive-type>\n\
        <value-type name='B'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    TypeDatabase* typeDb = TypeDatabase::instance();
    PrimitiveTypeEntry* typeA = typeDb->findPrimitiveType(QLatin1String("A"));
    QVERIFY(typeA);

    CustomConversion* conversion = typeA->customConversion();
    QVERIFY(conversion);

    QCOMPARE(typeA, conversion->ownerType());
    QCOMPARE(conversion->nativeToTargetConversion().simplified(),
             QLatin1String("DoThis(); return ConvertFromCppToPython(%IN);"));

    QVERIFY(conversion->replaceOriginalTargetToNativeConversions());
    QVERIFY(conversion->hasTargetToNativeConversions());
    QCOMPARE(conversion->targetToNativeConversions().size(), 3);

    CustomConversion::TargetToNativeConversion* toNative = conversion->targetToNativeConversions().at(0);
    QVERIFY(toNative);
    QCOMPARE(toNative->sourceTypeName(), QLatin1String("TargetNone"));
    QVERIFY(toNative->isCustomType());
    QCOMPARE(toNative->sourceType(), nullptr);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("%IN == Target_None"));
    QCOMPARE(toNative->conversion().simplified(),
             QLatin1String("DoThat(); DoSomething(); %OUT = A();"));

    toNative = conversion->targetToNativeConversions().at(1);
    QVERIFY(toNative);
    QCOMPARE(toNative->sourceTypeName(), QLatin1String("B"));
    QVERIFY(!toNative->isCustomType());
    TypeEntry* typeB = typeDb->findType(QLatin1String("B"));
    QVERIFY(typeB);
    QCOMPARE(toNative->sourceType(), typeB);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("CheckIfInputObjectIsB(%IN)"));
    QCOMPARE(toNative->conversion().trimmed(), QLatin1String("%OUT = %IN.createA();"));

    toNative = conversion->targetToNativeConversions().at(2);
    QVERIFY(toNative);
    QCOMPARE(toNative->sourceTypeName(), QLatin1String("String"));
    QVERIFY(toNative->isCustomType());
    QCOMPARE(toNative->sourceType(), nullptr);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("String_Check(%IN)"));
    QCOMPARE(toNative->conversion().trimmed(), QLatin1String("%OUT = new A(String_AsString(%IN), String_GetSize(%IN));"));
}

void TestConversionRuleTag::testConversionRuleTagAdd()
{
    const char cppCode[] = "\
    struct Date {\n\
        Date();\n\
        Date(int, int, int);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <value-type name='Date'>\n\
            <conversion-rule>\n\
                <target-to-native replace='no'>\n\
                    <add-conversion type='TargetDate' check='TargetDate_Check(%IN)'>\n\
if (!TargetDateTimeAPI) TargetDateTime_IMPORT;\n\
%OUT = new Date(TargetDate_Day(%IN), TargetDate_Month(%IN), TargetDate_Year(%IN));\n\
                    </add-conversion>\n\
                </target-to-native>\n\
            </conversion-rule>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("Date"));
    QVERIFY(classA);

    CustomConversion* conversion = classA->typeEntry()->customConversion();
    QVERIFY(conversion);

    QCOMPARE(conversion->nativeToTargetConversion(), QString());

    QVERIFY(!conversion->replaceOriginalTargetToNativeConversions());
    QVERIFY(conversion->hasTargetToNativeConversions());
    QCOMPARE(conversion->targetToNativeConversions().size(), 1);

    CustomConversion::TargetToNativeConversion* toNative = conversion->targetToNativeConversions().first();
    QVERIFY(toNative);
    QCOMPARE(toNative->sourceTypeName(), QLatin1String("TargetDate"));
    QVERIFY(toNative->isCustomType());
    QCOMPARE(toNative->sourceType(), nullptr);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("TargetDate_Check(%IN)"));
    QCOMPARE(toNative->conversion().trimmed(),
             QLatin1String("if (!TargetDateTimeAPI) TargetDateTime_IMPORT;\n%OUT = new Date(TargetDate_Day(%IN), TargetDate_Month(%IN), TargetDate_Year(%IN));"));
}

void TestConversionRuleTag::testConversionRuleTagWithInsertTemplate()
{
    const char cppCode[] = "struct A {};";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='int'/>\n\
        <!-- single line -->\n\
        <template name='native_to_target'>return ConvertFromCppToPython(%IN);</template>\n\
        <!-- multi-line -->\n\
        <template name='target_to_native'>\n\
%OUT = %IN.createA();\n\
        </template>\n\
        <primitive-type name='A'>\n\
            <conversion-rule>\n\
                <native-to-target>\n\
                    <insert-template name='native_to_target'/>\n\
                </native-to-target>\n\
                <target-to-native>\n\
                    <add-conversion type='TargetType'>\n\
                        <insert-template name='target_to_native'/>\n\
                    </add-conversion>\n\
                </target-to-native>\n\
            </conversion-rule>\n\
        </primitive-type>\n\
    </typesystem>\n";

    const char nativeToTargetExpected[] =
    "// TEMPLATE - native_to_target - START\n"
    "return ConvertFromCppToPython(%IN);\n"
    "// TEMPLATE - native_to_target - END";

    const char targetToNativeExpected[] =
    "// TEMPLATE - target_to_native - START\n"
    "%OUT = %IN.createA();\n"
    "// TEMPLATE - target_to_native - END";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    TypeDatabase* typeDb = TypeDatabase::instance();
    PrimitiveTypeEntry* typeA = typeDb->findPrimitiveType(QLatin1String("A"));
    QVERIFY(typeA);

    CustomConversion* conversion = typeA->customConversion();
    QVERIFY(conversion);

    QCOMPARE(typeA, conversion->ownerType());
    QCOMPARE(conversion->nativeToTargetConversion().trimmed(),
             QLatin1String(nativeToTargetExpected));

    QVERIFY(conversion->hasTargetToNativeConversions());
    QCOMPARE(conversion->targetToNativeConversions().size(), 1);

    CustomConversion::TargetToNativeConversion* toNative = conversion->targetToNativeConversions().first();
    QVERIFY(toNative);
    QCOMPARE(toNative->conversion().trimmed(),
             QLatin1String(targetToNativeExpected));
}

QTEST_APPLESS_MAIN(TestConversionRuleTag)
