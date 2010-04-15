/*
 * This file is part of the API Extractor project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "apiextractor.h"
#include <QDir>
#include <QDebug>
#include <QTemporaryFile>
#include <iostream>

#include "reporthandler.h"
#include "typesystem.h"
#include "fileout.h"
#include "parser/rpp/pp.h"
#include "abstractmetabuilder.h"
#include "apiextractorversion.h"
#include "typedatabase.h"

static bool preprocess(const QString& sourceFile,
                       QFile& targetFile,
                       const QStringList& includes);

ApiExtractor::ApiExtractor() : m_builder(0)
{
    // Environment TYPESYSTEMPATH
    QString envTypesystemPaths = getenv("TYPESYSTEMPATH");
    if (!envTypesystemPaths.isEmpty())
        TypeDatabase::instance()->addTypesystemPath(envTypesystemPaths);
    ReportHandler::setContext("ApiExtractor");
}

ApiExtractor::~ApiExtractor()
{
    delete m_builder;
}

void ApiExtractor::addTypesystemSearchPath (const QString& path)
{
    TypeDatabase::instance()->addTypesystemPath(path);
}

void ApiExtractor::addTypesystemSearchPath(const QStringList& paths)
{
    foreach (QString path, paths)
        addTypesystemSearchPath(path);
}

void ApiExtractor::addIncludePath(const QString& path)
{
    m_includePaths << path;
}

void ApiExtractor::addIncludePath(const QStringList& paths)
{
    m_includePaths << paths;
}

void ApiExtractor::setLogDirectory(const QString& logDir)
{
    m_logDirectory = logDir;
}

void ApiExtractor::setCppFileName(const QString& cppFileName)
{
    m_cppFileName = cppFileName;
}

void ApiExtractor::setTypeSystem(const QString& typeSystemFileName)
{
    m_typeSystemFileName = typeSystemFileName;
}

void ApiExtractor::setDebugLevel(ReportHandler::DebugLevel debugLevel)
{
    ReportHandler::setDebugLevel(debugLevel);
}

void ApiExtractor::setSuppressWarnings ( bool value )
{
    TypeDatabase::instance()->setSuppressWarnings(value);
}

void ApiExtractor::setSilent ( bool value )
{
    ReportHandler::setSilent(value);
}

AbstractMetaEnumList ApiExtractor::globalEnums() const
{
    Q_ASSERT(m_builder);
    return m_builder->globalEnums();
}

AbstractMetaFunctionList ApiExtractor::globalFunctions() const
{
    Q_ASSERT(m_builder);
    return m_builder->globalFunctions();
}

AbstractMetaClassList ApiExtractor::classes() const
{
    Q_ASSERT(m_builder);
    return m_builder->classes();
}

PrimitiveTypeEntryList ApiExtractor::primitiveTypes() const
{
    return TypeDatabase::instance()->primitiveTypes();
}

ContainerTypeEntryList ApiExtractor::containerTypes() const
{
    return TypeDatabase::instance()->containerTypes();
}

QSet<QString> ApiExtractor::qtMetaTypeDeclaredTypeNames() const
{
    Q_ASSERT(m_builder);
    return m_builder->qtMetaTypeDeclaredTypeNames();
}

int ApiExtractor::classCount() const
{
    Q_ASSERT(m_builder);
    return m_builder->classes().count();
}

bool ApiExtractor::run()
{
    if (m_builder)
        return false;

    if (m_typeSystemFileName.isEmpty()) {
        std::cerr << "You must specify a Type System file." << std::endl;
        return false;
    } else if (!TypeDatabase::instance()->parseFile(m_typeSystemFileName)) {
        std::cerr << "Cannot parse file: " << qPrintable(m_typeSystemFileName);
        return false;
    }

    QTemporaryFile ppFile;
    // run rpp pre-processor
    if (!preprocess(m_cppFileName, ppFile, m_includePaths)) {
        std::cerr << "Preprocessor failed on file: " << qPrintable(m_cppFileName);
        return false;
    }
    ppFile.seek(0);
    m_builder = new AbstractMetaBuilder;
    m_builder->setLogDirectory(m_logDirectory);
    m_builder->build(&ppFile);

    return true;
}

static bool preprocess(const QString& sourceFile,
                       QFile& targetFile,
                       const QStringList& includes)
{
    rpp::pp_environment env;
    rpp::pp preprocess(env);

    rpp::pp_null_output_iterator null_out;

    const char *ppconfig = ":/trolltech/generator/pp-qt-configuration";

    QFile file(ppconfig);
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Preprocessor configuration file not found " << ppconfig << std::endl;
        return false;
    }

    QByteArray ba = file.readAll();
    file.close();
    preprocess.operator()(ba.constData(), ba.constData() + ba.size(), null_out);

    preprocess.push_include_path(".");
    foreach (QString include, includes)
        preprocess.push_include_path(QDir::convertSeparators(include).toStdString());
    preprocess.push_include_path("/usr/include");

    QString currentDir = QDir::current().absolutePath();
    QFileInfo sourceInfo(sourceFile);
    if (!sourceInfo.exists()) {
        std::cerr << "File not found " << qPrintable(sourceFile) << std::endl;
        return false;
    }
    QDir::setCurrent(sourceInfo.absolutePath());

    std::string result;
    result.reserve(20 * 1024);  // 20K

    result += "# 1 \"builtins\"\n";
    result += "# 1 \"";
    result += sourceFile.toStdString();
    result += "\"\n";

    preprocess.file(sourceInfo.fileName().toStdString(),
                    rpp::pp_output_iterator<std::string> (result));

    QDir::setCurrent(currentDir);

    if (!targetFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        std::cerr << "Failed to write preprocessed file: " << qPrintable(targetFile.fileName()) << std::endl;
        return false;
    }

    targetFile.write(result.c_str(), result.length());
    return true;
}

