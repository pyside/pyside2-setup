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

#include "testremoveimplconv.h"
#include "testutil.h"
#include <QtTest/QTest>
#include <abstractmetalang.h>
#include <typesystem.h>

// When a constructor able to trigger implicity conversions is removed
// it should not appear in the implicity conversion list.
void TestRemoveImplConv::testRemoveImplConv()
{
    const char* cppCode ="\
    struct A {};\n\
    struct B {};\n\
    struct C {\n\
        C(const A&);\n\
        C(const B&);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'/>\n\
        <value-type name='B'/>\n\
        <value-type name='C'>\n\
            <modify-function signature='C(const A&amp;)' remove='all'/>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    const AbstractMetaClass *classB = AbstractMetaClass::findClass(classes, QLatin1String("B"));
    QVERIFY(classB);
    const AbstractMetaClass *classC = AbstractMetaClass::findClass(classes, QLatin1String("C"));
    QVERIFY(classC);
    AbstractMetaFunctionList implConv = classC->implicitConversions();
    QCOMPARE(implConv.count(), 1);
    QCOMPARE(implConv.constFirst()->arguments().constFirst().type().typeEntry(),
             classB->typeEntry());
}

QTEST_APPLESS_MAIN(TestRemoveImplConv)
