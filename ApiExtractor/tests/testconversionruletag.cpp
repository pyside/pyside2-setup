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

#include "testconversionruletag.h"
#include <QtTest/QTest>
#include "testutil.h"
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

    const char cppCode[] = "struct A {};";
    QString xmlCode = QLatin1String("\
    <typesystem package='Foo'>\
        <value-type name='A'>\
            <conversion-rule file='") + file.fileName() + QLatin1String("' />\
        </value-type>\
    </typesystem>");
    TestUtil t(cppCode, xmlCode.toLocal8Bit().data());
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    const ComplexTypeEntry* typeEntry = classA->typeEntry();
    QVERIFY(typeEntry->hasConversionRule());
    QCOMPARE(typeEntry->conversionRule(), QLatin1String(conversionData));
}

void TestConversionRuleTag::testConversionRuleTagReplace()
{
    const char cppCode[] = "\
    struct A {\
        A();\
        A(const char*, int);\
    };\
    struct B {\
        A createA();\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='int'/>\
        <primitive-type name='char'/>\
        <primitive-type name='A'>\
            <conversion-rule>\
                <native-to-target>\
                DoThis();\
                return ConvertFromCppToPython(%IN);\
                </native-to-target>\
                <target-to-native>\
                    <add-conversion type='TargetNone' check='%IN == Target_None'>\
                    DoThat();\
                    DoSomething();\
                    %OUT = A();\
                    </add-conversion>\
                    <add-conversion type='B' check='CheckIfInputObjectIsB(%IN)'>\
                    %OUT = %IN.createA();\
                    </add-conversion>\
                    <add-conversion type='String' check='String_Check(%IN)'>\
                    %OUT = new A(String_AsString(%IN), String_GetSize(%IN));\
                    </add-conversion>\
                </target-to-native>\
            </conversion-rule>\
        </primitive-type>\
        <value-type name='B'/>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    TypeDatabase* typeDb = TypeDatabase::instance();
    PrimitiveTypeEntry* typeA = typeDb->findPrimitiveType(QLatin1String("A"));
    QVERIFY(typeA);

    CustomConversion* conversion = typeA->customConversion();
    QVERIFY(conversion);

    QCOMPARE(typeA, conversion->ownerType());
    QCOMPARE(conversion->nativeToTargetConversion().trimmed(), QLatin1String("DoThis();                return ConvertFromCppToPython(%IN);"));

    QVERIFY(conversion->replaceOriginalTargetToNativeConversions());
    QVERIFY(conversion->hasTargetToNativeConversions());
    QCOMPARE(conversion->targetToNativeConversions().size(), 3);

    CustomConversion::TargetToNativeConversion* toNative = conversion->targetToNativeConversions().at(0);
    QVERIFY(toNative);
    QCOMPARE(toNative->sourceTypeName(), QLatin1String("TargetNone"));
    QVERIFY(toNative->isCustomType());
    QCOMPARE(toNative->sourceType(), (const TypeEntry*)0);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("%IN == Target_None"));
    QCOMPARE(toNative->conversion().trimmed(), QLatin1String("DoThat();                    DoSomething();                    %OUT = A();"));

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
    QCOMPARE(toNative->sourceType(), (const TypeEntry*)0);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("String_Check(%IN)"));
    QCOMPARE(toNative->conversion().trimmed(), QLatin1String("%OUT = new A(String_AsString(%IN), String_GetSize(%IN));"));
}

void TestConversionRuleTag::testConversionRuleTagAdd()
{
    const char cppCode[] = "\
    struct Date {\
        Date();\
        Date(int, int, int);\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='int'/>\
        <value-type name='Date'>\
            <conversion-rule>\
                <target-to-native replace='no'>\
                    <add-conversion type='TargetDate' check='TargetDate_Check(%IN)'>\
                    if (!TargetDateTimeAPI) TargetDateTime_IMPORT;\
                    %OUT = new Date(TargetDate_Day(%IN), TargetDate_Month(%IN), TargetDate_Year(%IN));\
                    </add-conversion>\
                </target-to-native>\
            </conversion-rule>\
        </value-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClass* classA = t.builder()->classes().findClass(QLatin1String("Date"));
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
    QCOMPARE(toNative->sourceType(), (const TypeEntry*)0);
    QCOMPARE(toNative->sourceTypeCheck(), QLatin1String("TargetDate_Check(%IN)"));
    QCOMPARE(toNative->conversion().trimmed(), QLatin1String("if (!TargetDateTimeAPI) TargetDateTime_IMPORT;                    %OUT = new Date(TargetDate_Day(%IN), TargetDate_Month(%IN), TargetDate_Year(%IN));"));
}

void TestConversionRuleTag::testConversionRuleTagWithInsertTemplate()
{
    const char cppCode[] = "struct A {};";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='int'/>\
        <template name='native_to_target'>\
        return ConvertFromCppToPython(%IN);\
        </template>\
        <template name='target_to_native'>\
        %OUT = %IN.createA();\
        </template>\
        <primitive-type name='A'>\
            <conversion-rule>\
                <native-to-target>\
                    <insert-template name='native_to_target'/>\
                </native-to-target>\
                <target-to-native>\
                    <add-conversion type='TargetType'>\
                        <insert-template name='target_to_native'/>\
                    </add-conversion>\
                </target-to-native>\
            </conversion-rule>\
        </primitive-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    TypeDatabase* typeDb = TypeDatabase::instance();
    PrimitiveTypeEntry* typeA = typeDb->findPrimitiveType(QLatin1String("A"));
    QVERIFY(typeA);

    CustomConversion* conversion = typeA->customConversion();
    QVERIFY(conversion);

    QCOMPARE(typeA, conversion->ownerType());
    QCOMPARE(conversion->nativeToTargetConversion().trimmed(),
             QLatin1String("// TEMPLATE - native_to_target - START        return ConvertFromCppToPython(%IN);        // TEMPLATE - native_to_target - END"));

    QVERIFY(conversion->hasTargetToNativeConversions());
    QCOMPARE(conversion->targetToNativeConversions().size(), 1);

    CustomConversion::TargetToNativeConversion* toNative = conversion->targetToNativeConversions().first();
    QVERIFY(toNative);
    QCOMPARE(toNative->conversion().trimmed(),
             QLatin1String("// TEMPLATE - target_to_native - START        %OUT = %IN.createA();        // TEMPLATE - target_to_native - END"));
}

QTEST_APPLESS_MAIN(TestConversionRuleTag)
