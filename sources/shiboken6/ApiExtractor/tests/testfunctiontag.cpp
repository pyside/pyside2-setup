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

#include "testfunctiontag.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetafunction.h>
#include <modifications.h>
#include <typesystem.h>

void TestFunctionTag::testFunctionTagForSpecificSignature()
{
    const char cppCode[] = "void globalFunction(int); void globalFunction(float); void dummy();\n";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\n\
        <primitive-type name='int'/>\n\
        <primitive-type name='float'/>\n\
        <function signature='globalFunction(int)'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    const TypeEntry *func = TypeDatabase::instance()->findType(QLatin1String("globalFunction"));
    QVERIFY(func);
    QCOMPARE(builder->globalFunctions().size(), 1);
}

void TestFunctionTag::testFunctionTagForAllSignatures()
{
    const char cppCode[] = "void globalFunction(int); void globalFunction(float); void dummy();\n";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\n\
        <primitive-type name='int'/>\n\
        <primitive-type name='float'/>\n\
        <function signature='globalFunction(int)'/>\n\
        <function signature='globalFunction(float)'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    const TypeEntry *func = TypeDatabase::instance()->findType(QLatin1String("globalFunction"));
    QVERIFY(func);
    QCOMPARE(builder->globalFunctions().size(), 2);
}

void TestFunctionTag::testRenameGlobalFunction()
{
    const char* cppCode ="void global_function_with_ugly_name();\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <function signature='global_function_with_ugly_name()' rename='smooth'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());

    const TypeEntry *func = TypeDatabase::instance()->findType(QLatin1String("global_function_with_ugly_name"));
    QVERIFY(func);

    QCOMPARE(builder->globalFunctions().size(), 1);
    const AbstractMetaFunction *metaFunc = builder->globalFunctions().constFirst();

    QVERIFY(metaFunc);
    QCOMPARE(metaFunc->modifications().size(), 1);
    QVERIFY(metaFunc->modifications().constFirst().isRenameModifier());
    QCOMPARE(metaFunc->modifications().constFirst().renamedTo(),
             QLatin1String("smooth"));

    QCOMPARE(metaFunc->name(), QLatin1String("smooth"));
    QCOMPARE(metaFunc->originalName(), QLatin1String("global_function_with_ugly_name"));
    QCOMPARE(metaFunc->minimalSignature(), QLatin1String("global_function_with_ugly_name()"));
}

QTEST_APPLESS_MAIN(TestFunctionTag)

