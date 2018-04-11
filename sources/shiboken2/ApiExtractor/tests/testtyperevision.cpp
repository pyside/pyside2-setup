/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#include "testtyperevision.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestTypeRevision::testRevisionAttr()
{
    const char* cppCode = "class Rev_0 {};"
                          "class Rev_1 {};"
                          "class Rev_2 { public: enum Rev_3 { X }; enum Rev_5 { Y }; };";
    const char* xmlCode = "<typesystem package=\"Foo\">"
                        "<value-type name=\"Rev_0\"/>"
                        "<value-type name=\"Rev_1\" revision=\"1\"/>"
                        "<object-type name=\"Rev_2\" revision=\"2\">"
                        "    <enum-type name=\"Rev_3\" revision=\"3\" flags=\"Flag_4\" flags-revision=\"4\" />"
                        "    <enum-type name=\"Rev_5\" revision=\"5\" flags=\"Flag_5\" />"
                        "</object-type>"
                        "</typesystem>";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *rev0 = AbstractMetaClass::findClass(classes, QLatin1String("Rev_0"));
    QCOMPARE(getTypeRevision(rev0->typeEntry()), 0);

    const AbstractMetaClass *rev1 = AbstractMetaClass::findClass(classes, QLatin1String("Rev_1"));
    QCOMPARE(getTypeRevision(rev1->typeEntry()), 1);

    AbstractMetaClass *rev2 = AbstractMetaClass::findClass(classes, QLatin1String("Rev_2"));
    QCOMPARE(getTypeRevision(rev2->typeEntry()), 2);

    AbstractMetaEnum* rev3 = rev2->findEnum(QLatin1String("Rev_3"));
    QCOMPARE(getTypeRevision(rev3->typeEntry()), 3);
    FlagsTypeEntry* rev4 = rev3->typeEntry()->flags();
    QCOMPARE(getTypeRevision(rev4), 4);
    AbstractMetaEnum* rev5 = rev2->findEnum(QLatin1String("Rev_5"));
    QCOMPARE(getTypeRevision(rev5->typeEntry()), 5);
    QCOMPARE(getTypeRevision(rev5->typeEntry()->flags()), 5);
}

QTEST_APPLESS_MAIN(TestTypeRevision)


