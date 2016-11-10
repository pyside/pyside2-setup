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

#include "testfunctiontag.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestFunctionTag::testFunctionTagForSpecificSignature()
{
    const char cppCode[] = "void globalFunction(int); void globalFunction(float); void dummy()";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <primitive-type name='int'/> \
        <primitive-type name='float'/> \
        <function signature='globalFunction(int)'/>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);

    FunctionTypeEntry* func = (FunctionTypeEntry*) TypeDatabase::instance()->findType(QLatin1String("globalFunction"));
    QVERIFY(func);
    QCOMPARE(t.builder()->globalFunctions().size(), 1);
}

void TestFunctionTag::testFunctionTagForAllSignatures()
{
    const char cppCode[] = "void globalFunction(int); void globalFunction(float); void dummy();";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\">\
        <primitive-type name='int'/> \
        <primitive-type name='float'/> \
        <function signature='globalFunction(int)'/>\
        <function signature='globalFunction(float)'/>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);

    FunctionTypeEntry* func = (FunctionTypeEntry*) TypeDatabase::instance()->findType(QLatin1String("globalFunction"));
    QVERIFY(func);
    QCOMPARE(t.builder()->globalFunctions().size(), 2);
}

void TestFunctionTag::testRenameGlobalFunction()
{
    const char* cppCode ="void global_function_with_ugly_name();";
    const char* xmlCode = "\
    <typesystem package='Foo'> \
        <function signature='global_function_with_ugly_name()' rename='smooth' />\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);

    FunctionTypeEntry* func = (FunctionTypeEntry*) TypeDatabase::instance()->findType(QLatin1String("global_function_with_ugly_name"));
    QVERIFY(func);

    QCOMPARE(t.builder()->globalFunctions().size(), 1);
    const AbstractMetaFunction* metaFunc = t.builder()->globalFunctions().first();

    QVERIFY(metaFunc);
    QCOMPARE(metaFunc->modifications().size(), 1);
    QVERIFY(metaFunc->modifications().first().isRenameModifier());
    QCOMPARE(metaFunc->modifications().first().renamedTo(), QLatin1String("smooth"));

    QCOMPARE(metaFunc->name(), QLatin1String("smooth"));
    QCOMPARE(metaFunc->originalName(), QLatin1String("global_function_with_ugly_name"));
    QCOMPARE(metaFunc->minimalSignature(), QLatin1String("global_function_with_ugly_name()"));
}

QTEST_APPLESS_MAIN(TestFunctionTag)

