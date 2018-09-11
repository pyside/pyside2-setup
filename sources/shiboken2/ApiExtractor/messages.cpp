/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "messages.h"
#include "abstractmetalang.h"
#include "typesystem.h"
#include <codemodel.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

static inline QString colonColon() { return QStringLiteral("::"); }

// abstractmetabuilder.cpp

QString msgNoFunctionForModification(const QString &signature,
                                     const QString &originalSignature,
                                     const QString &className,
                                     const QStringList &possibleSignatures,
                                     const AbstractMetaFunctionList &allFunctions)
{
    QString result;
    QTextStream str(&result);
    str << "signature '" << signature << '\'';
    if (!originalSignature.isEmpty() && originalSignature != signature)
        str << " (specified as '" << originalSignature << "')";
    str << " for function modification in '"
        << className << "' not found.";
    if (!possibleSignatures.isEmpty()) {
        str << "\n  Possible candidates:\n";
        for (const auto &s : possibleSignatures)
            str << "    " << s << '\n';
    } else if (!allFunctions.isEmpty()) {
        str << "\n  No candidates were found. Member functions:\n";
        const int maxCount = qMin(10, allFunctions.size());
        for (int f = 0; f < maxCount; ++f)
            str << "    " << allFunctions.at(f)->minimalSignature() << '\n';
        if (maxCount < allFunctions.size())
            str << "    ...\n";
    }
    return result;
}

template <class Stream>
static void msgFormatEnumType(Stream &str,
                              const EnumModelItem &enumItem,
                              const QString &className)
{
    switch (enumItem->enumKind()) {
    case CEnum:
        str << "Enum '" << enumItem->qualifiedName().join(colonColon()) << '\'';
        break;
    case AnonymousEnum: {
        const EnumeratorList &values = enumItem->enumerators();
        str << "Anonymous enum (";
        switch (values.size()) {
        case 0:
            break;
        case 1:
            str << values.constFirst()->name();
            break;
        case 2:
            str << values.at(0)->name() << ", " << values.at(1)->name();
            break;
        default:
            str << values.at(0)->name() << ", ... , "
                << values.at(values.size() - 1)->name();
            break;
        }
        str << ')';
    }
        break;
    case EnumClass:
        str << "Scoped enum '" << enumItem->qualifiedName().join(colonColon()) << '\'';
        break;
    }
    if (!className.isEmpty())
        str << " (class: " << className << ')';
}

QString msgNoEnumTypeEntry(const EnumModelItem &enumItem,
                           const QString &className)
{
    QString result;
    QTextStream str(&result);
    msgFormatEnumType(str, enumItem, className);
    str << " does not have a type entry";
    return result;
}

QString msgNoEnumTypeConflict(const EnumModelItem &enumItem,
                              const QString &className,
                              const TypeEntry *t)
{
    QString result;
    QDebug debug(&result); // Use the debug operator for TypeEntry::Type
    debug.noquote();
    debug.nospace();
    msgFormatEnumType(debug, enumItem, className);
    debug << " is not an enum (type: " << t->type() << ')';
    return result;
}

QString msgUnmatchedParameterType(const ArgumentModelItem &arg, int n,
                                  const QString &why)
{
    QString result;
    QTextStream str(&result);
    str << "unmatched type '" << arg->type().toString() << "' in parameter #"
        << (n + 1);
    if (!arg->name().isEmpty())
        str << " \"" << arg->name() << '"';
    str << ": " << why;
    return result;
}

QString msgUnmatchedReturnType(const FunctionModelItem &functionItem,
                               const QString &why)
{
    return QLatin1String("unmatched return type '")
        + functionItem->type().toString()
        + QLatin1String("': ") + why;
}

QString msgSkippingFunction(const FunctionModelItem &functionItem,
                            const QString &signature, const QString &why)
{
    QString result;
    QTextStream str(&result);
    str << "skipping ";
    if (functionItem->isAbstract())
        str << "abstract ";
    str << "function '" << signature << "', " << why;
    if (functionItem->isAbstract()) {
        str << "\nThis will lead to compilation errors due to not "
               "being able to instantiate the wrapper.";
    }
    return result;
}

QString msgCannotSetArrayUsage(const QString &function, int i, const QString &reason)
{
    return function +  QLatin1String(": Cannot use parameter ")
        + QString::number(i + 1) + QLatin1String(" as an array: ") + reason;
}

QString msgUnableToTranslateType(const QString &t, const QString &why)
{
    return QLatin1String("Unable to translate type \"")
        + t + QLatin1String("\": ") + why;
}

QString msgUnableToTranslateType(const TypeInfo &typeInfo,
                                 const QString &why)
{
    return msgUnableToTranslateType(typeInfo.toString(), why);
}

QString msgCannotFindTypeEntry(const QString &t)
{
    return QLatin1String("Cannot find type entry for \"") + t + QLatin1String("\".");
}

QString msgCannotTranslateTemplateArgument(int i,
                                           const TypeInfo &typeInfo,
                                           const QString &why)
{
    QString result;
    QTextStream(&result) << "Unable to translate template argument "
        << (i + 1) << typeInfo.toString() << ": " << why;
    return result;
}

// abstractmetalang.cpp

QString msgDisallowThread(const AbstractMetaFunction *f)
{
    QString result;
    QTextStream str(&result);
    str <<"Disallowing threads for ";
    if (auto c = f->declaringClass())
        str << c->name() << "::";
    str << f->name() << "().";
    return result;
}

// docparser.cpp

QString msgCannotFindDocumentation(const QString &fileName,
                                   const char *what, const QString &name,
                                   const QString &query)
{
    QString result;
    QTextStream(&result) << "Cannot find documentation for " << what
        << ' ' << name << " in:\n    " << QDir::toNativeSeparators(fileName)
        << "\n  using query:\n    " << query;
    return result;
}

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaFunction *function,
                                   const QString &query)
{
    const QString name = metaClass->name() + QLatin1String("::")
        + function->minimalSignature();
    return msgCannotFindDocumentation(fileName, "function", name, query);
}

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaEnum *e,
                                   const QString &query)
{
    return msgCannotFindDocumentation(fileName, "enum",
                                      metaClass->name() + QLatin1String("::") + e->name(),
                                      query);
}

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaField *f,
                                   const QString &query)
{
    return msgCannotFindDocumentation(fileName, "field",
                                      metaClass->name() + QLatin1String("::") + f->name(),
                                      query);
}

QString msgXpathDocModificationError(const DocModificationList& mods,
                                     const QString &what)
{
    QString result;
    QTextStream str(&result);
    str << "Error when applying modifications (";
    for (const DocModification &mod : mods) {
        if (mod.mode() == TypeSystem::DocModificationXPathReplace) {
            str << '"' << mod.xpath() << "\" -> \"";
            const QString simplified = mod.code().simplified();
            if (simplified.size() > 20)
                str << simplified.leftRef(20) << "...";
            else
                str << simplified;
            str << '"';
        }
    }
    str << "): " << what;
    return result;
}

// fileout.cpp

QString msgCannotOpenForReading(const QFile &f)
{
    return QStringLiteral("Failed to open file '%1' for reading: %2")
           .arg(QDir::toNativeSeparators(f.fileName()), f.errorString());
}

QString msgCannotOpenForWriting(const QFile &f)
{
    return QStringLiteral("Failed to open file '%1' for writing: %2")
           .arg(QDir::toNativeSeparators(f.fileName()), f.errorString());
}

// main.cpp

QString msgLeftOverArguments(const QMap<QString, QString> &remainingArgs)
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

QString msgInvalidVersion(const QString &package, const QString &version)
{
    return QLatin1String("Invalid version \"") + version
        + QLatin1String("\" specified for package ") + package + QLatin1Char('.');
}

QString msgCyclicDependency(const QString &funcName, const QString &graphName,
                            const QVector<const AbstractMetaFunction *> &involvedConversions)
{
    QString result;
    QTextStream str(&result);
    str << "Cyclic dependency found on overloaddata for \"" << funcName
         << "\" method! The graph boy saved the graph at \""
         << QDir::toNativeSeparators(graphName) << "\".";
    if (const int count = involvedConversions.size()) {
        str << " Implicit conversions (" << count << "): ";
        for (int i = 0; i < count; ++i) {
            if (i)
                str << ", \"";
            str << involvedConversions.at(i)->signature() << '"';
            if (const AbstractMetaClass *c = involvedConversions.at(i)->implementingClass())
                str << '(' << c->name() << ')';
        }
    }
    return result;
}

// shibokengenerator.cpp

QString msgUnknownOperator(const AbstractMetaFunction* func)
{
    QString result = QLatin1String("Unknown operator: \"") + func->originalName()
                     + QLatin1Char('"');
    if (const AbstractMetaClass *c = func->implementingClass())
        result += QLatin1String(" in class: ") + c->name();
    return result;
}

QString msgWrongIndex(const char *varName, const QString &capture,
                      const AbstractMetaFunction *func)
{
    QString result;
    QTextStream str(&result);
    str << "Wrong index for " << varName << " variable (" << capture << ") on ";
    if (const AbstractMetaClass *c = func->implementingClass())
        str << c->name() << "::";
    str << func->signature();
    return  result;
}

QString msgCannotFindType(const QString &type, const QString &variable,
                          const QString &why)
{
    QString result;
    QTextStream(&result) << "Could not find type '"
        << type << "' for use in '" << variable << "' conversion: " << why
        << "\nMake sure to use the full C++ name, e.g. 'Namespace::Class'.";
    return result;
}

QString msgCannotBuildMetaType(const QString &s)
{
    return QLatin1String("Unable to build meta type for \"")
        + s + QLatin1String("\": ");
}

QString msgCouldNotFindMinimalConstructor(const QString &where, const QString &type)
{
    return where + QLatin1String(": Could not find a minimal constructor for type '")
       + type + QLatin1String("'. This will result in a compilation error.");
}

// typedatabase.cpp

QString msgRejectReason(const TypeRejection &r, const QString &needle)
{
    QString result;
    QTextStream str(&result);
    switch (r.matchType) {
    case TypeRejection::ExcludeClass:
        str << " matches class exclusion \"" << r.className.pattern() << '"';
        break;
    case TypeRejection::Function:
    case TypeRejection::Field:
    case TypeRejection::Enum:
        str << " matches class \"" << r.className.pattern() << "\" and \""
            << r.pattern.pattern() << '"';
        break;
    case TypeRejection::ArgumentType:
    case TypeRejection::ReturnType:
        str << " matches class \"" << r.className.pattern() << "\" and \""
            << needle << "\" matches \"" << r.pattern.pattern() << '"';
        break;
    case TypeRejection::Invalid:
        break;
    }
    return result;
}

// qtdocgenerator.cpp

QString msgTagWarning(const QXmlStreamReader &reader, const QString &context,
                      const QString &tag, const QString &message)
{
    QString result;
    QTextStream str(&result);
    str << "While handling <";
    const QStringRef currentTag = reader.name();
    if (currentTag.isEmpty())
        str << tag;
    else
        str << currentTag;
    str << "> in " << context << ", line "<< reader.lineNumber()
        << ": " << message;
    return result;
}

QString msgFallbackWarning(const QXmlStreamReader &reader, const QString &context,
                           const QString &tag, const QString &location,
                           const QString &identifier, const QString &fallback)
{
    QString message = QLatin1String("Falling back to \"")
        + QDir::toNativeSeparators(fallback) + QLatin1String("\" for \"")
        + location + QLatin1Char('"');
    if (!identifier.isEmpty())
        message += QLatin1String(" [") + identifier + QLatin1Char(']');
    return msgTagWarning(reader, context, tag, message);
}
