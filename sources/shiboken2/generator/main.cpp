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

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLinkedList>
#include <QLibrary>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <iostream>
#include <apiextractor.h>
#include <fileout.h>
#include <typedatabase.h>
#include "generator.h"
#include "shibokenconfig.h"
#include "cppgenerator.h"
#include "headergenerator.h"
#include "qtdocgenerator.h"

#ifdef _WINDOWS
    #define PATH_SPLITTER ";"
#else
    #define PATH_SPLITTER ":"
#endif

static inline QString languageLevelOption() { return QStringLiteral("language-level"); }
static inline QString includePathOption() { return QStringLiteral("include-paths"); }
static inline QString frameworkIncludePathOption() { return QStringLiteral("framework-include-paths"); }
static inline QString systemIncludePathOption() { return QStringLiteral("system-include-paths"); }
static inline QString typesystemPathOption() { return QStringLiteral("typesystem-paths"); }
static inline QString helpOption() { return QStringLiteral("help"); }
static inline QString diffOption() { return QStringLiteral("diff"); }
static inline QString dryrunOption() { return QStringLiteral("dry-run"); }

static const char helpHint[] = "Note: use --help or -h for more information.\n";

typedef QMap<QString, QString> CommandArgumentMap;

static QString msgLeftOverArguments(const CommandArgumentMap &remainingArgs)
{
    QString message;
    QTextStream str(&message);
    str << "shiboken: Called with wrong arguments:";
    for (auto it = remainingArgs.cbegin(), end = remainingArgs.cend(); it != end; ++it) {
        str << ' ' << it.key();
        if (!it.value().isEmpty())
            str << ' ' << it.value();
    }
    str << "\nCommand line: " << QCoreApplication::arguments().join(QLatin1Char(' '));
    return message;
}

typedef Generator::OptionDescriptions OptionDescriptions;

static void printOptions(QTextStream& s, const OptionDescriptions& options)
{
    s.setFieldAlignment(QTextStream::AlignLeft);
    for (const auto &od : options) {
        if (!od.first.startsWith(QLatin1Char('-')))
            s << "--";
        s << od.first;
        if (od.second.isEmpty()) {
            s << ", ";
        } else {
            s << endl;
            const auto lines = od.second.splitRef(QLatin1Char('\n'));
            for (const auto &line : lines)
                s << "        " << line << endl;
            s << endl;
        }
    }
}

typedef void (*getGeneratorsFunc)(QLinkedList<Generator*>*);

static bool processProjectFile(QFile& projectFile, QMap<QString, QString>& args)
{
    QByteArray line = projectFile.readLine().trimmed();
    if (line.isEmpty() || line != "[generator-project]")
        return false;

    QStringList includePaths;
    QStringList frameworkIncludePaths;
    QStringList systemIncludePaths;
    QStringList typesystemPaths;
    QStringList apiVersions;
    QString languageLevel;

    while (!projectFile.atEnd()) {
        line = projectFile.readLine().trimmed();
        if (line.isEmpty())
            continue;

        int split = line.indexOf('=');
        QByteArray key;
        QString value;
        if (split > 0) {
            key = line.left(split - 1).trimmed();
            value = QString::fromUtf8(line.mid(split + 1).trimmed());
        } else {
            key = line;
        }

        if (key == "include-path")
            includePaths << QDir::toNativeSeparators(value);
        else if (key == "framework-include-path")
            frameworkIncludePaths << QDir::toNativeSeparators(value);
        else if (key == "system-include-paths")
            systemIncludePaths << QDir::toNativeSeparators(value);
        else if (key == "typesystem-path")
            typesystemPaths << QDir::toNativeSeparators(value);
        else if (key == "language-level")
            languageLevel = value;
        else if (key == "api-version")
            apiVersions << value;
        else if (key == "header-file")
            args.insert(QLatin1String("arg-1"), value);
        else if (key == "typesystem-file")
            args.insert(QLatin1String("arg-2"), value);
        else
            args.insert(QString::fromUtf8(key), value);
    }

    if (!includePaths.isEmpty())
        args.insert(includePathOption(), includePaths.join(QLatin1String(PATH_SPLITTER)));

    if (!frameworkIncludePaths.isEmpty())
        args.insert(frameworkIncludePathOption(),
                    frameworkIncludePaths.join(QLatin1String(PATH_SPLITTER)));
    if (!systemIncludePaths.isEmpty()) {
        args.insert(systemIncludePathOption(),
                    systemIncludePaths.join(QLatin1String(PATH_SPLITTER)));
    }

    if (!typesystemPaths.isEmpty())
        args.insert(typesystemPathOption(), typesystemPaths.join(QLatin1String(PATH_SPLITTER)));
    if (!apiVersions.isEmpty())
        args.insert(QLatin1String("api-version"), apiVersions.join(QLatin1Char('|')));
    if (!languageLevel.isEmpty())
        args.insert(languageLevelOption(), languageLevel);
    return true;
}

static CommandArgumentMap getInitializedArguments()
{
    CommandArgumentMap args;
    QStringList arguments = QCoreApplication::arguments();
    QString appName = arguments.constFirst();
    arguments.removeFirst();

    QString projectFileName;
    for (const QString &arg : qAsConst(arguments)) {
        if (arg.startsWith(QLatin1String("--project-file"))) {
            int split = arg.indexOf(QLatin1Char('='));
            if (split > 0)
                projectFileName = arg.mid(split + 1).trimmed();
            break;
        }
    }

    if (projectFileName.isNull())
        return args;

    if (!QFile::exists(projectFileName)) {
        std::cerr << qPrintable(appName) << ": Project file \"";
        std::cerr << qPrintable(projectFileName) << "\" not found.";
        std::cerr << std::endl;
        return args;
    }

    QFile projectFile(projectFileName);
    if (!projectFile.open(QIODevice::ReadOnly))
        return args;

    if (!processProjectFile(projectFile, args)) {
        std::cerr << qPrintable(appName) << ": first line of project file \"";
        std::cerr << qPrintable(projectFileName) << "\" must be the string \"[generator-project]\"";
        std::cerr << std::endl;
        return args;
    }

    return args;
}

// Concatenate values of path arguments that can occur multiple times on the
// command line.
static void addPathOptionValue(const QString &option, const QString &value,
                               CommandArgumentMap &args)
{
    const CommandArgumentMap::iterator it = args.find(option);
    if (it != args.end())
        it.value().append(QLatin1String(PATH_SPLITTER) + value);
    else
        args.insert(option, value);
}

static void getCommandLineArg(QString arg, int &argNum, QMap<QString, QString> &args)
{
    if (arg.startsWith(QLatin1String("--"))) {
        arg.remove(0, 2);
        const int split = arg.indexOf(QLatin1Char('='));
        if (split < 0) {
            args.insert(arg, QString());
            return;
        }
        const QString option = arg.left(split);
        const QString value = arg.mid(split + 1).trimmed();
        if (option == includePathOption() || option == frameworkIncludePathOption()
            || option == systemIncludePathOption() || option == typesystemPathOption()) {
            addPathOptionValue(option, value, args);
        } else {
            args.insert(option, value);
        }
        return;
    }
    if (arg.startsWith(QLatin1Char('-'))) {
        arg.remove(0, 1);
        if (arg.startsWith(QLatin1Char('I'))) // Shorthand path arguments -I/usr/include...
            addPathOptionValue(includePathOption(), arg.mid(1), args);
        else if (arg.startsWith(QLatin1Char('F')))
            addPathOptionValue(frameworkIncludePathOption(), arg.mid(1), args);
        else if (arg.startsWith(QLatin1String("isystem")))
            addPathOptionValue(systemIncludePathOption(), arg.mid(7), args);
        else if (arg.startsWith(QLatin1Char('T')))
            addPathOptionValue(typesystemPathOption(), arg.mid(1), args);
        else if (arg == QLatin1String("h"))
            args.insert(helpOption(), QString());
        else if (arg.startsWith(QLatin1String("std=")))
            args.insert(languageLevelOption(), arg.mid(4));
        else
            args.insert(arg, QString());
        return;
    }
    argNum++;
    args.insert(QStringLiteral("arg-") + QString::number(argNum), arg);
}

static QMap<QString, QString> getCommandLineArgs()
{
    QMap<QString, QString> args = getInitializedArguments();
    QStringList arguments = QCoreApplication::arguments();
    arguments.removeFirst();

    int argNum = 0;
    for (const QString &carg : qAsConst(arguments))
        getCommandLineArg(carg.trimmed(), argNum, args);

    return args;
}

static inline Generators docGenerators()
{
    Generators result;
#ifdef DOCSTRINGS_ENABLED
    result.append(GeneratorPtr(new QtDocGenerator));
#endif
    return result;
}

static inline Generators shibokenGenerators()
{
    Generators result;
    result << GeneratorPtr(new CppGenerator) << GeneratorPtr(new HeaderGenerator);
    return result;
}

static inline QString languageLevelDescription()
{
    return QLatin1String("C++ Language level (c++11..c++17, default=")
        + QLatin1String(clang::languageLevelOption(clang::emulatedCompilerLanguageLevel()))
        + QLatin1Char(')');
}

void printUsage()
{
    QTextStream s(stdout);
    s << "Usage:\n  "
      << "shiboken [options] header-file typesystem-file\n\n"
      << "General options:\n";
    const QString pathSyntax = QLatin1String("<path>[" PATH_SPLITTER "<path>" PATH_SPLITTER "...]");
    OptionDescriptions generalOptions = OptionDescriptions()
        << qMakePair(QLatin1String("api-version=<\"package mask\">,<\"version\">"),
                     QLatin1String("Specify the supported api version used to generate the bindings"))
        << qMakePair(QLatin1String("debug-level=[sparse|medium|full]"),
                     QLatin1String("Set the debug level"))
        << qMakePair(QLatin1String("documentation-only"),
                     QLatin1String("Do not generates any code, just the documentation"))
        << qMakePair(QLatin1String("drop-type-entries=\"<TypeEntry0>[;TypeEntry1;...]\""),
                     QLatin1String("Semicolon separated list of type system entries (classes, namespaces,\n"
                                   "global functions and enums) to be dropped from generation."))
        << qMakePair(QLatin1String("-F<path>"), QString())
        << qMakePair(QLatin1String("framework-include-paths=") + pathSyntax,
                     QLatin1String("Framework include paths used by the C++ parser"))
        << qMakePair(QLatin1String("-isystem<path>"), QString())
        << qMakePair(QLatin1String("system-include-paths=") + pathSyntax,
                     QLatin1String("System include paths used by the C++ parser"))
        << qMakePair(QLatin1String("generator-set=<\"generator module\">"),
                     QLatin1String("generator-set to be used. e.g. qtdoc"))
        << qMakePair(diffOption(),
                     QLatin1String("Print a diff of wrapper files"))
        << qMakePair(dryrunOption(),
                     QLatin1String("Dry run, do not generate wrapper files"))
        << qMakePair(QLatin1String("-h"), QString())
        << qMakePair(helpOption(),
                     QLatin1String("Display this help and exit"))
        << qMakePair(QLatin1String("-I<path>"), QString())
        << qMakePair(QLatin1String("include-paths=") + pathSyntax,
                     QLatin1String("Include paths used by the C++ parser"))
        << qMakePair(languageLevelOption() + QLatin1String("=, -std=<level>"),
                     languageLevelDescription())
        << qMakePair(QLatin1String("license-file=<license-file>"),
                     QLatin1String("File used for copyright headers of generated files"))
        << qMakePair(QLatin1String("no-suppress-warnings"),
                     QLatin1String("Show all warnings"))
        << qMakePair(QLatin1String("output-directory=<path>"),
                     QLatin1String("The directory where the generated files will be written"))
        << qMakePair(QLatin1String("project-file=<file>"),
                     QLatin1String("text file containing a description of the binding project.\n"
                                   "Replaces and overrides command line arguments"))
        << qMakePair(QLatin1String("silent"),
                     QLatin1String("Avoid printing any message"))
        << qMakePair(QLatin1String("-T<path>"), QString())
        << qMakePair(QLatin1String("typesystem-paths=") + pathSyntax,
                     QLatin1String("Paths used when searching for typesystems"))
        << qMakePair(QLatin1String("version"),
                     QLatin1String("Output version information and exit"));
    printOptions(s, generalOptions);

    const Generators generators = shibokenGenerators() + docGenerators();
    for (const GeneratorPtr &generator : generators) {
        const OptionDescriptions options = generator->options();
        if (!options.isEmpty()) {
            s << endl << generator->name() << " options:\n\n";
            printOptions(s, generator->options());
        }
    }
}

static inline void printVerAndBanner()
{
    std::cout << "shiboken v" SHIBOKEN_VERSION << std::endl;
    std::cout << "Copyright (C) 2016 The Qt Company Ltd." << std::endl;
}

static inline void errorPrint(const QString& s)
{
    QStringList arguments = QCoreApplication::arguments();
    arguments.pop_front();
    std::cerr << "shiboken: " << qPrintable(s)
        << "\nCommand line: " << qPrintable(arguments.join(QLatin1Char(' '))) << '\n';
}

static QString msgInvalidVersion(const QString &package, const QString &version)
{
    return QLatin1String("Invalid version \"") + version
        + QLatin1String("\" specified for package ") + package + QLatin1Char('.');
}

static void parseIncludePathOption(const QString &option, HeaderType headerType,
                                   CommandArgumentMap &args,
                                   ApiExtractor &extractor)
{
    const CommandArgumentMap::iterator it = args.find(option);
    if (it != args.end()) {
        const QStringList includePathListList =
            it.value().split(QLatin1String(PATH_SPLITTER), QString::SkipEmptyParts);
        args.erase(it);
        for (const QString &s : includePathListList)
            extractor.addIncludePath(HeaderPath{QFile::encodeName(s), headerType});
    }
}

int main(int argc, char *argv[])
{
    // PYSIDE-757: Request a deterministic ordering of QHash in the code model
    // and type system.
    qSetGlobalQHashSeed(0);
    QElapsedTimer timer;
    timer.start();
    // needed by qxmlpatterns
    QCoreApplication app(argc, argv);
    ReportHandler::install();
    qCDebug(lcShiboken()).noquote().nospace() << QCoreApplication::arguments().join(QLatin1Char(' '));

    // Store command arguments in a map
    CommandArgumentMap args = getCommandLineArgs();
    Generators generators;

    CommandArgumentMap::iterator ait = args.find(QLatin1String("version"));
    if (ait != args.end()) {
        args.erase(ait);
        printVerAndBanner();
        return EXIT_SUCCESS;
    }

    QString generatorSet;
    ait = args.find(QLatin1String("generator-set"));
    if (ait == args.end()) // Also check QLatin1String("generatorSet") command line argument for backward compatibility.
        ait = args.find(QLatin1String("generatorSet"));
    if (ait != args.end()) {
        generatorSet = ait.value();
        args.erase(ait);
    }

    // Pre-defined generator sets.
    if (generatorSet == QLatin1String("qtdoc")) {
        generators = docGenerators();
        if (generators.isEmpty()) {
            errorPrint(QLatin1String("Doc strings extractions was not enabled in this shiboken build."));
            return EXIT_FAILURE;
        }
    } else if (generatorSet.isEmpty() || generatorSet == QLatin1String("shiboken")) {
        generators = shibokenGenerators();
    } else {
        errorPrint(QLatin1String("Unknown generator set, try \"shiboken\" or \"qtdoc\"."));
        return EXIT_FAILURE;
    }

    ait = args.find(QLatin1String("help"));
    if (ait != args.end()) {
        args.erase(ait);
        printUsage();
        return EXIT_SUCCESS;
    }

    ait = args.find(diffOption());
    if (ait != args.end()) {
        args.erase(ait);
        FileOut::diff = true;
    }

    ait = args.find(dryrunOption());
    if (ait != args.end()) {
        args.erase(ait);
        FileOut::dummy = true;
    }

    QString licenseComment;
    ait = args.find(QLatin1String("license-file"));
    if (ait != args.end()) {
        QFile licenseFile(ait.value());
        args.erase(ait);
        if (licenseFile.open(QIODevice::ReadOnly)) {
            licenseComment = QString::fromUtf8(licenseFile.readAll());
        } else {
            errorPrint(QStringLiteral("Could not open the file \"%1\" containing the license heading: %2").
                       arg(QDir::toNativeSeparators(licenseFile.fileName()), licenseFile.errorString()));
            return EXIT_FAILURE;
        }
    }

    QString outputDirectory = QLatin1String("out");
    ait = args.find(QLatin1String("output-directory"));
    if (ait != args.end()) {
        outputDirectory = ait.value();
        args.erase(ait);
    }

    if (!QDir(outputDirectory).exists()) {
        if (!QDir().mkpath(outputDirectory)) {
            qCWarning(lcShiboken).noquote().nospace()
                << "Can't create output directory: " << QDir::toNativeSeparators(outputDirectory);
            return EXIT_FAILURE;
        }
    }

    // Create and set-up API Extractor
    ApiExtractor extractor;
    extractor.setLogDirectory(outputDirectory);

    ait = args.find(QLatin1String("silent"));
    if (ait != args.end()) {
        extractor.setSilent(true);
        args.erase(ait);
    } else {
        ait = args.find(QLatin1String("debug-level"));
        if (ait != args.end()) {
            const QString level = ait.value();
            args.erase(ait);
            if (level == QLatin1String("sparse"))
                extractor.setDebugLevel(ReportHandler::SparseDebug);
            else if (level == QLatin1String("medium"))
                extractor.setDebugLevel(ReportHandler::MediumDebug);
            else if (level == QLatin1String("full"))
                extractor.setDebugLevel(ReportHandler::FullDebug);
        }
    }
    ait = args.find(QLatin1String("no-suppress-warnings"));
    if (ait != args.end()) {
        args.erase(ait);
        extractor.setSuppressWarnings(false);
    }
    ait = args.find(QLatin1String("api-version"));
    if (ait != args.end()) {
        const QStringList &versions = ait.value().split(QLatin1Char('|'));
        args.erase(ait);
        for (const QString &fullVersion : versions) {
            QStringList parts = fullVersion.split(QLatin1Char(','));
            QString package;
            QString version;
            package = parts.count() == 1 ? QLatin1String("*") : parts.constFirst();
            version = parts.constLast();
            if (!extractor.setApiVersion(package, version)) {
                errorPrint(msgInvalidVersion(package, version));
                return EXIT_FAILURE;
            }
        }
    }

    ait = args.find(QLatin1String("drop-type-entries"));
    if (ait != args.end()) {
        extractor.setDropTypeEntries(ait.value());
        args.erase(ait);
    }

    ait = args.find(QLatin1String("typesystem-paths"));
    if (ait != args.end()) {
        extractor.addTypesystemSearchPath(ait.value().split(QLatin1String(PATH_SPLITTER)));
        args.erase(ait);
    }

    parseIncludePathOption(includePathOption(), HeaderType::Standard,
                           args, extractor);
    parseIncludePathOption(frameworkIncludePathOption(), HeaderType::Framework,
                           args, extractor);
    parseIncludePathOption(systemIncludePathOption(), HeaderType::System,
                           args, extractor);

    ait = args.find(QLatin1String("arg-1"));
    if (ait == args.end()) {
        errorPrint(QLatin1String("Required argument header-file is missing."));
        return EXIT_FAILURE;
    }
    const QString cppFileName = ait.value();
    args.erase(ait);
    const QFileInfo cppFileNameFi(cppFileName);
    if (!cppFileNameFi.isFile() && !cppFileNameFi.isSymLink()) {
        errorPrint(QLatin1Char('"') + cppFileName + QLatin1String("\" does not exist."));
        return EXIT_FAILURE;
    }

    ait = args.find(QLatin1String("arg-2"));
    if (ait == args.end()) {
        errorPrint(QLatin1String("Required argument typesystem-file is missing."));
        return EXIT_FAILURE;
    }
    const QString typeSystemFileName = ait.value();
    args.erase(ait);
    QString messagePrefix = QFileInfo(typeSystemFileName).baseName();
    if (messagePrefix.startsWith(QLatin1String("typesystem_")))
        messagePrefix.remove(0, 11);
    ReportHandler::setPrefix(QLatin1Char('(') + messagePrefix + QLatin1Char(')'));

    // Pass option to all generators (Cpp/Header generator have the same options)
    for (ait = args.begin(); ait != args.end(); ) {
        bool found = false;
        for (const GeneratorPtr &generator : qAsConst(generators))
            found |= generator->handleOption(ait.key(), ait.value());
        if (found)
            ait = args.erase(ait);
        else
            ++ait;
    }

    ait = args.find(languageLevelOption());
    if (ait != args.end()) {
        const QByteArray languageLevelBA = ait.value().toLatin1();
        args.erase(ait);
        const LanguageLevel level = clang::languageLevelFromOption(languageLevelBA.constData());
        if (level == LanguageLevel::Default) {
            std::cout << "Invalid argument for language level: \""
                << languageLevelBA.constData() << "\"\n" << helpHint;
            return EXIT_FAILURE;
        }
        extractor.setLanguageLevel(level);
    }

    /* Make sure to remove the project file's arguments (if any) and
     * --project-file, also the arguments of each generator before
     * checking if there isn't any existing arguments in argsHandler.
     */
    args.remove(QLatin1String("project-file"));
    CommandArgumentMap projectFileArgs = getInitializedArguments();
    for (auto it = projectFileArgs.cbegin(), end = projectFileArgs.cend(); it != end; ++it)
        args.remove(it.key());

    if (!args.isEmpty()) {
        errorPrint(msgLeftOverArguments(args));
        std::cout << helpHint;
        return EXIT_FAILURE;
    }

    if (typeSystemFileName.isEmpty()) {
        std::cout << "You must specify a Type System file." << std::endl << helpHint;
        return EXIT_FAILURE;
    }

    extractor.setCppFileName(cppFileNameFi.absoluteFilePath());
    extractor.setTypeSystem(typeSystemFileName);

    if (!extractor.run()) {
        errorPrint(QLatin1String("Error running ApiExtractor."));
        return EXIT_FAILURE;
    }

    if (!extractor.classCount())
        qCWarning(lcShiboken) << "No C++ classes found!";

    qCDebug(lcShiboken) << extractor << '\n'
        << *TypeDatabase::instance();

    for (const GeneratorPtr &g : qAsConst(generators)) {
        g->setOutputDirectory(outputDirectory);
        g->setLicenseComment(licenseComment);
         if (!g->setup(extractor) || !g->generate()) {
             errorPrint(QLatin1String("Error running generator: ")
                        + QLatin1String(g->name()) + QLatin1Char('.'));
             return EXIT_FAILURE;
         }
    }

    QByteArray doneMessage = "Done, " + QByteArray::number(timer.elapsed()) + "ms";
    if (const int w = ReportHandler::warningCount())
        doneMessage += ", " + QByteArray::number(w) + " warnings";
    if (const int sw = ReportHandler::suppressedCount())
        doneMessage += " (" + QByteArray::number(sw) + " known issues)";
    qCDebug(lcShiboken()).noquote().nospace() << doneMessage;
    std::cout << doneMessage.constData() << std::endl;

    return EXIT_SUCCESS;
}
