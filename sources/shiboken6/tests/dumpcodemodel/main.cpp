/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <abstractmetabuilder_p.h>
#include <parser/codemodel.h>
#include <clangparser/compilersupport.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamWriter>

#include <iostream>
#include <algorithm>
#include <iterator>

static bool optJoinNamespaces = false;

static inline QString languageLevelDescription()
{
    return QLatin1String("C++ Language level (c++11..c++17, default=")
        + QLatin1String(clang::languageLevelOption(clang::emulatedCompilerLanguageLevel()))
        + QLatin1Char(')');
}

static void formatDebugOutput(const FileModelItem &dom, bool verbose)
{
    QString output;
    {
        QDebug debug(&output);
        if (verbose)
            debug.setVerbosity(3);
        debug << dom.data();
    }
    std::cout << qPrintable(output) << '\n';
}

static const char *primitiveTypes[] = {
    "int", "unsigned", "short", "unsigned short", "long", "unsigned long",
    "float", "double"
};

static inline QString nameAttribute() { return QStringLiteral("name"); }

static void formatXmlClass(QXmlStreamWriter &writer, const ClassModelItem &klass);

static void formatXmlEnum(QXmlStreamWriter &writer, const EnumModelItem &en)
{
    writer.writeStartElement(QStringLiteral("enum-type"));
    writer.writeAttribute(nameAttribute(), en->name());
    writer.writeEndElement();
}

static bool useClass(const ClassModelItem &c)
{
    return c->classType() != CodeModel::Union && c->templateParameters().isEmpty()
        && !c->name().isEmpty(); // No anonymous structs
}

static void formatXmlScopeMembers(QXmlStreamWriter &writer, const ScopeModelItem &nsp)
{
    for (const auto &klass : nsp->classes()) {
        if (useClass(klass))
            formatXmlClass(writer, klass);
    }
    for (const auto &en : nsp->enums())
        formatXmlEnum(writer, en);
}

static bool isPublicCopyConstructor(const FunctionModelItem &f)
{
    return f->functionType() == CodeModel::CopyConstructor
        && f->accessPolicy() == CodeModel::Public && !f->isDeleted();
}

static void formatXmlLocationComment(QXmlStreamWriter &writer, const CodeModelItem &i)
{
    QString comment;
    QTextStream(&comment) << ' ' << i->fileName() << ':' << i->startLine() << ' ';
    writer.writeComment(comment);
}

static void formatXmlClass(QXmlStreamWriter &writer, const ClassModelItem &klass)
{
    // Heuristics for value types: check on public copy constructors.
    const auto functions = klass->functions();
    const bool isValueType = std::any_of(functions.cbegin(), functions.cend(),
                                         isPublicCopyConstructor);
    formatXmlLocationComment(writer, klass);
    writer.writeStartElement(isValueType ? QStringLiteral("value-type")
                                         : QStringLiteral("object-type"));
    writer.writeAttribute(nameAttribute(), klass->name());
    formatXmlScopeMembers(writer, klass);
    writer.writeEndElement();
}

// Check whether a namespace is relevant for type system
// output, that is, has non template classes, functions or enumerations.
static bool hasMembers(const NamespaceModelItem &nsp)
{
    if (!nsp->namespaces().isEmpty() || !nsp->enums().isEmpty()
        || !nsp->functions().isEmpty()) {
        return true;
    }
    const auto classes = nsp->classes();
    return std::any_of(classes.cbegin(), classes.cend(), useClass);
}

static void startXmlNamespace(QXmlStreamWriter &writer, const NamespaceModelItem &nsp)
{
    formatXmlLocationComment(writer, nsp);
    writer.writeStartElement(QStringLiteral("namespace-type"));
    writer.writeAttribute(nameAttribute(), nsp->name());
}

static void formatXmlNamespaceMembers(QXmlStreamWriter &writer, const NamespaceModelItem &nsp)
{
    auto nestedNamespaces = nsp->namespaces();
    for (int i = nestedNamespaces.size() - 1; i >= 0; --i) {
        if (!hasMembers(nestedNamespaces.at(i)))
            nestedNamespaces.removeAt(i);
    }
    while (!nestedNamespaces.isEmpty()) {
        auto current = nestedNamespaces.takeFirst();
        startXmlNamespace(writer, current);
        formatXmlNamespaceMembers(writer, current);
        if (optJoinNamespaces) {
            // Write out members of identical namespaces and remove
            const QString name = current->name();
            for (int i = 0; i < nestedNamespaces.size(); ) {
                if (nestedNamespaces.at(i)->name() == name) {
                    formatXmlNamespaceMembers(writer, nestedNamespaces.at(i));
                    nestedNamespaces.removeAt(i);
                } else {
                    ++i;
                }
            }
        }
        writer.writeEndElement();
    }

    for (auto func : nsp->functions()) {
        const QString signature = func->typeSystemSignature();
        if (!signature.contains(QLatin1String("operator"))) { // Skip free operators
            writer.writeStartElement(QStringLiteral("function"));
            writer.writeAttribute(QStringLiteral("signature"), signature);
            writer.writeEndElement();
        }
    }
    formatXmlScopeMembers(writer, nsp);
}

static void formatXmlOutput(const FileModelItem &dom)
{
    QString output;
    QXmlStreamWriter writer(&output);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("typesystem"));
    writer.writeAttribute(QStringLiteral("package"), QStringLiteral("insert_name"));
    writer.writeComment(QStringLiteral("Auto-generated ") +
                        QDateTime::currentDateTime().toString(Qt::ISODate));
    for (auto p : primitiveTypes) {
        writer.writeStartElement(QStringLiteral("primitive-type"));
        writer.writeAttribute(nameAttribute(), QLatin1String(p));
        writer.writeEndElement();
    }
    formatXmlNamespaceMembers(writer, dom);
    writer.writeEndElement();
    writer.writeEndDocument();
    std::cout << qPrintable(output) << '\n';
}

static const char descriptionFormat[] = R"(
Type system dumper

Parses a C++ header and dumps out the classes found in typesystem XML syntax.
Arguments are arguments to the compiler the last of which should be the header
or source file.
It is recommended to create a .hh include file including the desired headers
and pass that along with the required include paths.

Based on Qt %1 and LibClang v%2.)";

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    const QString description =
        QString::fromLatin1(descriptionFormat).arg(QLatin1String(qVersion()),
                                                   clang::libClangVersion().toString());
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption verboseOption(QStringLiteral("verbose"),
                                     QStringLiteral("Display verbose output about types"));
    parser.addOption(verboseOption);
    QCommandLineOption debugOption(QStringLiteral("debug"),
                                     QStringLiteral("Display debug output"));
    parser.addOption(debugOption);

    QCommandLineOption joinNamespacesOption({QStringLiteral("j"), QStringLiteral("join-namespaces")},
                                            QStringLiteral("Join namespaces"));
    parser.addOption(joinNamespacesOption);

    QCommandLineOption languageLevelOption(QStringLiteral("std"),
                                           languageLevelDescription(),
                                           QStringLiteral("level"));
    parser.addOption(languageLevelOption);
    parser.addPositionalArgument(QStringLiteral("argument"),
                                 QStringLiteral("C++ compiler argument"),
                                 QStringLiteral("argument(s)"));

    parser.process(app);
    const QStringList &positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty())
        parser.showHelp(1);

    QByteArrayList arguments;
    std::transform(positionalArguments.cbegin(), positionalArguments.cend(),
                   std::back_inserter(arguments), QFile::encodeName);

    LanguageLevel level = LanguageLevel::Default;
    if (parser.isSet(languageLevelOption)) {
        const QByteArray value = parser.value(languageLevelOption).toLatin1();
        level = clang::languageLevelFromOption(value.constData());
        if (level == LanguageLevel::Default) {
            std::cerr << "Invalid value \"" << value.constData()
                << "\" for language level option.\n";
            return -2;
        }
    }

    optJoinNamespaces = parser.isSet(joinNamespacesOption);

    const FileModelItem dom = AbstractMetaBuilderPrivate::buildDom(arguments, level, 0);
    if (dom.isNull()) {
        QString message = QLatin1String("Unable to parse ") + positionalArguments.join(QLatin1Char(' '));
        std::cerr << qPrintable(message) << '\n';
        return -2;
    }

    if (parser.isSet(debugOption))
        formatDebugOutput(dom, parser.isSet(verboseOption));
    else
        formatXmlOutput(dom);

    return 0;
}
