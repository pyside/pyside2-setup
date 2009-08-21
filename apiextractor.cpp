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
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <iostream>

#include "reporthandler.h"
#include "typesystem.h"
#include "fileout.h"
#include "parser/rpp/pp.h"
#include "abstractmetabuilder.h"
#include "generator.h"
#include "apiextractorversion.h"

static bool preprocess(const QString& sourceFile,
                       const QString& targetFile,
                       const QString& commandLineIncludes);

ApiExtractor::ApiExtractor(int argc, char** argv) : m_versionHandler(0)
{
    m_programName = argv[0];
    // store args in m_args map
    int argNum = 0;
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);
        arg = arg.trimmed();
        if (arg.startsWith("--")) {
            int split = arg.indexOf("=");
            if (split > 0)
                m_args[arg.mid(2).left(split-2)] = arg.mid(split + 1).trimmed();
            else
                m_args[arg.mid(2)] = QString();
        } else if (arg.startsWith("-")) {
            m_args[arg.mid(1)] = QString();
        } else {
            argNum++;
            m_args[QString("arg-%1").arg(argNum)] = arg;
        }
    }

    // Environment TYPESYSTEMPATH
    QString envTypesystemPaths = getenv("TYPESYSTEMPATH");
    TypeDatabase::instance()->addTypesystemPath(envTypesystemPaths);
    ReportHandler::setContext("ApiExtractor");
}

ApiExtractor::~ApiExtractor()
{
    qDeleteAll(m_generators);
}

void ApiExtractor::addGenerator(Generator* generator)
{
    m_generators << generator;
}

bool ApiExtractor::parseGeneralArgs()
{
    // set debug level
    if (m_args.contains("silent")) {
        ReportHandler::setSilent(true);
    } else if (m_args.contains("debug-level")) {
        QString level = m_args.value("debug-level");
        if (level == "sparse")
            ReportHandler::setDebugLevel(ReportHandler::SparseDebug);
        else if (level == "medium")
            ReportHandler::setDebugLevel(ReportHandler::MediumDebug);
        else if (level == "full")
            ReportHandler::setDebugLevel(ReportHandler::FullDebug);
    }

    if (m_args.contains("no-suppress-warnings")) {
        TypeDatabase *db = TypeDatabase::instance();
        db->setSuppressWarnings(false);
    }

    if (m_args.contains("dummy"))
        FileOut::dummy = true;

    if (m_args.contains("diff"))
        FileOut::diff = true;

    if (m_args.count() == 1)
        return false;

    if (m_args.contains("typesystem-paths"))
        TypeDatabase::instance()->addTypesystemPath(m_args.value("typesystem-paths"));

    m_globalHeaderFileName = m_args.value("arg-1");
    m_typeSystemFileName = m_args.value("arg-2");
    if (m_args.contains("arg-3"))
        return false;

    if (m_globalHeaderFileName.isEmpty() || m_typeSystemFileName.isEmpty())
        return false;
    return true;
}

int ApiExtractor::exec()
{
    if (m_args.contains("version")) {
        if (m_versionHandler)
            m_versionHandler("ApiExtractor v" APIEXTRACTOR_VERSION);
        else
            std::cout << m_programName << " using ApiExtractor v" APIEXTRACTOR_VERSION << std::endl;
        return 0;
    } else if (!parseGeneralArgs()) {
        printUsage();
        return 1;
    }

    QLatin1String ppFileName(".preprocessed.tmp");

    if (!TypeDatabase::instance()->parseFile(m_typeSystemFileName)) {
        std::cerr << "Cannot parse file: " << qPrintable(m_typeSystemFileName);
        return 1;
    }

    if (!preprocess(m_globalHeaderFileName, ppFileName, m_args.value("include-paths"))) {
        std::cerr << "Preprocessor failed on file: " << qPrintable(m_globalHeaderFileName);
        return 1;
    }

    QString licenseComment;
    if (m_args.contains("license-file") && !m_args.value("license-file").isEmpty()) {
        QString license_filename = m_args.value("license-file");
        if (QFile::exists(license_filename)) {
            QFile license_file(license_filename);
            if (license_file.open(QIODevice::ReadOnly))
                licenseComment = license_file.readAll();
        } else {
            std::cerr << "Couldn't find the file containing the license heading: ";
            std::cerr << qPrintable(license_filename);
            return 1;
        }
    }

    AbstractMetaBuilder builder;
    QFile ppFile(ppFileName);
    builder.build(&ppFile);

    QString outputDirectory = m_args.contains("output-directory") ? m_args["output-directory"] : "out";
    bool docOnly = m_args.contains("documentation-only");
    foreach (Generator* g, m_generators) {
        bool docGen = g->type() == Generator::DocumentationType;
        bool missingDocInfo = m_args["library-source-dir"].isEmpty()
                              || m_args["documentation-data-dir"].isEmpty();
        if ((!docGen && docOnly) || (docGen && missingDocInfo)) {
            std::cout << "Skipping " << g->name() << std::endl;
            continue;
        }
        g->setOutputDirectory(outputDirectory);
        g->setLicenseComment(licenseComment);
        g->setBuilder(&builder);
        std::cout << "Running " << g->name() << std::endl;
        if (g->prepareGeneration(m_args))
            g->generate();
    }

    std::cout << "Done, " << ReportHandler::warningCount();
    std::cout << " warnings (" << ReportHandler::suppressedCount() << " known issues)";
    std::cout << std::endl;
    return 0;
}

static void printOptions(QTextStream& s, const QMap<QString, QString>& options) {
    QMap<QString, QString>::const_iterator it = options.constBegin();
    s.setFieldAlignment(QTextStream::AlignLeft);
    for (; it != options.constEnd(); ++it) {
        s << "  --";
        s.setFieldWidth(38);
        s << it.key() << it.value();
        s.setFieldWidth(0);
        s << endl;
    }
}

void ApiExtractor::printUsage()
{
    #if defined(Q_OS_WIN32)
    #define PATHSPLITTER ";"
    #else
    #define PATHSPLITTER ":"
    #endif
    QTextStream s(stdout);
    s << "Usage:\n  "
      << m_programName << " [options] header-file typesystem-file\n\n"
      "General options:\n";
    QMap<QString, QString> generalOptions;
    generalOptions.insert("debug-level=[sparse|medium|full]", "Set the debug level");
    generalOptions.insert("silent", "Avoid printing any message");
    generalOptions.insert("help", "Display this help and exit");
    generalOptions.insert("no-suppress-warnings", "Show all warnings");
    generalOptions.insert("output-directory=[dir]", "The directory where the generated files will be written");
    generalOptions.insert("include-paths=<path>[" PATHSPLITTER "<path>" PATHSPLITTER "...]", "Include paths used by the C++ parser");
    generalOptions.insert("typesystem-paths=<path>[" PATHSPLITTER "<path>" PATHSPLITTER "...]", "Paths used when searching for typesystems");
    generalOptions.insert("documentation-only", "Do not generates any code, just the documentation");
    generalOptions.insert("license-file=[licensefile]", "File used for copyright headers of generated files");
    generalOptions.insert("version", "Output version information and exit");
    printOptions(s, generalOptions);

    foreach (Generator* generator, m_generators) {
        QMap<QString, QString> options = generator->options();
        if (!options.isEmpty()) {
            s << endl << generator->name() << " options:\n";
            printOptions(s, generator->options());
        }
    }
}


static bool preprocess(const QString& sourceFile,
                       const QString& targetFile,
                       const QString& commandLineIncludes)
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

    QStringList includes;

#if defined(Q_OS_WIN32)
    char *pathSplitter = const_cast<char *>(";");
#else
    char *pathSplitter = const_cast<char *>(":");
#endif

    // Environment INCLUDE
    QString includePath = getenv("INCLUDE");
    if (!includePath.isEmpty())
        includes += includePath.split(pathSplitter);

    // Includes from the command line
    if (!commandLineIncludes.isEmpty())
        includes += commandLineIncludes.split(pathSplitter);

    includes << QLatin1String(".");
    includes << QLatin1String("/usr/include");

    foreach (QString include, includes)
        preprocess.push_include_path(QDir::convertSeparators(include).toStdString());

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

    QFile f(targetFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to write preprocessed file: " << qPrintable(targetFile) << std::endl;
        return false;
    }

    f.write(result.c_str(), result.length());

    return true;
}

