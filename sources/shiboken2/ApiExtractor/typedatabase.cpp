/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "typedatabase.h"
#include "typesystem.h"
#include "typesystemparser.h"

#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QVector>
#include <QtCore/QRegularExpression>
#include <QtCore/QVersionNumber>
#include <QtCore/QXmlStreamReader>
#include "reporthandler.h"
// #include <tr1/tuple>
#include <algorithm>

// package -> api-version

static QString wildcardToRegExp(QString w)
{
    w.replace(QLatin1Char('?'), QLatin1Char('.'));
    w.replace(QLatin1Char('*'), QStringLiteral(".*"));
    return w;
}

using ApiVersion =QPair<QRegularExpression, QVersionNumber>;
using ApiVersions = QVector<ApiVersion>;

Q_GLOBAL_STATIC(ApiVersions, apiVersions)

TypeDatabase::TypeDatabase()
{
    addType(new VoidTypeEntry());
    addType(new VarargsTypeEntry());
}

TypeDatabase::~TypeDatabase() = default;

TypeDatabase* TypeDatabase::instance(bool newInstance)
{
    static TypeDatabase *db = nullptr;
    if (!db || newInstance) {
        delete db;
        db = new TypeDatabase;
    }
    return db;
}

// A list of regex/replacements to fix int types like "ushort" to "unsigned short"
// unless present in TypeDatabase
struct IntTypeNormalizationEntry
{
    QRegularExpression regex;
    QString replacement;
};

using IntTypeNormalizationEntries = QVector<IntTypeNormalizationEntry>;

static const IntTypeNormalizationEntries &intTypeNormalizationEntries()
{
    static IntTypeNormalizationEntries result;
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        for (auto t : {"char", "short", "int", "long"}) {
            const QString intType = QLatin1String(t);
            if (!TypeDatabase::instance()->findType(QLatin1Char('u') + intType)) {
                IntTypeNormalizationEntry entry;
                entry.replacement = QStringLiteral("unsigned ") + intType;
                entry.regex.setPattern(QStringLiteral("\\bu") + intType + QStringLiteral("\\b"));
                Q_ASSERT(entry.regex.isValid());
                result.append(entry);
            }
        }
    }
    return result;
}

QString TypeDatabase::normalizedSignature(const QString &signature)
{
    QString normalized = QLatin1String(QMetaObject::normalizedSignature(signature.toUtf8().constData()));

    if (instance() && signature.contains(QLatin1String("unsigned"))) {
        const IntTypeNormalizationEntries &entries = intTypeNormalizationEntries();
        for (const auto &entry : entries)
            normalized.replace(entry.regex, entry.replacement);
    }

    return normalized;
}

QStringList TypeDatabase::requiredTargetImports() const
{
    return m_requiredTargetImports;
}

void TypeDatabase::addRequiredTargetImport(const QString& moduleName)
{
    if (!m_requiredTargetImports.contains(moduleName))
        m_requiredTargetImports << moduleName;
}

void TypeDatabase::addTypesystemPath(const QString& typesystem_paths)
{
    #if defined(Q_OS_WIN32)
    const char path_splitter = ';';
    #else
    const char path_splitter = ':';
    #endif
    m_typesystemPaths += typesystem_paths.split(QLatin1Char(path_splitter));
}

IncludeList TypeDatabase::extraIncludes(const QString& className) const
{
    ComplexTypeEntry* typeEntry = findComplexType(className);
    return typeEntry ? typeEntry->extraIncludes() : IncludeList();
}

void TypeDatabase::addSystemInclude(const QString &name)
{
    m_systemIncludes.append(name.toUtf8());
}

// Add a lookup for the short name excluding inline namespaces
// so that "std::shared_ptr" finds "std::__1::shared_ptr" as well.
// Note: This inserts duplicate TypeEntry * into m_entries.
void TypeDatabase::addInlineNamespaceLookups(const NamespaceTypeEntry *n)
{
    QVector<TypeEntry *> additionalEntries; // Store before modifying the hash
    for (TypeEntry *entry : m_entries) {
        if (entry->isChildOf(n))
            additionalEntries.append(entry);
    }
    for (const auto &ae : additionalEntries)
        m_entries.insert(ae->shortName(), ae);
}

ContainerTypeEntry* TypeDatabase::findContainerType(const QString &name) const
{
    QString template_name = name;

    int pos = name.indexOf(QLatin1Char('<'));
    if (pos > 0)
        template_name = name.left(pos);

    TypeEntry* type_entry = findType(template_name);
    if (type_entry && type_entry->isContainer())
        return static_cast<ContainerTypeEntry*>(type_entry);
    return nullptr;
}

static bool inline useType(const TypeEntry *t)
{
    return !t->isPrimitive()
        || static_cast<const PrimitiveTypeEntry *>(t)->preferredTargetLangType();
}

FunctionTypeEntry* TypeDatabase::findFunctionType(const QString& name) const
{
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (entry->type() == TypeEntry::FunctionType && useType(entry))
            return static_cast<FunctionTypeEntry*>(entry);
    }
    return nullptr;
}

void TypeDatabase::addTypeSystemType(const TypeSystemTypeEntry *e)
{
    m_typeSystemEntries.append(e);
}

const TypeSystemTypeEntry *TypeDatabase::findTypeSystemType(const QString &name) const
{
    for (auto entry : m_typeSystemEntries) {
        if (entry->name() == name)
            return entry;
    }
    return nullptr;
}

const TypeSystemTypeEntry *TypeDatabase::defaultTypeSystemType() const
{
    return m_typeSystemEntries.value(0, nullptr);
}

QString TypeDatabase::defaultPackageName() const
{
    Q_ASSERT(!m_typeSystemEntries.isEmpty());
    return m_typeSystemEntries.constFirst()->name();
}

TypeEntry* TypeDatabase::findType(const QString& name) const
{
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (useType(entry))
            return entry;
    }
    return nullptr;
}

template <class Predicate>
TypeEntries TypeDatabase::findTypesHelper(const QString &name, Predicate pred) const
{
    TypeEntries result;
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (pred(entry))
            result.append(entry);
    }
    return result;
}

TypeEntries TypeDatabase::findTypes(const QString &name) const
{
    return findTypesHelper(name, useType);
}

static bool useCppType(const TypeEntry *t)
{
    bool result = false;
    switch (t->type()) {
    case TypeEntry::PrimitiveType:
    case TypeEntry::VoidType:
    case TypeEntry::FlagsType:
    case TypeEntry::EnumType:
    case TypeEntry::TemplateArgumentType:
    case TypeEntry::BasicValueType:
    case TypeEntry::ContainerType:
    case TypeEntry::ObjectType:
    case TypeEntry::ArrayType:
    case TypeEntry::CustomType:
    case TypeEntry::SmartPointerType:
    case TypeEntry::TypedefType:
        result = useType(t);
        break;
    default:
        break;
    }
    return result;
}

TypeEntries TypeDatabase::findCppTypes(const QString &name) const
{
    return findTypesHelper(name, useCppType);
}

TypeEntryMultiMapConstIteratorRange TypeDatabase::findTypeRange(const QString &name) const
{
    const auto range = m_entries.equal_range(name);
    return {range.first, range.second};
}

PrimitiveTypeEntryList TypeDatabase::primitiveTypes() const
{
    PrimitiveTypeEntryList returned;
    for (auto it = m_entries.cbegin(), end = m_entries.cend(); it != end; ++it) {
        TypeEntry *typeEntry = it.value();
        if (typeEntry->isPrimitive())
            returned.append(static_cast<PrimitiveTypeEntry *>(typeEntry));
    }
    return returned;
}

ContainerTypeEntryList TypeDatabase::containerTypes() const
{
    ContainerTypeEntryList returned;
    for (auto it = m_entries.cbegin(), end = m_entries.cend(); it != end; ++it) {
        TypeEntry *typeEntry = it.value();
        if (typeEntry->isContainer())
            returned.append(static_cast<ContainerTypeEntry *>(typeEntry));
    }
    return returned;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeRejection &r)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TypeRejection(type=" << r.matchType << ", class="
        << r.className.pattern() << ", pattern=" << r.pattern.pattern() << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

void TypeDatabase::addRejection(const TypeRejection &r)
{
    m_rejections << r;
}

static inline QString msgRejectReason(const TypeRejection &r, const QString &needle = QString())
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
        str << " matches class \"" << r.className.pattern() << "\" and \"" << r.pattern.pattern() << '"';
        break;
    case TypeRejection::ArgumentType:
    case TypeRejection::ReturnType:
        str << " matches class \"" << r.className.pattern() << "\" and \"" << needle
            << "\" matches \"" << r.pattern.pattern() << '"';
        break;
    case TypeRejection::Invalid:
        break;
    }
    return result;
}

// Match class name only
bool TypeDatabase::isClassRejected(const QString& className, QString *reason) const
{
    for (const TypeRejection& r : m_rejections) {
        if (r.matchType == TypeRejection::ExcludeClass && r.className.match(className).hasMatch()) {
            if (reason)
                *reason = msgRejectReason(r);
            return true;
        }
    }
    return false;
}

// Match class name and function/enum/field
static bool findRejection(const QVector<TypeRejection> &rejections,
                          TypeRejection::MatchType matchType,
                          const QString& className, const QString& name,
                          QString *reason = nullptr)
{
    Q_ASSERT(matchType != TypeRejection::ExcludeClass);
    for (const TypeRejection& r : rejections) {
        if (r.matchType == matchType && r.pattern.match(name).hasMatch()
            && r.className.match(className).hasMatch()) {
            if (reason)
                *reason = msgRejectReason(r, name);
            return true;
        }
    }
    return false;
}

bool TypeDatabase::isEnumRejected(const QString& className, const QString& enumName, QString *reason) const
{
    return findRejection(m_rejections, TypeRejection::Enum, className, enumName, reason);
}

TypeEntry *TypeDatabase::resolveTypeDefEntry(TypedefEntry *typedefEntry,
                                             QString *errorMessage)
{
    QString sourceName = typedefEntry->sourceType();
    const int lessThanPos = sourceName.indexOf(QLatin1Char('<'));
    if (lessThanPos != -1)
        sourceName.truncate(lessThanPos);
    ComplexTypeEntry *source = nullptr;
    for (TypeEntry *e : findTypeRange(sourceName)) {
        switch (e->type()) {
        case TypeEntry::BasicValueType:
        case TypeEntry::ContainerType:
        case TypeEntry::ObjectType:
        case TypeEntry::SmartPointerType:
            source = dynamic_cast<ComplexTypeEntry *>(e);
            Q_ASSERT(source);
            break;
        default:
            break;
        }
    }
    if (!source) {
        if (errorMessage)
            *errorMessage = QLatin1String("Unable to resolve typedef \"")
                            + typedefEntry->sourceType() + QLatin1Char('"');
        return nullptr;
    }

    auto *result = static_cast<ComplexTypeEntry *>(source->clone());
    result->useAsTypedef(typedefEntry);
    typedefEntry->setSource(source);
    typedefEntry->setTarget(result);
    m_typedefEntries.insert(typedefEntry->qualifiedCppName(), typedefEntry);
    return result;
}

bool TypeDatabase::addType(TypeEntry *e, QString *errorMessage)
{
    if (e->type() == TypeEntry::TypedefType) {
        e = resolveTypeDefEntry(static_cast<TypedefEntry *>(e), errorMessage);
        if (Q_UNLIKELY(!e))
            return false;
    }
    m_entries.insert(e->qualifiedCppName(), e);
    return true;
}

bool TypeDatabase::isFunctionRejected(const QString& className, const QString& functionName,
                                      QString *reason) const
{
    return findRejection(m_rejections, TypeRejection::Function, className, functionName, reason);
}

bool TypeDatabase::isFieldRejected(const QString& className, const QString& fieldName,
                                   QString *reason) const
{
    return findRejection(m_rejections, TypeRejection::Field, className, fieldName, reason);
}

bool TypeDatabase::isArgumentTypeRejected(const QString& className, const QString& typeName,
                                          QString *reason) const
{
    return findRejection(m_rejections, TypeRejection::ArgumentType, className, typeName, reason);
}

bool TypeDatabase::isReturnTypeRejected(const QString& className, const QString& typeName,
                                        QString *reason) const
{
    return findRejection(m_rejections, TypeRejection::ReturnType, className, typeName, reason);
}

FlagsTypeEntry* TypeDatabase::findFlagsType(const QString &name) const
{
    TypeEntry *fte = findType(name);
    if (!fte) {
        fte = m_flagsEntries.value(name);
        if (!fte) {
            //last hope, search for flag without scope  inside of flags hash
            for (auto it = m_flagsEntries.cbegin(), end = m_flagsEntries.cend(); it != end; ++it) {
                if (it.key().endsWith(name)) {
                    fte = it.value();
                    break;
                }
            }
        }
    }
    return static_cast<FlagsTypeEntry *>(fte);
}

void TypeDatabase::addFlagsType(FlagsTypeEntry *fte)
{
    m_flagsEntries[fte->originalName()] = fte;
}

void TypeDatabase::addTemplate(TemplateEntry *t)
{
    m_templates[t->name()] = t;
}

void TypeDatabase::addGlobalUserFunctions(const AddedFunctionList &functions)
{
    m_globalUserFunctions << functions;
}

AddedFunctionList TypeDatabase::findGlobalUserFunctions(const QString& name) const
{
    AddedFunctionList addedFunctions;
    for (const AddedFunctionPtr &func : m_globalUserFunctions) {
        if (func->name() == name)
            addedFunctions.append(func);
    }
    return addedFunctions;
}

void TypeDatabase::addGlobalUserFunctionModifications(const FunctionModificationList &functionModifications)
{
    m_functionMods << functionModifications;
}

QString TypeDatabase::globalNamespaceClassName(const TypeEntry * /*entry*/)
{
    return QLatin1String("Global");
}

FunctionModificationList TypeDatabase::functionModifications(const QString& signature) const
{
    FunctionModificationList lst;
    for (int i = 0; i < m_functionMods.count(); ++i) {
        const FunctionModification& mod = m_functionMods.at(i);
        if (mod.matches(signature))
            lst << mod;
    }

    return lst;
}

bool TypeDatabase::addSuppressedWarning(const QString &warning, QString *errorMessage)
{
    QString pattern;
    if (warning.startsWith(QLatin1Char('^')) && warning.endsWith(QLatin1Char('$'))) {
        pattern = warning;
    } else {
        // Legacy syntax: Use wildcards '*' (unless escaped by '\')
        QVector<int> asteriskPositions;
        const int warningSize = warning.size();
        for (int i = 0; i < warningSize; ++i) {
            if (warning.at(i) == QLatin1Char('\\'))
                ++i;
            else if (warning.at(i) == QLatin1Char('*'))
                asteriskPositions.append(i);
        }
        asteriskPositions.append(warningSize);

        pattern.append(QLatin1Char('^'));
        int lastPos = 0;
        for (int a = 0, aSize = asteriskPositions.size(); a < aSize; ++a) {
            if (a)
                pattern.append(QStringLiteral(".*"));
            const int nextPos = asteriskPositions.at(a);
            if (nextPos > lastPos)
                pattern.append(QRegularExpression::escape(warning.mid(lastPos, nextPos - lastPos)));
            lastPos = nextPos + 1;
        }
        pattern.append(QLatin1Char('$'));
    }

    QRegularExpression expression(pattern);
    if (!expression.isValid()) {
        *errorMessage = QLatin1String("Invalid message pattern \"") + warning
            + QLatin1String("\": ") + expression.errorString();
        return false;
    }
    expression.setPatternOptions(expression.patternOptions() | QRegularExpression::MultilineOption);

    m_suppressedWarnings.append(expression);
    return true;
}

bool TypeDatabase::isSuppressedWarning(const QString& s) const
{
    if (!m_suppressWarnings)
        return false;

    for (const QRegularExpression &warning : m_suppressedWarnings) {
        if (warning.match(s).hasMatch())
             return true;
    }

    return false;
}

QString TypeDatabase::modifiedTypesystemFilepath(const QString& tsFile, const QString &currentPath) const
{
    const QFileInfo tsFi(tsFile);
    if (tsFi.isAbsolute()) // No point in further lookups
        return tsFi.absoluteFilePath();
    if (tsFi.isFile()) // Make path absolute
        return tsFi.absoluteFilePath();
    if (!currentPath.isEmpty()) {
        const QFileInfo fi(currentPath + QLatin1Char('/') + tsFile);
        if (fi.isFile())
            return fi.absoluteFilePath();
    }
    for (const QString &path : m_typesystemPaths) {
        const QFileInfo fi(path + QLatin1Char('/') + tsFile);
        if (fi.isFile())
            return fi.absoluteFilePath();
    }
    return tsFile;
}

bool TypeDatabase::parseFile(const QString &filename, bool generate)
{
    return parseFile(filename, QString(), generate);
}

bool TypeDatabase::parseFile(const QString &filename, const QString &currentPath, bool generate)
{

    QString filepath = modifiedTypesystemFilepath(filename, currentPath);
    if (m_parsedTypesystemFiles.contains(filepath))
        return m_parsedTypesystemFiles[filepath];

    m_parsedTypesystemFiles[filepath] = true; // Prevent recursion when including self.

    QFile file(filepath);
    if (!file.exists()) {
        m_parsedTypesystemFiles[filepath] = false;
        QString message = QLatin1String("Can't find ") + filename;
        if (!currentPath.isEmpty())
            message += QLatin1String(", current path: ") + currentPath;
        message += QLatin1String(", typesystem paths: ") + m_typesystemPaths.join(QLatin1String(", "));
        qCWarning(lcShiboken).noquote().nospace() << message;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_parsedTypesystemFiles[filepath] = false;
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't open " << QDir::toNativeSeparators(filename) << ": " << file.errorString();
        return false;
    }

    int count = m_entries.size();
    bool ok = parseFile(&file, generate);
    m_parsedTypesystemFiles[filepath] = ok;
    int newCount = m_entries.size();

    if (ReportHandler::isDebug(ReportHandler::SparseDebug)) {
          qCDebug(lcShiboken)
              << QStringLiteral("Parsed: '%1', %2 new entries").arg(filename).arg(newCount - count);
    }
    return ok;
}

bool TypeDatabase::parseFile(QIODevice* device, bool generate)
{
    QXmlStreamReader reader(device);
    TypeSystemParser handler(this, generate);
    const bool result = handler.parse(reader);
    if (!result)
        qCWarning(lcShiboken, "%s", qPrintable(handler.errorString()));
    return result;
}

PrimitiveTypeEntry *TypeDatabase::findPrimitiveType(const QString& name) const
{
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (entry->isPrimitive()) {
            auto *pe = static_cast<PrimitiveTypeEntry *>(entry);
            if (pe->preferredTargetLangType())
                return pe;
        }
    }

    return nullptr;
}

ComplexTypeEntry* TypeDatabase::findComplexType(const QString& name) const
{
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (entry->isComplex() && useType(entry))
            return static_cast<ComplexTypeEntry*>(entry);
    }
    return nullptr;
}

ObjectTypeEntry* TypeDatabase::findObjectType(const QString& name) const
{
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (entry && entry->isObject() && useType(entry))
            return static_cast<ObjectTypeEntry*>(entry);
    }
    return nullptr;
}

NamespaceTypeEntryList TypeDatabase::findNamespaceTypes(const QString& name) const
{
    NamespaceTypeEntryList result;
    const auto entries = findTypeRange(name);
    for (TypeEntry *entry : entries) {
        if (entry->isNamespace())
            result.append(static_cast<NamespaceTypeEntry*>(entry));
    }
    return result;
}

NamespaceTypeEntry *TypeDatabase::findNamespaceType(const QString& name,
                                                    const QString &fileName) const
{
    const auto entries = findNamespaceTypes(name);
    // Preferably check on matching file name first, if a pattern was given.
    if (!fileName.isEmpty()) {
        for (NamespaceTypeEntry *entry : entries) {
            if (entry->hasPattern() && entry->matchesFile(fileName))
                return entry;
        }
    }
    for (NamespaceTypeEntry *entry : entries) {
        if (!entry->hasPattern())
            return entry;
    }
    return nullptr;
}

bool TypeDatabase::shouldDropTypeEntry(const QString& fullTypeName) const
{
    return m_dropTypeEntries.contains(fullTypeName);
}

void TypeDatabase::setDropTypeEntries(QStringList dropTypeEntries)
{
    m_dropTypeEntries = dropTypeEntries;
    m_dropTypeEntries.sort();
}

static bool computeTypeIndexes = true;
static int maxTypeIndex;

static bool typeEntryLessThan(const TypeEntry* t1, const TypeEntry* t2)
{
    if (t1->revision() < t2->revision())
        return true;
    return t1->revision() == t2->revision()
        && t1->qualifiedCppName() < t2->qualifiedCppName();
}

static void _computeTypeIndexes()
{
    TypeDatabase* tdb = TypeDatabase::instance();

    TypeEntryList list;

    // Group type entries by revision numbers
    const auto &allEntries = tdb->entries();
    list.reserve(allEntries.size());
    for (auto  tit = allEntries.cbegin(), end = allEntries.cend(); tit != end; ++tit) {
        TypeEntry *entry = tit.value();
        if (entry->isPrimitive()
            || entry->isContainer()
            || entry->isFunction()
            || !entry->generateCode()
            || entry->isEnumValue()
            || entry->isVarargs()
            || entry->isTypeSystem()
            || entry->isVoid()
            || entry->isCustom())
            continue;
        if (!list.contains(entry)) // Remove duplicates
            list.append(entry);
    }

    // Sort the type entries by revision, name
    std::sort(list.begin(), list.end(), typeEntryLessThan);

    maxTypeIndex = 0;
    for (TypeEntry *e : qAsConst(list))
        e->setSbkIndex(maxTypeIndex++);
    computeTypeIndexes = false;
}

// Build the C++ name excluding any inline namespaces
// ("std::__1::shared_ptr" -> "std::shared_ptr"
QString TypeEntry::shortName() const
{
    if (m_cachedShortName.isEmpty()) {
        QVarLengthArray<const TypeEntry *> parents;
        bool foundInlineNamespace = false;
        for (auto p = m_parent; p != nullptr && p->type() != TypeEntry::TypeSystemType; p = p->parent()) {
            if (p->type() == TypeEntry::NamespaceType
                && static_cast<const NamespaceTypeEntry *>(p)->isInlineNamespace()) {
                foundInlineNamespace = true;
            } else {
                parents.append(p);
            }
        }
        if (foundInlineNamespace) {
            m_cachedShortName.reserve(m_name.size());
            for (int i = parents.size() - 1; i >= 0; --i) {
                m_cachedShortName.append(parents.at(i)->entryName());
                m_cachedShortName.append(QLatin1String("::"));
            }
            m_cachedShortName.append(m_entryName);
        } else {
            m_cachedShortName = m_name;
        }
    }
    return m_cachedShortName;
}

void TypeEntry::setRevision(int r)
{
    if (m_revision != r) {
        m_revision = r;
        computeTypeIndexes = true;
    }
}

int TypeEntry::sbkIndex() const
{
    if (computeTypeIndexes)
        _computeTypeIndexes();
    return m_sbkIndex;
}

int getMaxTypeIndex()
{
    if (computeTypeIndexes)
        _computeTypeIndexes();
    return maxTypeIndex;
}

void TypeDatabase::clearApiVersions()
{
    apiVersions()->clear();
}

bool TypeDatabase::setApiVersion(const QString& packageWildcardPattern, const QString &version)
{
    const QString packagePattern = wildcardToRegExp(packageWildcardPattern.trimmed());
    const QVersionNumber versionNumber = QVersionNumber::fromString(version);
    if (versionNumber.isNull())
        return false;
    ApiVersions &versions = *apiVersions();
    for (int i = 0, size = versions.size(); i < size; ++i) {
        if (versions.at(i).first.pattern() == packagePattern) {
            versions[i].second = versionNumber;
            return true;
        }
    }
    const QRegularExpression packageRegex(packagePattern);
    if (!packageRegex.isValid())
        return false;
    versions.append(qMakePair(packageRegex, versionNumber));
    return true;
}

bool TypeDatabase::checkApiVersion(const QString &package,
                                   const VersionRange &vr)
{
    const ApiVersions &versions = *apiVersions();
    if (versions.isEmpty()) // Nothing specified: use latest.
        return true;
    for (int i = 0, size = versions.size(); i < size; ++i) {
        if (versions.at(i).first.match(package).hasMatch())
            return versions.at(i).second >= vr.since
                && versions.at(i).second <= vr.until;
    }
    return false;
}

#ifndef QT_NO_DEBUG_STREAM

#define FORMAT_BOOL(name, var) \
    if (var) \
         d << ", [" << name << ']';

#define FORMAT_NONEMPTY_STRING(name, var) \
    if (!var.isEmpty()) \
         d << ", " << name << "=\"" << var << '"';

#define FORMAT_LIST_SIZE(name, var) \
    if (!var.isEmpty()) \
         d << ", " << var.size() << ' '  << name;

template <class Container, class Separator>
static void formatList(QDebug &d, const char *name, const Container &c, Separator sep)
{
    if (const int size = c.size()) {
        d << ", " << name << '[' << size << "]=(";
        for (int i = 0; i < size; ++i) {
            if (i)
                d << sep;
             d << c.at(i);
        }
        d << ')';
    }
}

void TypeEntry::formatDebug(QDebug &d) const
{
    const QString cppName = qualifiedCppName();
    d << '"' << m_name << '"';
    if (m_name != cppName)
        d << "\", cppName=\"" << cppName << '"';
    d << ", type=" << m_type << ", codeGeneration=0x"
        << Qt::hex << m_codeGeneration << Qt::dec
        << ", target=\"" << targetLangName() << '"';
    FORMAT_NONEMPTY_STRING("package", m_targetLangPackage)
    FORMAT_BOOL("stream", m_stream)
    FORMAT_LIST_SIZE("codeSnips", m_codeSnips)
    FORMAT_NONEMPTY_STRING("conversionRule", m_conversionRule)
    if (!m_version.isNull() && m_version > QVersionNumber(0, 0))
        d << ", version=" << m_version;
    if (m_revision)
        d << ", revision=" << m_revision;
    if (m_sbkIndex)
        d << ", sbkIndex=" << m_sbkIndex;
    if (m_include.isValid())
        d << ", include=" << m_include;
    formatList(d, "extraIncludes", m_extraIncludes, ", ");
}

void ComplexTypeEntry::formatDebug(QDebug &d) const
{
    TypeEntry::formatDebug(d);
    FORMAT_BOOL("polymorphicBase", m_polymorphicBase)
    FORMAT_BOOL("genericClass", m_genericClass)
    FORMAT_BOOL("deleteInMainThread", m_deleteInMainThread)
    if (m_typeFlags != 0)
        d << ", typeFlags=" << m_typeFlags;
    d << ", copyableFlag=" << m_copyableFlag
        << ", except=" << int(m_exceptionHandling);
    FORMAT_NONEMPTY_STRING("defaultSuperclass", m_defaultSuperclass)
    FORMAT_NONEMPTY_STRING("polymorphicIdValue", m_polymorphicIdValue)
    FORMAT_NONEMPTY_STRING("targetType", m_targetType)
    FORMAT_NONEMPTY_STRING("hash", m_hashFunction)
    FORMAT_LIST_SIZE("addedFunctions", m_addedFunctions)
    formatList(d, "functionMods", m_functionMods, ", ");
    FORMAT_LIST_SIZE("fieldMods", m_fieldMods)
}

void TypedefEntry::formatDebug(QDebug &d) const
{
    ComplexTypeEntry::formatDebug(d);
    d << ", sourceType=\"" << m_sourceType << '"'
        << ", source=" << m_source << ", target=" << m_target;
}

void EnumTypeEntry::formatDebug(QDebug &d) const
{
    TypeEntry::formatDebug(d);
    if (m_flags)
        d << ", flags=(" << m_flags << ')';
}

void NamespaceTypeEntry::formatDebug(QDebug &d) const
{
    ComplexTypeEntry::formatDebug(d);
    auto pattern = m_filePattern.pattern();
    FORMAT_NONEMPTY_STRING("pattern", pattern)
    d << ",visibility=" << m_visibility;
    if (m_inlineNamespace)
        d << "[inline]";
}

void ContainerTypeEntry::formatDebug(QDebug &d) const
{
    ComplexTypeEntry::formatDebug(d);
    d << ", type=" << m_type << ",\"" << typeName() << '"';
}

void SmartPointerTypeEntry::formatDebug(QDebug &d) const
{
    ComplexTypeEntry::formatDebug(d);
    if (!m_instantiations.isEmpty()) {
        d << ", instantiations[" << m_instantiations.size() << "]=(";
        for (auto i : m_instantiations)
            d << i->name() << ',';
        d << ')';
    }
}

QDebug operator<<(QDebug d, const TypeEntry *te)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TypeEntry(";
    if (te)
        te->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const TemplateEntry *te)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TemplateEntry(";
    if (te) {
        d << '"' << te->name() << '"';
    } else {
        d << '0';
    }
    d << ')';
    return d;
}

void TypeDatabase::formatDebug(QDebug &d) const
{
    d << "TypeDatabase("
      << "entries[" << m_entries.size() << "]=";
    for (auto it = m_entries.cbegin(), end = m_entries.cend(); it != end; ++it)
        d << "  " << it.value() << '\n';
    if (!m_templates.isEmpty()) {
        d << "templates[" << m_templates.size() << "]=(";
        const auto begin = m_templates.cbegin();
        for (auto  it = begin, end = m_templates.cend(); it != end; ++it) {
            if (it != begin)
                d << ", ";
            d << it.value();
        }
        d << ")\n";
    }
    if (!m_flagsEntries.isEmpty()) {
        d << "flags[" << m_flagsEntries.size() << "]=(";
        const auto begin = m_flagsEntries.cbegin();
        for (auto it = begin, end = m_flagsEntries.cend(); it != end; ++it) {
            if (it != begin)
                d << ", ";
            d << it.value();
        }
        d << ")\n";
    }
    d <<"\nglobalUserFunctions=" << m_globalUserFunctions << '\n';
    formatList(d, "globalFunctionMods", m_functionMods, '\n');
    d << ')';
}

QDebug operator<<(QDebug d, const TypeDatabase &db)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    db.formatDebug(d);
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
