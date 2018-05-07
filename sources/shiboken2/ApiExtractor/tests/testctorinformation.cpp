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

#include "testctorinformation.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestCtorInformation::testCtorIsPrivate()
{
    const char* cppCode = "class Control { public: Control() {} };\n\
                           class Subject { private: Subject() {} };\n\
                           class CtorLess { };\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
                                <value-type name='Control'/>\n\
                                <object-type name='Subject'/>\n\
                                <value-type name='CtorLess'/>\n\
                           </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Control"))->hasNonPrivateConstructor(), true);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("Subject"))->hasNonPrivateConstructor(), false);
    QCOMPARE(AbstractMetaClass::findClass(classes, QLatin1String("CtorLess"))->hasNonPrivateConstructor(), true);
}

void TestCtorInformation::testHasNonPrivateCtor()
{
    const char* cppCode = "template<typename T>\n\
                           struct Base { Base(double) {} };\n\
                           typedef Base<int> Derived;\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
                                <primitive-type name='int'/>\n\
                                <primitive-type name='double'/>\n\
                                <object-type name='Base' generate='no'/>\n\
                                <object-type name='Derived'/>\n\
                           </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    const AbstractMetaClass *base = AbstractMetaClass::findClass(classes, QLatin1String("Base"));
    QCOMPARE(base->hasNonPrivateConstructor(), true);
    const AbstractMetaClass *derived = AbstractMetaClass::findClass(classes, QLatin1String("Derived"));
    QCOMPARE(derived->hasNonPrivateConstructor(), true);
}

QTEST_APPLESS_MAIN(TestCtorInformation)
