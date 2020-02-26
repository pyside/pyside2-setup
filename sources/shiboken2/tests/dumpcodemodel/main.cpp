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

static void formatXmlNamespace(QXmlStreamWriter &writer, const NamespaceModelItem &nsp);
static void formatXmlClass(QXmlStreamWriter &writer, const ClassModelItem &klass);

static void formatXmlEnum(QXmlStreamWriter &writer, const EnumModelItem &en)
{
    writer.writeStartElement(QStringLiteral("enum-type"));
    writer.writeAttribute(nameAttribute(), en->name());
    writer.writeEndElement();
}

static void formatXmlScopeMembers(QXmlStreamWriter &writer, const ScopeModelItem &nsp)
{
    for (const auto &klass : nsp->classes()) {
        if (klass->classType() != CodeModel::Union && klass->templateParameters().isEmpty())
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

static void formatXmlClass(QXmlStreamWriter &writer, const ClassModelItem &klass)
{
    // Heuristics for value types: check on public copy constructors.
    const auto functions = klass->functions();
    const bool isValueType = std::any_of(functions.cbegin(), functions.cend(),
                                         isPublicCopyConstructor);
    writer.writeStartElement(isValueType ? QStringLiteral("value-type")
                                         : QStringLiteral("object-type"));
    writer.writeAttribute(nameAttribute(), klass->name());
    formatXmlScopeMembers(writer, klass);
    writer.writeEndElement();
}

static void formatXmlNamespaceMembers(QXmlStreamWriter &writer, const NamespaceModelItem &nsp)
{
    for (const auto &nested : nsp->namespaces())
        formatXmlNamespace(writer, nested);
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

static void formatXmlNamespace(QXmlStreamWriter &writer, const NamespaceModelItem &nsp)
{
    writer.writeStartElement(QStringLiteral("namespace-type"));
    writer.writeAttribute(nameAttribute(), nsp->name());
    formatXmlNamespaceMembers(writer, nsp);
    writer.writeEndElement();
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
