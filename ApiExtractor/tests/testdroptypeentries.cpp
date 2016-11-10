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

#include "testdroptypeentries.h"
#include <QtTest/QTest>
#include "testutil.h"

static const char* cppCode ="\
    struct ValueA {};\
    struct ValueB {};\
    struct ObjectA {};\
    struct ObjectB {};\
    namespace NamespaceA {\
        struct InnerClassA {};\
        namespace InnerNamespaceA {}\
    }\
    namespace NamespaceB {}\
    enum EnumA { Value0 };\
    enum EnumB { Value1 };\
    void funcA();\
    void funcB();\
";

static const char* xmlCode = "\
<typesystem package='Foo'>\
    <value-type name='ValueA' />\
    <value-type name='ValueB' />\
    <object-type name='ObjectA' />\
    <object-type name='ObjectB' />\
    <namespace-type name='NamespaceA'>\
        <value-type name='InnerClassA' />\
        <namespace-type name='InnerNamespaceA' />\
    </namespace-type>\
    <namespace-type name='NamespaceB' />\
    <enum-type name='EnumA' />\
    <enum-type name='EnumB' />\
    <function signature='funcA()' />\
    <function signature='funcB()' />\
</typesystem>";

void TestDropTypeEntries::testDropEntries()
{
    QStringList droppedEntries(QLatin1String("Foo.ValueB"));
    droppedEntries << QLatin1String("Foo.ObjectB") << QLatin1String("Foo.NamespaceA.InnerClassA");
    droppedEntries << QLatin1String("Foo.NamespaceB") << QLatin1String("Foo.EnumB") << QLatin1String("Foo.funcB()");
    droppedEntries << QLatin1String("Foo.NamespaceA.InnerNamespaceA");
    TestUtil t(cppCode, xmlCode, false, 0, droppedEntries);

    AbstractMetaClassList classes = t.builder()->classes();
    QVERIFY(classes.findClass(QLatin1String("ValueA")));
    QVERIFY(!classes.findClass(QLatin1String("ValueB")));
    QVERIFY(classes.findClass(QLatin1String("ObjectA")));
    QVERIFY(!classes.findClass(QLatin1String("ObjectB")));
    QVERIFY(classes.findClass(QLatin1String("NamespaceA")));
    QVERIFY(!classes.findClass(QLatin1String("NamespaceA::InnerClassA")));
    QVERIFY(!classes.findClass(QLatin1String("NamespaceB")));

    AbstractMetaEnumList globalEnums = t.builder()->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.first()->name(), QLatin1String("EnumA"));

    TypeDatabase* td = TypeDatabase::instance();
    QVERIFY(td->findType(QLatin1String("funcA")));
    QVERIFY(!td->findType(QLatin1String("funcB")));
}

void TestDropTypeEntries::testDontDropEntries()
{
    TestUtil t(cppCode, xmlCode, false);

    AbstractMetaClassList classes = t.builder()->classes();
    QVERIFY(classes.findClass(QLatin1String("ValueA")));
    QVERIFY(classes.findClass(QLatin1String("ValueB")));
    QVERIFY(classes.findClass(QLatin1String("ObjectA")));
    QVERIFY(classes.findClass(QLatin1String("ObjectB")));
    QVERIFY(classes.findClass(QLatin1String("NamespaceA")));
    QVERIFY(classes.findClass(QLatin1String("NamespaceA::InnerClassA")));
    QVERIFY(classes.findClass(QLatin1String("NamespaceB")));

    QCOMPARE(t.builder()->globalEnums().size(), 2);

    TypeDatabase* td = TypeDatabase::instance();
    QVERIFY(td->findType(QLatin1String("funcA")));
    QVERIFY(td->findType(QLatin1String("funcB")));
}

static const char* cppCode2 ="\
    struct ValueA {\
        void func();\
    };\
";

static const char* xmlCode2 = "\
<typesystem package='Foo'>\
    <value-type name='ValueA'>\
        <modify-function signature='func()'>\
            <remove class='all' />\
        </modify-function>\
    </value-type>\
</typesystem>";

void TestDropTypeEntries::testDropEntryWithChildTags()
{
    QStringList droppedEntries(QLatin1String("Foo.ValueA"));
    TestUtil t(cppCode2, xmlCode2, false, 0, droppedEntries);
    QVERIFY(!t.builder()->classes().findClass(QLatin1String("ValueA")));
}

void TestDropTypeEntries::testDontDropEntryWithChildTags()
{
    TestUtil t(cppCode2, xmlCode2, false);
    QVERIFY(t.builder()->classes().findClass(QLatin1String("ValueA")));
}

QTEST_APPLESS_MAIN(TestDropTypeEntries)
