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

#include "testremoveoperatormethod.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestRemoveOperatorMethod::testRemoveOperatorMethod()
{
    const char* cppCode ="\
    struct A {\
        A& operator>>(char&);\
        A& operator>>(char*);\
        A& operator>>(signed short&);\
        A& operator>>(unsigned short&);\
        A& operator>>(signed int&);\
        A& operator>>(unsigned int&);\
        A& operator>>(signed long&);\
        A& operator>>(unsigned long&);\
        A& operator>>(__int64&);\
        A& operator>>(unsigned __int64&);\
        A& operator>>(float&);\
        A& operator>>(double&);\
        A& operator>>(Char&);\
        A& operator>>(ByteArray&);\
        A& operator>>(String&);\
    };\
    struct Char {};\
    struct ByteArray {};\
    struct String {};\
    ";
    const char* xmlCode = "\
    <typesystem package='Foo'>\
        <primitive-type name='char' />\
        <primitive-type name='signed short' />\
        <primitive-type name='unsigned short' />\
        <primitive-type name='signed int' />\
        <primitive-type name='unsigned int' />\
        <primitive-type name='signed long' />\
        <primitive-type name='unsigned long' />\
        <primitive-type name='__int64' />\
        <primitive-type name='unsigned __int64' />\
        <primitive-type name='float' />\
        <primitive-type name='double' />\
        <primitive-type name='Char' />\
        <primitive-type name='String' />\
        <value-type name='ByteArray' />\
        <object-type name='A'>\
            <modify-function signature='operator&gt;&gt;(char&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(char*)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(signed short&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(unsigned short&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(signed int&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(unsigned int&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(signed long&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(unsigned long&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(__int64&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(unsigned __int64&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(float&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(double&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(Char&amp;)' remove='all'/>\
            <modify-function signature='operator&gt;&gt;(String&amp;)' remove='all'/>\
        </object-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode, false);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass(QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().size(), 15);
    QStringList removedSignatures;
    removedSignatures.append(QLatin1String("operator>>(char&)"));
    removedSignatures.append(QLatin1String("operator>>(char*)"));
    removedSignatures.append(QLatin1String("operator>>(signed short&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned short&)"));
    removedSignatures.append(QLatin1String("operator>>(signed int&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned int&)"));
    removedSignatures.append(QLatin1String("operator>>(signed long&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned long&)"));
    removedSignatures.append(QLatin1String("operator>>(__int64&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned __int64&)"));
    removedSignatures.append(QLatin1String("operator>>(float&)"));
    removedSignatures.append(QLatin1String("operator>>(double&)"));
    removedSignatures.append(QLatin1String("operator>>(Char&)"));
    removedSignatures.append(QLatin1String("operator>>(String&)"));
    int notRemoved = classA->functions().size();
    foreach (const AbstractMetaFunction* f, classA->functions()) {
        QCOMPARE(f->isModifiedRemoved(), bool(removedSignatures.contains(f->minimalSignature())));
        notRemoved -= int(f->isModifiedRemoved());
    }
    QCOMPARE(notRemoved, 2);
}

QTEST_APPLESS_MAIN(TestRemoveOperatorMethod)

