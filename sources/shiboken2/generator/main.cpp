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

static inline QString includePathOption() { return QStringLiteral("include-paths"); }
static inline QString frameworkIncludePathOption() { return QStringLiteral("framework-include-paths"); }
static inline QString systemIncludePathOption() { return QStringLiteral("system-include-paths"); }
static inline QString typesystemPathOption() { return QStringLiteral("typesystem-paths"); }
static inline QString helpOption() { return QStringLiteral("help"); }
static const char helpHint[] = "Note: use --help or -h for more information.\n";

namespace {

class ArgsHandler
{
public:
    explicit ArgsHandler(const QMap<QString, QString>& other);
    virtual ~ArgsHandler();

    inline QMap<QString, QString>& args() const
    {
        return *m_args;
    }

    inline bool argExists(const QString& s) const
    {
        return m_args->contains(s);
    }

    QString removeArg(const QString& s);
    bool argExistsRemove(const QString& s);

    inline QString argValue(const QString& s) const
    {
        return m_args->value(s);
    }

    inline bool noArgs() const
    {
        return m_args->isEmpty();
    }

    QString errorMessage() const;

private:
    QMap<QString, QString>* m_args;
};

ArgsHandler::ArgsHandler(const QMap<QString, QString>& other)
    : m_args(new QMap<QString, QString>(other))
{
}

ArgsHandler::~ArgsHandler()
{
    delete m_args;
}

QString ArgsHandler::removeArg(const QString& s)
{
    QString retval;

    if (argExists(s)) {
        retval = argValue(s);
        m_args->remove(s);
    }

    return retval;
}

bool ArgsHandler::argExistsRemove(const QString& s)
{
    bool retval = false;

    if (argExists(s)) {
        retval = true;
        m_args->remove(s);
    }

    return retval;
}

QString ArgsHandler::errorMessage() const
{
    typedef QMap<QString, QString>::ConstIterator StringMapConstIt;

    QString message;
    QTextStream str(&message);
    str << "shiboken: Called with wrong arguments:";
    for (StringMapConstIt it = m_args->cbegin(), end = m_args->cend(); it != end; ++it) {
        str << ' ' << it.key();
        if (!it.value().isEmpty())
            str << ' ' << it.value();
    }
    str << "\nCommand line: " << QCoreApplication::arguments().join(QLatin1Char(' '));
    return message;
}
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
    return true;
}

static QMap<QString, QString> getInitializedArguments()
{
    QMap<QString, QString> args;
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
                               QMap<QString, QString> &args)
{
    const QMap<QString, QString>::iterator it = args.find(option);
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
        << qMakePair(QLatin1String("-F") + pathSyntax, QString())
        << qMakePair(QLatin1String("framework-include-paths=") + pathSyntax,
                     QLatin1String("Framework include paths used by the C++ parser"))
        << qMakePair(QLatin1String("-isystem") + pathSyntax, QString())
        << qMakePair(QLatin1String("system-include-paths=") + pathSyntax,
                     QLatin1String("System include paths used by the C++ parser"))
        << qMakePair(QLatin1String("generator-set=<\"generator module\">"),
                     QLatin1String("generator-set to be used. e.g. qtdoc"))
        << qMakePair(QLatin1String("-h"), QString())
        << qMakePair(helpOption(),
                     QLatin1String("Display this help and exit"))
        << qMakePair(QLatin1String("-I") + pathSyntax, QString())
        << qMakePair(QLatin1String("include-paths=") + pathSyntax,
                     QLatin1String("Include paths used by the C++ parser"))
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
        << qMakePair(QLatin1String("-T") + pathSyntax, QString())
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
                                   ArgsHandler &args,
                                   ApiExtractor &extractor)
{
    const QString path = args.removeArg(option);
    if (!path.isEmpty()) {
        const QStringList includePathListList = path.split(QLatin1String(PATH_SPLITTER));
        for (const QString &s : includePathListList)
            extractor.addIncludePath(HeaderPath{QFile::encodeName(s), headerType});
    }
}

int main(int argc, char *argv[])
{
    QElapsedTimer timer;
    timer.start();
    // needed by qxmlpatterns
    QCoreApplication app(argc, argv);
    ReportHandler::install();
    qCDebug(lcShiboken()).noquote().nospace() << QCoreApplication::arguments().join(QLatin1Char(' '));

    // Store command arguments in a map
    QMap<QString, QString> args = getCommandLineArgs();
    ArgsHandler argsHandler(args);
    Generators generators;

    if (argsHandler.argExistsRemove(QLatin1String("version"))) {
        printVerAndBanner();
        return EXIT_SUCCESS;
    }

    QString generatorSet = argsHandler.removeArg(QLatin1String("generator-set"));
    // Also check QLatin1String("generatorSet") command line argument for backward compatibility.
    if (generatorSet.isEmpty())
        generatorSet = argsHandler.removeArg(QLatin1String("generatorSet"));

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

    if (argsHandler.argExistsRemove(QLatin1String("help"))) {
        printUsage();
        return EXIT_SUCCESS;
    }

    QString licenseComment;
    QString licenseFileName = argsHandler.removeArg(QLatin1String("license-file"));
    if (!licenseFileName.isEmpty()) {
        if (QFile::exists(licenseFileName)) {
            QFile licenseFile(licenseFileName);
            if (licenseFile.open(QIODevice::ReadOnly))
                licenseComment = QString::fromUtf8(licenseFile.readAll());
        } else {
            errorPrint(QStringLiteral("Couldn't find the file containing the license heading: %1").
                       arg(licenseFileName));
            return EXIT_FAILURE;
        }
    }

    QString outputDirectory = argsHandler.removeArg(QLatin1String("output-directory"));
    if (outputDirectory.isEmpty())
        outputDirectory = QLatin1String("out");

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

    if (argsHandler.argExistsRemove(QLatin1String("silent"))) {
        extractor.setSilent(true);
    } else {
        QString level = argsHandler.removeArg(QLatin1String("debug-level"));
        if (!level.isEmpty()) {
            if (level == QLatin1String("sparse"))
                extractor.setDebugLevel(ReportHandler::SparseDebug);
            else if (level == QLatin1String("medium"))
                extractor.setDebugLevel(ReportHandler::MediumDebug);
            else if (level == QLatin1String("full"))
                extractor.setDebugLevel(ReportHandler::FullDebug);
        }
    }
    if (argsHandler.argExistsRemove(QLatin1String("no-suppress-warnings")))
        extractor.setSuppressWarnings(false);

    if (argsHandler.argExists(QLatin1String("api-version"))) {
        const QStringList &versions = argsHandler.removeArg(QLatin1String("api-version")).split(QLatin1Char('|'));
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

    if (argsHandler.argExists(QLatin1String("drop-type-entries")))
        extractor.setDropTypeEntries(argsHandler.removeArg(QLatin1String("drop-type-entries")));

    QString path = argsHandler.removeArg(QLatin1String("typesystem-paths"));
    if (!path.isEmpty())
        extractor.addTypesystemSearchPath(path.split(QLatin1String(PATH_SPLITTER)));

    parseIncludePathOption(includePathOption(), HeaderType::Standard,
                           argsHandler, extractor);
    parseIncludePathOption(frameworkIncludePathOption(), HeaderType::Framework,
                           argsHandler, extractor);
    parseIncludePathOption(systemIncludePathOption(), HeaderType::System,
                           argsHandler, extractor);

    QString cppFileName = argsHandler.removeArg(QLatin1String("arg-1"));
    const QFileInfo cppFileNameFi(cppFileName);
    if (!cppFileNameFi.isFile() && !cppFileNameFi.isSymLink()) {
        errorPrint(QLatin1Char('"') + cppFileName + QLatin1String("\" does not exist."));
        return EXIT_FAILURE;
    }

    QString typeSystemFileName = argsHandler.removeArg(QLatin1String("arg-2"));
    QString messagePrefix = QFileInfo(typeSystemFileName).baseName();
    if (messagePrefix.startsWith(QLatin1String("typesystem_")))
        messagePrefix.remove(0, 11);
    ReportHandler::setPrefix(QLatin1Char('(') + messagePrefix + QLatin1Char(')'));

    /* Make sure to remove the project file's arguments (if any) and
     * --project-file, also the arguments of each generator before
     * checking if there isn't any existing arguments in argsHandler.
     */
    argsHandler.removeArg(QLatin1String("project-file"));
    QMap<QString, QString> projectFileArgs = getInitializedArguments();
    if (!projectFileArgs.isEmpty()) {
        QMap<QString, QString>::const_iterator it =
                                projectFileArgs.constBegin();
        for ( ; it != projectFileArgs.constEnd(); ++it)
            argsHandler.removeArg(it.key());
    }
    for (const GeneratorPtr &generator : qAsConst(generators)) {
        const OptionDescriptions &options = generator->options();
        for (const auto &od : options)
            argsHandler.removeArg(od.first);
    }

    if (!argsHandler.noArgs()) {
        errorPrint(argsHandler.errorMessage());
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

    qCDebug(lcShiboken) << extractor;

    for (const GeneratorPtr &g : qAsConst(generators)) {
        g->setOutputDirectory(outputDirectory);
        g->setLicenseComment(licenseComment);
         if (g->setup(extractor, args)) {
             if (!g->generate()) {
                 errorPrint(QLatin1String("Error running generator: ")
                            + QLatin1String(g->name()) + QLatin1Char('.'));
                 return EXIT_FAILURE;
             }
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
