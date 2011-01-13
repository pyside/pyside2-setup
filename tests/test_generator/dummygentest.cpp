/*
 * This file is part of the PySide project.
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#define HEADER_CONTENTS     "struct Dummy {};"
#define TYPESYSTEM_CONTENTS "<typesystem package='dummy'><value-type name='Dummy'/></typesystem>"
#define GENERATED_CONTENTS  "// Generated code for class: Dummy"
#define GENERATED_FILE      "dummy/dummy_generated.txt"

void DummyGenTest::testCallGenRunnerWithFullPathToDummyGenModule()
{
    QTemporaryFile headerFile;
    headerFile.open();
    QCOMPARE(headerFile.write(HEADER_CONTENTS), qint64(sizeof(HEADER_CONTENTS)-1));
    headerFile.close();

    QTemporaryFile typesystemFile;
    typesystemFile.open();
    QCOMPARE(typesystemFile.write(TYPESYSTEM_CONTENTS), qint64(sizeof(TYPESYSTEM_CONTENTS)-1));
    typesystemFile.close();

    QString generatedFileName = QString("%1/" GENERATED_FILE).arg(QDir::tempPath());
    QFile::remove(generatedFileName);

    QStringList args;
    args.append("--generator-set=" DUMMYGENERATOR_BINARY_DIR "/dummy_generator" MODULE_EXTENSION);
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFile.fileName());
    args.append(typesystemFile.fileName());
    int result = QProcess::execute("generatorrunner", args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFileName);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

void DummyGenTest::testCallGenRunnerWithNameOfDummyGenModule()
{
    QTemporaryFile headerFile;
    headerFile.open();
    QCOMPARE(headerFile.write(HEADER_CONTENTS), qint64(sizeof(HEADER_CONTENTS)-1));
    headerFile.close();

    QTemporaryFile typesystemFile;
    typesystemFile.open();
    QCOMPARE(typesystemFile.write(TYPESYSTEM_CONTENTS), qint64(sizeof(TYPESYSTEM_CONTENTS)-1));
    typesystemFile.close();

    QString generatedFileName = QString("%1/" GENERATED_FILE).arg(QDir::tempPath());
    QFile::remove(generatedFileName);

    QStringList args;
    args.append("--generator-set=dummy");
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFile.fileName());
    args.append(typesystemFile.fileName());
    int result = QProcess::execute("generatorrunner", args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFileName);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

void DummyGenTest::testCallDummyGeneratorExecutable()
{
    QTemporaryFile headerFile;
    headerFile.open();
    QCOMPARE(headerFile.write(HEADER_CONTENTS), qint64(sizeof(HEADER_CONTENTS)-1));
    headerFile.close();

    QTemporaryFile typesystemFile;
    typesystemFile.open();
    QCOMPARE(typesystemFile.write(TYPESYSTEM_CONTENTS), qint64(sizeof(TYPESYSTEM_CONTENTS)-1));
    typesystemFile.close();

    QString generatedFileName = QString("%1/" GENERATED_FILE).arg(QDir::tempPath());
    QFile::remove(generatedFileName);

    QStringList args;
    args.append(QString("--output-directory=%1").arg(QDir::tempPath()));
    args.append(headerFile.fileName());
    args.append(typesystemFile.fileName());
    int result = QProcess::execute(DUMMYGENERATOR_BINARY, args);
    QCOMPARE(result, 0);

    QFile generatedFile(generatedFileName);
    generatedFile.open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile.readAll().trimmed(), QByteArray(GENERATED_CONTENTS).trimmed());
    generatedFile.close();

    QVERIFY(generatedFile.remove());
}

QTEST_APPLESS_MAIN(DummyGenTest)

#include "dummygentest.moc"

