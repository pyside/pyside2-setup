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

#include "testcodeinjection.h"
#include <QFileInfo>
#include <QDir>
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestCodeInjections::testReadFile_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QString>("snippet");
    QTest::addColumn<QString>("expected");

    QTest::newRow("utf8")
        << QString::fromLatin1(":/utf8code.txt")
        << QString()
        << QString::fromUtf8("\xC3\xA1\xC3\xA9\xC3\xAD\xC3\xB3\xC3\xBA");

    QTest::newRow("snippet")
        << QString::fromLatin1(":/injectedcode.txt")
        << QString::fromLatin1("label")
        << QString::fromLatin1("code line");
}

void TestCodeInjections::testReadFile()
{
    QFETCH(QString, filePath);
    QFETCH(QString, snippet);
    QFETCH(QString, expected);

    const char* cppCode ="struct A {};\n";
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    QString attribute = QLatin1String("file='") + filePath + QLatin1Char('\'');
    if (!snippet.isEmpty())
        attribute += QLatin1String(" snippet='") + snippet + QLatin1Char('\'');

    QString xmlCode = QLatin1String("\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <conversion-rule ") + attribute + QLatin1String("/>\n\
            <inject-code class='target' ") + attribute + QLatin1String("/>\n\
        </value-type>\n\
        <value-type name='A::B'/>\n\
    </typesystem>\n");
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode.toLocal8Bit().constData()));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QCOMPARE(classA->typeEntry()->codeSnips().count(), 1);
    QString code = classA->typeEntry()->codeSnips().first().code();
    QVERIFY(code.indexOf(expected) != -1);
    code = classA->typeEntry()->conversionRule();
    QVERIFY(code.indexOf(expected) != -1);
}

void TestCodeInjections::testInjectWithValidApiVersion()
{
    const char* cppCode ="struct A {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <inject-code class='target' since='1.0'>\n\
                test Inject code\n\
            </inject-code>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, QLatin1String("1.0")));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QCOMPARE(classA->typeEntry()->codeSnips().count(), 1);
}

void TestCodeInjections::testInjectWithInvalidApiVersion()
{
    const char* cppCode ="struct A {};\n";
    const char* xmlCode  = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <inject-code class='target' since='1.0'>\n\
                test Inject code\n\
            </inject-code>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, QLatin1String("0.1")));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QCOMPARE(classA->typeEntry()->codeSnips().count(), 0);
}



QTEST_APPLESS_MAIN(TestCodeInjections)
