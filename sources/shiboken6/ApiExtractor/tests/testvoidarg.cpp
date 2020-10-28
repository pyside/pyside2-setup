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

#include "testvoidarg.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestVoidArg::testVoidParsedFunction()
{
    const char cppCode[] = "struct A { void a(void); };";
    const char xmlCode[] = "\n\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'/>\n\
    </typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("a"));
    QCOMPARE(addedFunc->arguments().count(), 0);
}

void TestVoidArg::testVoidAddedFunction()
{
    const char cppCode[] = "struct A { };";
    const char xmlCode[] = "\n\
    <typesystem package=\"Foo\">\n\
        <value-type name='A' >\n\
            <add-function signature=\"a(void)\"/>\n\
        </value-type>\n\
    </typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("a"));
    QCOMPARE(addedFunc->arguments().count(), 0);

}

void TestVoidArg::testVoidPointerParsedFunction()
{
    const char cppCode[] = "struct A { void a(void*); };";
    const char xmlCode[] = "\n\
    <typesystem package=\"Foo\">\n\
        <value-type name='A' />\n\
    </typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaFunction* addedFunc = classA->findFunction(QLatin1String("a"));
    QCOMPARE(addedFunc->arguments().count(), 1);

}

QTEST_APPLESS_MAIN(TestVoidArg)
