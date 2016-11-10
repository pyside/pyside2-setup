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

#include "testnamespace.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestNamespace::testNamespaceMembers()
{
    const char* cppCode = "\
    namespace Namespace\
    {\
        enum Option {\
            OpZero,\
            OpOne\
        };\
        void foo(Option opt);\
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <namespace-type name='Namespace'>\
            <enum-type name='Option' /> \
        </namespace-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* ns = classes.findClass(QLatin1String("Namespace"));
    QVERIFY(ns);
    const AbstractMetaEnum* metaEnum = ns->findEnum(QLatin1String("Option"));
    QVERIFY(metaEnum);
    const AbstractMetaFunction* func = ns->findFunction(QLatin1String("foo"));
    QVERIFY(func);
}

void TestNamespace::testNamespaceInnerClassMembers()
{
    const char* cppCode = "\
    namespace OuterNamespace\
    {\
        namespace InnerNamespace {\
            struct SomeClass {\
                void method();\
            };\
        };\
    };";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <namespace-type name='OuterNamespace'>\
            <namespace-type name='InnerNamespace'>\
                <value-type name='SomeClass' /> \
            </namespace-type>\
        </namespace-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* ons = classes.findClass(QLatin1String("OuterNamespace"));
    QVERIFY(ons);
    AbstractMetaClass* ins = classes.findClass(QLatin1String("OuterNamespace::InnerNamespace"));
    QVERIFY(ins);
    AbstractMetaClass* sc = classes.findClass(QLatin1String("OuterNamespace::InnerNamespace::SomeClass"));
    QVERIFY(sc);
    const AbstractMetaFunction* meth = sc->findFunction(QLatin1String("method"));
    QVERIFY(meth);
}

QTEST_APPLESS_MAIN(TestNamespace)

