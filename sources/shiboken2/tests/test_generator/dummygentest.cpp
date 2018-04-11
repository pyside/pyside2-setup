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

#include "dummygentest.h"
#include "dummygenerator.h"
#include "dummygentestconfig.h"
#include <QTemporaryFile>
#include <QtTest/QTest>
#include <QProcess>

#define GENERATED_CONTENTS  "// Generated code for class: Dummy"

void DummyGenTest::initTestCase()
{
    int argc = 0;
    char* argv[] = {NULL};
    QCoreApplication app(argc, argv);
    workDir = QDir::currentPath();

    headerFilePath = workDir + "/test_global.h";
    typesystemFilePath = workDir + "/test_typesystem.xml";
    projectFilePath = workDir + "/dummygentest-project.txt";
    generatedFilePath = QString("%1/dummy/dummy_generated.txt").arg(QDir::tempPath());
}

void DummyGenTest::testCallGenRunnerWithFullPathToDummyGenModule()
{
    QStringList args;
    args.append("--generator-set=" DUMMYGENERATOR_BINARY_DIR "/dummy_generator" MODULE_EXTENSION);
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFilePath);
    args.append(typesystemFilePath);
    int result = QProcess::execute("generatorrunner", args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFilePath);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

void DummyGenTest::testCallGenRunnerWithNameOfDummyGenModule()
{
    QStringList args;
    args.append("--generator-set=dummy");
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFilePath);
    args.append(typesystemFilePath);
    int result = QProcess::execute("generatorrunner", args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFilePath);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

void DummyGenTest::testCallDummyGeneratorExecutable()
{
    QStringList args;
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFilePath);
    args.append(typesystemFilePath);
    int result = QProcess::execute(DUMMYGENERATOR_BINARY, args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFilePath);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

void DummyGenTest::testProjectFileArgumentsReading()
{
    QStringList args(QString("--project-file=%1/dummygentest-project.txt").arg(workDir));
    int result = QProcess::execute("generatorrunner", args);
    QCOMPARE(result, 0);

    QFile logFile(workDir + "/dummygen-args.log");
    logFile.open(QIODevice::ReadOnly);
    QStringList logContents;
    while (!logFile.atEnd())
        logContents << logFile.readLine().trimmed();
    logContents.sort();
    QCOMPARE(logContents[0], QString("api-version = 1.2.3"));
    QCOMPARE(logContents[1], QString("debug = sparse"));
    QVERIFY(logContents[2].startsWith("dump-arguments = "));
    QVERIFY(logContents[2].endsWith("dummygen-args.log"));
    QCOMPARE(logContents[3], QString("generator-set = dummy"));
    QVERIFY(logContents[4].startsWith("header-file = "));
    QVERIFY(logContents[4].endsWith("test_global.h"));
    QCOMPARE(logContents[5],
             QDir::toNativeSeparators(QString("include-paths = /include/path/location1%1/include/path/location2").arg(PATH_SPLITTER)));
    QCOMPARE(logContents[6], QString("no-suppress-warnings"));
    QCOMPARE(logContents[7], QString("output-directory = /tmp/output"));
    QVERIFY(logContents[8].startsWith("project-file = "));
    QVERIFY(logContents[8].endsWith("dummygentest-project.txt"));
    QVERIFY(logContents[9].startsWith("typesystem-file = "));
    QVERIFY(logContents[9].endsWith("test_typesystem.xml"));
    QCOMPARE(logContents[10],
             QDir::toNativeSeparators(QString("typesystem-paths = /typesystem/path/location1%1/typesystem/path/location2").arg(PATH_SPLITTER)));
}

QTEST_APPLESS_MAIN(DummyGenTest)

#include "dummygentest.moc"

