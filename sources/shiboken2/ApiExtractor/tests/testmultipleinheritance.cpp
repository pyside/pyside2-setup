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

#include "testmultipleinheritance.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestMultipleInheritance::testVirtualClass()
{
    const char* cppCode ="\
    struct A {\n\
        virtual ~A();\n\
        virtual void theBug();\n\
    };\n\
    struct B {\n\
        virtual ~B();\n\
    };\n\
    struct C : A, B {\n\
    };\n\
    struct D : C {\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package=\"Foo\">\n\
        <object-type name='A' />\n\
        <object-type name='B' />\n\
        <object-type name='C' />\n\
        <object-type name='D' />\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 4);

    const AbstractMetaClass *classD = AbstractMetaClass::findClass(classes, QLatin1String("D"));
    bool functionFound = false;
    foreach (AbstractMetaFunction* f, classD->functions()) {
        if (f->name() == QLatin1String("theBug")) {
            functionFound = true;
            break;
        }
    }
    QVERIFY(functionFound);

}

QTEST_APPLESS_MAIN(TestMultipleInheritance)
