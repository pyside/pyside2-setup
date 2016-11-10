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

#include "testnestedtypes.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestNestedTypes::testNestedTypesModifications()
{
    const char* cppCode ="\
    namespace OuterNamespace {\
        namespace InnerNamespace {\
            struct SomeClass {\
                void method() {}\
            };\
        };\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <namespace-type name='OuterNamespace'>\
            <namespace-type name='InnerNamespace'>\
                <inject-code class='native'>custom_code1();</inject-code>\
                <add-function signature='method()' return-type='OuterNamespace::InnerNamespace::SomeClass'>\
                    <inject-code class='target'>custom_code2();</inject-code>\
                </add-function>\
                <object-type name='SomeClass' target-lang-name='RenamedSomeClass'>\
                    <modify-function signature='method()' remove='all'/>\
                </object-type>\
            </namespace-type>\
        </namespace-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();

    AbstractMetaClass* ons = classes.findClass(QLatin1String("OuterNamespace"));
    QVERIFY(ons);

    AbstractMetaClass* ins = classes.findClass(QLatin1String("OuterNamespace::InnerNamespace"));
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

    AbstractMetaClass* sc = classes.findClass(QLatin1String("OuterNamespace::InnerNamespace::SomeClass"));
    QVERIFY(ins);
    QCOMPARE(sc->functions().count(), 2); // default constructor and removed method
    AbstractMetaFunction* removedFunc = sc->functions().last();
    QVERIFY(removedFunc->isModifiedRemoved());
}


void TestNestedTypes::testDuplicationOfNestedTypes()
{
    const char* cppCode ="\
    namespace Namespace {\
        class SomeClass {};\
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <namespace-type name='Namespace'>\
            <value-type name='SomeClass'>\
                <add-function signature='createSomeClass(Namespace::SomeClass)'/>\
            </value-type>\
        </namespace-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* nspace = classes.findClass(QLatin1String("Namespace"));
    QVERIFY(nspace);
    AbstractMetaClass* cls1 = classes.findClass(QLatin1String("SomeClass"));
    QVERIFY(cls1);
    AbstractMetaClass* cls2 = classes.findClass(QLatin1String("Namespace::SomeClass"));
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
