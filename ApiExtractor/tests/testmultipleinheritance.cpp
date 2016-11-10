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

void TestMultipleInheritance::testVirtualClass()
{
    const char* cppCode ="\
    struct A {\
        virtual ~A();\
        virtual void theBug();\
    };\
    struct B {\
        virtual ~B();\
    };\
    struct C : A, B {\
    };\
    struct D : C {\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <object-type name='A' /> \
        <object-type name='B' /> \
        <object-type name='C' /> \
        <object-type name='D' /> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 4);

    AbstractMetaClass* classD = classes.findClass(QLatin1String("D"));
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
