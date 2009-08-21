/*
 * This file is part of the Boost Python Generator project.
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

#include <QCoreApplication>
#include <QLinkedList>
#include <QLibrary>
#include <iostream>
#include <apiextractor/apiextractor.h>
#include "boostpythongeneratorversion.h"
#include "generator.h"

#if defined(Q_OS_WIN32)
    #define PATH_SPLITTER ";"
#else
    #define PATH_SPLITTER ":"
#endif


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

typedef QLinkedList<Generator*> (*getGeneratorsFunc)();
typedef QLinkedList<Generator*> GeneratorList;

QMap<QString, QString> getCommandLineArgs(int argc, char** argv)
{
    QMap<QString, QString> args;
    int argNum = 0;
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);
        arg = arg.trimmed();
        if (arg.startsWith("--")) {
            int split = arg.indexOf("=");
            if (split > 0)
                args[arg.mid(2).left(split-2)] = arg.mid(split + 1).trimmed();
            else
                args[arg.mid(2)] = QString();
        } else if (arg.startsWith("-")) {
            args[arg.mid(1)] = QString();
        } else {
            argNum++;
            args[QString("arg-%1").arg(argNum)] = arg;
        }
    }
    return args;
}

void printUsage(const GeneratorList& generators)
{
    #if defined(Q_OS_WIN32)
    #define PATHSPLITTER ";"
    #else
    #define PATHSPLITTER ":"
    #endif
    QTextStream s(stdout);
    s << "Usage:\n  "
    << "generator [options] header-file typesystem-file\n\n"
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
    generalOptions.insert("generatorSet", "generatorSet to be used. e.g. boostpython");
    printOptions(s, generalOptions);

    foreach (Generator* generator, generators) {
        QMap<QString, QString> options = generator->options();
        if (!options.isEmpty()) {
            s << endl << generator->name() << " options:\n";
            printOptions(s, generator->options());
        }
    }
}

int main(int argc, char *argv[])
{
    // needed by qxmlpatterns
    QCoreApplication app(argc, argv);

    // Store command arguments in a map
    QMap<QString, QString> args = getCommandLineArgs(argc, argv);
    GeneratorList generators;

    if (args.contains("version")) {
        std::cout << "generator v" BOOSTPYTHONGENERATOR_VERSION << std::endl;
        std::cout << "Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)" << std::endl;
        return EXIT_SUCCESS;
    }

    // Try to load a generator
    QString generatorSet = args.value("generatorSet");
    if (!generatorSet.isEmpty()) {
        QLibrary plugin(generatorSet);
        getGeneratorsFunc getGenerators = reinterpret_cast<getGeneratorsFunc>(plugin.resolve("getGenerators"));
        if (getGenerators)
            generators = getGenerators();
        else {
            std::cerr << argv[0] << ": Error loading generatorset plugin: " << qPrintable(plugin.errorString()) << std::endl;
            return EXIT_FAILURE;
        }
    } else if (!args.contains("help")) {
        std::cerr << argv[0] << ": You need to specify a generator with --generatorSet=GENERATOR_NAME" << std::endl;
        return EXIT_FAILURE;
    }

    if (args.contains("help")) {
        printUsage(generators);
        return EXIT_SUCCESS;
    }


    QString licenseComment;
    if (args.contains("license-file") && !args.value("license-file").isEmpty()) {
        QString licenseFileName = args.value("license-file");
        if (QFile::exists(licenseFileName)) {
            QFile licenseFile(licenseFileName);
            if (licenseFile.open(QIODevice::ReadOnly))
                licenseComment = licenseFile.readAll();
        } else {
            std::cerr << "Couldn't find the file containing the license heading: ";
            std::cerr << qPrintable(licenseFileName) << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Create and set-up API Extractor
    ApiExtractor extractor;

    if (args.contains("silent")) {
        extractor.setSilent(true);
    } else if (args.contains("debug-level")) {
        QString level = args.value("debug-level");
        if (level == "sparse")
            extractor.setDebugLevel(ReportHandler::SparseDebug);
        else if (level == "medium")
            extractor.setDebugLevel(ReportHandler::MediumDebug);
        else if (level == "full")
            extractor.setDebugLevel(ReportHandler::FullDebug);
    }
    if (args.contains("no-suppress-warnings"))
        extractor.setSuppressWarnings(false);

    if (args.contains("typesystem-paths"))
        extractor.addTypesystemSearchPath(args.value("typesystem-paths").split(PATH_SPLITTER));
    if (!args.value("include-paths").isEmpty())
        extractor.addIncludePath(args.value("include-paths").split(PATH_SPLITTER));


    QString cppFileName = args.value("arg-1");
    QString typeSystemFileName = args.value("arg-2");
    if (args.contains("arg-3")) {
        std::cerr << "Too many arguments!" << std::endl;
        return EXIT_FAILURE;
    }
    extractor.setCppFileName(cppFileName);
    extractor.setTypeSystem(typeSystemFileName);
    extractor.run();

    if (!extractor.classCount()) {
        std::cerr << "No C++ classes found!" << std::endl;
        return EXIT_FAILURE;
    }


    QString outputDirectory = args.contains("output-directory") ? args["output-directory"] : "out";
    foreach (Generator* g, generators) {
        g->setOutputDirectory(outputDirectory);
        g->setLicenseComment(licenseComment);
        if (g->setup(extractor, args))
            g->generate();
    }

    std::cout << "Done, " << ReportHandler::warningCount();
    std::cout << " warnings (" << ReportHandler::suppressedCount() << " known issues)";
    std::cout << std::endl;
}
