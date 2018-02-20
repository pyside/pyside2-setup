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

#include "testmodifydocumentation.h"

#include <QCoreApplication>
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>
#include <qtdocparser.h>

void TestModifyDocumentation::testModifyDocumentation()
{
    const char* cppCode ="struct B { void b(); }; class A {};\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <value-type name='B'>\n\
        <modify-function signature='b()' remove='all'/>\n\
    </value-type>\n\
    <value-type name='A'>\n\
    <modify-documentation xpath='description/para[3]'>\n\
    &lt;para>Some changed contents here&lt;/para>\n\
    </modify-documentation>\n\
    </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    AbstractMetaClass *classA = AbstractMetaClass::findClass(builder->classes(), QLatin1String("A"));
    QVERIFY(classA);
    DocModificationList docMods = classA->typeEntry()->docModifications();
    QCOMPARE(docMods.count(), 1);
    QCOMPARE(docMods[0].code().trimmed(), QLatin1String("<para>Some changed contents here</para>"));
    QCOMPARE(docMods[0].signature(), QString());
    QtDocParser docParser;
    docParser.setDocumentationDataDirectory(QDir::currentPath());
    docParser.fillDocumentation(classA);

    QVERIFY(!classA->documentation().value().trimmed().isEmpty());
    QCOMPARE(classA->documentation().value(), QLatin1String("<?xml version=\"1.0\"?>\n\
<description>oi\n\
                <para>Paragraph number 1</para>\n\
    <para>Paragraph number 2</para>\n\
    <para>Some changed contents here</para>\n\
</description>"));
}

// We expand QTEST_MAIN macro but using QCoreApplication instead of QApplication
// because this test needs an event loop but can't use QApplication to avoid a crash
// on our ARMEL/FRAMANTLE buildbot
int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    TestModifyDocumentation tc;
    return QTest::qExec(&tc, argc, argv);
}
