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

#include "testnamespace.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <abstractmetaenum.h>
#include <typesystem.h>

void NamespaceTest::testNamespaceMembers()
{
    const char* cppCode = "\
    namespace Namespace\n\
    {\n\
        enum Option {\n\
            OpZero,\n\
            OpOne\n\
        };\n\
        void foo(Option opt);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='Namespace'>\n\
            <enum-type name='Option' />\n\
        </namespace-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClass *ns = AbstractMetaClass::findClass(classes, QLatin1String("Namespace"));
    QVERIFY(ns);
    auto metaEnum = ns->findEnum(QLatin1String("Option"));
    QVERIFY(metaEnum.has_value());
    const AbstractMetaFunction* func = ns->findFunction(QLatin1String("foo"));
    QVERIFY(func);
}

void NamespaceTest::testNamespaceInnerClassMembers()
{
    const char* cppCode = "\
    namespace OuterNamespace\n\
    {\n\
        namespace InnerNamespace {\n\
            struct SomeClass {\n\
                void method();\n\
            };\n\
        };\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='OuterNamespace'>\n\
            <namespace-type name='InnerNamespace'>\n\
                <value-type name='SomeClass'/>\n\
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
    const AbstractMetaClass *sc = AbstractMetaClass::findClass(classes, QLatin1String("OuterNamespace::InnerNamespace::SomeClass"));
    QVERIFY(sc);
    const AbstractMetaFunction* meth = sc->findFunction(QLatin1String("method"));
    QVERIFY(meth);
}

QTEST_APPLESS_MAIN(NamespaceTest)

