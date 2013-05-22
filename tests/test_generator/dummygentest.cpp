/*
 * This file is part of the PySide project.
 *
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

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

