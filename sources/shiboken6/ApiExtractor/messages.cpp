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
#include "abstractmetaenum.h"
#include "abstractmetafield.h"
#include "abstractmetafunction.h"
#include "abstractmetalang.h"
#include "sourcelocation.h"
#include "typedatabase.h"
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

QString msgNoFunctionForModification(const AbstractMetaClass *klass,
                                     const QString &signature,
                                     const QString &originalSignature,
                                     const QStringList &possibleSignatures,
                                     const AbstractMetaFunctionList &allFunctions)
{
    QString result;
    QTextStream str(&result);
    str << klass->typeEntry()->sourceLocation() << "signature '"
        << signature << '\'';
    if (!originalSignature.isEmpty() && originalSignature != signature)
        str << " (specified as '" << originalSignature << "')";
    str << " for function modification in '"
        << klass->qualifiedCppName() << "' not found.";
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

QString msgAddedFunctionInvalidArgType(const QString &addedFuncName,
                                       const QString &typeName,
                                       int pos, const QString &why,
                                       const AbstractMetaClass *context)
{
    QString result;
    QTextStream str(&result);
    if (context)
        str << context->typeEntry()->sourceLocation();
    str << "Unable to translate type \"" << typeName  << "\" of argument "
        << pos << " of added function \"" << addedFuncName << "\": " << why;
    return result;
}

QString msgAddedFunctionInvalidReturnType(const QString &addedFuncName,
                                          const QString &typeName, const QString &why,
                                          const AbstractMetaClass *context)
{
    QString result;
    QTextStream str(&result);
    if (context)
        str << context->typeEntry()->sourceLocation();
    str << "Unable to translate return type \"" <<  typeName
        << "\" of added function \"" << addedFuncName << "\": "
        << why;
    return result;
}

QString msgNoEnumTypeEntry(const EnumModelItem &enumItem,
                           const QString &className)
{
    QString result;
    QTextStream str(&result);
    str << enumItem->sourceLocation();
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
    debug << enumItem->sourceLocation().toString();
    msgFormatEnumType(debug, enumItem, className);
    debug << " is not an enum (type: " << t->type() << ')';
    return result;
}

QString msgNamespaceNoTypeEntry(const NamespaceModelItem &item,
                                const QString &fullName)
{
    QString result;
    QTextStream str(&result);
    str << item->sourceLocation() << "namespace '" << fullName
        << "' does not have a type entry";
    return result;
}

QString msgAmbiguousVaryingTypesFound(const QString &qualifiedName, const TypeEntries &te)
{
    QString result = QLatin1String("Ambiguous types of varying types found for \"") + qualifiedName
        + QLatin1String("\": ");
    QDebug(&result) << te;
    return result;
}

QString msgAmbiguousTypesFound(const QString &qualifiedName, const TypeEntries &te)
{
    QString result = QLatin1String("Ambiguous types found for \"") + qualifiedName
        + QLatin1String("\": ");
    QDebug(&result) << te;
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
    str << functionItem->sourceLocation() << "skipping ";
    if (functionItem->isAbstract())
        str << "abstract ";
    str << "function '" << signature << "', " << why;
    if (functionItem->isAbstract()) {
        str << "\nThis will lead to compilation errors due to not "
               "being able to instantiate the wrapper.";
    }
    return result;
}

QString msgSkippingField(const VariableModelItem &field, const QString &className,
                         const QString &type)
{
    QString result;
    QTextStream str(&result);
    str << field->sourceLocation() << "skipping field '" << className
        << "::" << field->name() << "' with unmatched type '" << type << '\'';
    return result;
}

static const char msgCompilationError[] =
    "This could potentially lead to compilation errors.";

QString msgTypeNotDefined(const TypeEntry *entry)
{
    QString result;
    QTextStream str(&result);
    str << entry->sourceLocation() << "type '" <<entry->qualifiedCppName()
        << "' is specified in typesystem, but not defined. " << msgCompilationError;
    return result;
}

QString msgGlobalFunctionNotDefined(const FunctionTypeEntry *fte,
                                    const QString &signature)
{
    QString result;
    QTextStream str(&result);
    str << fte->sourceLocation() << "Global function '" << signature
        << "' is specified in typesystem, but not defined. " << msgCompilationError;
    return result;
}

QString msgStrippingArgument(const FunctionModelItem &f, int i,
                             const QString &originalSignature,
                             const ArgumentModelItem &arg)
{
    QString result;
    QTextStream str(&result);
    str << f->sourceLocation() << "Stripping argument #" << (i + 1) << " of "
        << originalSignature << " due to unmatched type \""
        << arg->type().toString() << "\" with default expression \""
        << arg->defaultValueExpression() << "\".";
    return result;
}

QString msgEnumNotDefined(const EnumTypeEntry *t)
{
    QString result;
    QTextStream str(&result);
    str << t->sourceLocation() << "enum '" << t->qualifiedCppName()
        << "' is specified in typesystem, but not declared.";
    return result;
}

QString msgUnknownBase(const AbstractMetaClass *metaClass, const QString &baseClassName)
{
    QString result;
    QTextStream str(&result);
    str << metaClass->sourceLocation() << "class '" << metaClass->name()
        << "' inherits from unknown base class '" << baseClassName << "'";
    return result;
}

QString msgArrayModificationFailed(const FunctionModelItem &functionItem,
                                   const QString &className,
                                   const QString &errorMessage)
{
    QString result;
    QTextStream str(&result);
    str << functionItem->sourceLocation() << "While traversing " << className
        << ": " << errorMessage;
    return result;
}

QString msgCannotResolveEntity(const QString &name, const QString &reason)
{
    return QLatin1String("Cannot resolve entity \"") + name
        + QLatin1String("\": ") + reason;
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

QString msgCannotFindTypeEntryForSmartPointer(const QString &t, const QString &smartPointerType)
{
    return QLatin1String("Cannot find type entry \"") + t
        + QLatin1String("\" for instantiation of \"") + smartPointerType + QLatin1String("\".");
}

QString msgInvalidSmartPointerType(const TypeInfo &i)
{
    return QLatin1String("Invalid smart pointer type \"") + i.toString() + QLatin1String("\".");
}

QString msgCannotFindSmartPointerInstantion(const TypeInfo &i)
{
    return QLatin1String("Cannot find instantiation of smart pointer type for \"")
        + i.toString() + QLatin1String("\".");
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

QString msgNamespaceToBeExtendedNotFound(const QString &namespaceName, const QString &packageName)
{
    return QLatin1String("The namespace '") + namespaceName
        + QLatin1String("' to be extended cannot be found in package ")
        + packageName + QLatin1Char('.');
}

QString msgPropertyTypeParsingFailed(const QString &name, const QString &typeName,
                                     const QString &why)
{
    QString result;
    QTextStream str(&result);
    str << "Unable to decide type of property: \"" << name << "\" (" << typeName
        << "): " << why;
    return result;
}

QString msgPropertyExists(const QString &className, const QString &name)
{
    return QLatin1String("class ") + className
        + QLatin1String(" already has a property \"") + name
        + QLatin1String("\" (defined by Q_PROPERTY).");
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
                str << QStringView{simplified}.left(20) << "...";
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

// generator.cpp

QString msgCannotUseEnumAsInt(const QString &name)
{
    return QLatin1String("Cannot convert the protected scoped enum \"") + name
        + QLatin1String("\" to type int when generating wrappers for the protected hack. "
                        "Compilation errors may occur when used as a function argument.");
}

QString msgConversionTypesDiffer(const QString &varType, const QString &conversionType)
{
    QString result;
    QTextStream str(&result);
    str << "Types of receiver variable ('" << varType
         << "') and %%CONVERTTOCPP type system variable ('" << conversionType
         << "') differ";
    QString strippedVarType = varType;
    QString strippedConversionType = conversionType;
    TypeInfo::stripQualifiers(&strippedVarType);
    TypeInfo::stripQualifiers(&strippedConversionType);
    if (strippedVarType == strippedConversionType)
        str << " in qualifiers. Please make sure the type is a distinct token";
    str << '.';
    return result;
}

QString msgCannotFindSmartPointer(const QString &instantiationType,
                                  const AbstractMetaClassList &pointers)
{
    QString result;
    QTextStream str(&result);
    str << "Unable to find smart pointer type for " << instantiationType << " (known types:";
    for (auto t : pointers) {
        auto typeEntry = t->typeEntry();
        str << ' ' << typeEntry->targetLangName() << '/' << typeEntry->qualifiedCppName();
    }
    str << ").";
    return result;
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

// typesystem.cpp

QString msgCannotFindNamespaceToExtend(const QString &name,
                                       const QString &extendsPackage)
{
    return QLatin1String("Cannot find namespace ") + name
        + QLatin1String(" in package ") + extendsPackage;
}

QString msgExtendingNamespaceRequiresPattern(const QString &name)
{
    return QLatin1String("Namespace ") + name
        + QLatin1String(" requires a file pattern since it extends another namespace.");
}

QString msgInvalidRegularExpression(const QString &pattern, const QString &why)
{
    return QLatin1String("Invalid pattern \"") + pattern + QLatin1String("\": ") + why;
}

QString msgNoRootTypeSystemEntry()
{
    return QLatin1String("Type system entry appears out of order, there does not seem to be a root type system element.");
}

QString msgIncorrectlyNestedName(const QString &name)
{
    return QLatin1String("Nesting types by specifying '::' is no longer supported (")
           + name + QLatin1String(").");
}

QString msgCannotFindView(const QString &viewedName, const QString &name)
{
    return QLatin1String("Unable to find viewed type ") + viewedName
        + QLatin1String(" for ") + name;
}

// qtdocgenerator.cpp

QString msgTagWarning(const QXmlStreamReader &reader, const QString &context,
                      const QString &tag, const QString &message)
{
    QString result;
    QTextStream str(&result);
    str << "While handling <";
    const auto currentTag = reader.name();
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
