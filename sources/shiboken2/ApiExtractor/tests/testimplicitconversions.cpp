/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#include "testimplicitconversions.h"
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>
#include <QtTest/QTest>

void TestImplicitConversions::testWithPrivateCtors()
{
    const char* cppCode ="\
    class B;\n\
    class C;\n\
    class A {\n\
        A(const B&);\n\
    public:\n\
        A(const C&);\n\
    };\n\
    class B {};\n\
    class C {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'/>\n\
        <value-type name='B'/>\n\
        <value-type name='C'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);

    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    const AbstractMetaClass *classC = AbstractMetaClass::findClass(classes, QLatin1String("C"));
    AbstractMetaFunctionList implicitConvs = classA->implicitConversions();
    QCOMPARE(implicitConvs.count(), 1);
    QCOMPARE(implicitConvs.first()->arguments().first()->type()->typeEntry(), classC->typeEntry());
}

void TestImplicitConversions::testWithModifiedVisibility()
{
    const char* cppCode ="\
    class B;\n\
    class A {\n\
    public:\n\
        A(const B&);\n\
    };\n\
    class B {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <modify-function signature='A(const B&amp;)'>\n\
                <access modifier='private'/>\n\
            </modify-function>\n\
        </value-type>\n\
        <value-type name='B'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    AbstractMetaFunctionList implicitConvs = classA->implicitConversions();
    QCOMPARE(implicitConvs.count(), 1);
    QCOMPARE(implicitConvs.first()->arguments().first()->type()->typeEntry(), classB->typeEntry());
}


void TestImplicitConversions::testWithAddedCtor()
{
    const char* cppCode ="\
    class B;\n\
    class A {\n\
    public:\n\
        A(const B&);\n\
    };\n\
    class B {};\n\
    class C {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <custom-type name='TARGETLANGTYPE'/>\n\
        <value-type name='A'>\n\
            <add-function signature='A(const C&amp;)'/>\n\
        </value-type>\n\
        <value-type name='B'>\n\
            <add-function signature='B(TARGETLANGTYPE*)'/>\n\
        </value-type>\n\
        <value-type name='C'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);

    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    AbstractMetaFunctionList implicitConvs = classA->implicitConversions();
    QCOMPARE(implicitConvs.count(), 2);

    // Added constructors with custom types should never result in implicit converters.
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    implicitConvs = classB->implicitConversions();
    QCOMPARE(implicitConvs.count(), 0);
}

void TestImplicitConversions::testWithExternalConversionOperator()
{
    const char* cppCode ="\
    class A {};\n\
    struct B {\n\
        operator A() const;\n\
    };\n";
    const char* xmlCode = "\n\
    <typesystem package='Foo'>\n\
        <value-type name='A'/>\n\
        <value-type name='B'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    AbstractMetaClass* classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    AbstractMetaFunctionList implicitConvs = classA->implicitConversions();
    QCOMPARE(implicitConvs.count(), 1);
    AbstractMetaFunctionList externalConvOps = classA->externalConversionOperators();
    QCOMPARE(externalConvOps.count(), 1);

    const AbstractMetaFunction* convOp = 0;
    for (const AbstractMetaFunction *func : classB->functions()) {
        if (func->isConversionOperator())
            convOp = func;
    }
    QVERIFY(convOp);
    QCOMPARE(implicitConvs.first(), convOp);
}

QTEST_APPLESS_MAIN(TestImplicitConversions)
