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

#include "testctorinformation.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestCtorInformation::testCtorIsPrivate()
{
    const char* cppCode = "class Control { public: Control() {} };\
                           class Subject { private: Subject() {} };\
                           class CtorLess { };";
    const char* xmlCode = "<typesystem package='Foo'>\
                                <value-type name='Control'/>\
                                <object-type name='Subject'/>\
                                <value-type name='CtorLess'/>\
                           </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 3);
    QCOMPARE(classes.findClass(QLatin1String("Control"))->hasNonPrivateConstructor(), true);
    QCOMPARE(classes.findClass(QLatin1String("Subject"))->hasNonPrivateConstructor(), false);
    QCOMPARE(classes.findClass(QLatin1String("CtorLess"))->hasNonPrivateConstructor(), true);
}

void TestCtorInformation::testHasNonPrivateCtor()
{
    const char* cppCode = "template<typename T>\
                           struct Base { Base(double) {} };\
                           typedef Base<int> Derived;\
                          ";
    const char* xmlCode = "<typesystem package='Foo'>\
                                <primitive-type name='int' />\
                                <primitive-type name='double' />\
                                <object-type name='Base' generate='no'/>\
                                <object-type name='Derived'/>\
                           </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    AbstractMetaClass* base = classes.findClass(QLatin1String("Base"));
    QCOMPARE(base->hasNonPrivateConstructor(), true);
    AbstractMetaClass* derived = classes.findClass(QLatin1String("Derived"));
    QCOMPARE(derived->hasNonPrivateConstructor(), true);
}

QTEST_APPLESS_MAIN(TestCtorInformation)
