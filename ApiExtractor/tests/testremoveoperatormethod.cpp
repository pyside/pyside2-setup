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
#include <abstractmetalang.h>
#include <typesystem.h>

void TestRemoveOperatorMethod::testRemoveOperatorMethod()
{
    const char* cppCode ="\
    #include <stdint.h>\n\
    \n\
    struct Char {};\n\
    struct ByteArray {};\n\
    struct String {};\n\
    \n\
    struct A {\n\
        A& operator>>(char&);\n\
        A& operator>>(char*);\n\
        A& operator>>(short&);\n\
        A& operator>>(unsigned short&);\n\
        A& operator>>(int&);\n\
        A& operator>>(unsigned int&);\n\
        A& operator>>(int64_t&);\n\
        A& operator>>(uint64_t&);\n\
        A& operator>>(float&);\n\
        A& operator>>(double&);\n\
        A& operator>>(Char&);\n\
        A& operator>>(ByteArray&);\n\
        A& operator>>(String&);\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <primitive-type name='char'/>\n\
        <primitive-type name='short'/>\n\
        <primitive-type name='unsigned short'/>\n\
        <primitive-type name='int'/>\n\
        <primitive-type name='unsigned int'/>\n\
        <primitive-type name='int64_t'/>\n\
        <primitive-type name='uint64_t'/>\n\
        <primitive-type name='float'/>\n\
        <primitive-type name='double'/>\n\
        <primitive-type name='Char'/>\n\
        <primitive-type name='String'/>\n\
        <value-type name='ByteArray'/>\n\
        <object-type name='A'>\n\
            <modify-function signature='operator&gt;&gt;(char&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(char*)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(short&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(unsigned short&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(int&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(unsigned int&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(int64_t&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(uint64_t&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(float&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(double&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(Char&amp;)' remove='all'/>\n\
            <modify-function signature='operator&gt;&gt;(String&amp;)' remove='all'/>\n\
        </object-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->functions().size(), 14);
    QStringList removedSignatures;
    removedSignatures.append(QLatin1String("operator>>(char&)"));
    removedSignatures.append(QLatin1String("operator>>(char*)"));
    removedSignatures.append(QLatin1String("operator>>(short&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned short&)"));
    removedSignatures.append(QLatin1String("operator>>(int&)"));
    removedSignatures.append(QLatin1String("operator>>(unsigned int&)"));
    removedSignatures.append(QLatin1String("operator>>(int64_t&)"));
    removedSignatures.append(QLatin1String("operator>>(uint64_t&)"));
    removedSignatures.append(QLatin1String("operator>>(float&)"));
    removedSignatures.append(QLatin1String("operator>>(double&)"));
    removedSignatures.append(QLatin1String("operator>>(Char&)"));
    removedSignatures.append(QLatin1String("operator>>(String&)"));
    int notRemoved = classA->functions().size();
    const AbstractMetaFunctionList &functions = classA->functions();
    for (const AbstractMetaFunction *f : functions) {
        QCOMPARE(f->isModifiedRemoved(), bool(removedSignatures.contains(f->minimalSignature())));
        notRemoved -= int(f->isModifiedRemoved());
    }
    QCOMPARE(notRemoved, 2);
}

QTEST_APPLESS_MAIN(TestRemoveOperatorMethod)

