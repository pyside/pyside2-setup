/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "qtxmltosphinxtest.h"
#include "qtxmltosphinx.h"
#include <QtTest/QTest>

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(lcQtXmlToSphinxTest, "qt.sphinxtabletest");

// QtXmlToSphinxDocGeneratorInterface
QString QtXmlToSphinxTest::expandFunction(const QString &) const
{
    return {};
}

QString QtXmlToSphinxTest::expandClass(const QString &, const QString &) const
{
    return {};
}

QString QtXmlToSphinxTest::resolveContextForMethod(const QString &, const QString &) const
{
    return {};
}

const QLoggingCategory &QtXmlToSphinxTest::loggingCategory() const
{
    return lcQtXmlToSphinxTest();
}

QString QtXmlToSphinxTest::transformXml(const QString &xml) const
{
    return QtXmlToSphinx(this, m_parameters, xml).result();
}

void QtXmlToSphinxTest::testTable_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("expected");

    QTest::newRow("emptyString") << QString() << QString();

    // testSimpleTable
    const char *xml = "\
<table>\
    <header>\
        <item>\
            <para>Header 1</para>\
        </item>\
        <item>\
            <para>Header 2</para>\
        </item>\
    </header>\
    <row>\
        <item>\
            <para>1 1</para>\
        </item>\
        <item>\
            <para>1 2</para>\
        </item>\
    </row>\
    <row>\
        <item>\
            <para>2 1</para>\
        </item>\
        <item>\
            <para>2 2</para>\
        </item>\
    </row>\
</table>";

    const char *expected = "\n\
    +--------+--------+\n\
    |Header 1|Header 2|\n\
    +--------+--------+\n\
    |1 1     |1 2     |\n\
    +--------+--------+\n\
    |2 1     |2 2     |\n\
    +--------+--------+\n\
\n";

    QTest::newRow("testSimpleTable")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);

    // testRowSpan
    xml = "\
<table>\
    <header>\
        <item>\
            <para>Header 1</para>\
        </item>\
        <item>\
            <para>Header 2</para>\
        </item>\
    </header>\
    <row>\
        <item colspan=\"2\">\
            <para>I'm a big text!</para>\
        </item>\
    </row>\
    <row>\
        <item>\
            <para>2 1</para>\
        </item>\
        <item>\
            <para>2 2</para>\
        </item>\
    </row>\
</table>";

    expected = "\n\
    +---------------+--------+\n\
    |Header 1       |Header 2|\n\
    +---------------+--------+\n\
    |I'm a big text!         |\n\
    +---------------+--------+\n\
    |2 1            |2 2     |\n\
    +---------------+--------+\n\
\n";

    QTest::newRow("testColSpan")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);

    // testRowSpan
    xml = "\
<table>\
    <header>\
        <item>\
            <para>Header 1</para>\
        </item>\
        <item>\
            <para>Header 2</para>\
        </item>\
    </header>\
    <row>\
        <item rowspan=\"2\">\
            <para>1.1</para>\
        </item>\
        <item>\
            <para>1.2</para>\
        </item>\
    </row>\
    <row>\
        <item>\
            <para>2 2</para>\
        </item>\
    </row>\
</table>";

    expected = "\n\
    +--------+--------+\n\
    |Header 1|Header 2|\n\
    +--------+--------+\n\
    |1.1     |1.2     |\n\
    +        +--------+\n\
    |        |2 2     |\n\
    +--------+--------+\n\
\n";

    QTest::newRow("testRowSpan")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);

    // testComplexTable
    xml = "\
<table>\
    <header>\
        <item>\
            <para>Header 1</para>\
        </item>\
        <item>\
            <para>Header 2</para>\
        </item>\
        <item>\
            <para>Header 3</para>\
        </item>\
    </header>\
    <row>\
        <item rowspan=\"2\">\
            <para>1.1</para>\
        </item>\
        <item colspan=\"2\">\
            <para>1.2</para>\
        </item>\
    </row>\
    <row>\
        <item>\
            <para>2 2</para>\
        </item>\
        <item>\
            <para>2 3</para>\
        </item>\
    </row>\
</table>";

    expected = "\n\
    +--------+--------+--------+\n\
    |Header 1|Header 2|Header 3|\n\
    +--------+--------+--------+\n\
    |1.1     |1.2              |\n\
    +        +--------+--------+\n\
    |        |2 2     |2 3     |\n\
    +--------+--------+--------+\n\
\n";

    QTest::newRow("testComplexTable")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);

    // testRowSpan2
    xml = "\
<table>\
    <header>\
        <item><para>h1</para></item>\
        <item><para>h2</para></item>\
        <item><para>h3</para></item>\
        <item><para>h4</para></item>\
    </header>\
    <row>\
        <item rowspan=\"6\"><para>A</para></item>\
        <item rowspan=\"6\"><para>B</para></item>\
        <item><para>C</para></item>\
        <item><para>D</para></item>\
    </row>\
    <row>\
        <item><para>E</para></item>\
        <item><para>F</para></item>\
    </row>\
    <row>\
        <item><para>E</para></item>\
        <item><para>F</para></item>\
    </row>\
    <row>\
        <item><para>E</para></item>\
        <item><para>F</para></item>\
    </row>\
    <row>\
        <item><para>E</para></item>\
        <item><para>F</para></item>\
    </row>\
    <row>\
        <item><para>E</para></item>\
        <item><para>F</para></item>\
    </row>\
</table>";

    expected = "\n\
    +--+--+--+--+\n\
    |h1|h2|h3|h4|\n\
    +--+--+--+--+\n\
    |A |B |C |D |\n\
    +  +  +--+--+\n\
    |  |  |E |F |\n\
    +  +  +--+--+\n\
    |  |  |E |F |\n\
    +  +  +--+--+\n\
    |  |  |E |F |\n\
    +  +  +--+--+\n\
    |  |  |E |F |\n\
    +  +  +--+--+\n\
    |  |  |E |F |\n\
    +--+--+--+--+\n\
\n";

    QTest::newRow("testRowSpan2")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);

    // testBrokenTable
    xml = "\
<table>\
    <header>\
        <item>\
            <para>Header 1</para>\
        </item>\
        <item>\
            <para>Header 2</para>\
        </item>\
    </header>\
    <row>\
        <item>\
            <para>1.1</para>\
        </item>\
        <item>\
            <para>1.2</para>\
        </item>\
    </row>\
    <row>\
        <item colspan=\"2\">\
            <para>2 2</para>\
        </item>\
        <item>\
            <para>2 3</para>\
        </item>\
        <item>\
            <para>2 4</para>\
        </item>\
        <item>\
            <para>2 5</para>\
        </item>\
    </row>\
    <row>\
        <item>\
            <para>3 1</para>\
        </item>\
        <item>\
            <para>3 2</para>\
        </item>\
        <item>\
            <para>3 3</para>\
        </item>\
    </row>\
</table>";

    expected = "\n\
    +--------+------------+\n\
    |Header 1|Header 2    |\n\
    +--------+------------+\n\
    |1.1     |1.2         |\n\
    +--------+------------+\n\
    |2 2       2 3 2 4 2 5|\n\
    +--------+------------+\n\
    |3 1     |3 2 3 3     |\n\
    +--------+------------+\n\
\n";

    QTest::newRow("testBrokenTable")
        << QString::fromLatin1(xml) << QString::fromLatin1(expected);
}

void QtXmlToSphinxTest::testTable()
{
    QFETCH(QString, xml);
    QFETCH(QString, expected);

    const QString actual = transformXml(xml);

    QEXPECT_FAIL("testBrokenTable", "testBrokenTable fails", Continue);
    QCOMPARE(actual, expected);
}

QTEST_APPLESS_MAIN( QtXmlToSphinxTest)
