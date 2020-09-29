/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
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

#include "apiextractor.h"
#include "abstractmetalang.h"

#include <QDir>
#include <QDebug>
#include <QTemporaryFile>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "reporthandler.h"
#include "typesystem.h"
#include "fileout.h"
#include "abstractmetabuilder.h"
#include "typedatabase.h"
#include "typesystem.h"

ApiExtractor::ApiExtractor()
{
    // Environment TYPESYSTEMPATH
    QString envTypesystemPaths = QFile::decodeName(qgetenv("TYPESYSTEMPATH"));
    if (!envTypesystemPaths.isEmpty())
        TypeDatabase::instance()->addTypesystemPath(envTypesystemPaths);
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
    for (const QString &path : paths)
        addTypesystemSearchPath(path);
}

void ApiExtractor::addIncludePath(const HeaderPath& path)
{
    m_includePaths << path;
}

void ApiExtractor::addIncludePath(const HeaderPaths& paths)
{
    m_includePaths << paths;
}

void ApiExtractor::setLogDirectory(const QString& logDir)
{
    m_logDirectory = logDir;
}

void ApiExtractor::setCppFileNames(const QFileInfoList &cppFileName)
{
    m_cppFileNames = cppFileName;
}

void ApiExtractor::setTypeSystem(const QString& typeSystemFileName)
{
    m_typeSystemFileName = typeSystemFileName;
}

void ApiExtractor::setSkipDeprecated(bool value)
{
    m_skipDeprecated = value;
    if (m_builder)
        m_builder->setSkipDeprecated(m_skipDeprecated);
}

void ApiExtractor::setSuppressWarnings ( bool value )
{
    TypeDatabase::instance()->setSuppressWarnings(value);
}

void ApiExtractor::setSilent ( bool value )
{
    ReportHandler::setSilent(value);
}

bool ApiExtractor::setApiVersion(const QString& package, const QString &version)
{
    return TypeDatabase::setApiVersion(package, version);
}

void ApiExtractor::setDropTypeEntries(QString dropEntries)
{
    dropEntries.remove(QLatin1Char(' '));
    QStringList entries = dropEntries.split(QLatin1Char(';'));
    TypeDatabase::instance()->setDropTypeEntries(entries);
}

const AbstractMetaEnumList &ApiExtractor::globalEnums() const
{
    Q_ASSERT(m_builder);
    return m_builder->globalEnums();
}

const AbstractMetaFunctionList &ApiExtractor::globalFunctions() const
{
    Q_ASSERT(m_builder);
    return m_builder->globalFunctions();
}

const AbstractMetaClassList &ApiExtractor::classes() const
{
    Q_ASSERT(m_builder);
    return m_builder->classes();
}

const AbstractMetaClassList &ApiExtractor::smartPointers() const
{
    Q_ASSERT(m_builder);
    return m_builder->smartPointers();
}

AbstractMetaClassList ApiExtractor::classesTopologicalSorted(const Dependencies &additionalDependencies) const
{
    Q_ASSERT(m_builder);
    return m_builder->classesTopologicalSorted(m_builder->classes(), additionalDependencies);
}

PrimitiveTypeEntryList ApiExtractor::primitiveTypes() const
{
    return TypeDatabase::instance()->primitiveTypes();
}

ContainerTypeEntryList ApiExtractor::containerTypes() const
{
    return TypeDatabase::instance()->containerTypes();
}

const AbstractMetaEnum* ApiExtractor::findAbstractMetaEnum(const TypeEntry* typeEntry) const
{
    return m_builder->findEnum(typeEntry);
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

    if (!TypeDatabase::instance()->parseFile(m_typeSystemFileName)) {
        std::cerr << "Cannot parse file: " << qPrintable(m_typeSystemFileName);
        return false;
    }

    const QString pattern = QDir::tempPath() + QLatin1Char('/')
        + m_cppFileNames.constFirst().baseName()
        + QStringLiteral("_XXXXXX.hpp");
    QTemporaryFile ppFile(pattern);
    bool autoRemove = !qEnvironmentVariableIsSet("KEEP_TEMP_FILES");
    // make sure that a tempfile can be written
    if (!ppFile.open()) {
        std::cerr << "could not create tempfile " << qPrintable(pattern)
            << ": " << qPrintable(ppFile.errorString()) << '\n';
        return false;
    }
    for (const auto &cppFileName : qAsConst(m_cppFileNames)) {
        ppFile.write("#include \"");
        ppFile.write(cppFileName.absoluteFilePath().toLocal8Bit());
        ppFile.write("\"\n");
    }
    const QString preprocessedCppFileName = ppFile.fileName();
    ppFile.close();
    m_builder = new AbstractMetaBuilder;
    m_builder->setLogDirectory(m_logDirectory);
    m_builder->setGlobalHeaders(m_cppFileNames);
    m_builder->setSkipDeprecated(m_skipDeprecated);
    m_builder->setHeaderPaths(m_includePaths);
    QByteArrayList arguments;
    arguments.reserve(m_includePaths.size() + 1);
    for (const HeaderPath &headerPath : qAsConst(m_includePaths))
        arguments.append(HeaderPath::includeOption(headerPath));
    arguments.append(QFile::encodeName(preprocessedCppFileName));
    if (ReportHandler::isDebug(ReportHandler::SparseDebug)) {
        qCInfo(lcShiboken).noquote().nospace()
            << "clang language level: " << int(m_languageLevel)
            << "\nclang arguments: " << arguments;
    }
    const bool result = m_builder->build(arguments, m_languageLevel);
    if (!result)
        autoRemove = false;
    if (!autoRemove) {
        ppFile.setAutoRemove(false);
        std::cerr << "Keeping temporary file: " << qPrintable(QDir::toNativeSeparators(preprocessedCppFileName)) << '\n';
    }
    return result;
}

LanguageLevel ApiExtractor::languageLevel() const
{
    return m_languageLevel;
}

void ApiExtractor::setLanguageLevel(LanguageLevel languageLevel)
{
    m_languageLevel = languageLevel;
}

#ifndef QT_NO_DEBUG_STREAM
template <class Container>
static void debugFormatSequence(QDebug &d, const char *key, const Container& c)
{
    if (c.isEmpty())
        return;
    const auto begin = c.begin();
    d << "\n  " << key << '[' << c.size() << "]=(";
    for (auto it = begin, end = c.end(); it != end; ++it) {
        if (it != begin)
            d << ", ";
        d << *it;
    }
    d << ')';
}

QDebug operator<<(QDebug d, const ApiExtractor &ae)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    if (ReportHandler::debugLevel() >= ReportHandler::FullDebug)
        d.setVerbosity(3); // Trigger verbose output of AbstractMetaClass
    d << "ApiExtractor(typeSystem=\"" << ae.typeSystem() << "\", cppFileNames=\""
      << ae.cppFileNames() << ", ";
    ae.m_builder->formatDebug(d);
    d << ')';
    return d;
}
#endif // QT_NO_DEBUG_STREAM
