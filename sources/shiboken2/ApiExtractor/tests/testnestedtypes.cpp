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

#include "testnestedtypes.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestNestedTypes::testNestedTypesModifications()
{
    const char* cppCode ="\
    namespace OuterNamespace {\n\
        namespace InnerNamespace {\n\
            struct SomeClass {\n\
                void method() {}\n\
            };\n\
        };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='OuterNamespace'>\n\
            <namespace-type name='InnerNamespace'>\n\
                <inject-code class='native'>custom_code1();</inject-code>\n\
                <add-function signature='method()' return-type='OuterNamespace::InnerNamespace::SomeClass'>\n\
                    <inject-code class='target'>custom_code2();</inject-code>\n\
                </add-function>\n\
                <object-type name='SomeClass' target-lang-name='RenamedSomeClass'>\n\
                    <modify-function signature='method()' remove='all'/>\n\
                </object-type>\n\
            </namespace-type>\n\
        </namespace-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();

    const AbstractMetaClass *ons = AbstractMetaClass::findClass(classes, QLatin1String("OuterNamespace"));
    QVERIFY(ons);

    const AbstractMetaClass *ins = AbstractMetaClass::findClass(classes, QLatin1String("OuterNamespace::InnerNamespace"));
    QVERIFY(ins);
    QCOMPARE(ins->functions().count(), 1);
    QCOMPARE(ins->typeEntry()->codeSnips().count(), 1);
    CodeSnip snip = ins->typeEntry()->codeSnips().first();
    QCOMPARE(snip.code(), QLatin1String("custom_code1();"));

    AbstractMetaFunction* addedFunc = ins->functions().first();
    QVERIFY(addedFunc->isUserAdded());
    QCOMPARE(addedFunc->visibility(), AbstractMetaFunction::Public);
    QCOMPARE(addedFunc->functionType(), AbstractMetaFunction::NormalFunction);
    QCOMPARE(addedFunc->type()->minimalSignature(), QLatin1String("OuterNamespace::InnerNamespace::SomeClass"));

    QCOMPARE(addedFunc->modifications().size(), 1);
    QVERIFY(addedFunc->modifications().first().isCodeInjection());
    snip = addedFunc->modifications().first().snips.first();
    QCOMPARE(snip.code(), QLatin1String("custom_code2();"));

    const AbstractMetaClass *sc = AbstractMetaClass::findClass(classes, QLatin1String("OuterNamespace::InnerNamespace::SomeClass"));
    QVERIFY(ins);
    QCOMPARE(sc->functions().count(), 2); // default constructor and removed method
    AbstractMetaFunction* removedFunc = sc->functions().last();
    QVERIFY(removedFunc->isModifiedRemoved());
}


void TestNestedTypes::testDuplicationOfNestedTypes()
{
    const char* cppCode ="\
    namespace Namespace {\n\
        class SomeClass {};\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='Namespace'>\n\
            <value-type name='SomeClass'>\n\
                <add-function signature='createSomeClass(Namespace::SomeClass)'/>\n\
            </value-type>\n\
        </namespace-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    const AbstractMetaClass *nspace = AbstractMetaClass::findClass(classes, QLatin1String("Namespace"));
    QVERIFY(nspace);
    const AbstractMetaClass *cls1 = AbstractMetaClass::findClass(classes, QLatin1String("SomeClass"));
    QVERIFY(cls1);
    const AbstractMetaClass *cls2 = AbstractMetaClass::findClass(classes, QLatin1String("Namespace::SomeClass"));
    QVERIFY(cls2);
    QCOMPARE(cls1, cls2);
    QCOMPARE(cls1->name(), QLatin1String("SomeClass"));
    QCOMPARE(cls1->qualifiedCppName(), QLatin1String("Namespace::SomeClass"));

    TypeEntry* t1 = TypeDatabase::instance()->findType(QLatin1String("Namespace::SomeClass"));
    QVERIFY(t1);
    TypeEntry* t2 = TypeDatabase::instance()->findType(QLatin1String("SomeClass"));
    QVERIFY(!t2);
}

QTEST_APPLESS_MAIN(TestNestedTypes)
