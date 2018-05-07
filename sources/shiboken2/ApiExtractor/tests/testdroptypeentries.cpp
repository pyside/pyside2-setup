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

#include "testdroptypeentries.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

static const char* cppCode ="\
    struct ValueA {};\n\
    struct ValueB {};\n\
    struct ObjectA {};\n\
    struct ObjectB {};\n\
    namespace NamespaceA {\n\
        struct InnerClassA {};\n\
        namespace InnerNamespaceA {}\n\
    }\n\
    namespace NamespaceB {}\n\
    enum EnumA { Value0 };\n\
    enum EnumB { Value1 };\n\
    void funcA();\n\
    void funcB();\n";

static const char* xmlCode = "\
<typesystem package='Foo'>\n\
    <value-type name='ValueA'/>\n\
    <value-type name='ValueB'/>\n\
    <object-type name='ObjectA'/>\n\
    <object-type name='ObjectB'/>\n\
    <namespace-type name='NamespaceA'>\n\
        <value-type name='InnerClassA'/>\n\
        <namespace-type name='InnerNamespaceA'/>\n\
    </namespace-type>\n\
    <namespace-type name='NamespaceB'/>\n\
    <enum-type name='EnumA'/>\n\
    <enum-type name='EnumB'/>\n\
    <function signature='funcA()'/>\n\
    <function signature='funcB()'/>\n\
</typesystem>\n";

void TestDropTypeEntries::testDropEntries()
{
    QStringList droppedEntries(QLatin1String("Foo.ValueB"));
    droppedEntries << QLatin1String("Foo.ObjectB") << QLatin1String("Foo.NamespaceA.InnerClassA");
    droppedEntries << QLatin1String("Foo.NamespaceB") << QLatin1String("Foo.EnumB") << QLatin1String("Foo.funcB()");
    droppedEntries << QLatin1String("Foo.NamespaceA.InnerNamespaceA");
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false, Q_NULLPTR, droppedEntries));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ValueA")));
    QVERIFY(!AbstractMetaClass::findClass(classes, QLatin1String("ValueB")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ObjectA")));
    QVERIFY(!AbstractMetaClass::findClass(classes, QLatin1String("ObjectB")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("NamespaceA")));
    QVERIFY(!AbstractMetaClass::findClass(classes, QLatin1String("NamespaceA::InnerClassA")));
    QVERIFY(!AbstractMetaClass::findClass(classes, QLatin1String("NamespaceB")));

    AbstractMetaEnumList globalEnums = builder->globalEnums();
    QCOMPARE(globalEnums.count(), 1);
    QCOMPARE(globalEnums.first()->name(), QLatin1String("EnumA"));

    TypeDatabase* td = TypeDatabase::instance();
    QVERIFY(td->findType(QLatin1String("funcA")));
    QVERIFY(!td->findType(QLatin1String("funcB")));
}

void TestDropTypeEntries::testDontDropEntries()
{
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ValueA")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ValueB")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ObjectA")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("ObjectB")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("NamespaceA")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("NamespaceA::InnerClassA")));
    QVERIFY(AbstractMetaClass::findClass(classes, QLatin1String("NamespaceB")));

    QCOMPARE(builder->globalEnums().size(), 2);

    TypeDatabase* td = TypeDatabase::instance();
    QVERIFY(td->findType(QLatin1String("funcA")));
    QVERIFY(td->findType(QLatin1String("funcB")));
}

static const char* cppCode2 ="\
    struct ValueA {\n\
        void func();\n\
    };\n";

static const char* xmlCode2 = "\
<typesystem package='Foo'>\n\
    <value-type name='ValueA'>\n\
        <modify-function signature='func()'>\n\
            <remove class='all'/>\n\
        </modify-function>\n\
    </value-type>\n\
</typesystem>\n";

void TestDropTypeEntries::testDropEntryWithChildTags()
{
    QStringList droppedEntries(QLatin1String("Foo.ValueA"));
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode2, xmlCode2, false, Q_NULLPTR, droppedEntries));
    QVERIFY(!builder.isNull());
    QVERIFY(!AbstractMetaClass::findClass(builder->classes(), QLatin1String("ValueA")));
}


void TestDropTypeEntries::testDontDropEntryWithChildTags()
{
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode2, xmlCode2, false));
    QVERIFY(!builder.isNull());
    QVERIFY(AbstractMetaClass::findClass(builder->classes(), QLatin1String("ValueA")));
}

QTEST_APPLESS_MAIN(TestDropTypeEntries)
