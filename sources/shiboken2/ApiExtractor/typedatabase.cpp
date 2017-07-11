/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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
#include "typesystem_p.h"

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

typedef QPair<QRegularExpression, QVersionNumber> ApiVersion;
typedef QVector<ApiVersion> ApiVersions;

Q_GLOBAL_STATIC(ApiVersions, apiVersions)

TypeDatabase::TypeDatabase() : m_suppressWarnings(true)
{
    addType(new VoidTypeEntry());
    addType(new VarargsTypeEntry());
}

TypeDatabase::~TypeDatabase()
{
}

TypeDatabase* TypeDatabase::instance(bool newInstance)
{
    static TypeDatabase* db = 0;
    if (!db || newInstance) {
        if (db)
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

typedef QVector<IntTypeNormalizationEntry> IntTypeNormalizationEntries;

static const IntTypeNormalizationEntries &intTypeNormalizationEntries()
{
    static IntTypeNormalizationEntries result;
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        static const char *intTypes[] = {"char", "short", "int", "long"};
        const size_t size = sizeof(intTypes) / sizeof(intTypes[0]);
        for (size_t i = 0; i < size; ++i) {
            const QString intType = QLatin1String(intTypes[i]);
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
        for (int i = 0, size = entries.size(); i < size; ++i)
            normalized.replace(entries.at(i).regex, entries.at(i).replacement);
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
    if (typeEntry)
        return typeEntry->extraIncludes();
    else
        return IncludeList();
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
    return 0;
}

FunctionTypeEntry* TypeDatabase::findFunctionType(const QString& name) const
{
    TypeEntry* entry = findType(name);
    if (entry && entry->type() == TypeEntry::FunctionType)
        return static_cast<FunctionTypeEntry*>(entry);
    return 0;
}

TypeEntry* TypeDatabase::findType(const QString& name) const
{
    const TypeEntryList &entries = findTypes(name);
    for (TypeEntry *entry : entries) {
        if (entry &&
            (!entry->isPrimitive() || static_cast<PrimitiveTypeEntry *>(entry)->preferredTargetLangType())) {
            return entry;
        }
    }
    return 0;
}

TypeEntryList TypeDatabase::findTypes(const QString &name) const
{
    return m_entries.value(name);
}

SingleTypeEntryHash TypeDatabase::entries() const
{
    TypeEntryHash entries = allEntries();

    SingleTypeEntryHash returned;
    for (TypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it)
        returned.insert(it.key(), findType(it.key()));

    return returned;
}

PrimitiveTypeEntryList TypeDatabase::primitiveTypes() const
{
    TypeEntryHash entries = allEntries();
    PrimitiveTypeEntryList returned;
    for (TypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        for (TypeEntry *typeEntry : it.value()) {
            if (typeEntry->isPrimitive())
                returned.append(static_cast<PrimitiveTypeEntry *>(typeEntry));
        }
    }
    return returned;
}

ContainerTypeEntryList TypeDatabase::containerTypes() const
{
    TypeEntryHash entries = allEntries();
    ContainerTypeEntryList returned;
    for (TypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        for (TypeEntry *typeEntry : it.value()) {
            if (typeEntry->isContainer())
                returned.append(static_cast<ContainerTypeEntry *>(typeEntry));
        }
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

void TypeDatabase::addType(TypeEntry *e)
{
    m_entries[e->qualifiedCppName()].append(e);
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
            for (SingleTypeEntryHash::const_iterator it = m_flagsEntries.cbegin(), end = m_flagsEntries.cend(); it != end; ++it) {
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
    for (const AddedFunction &func : m_globalUserFunctions) {
        if (func.name() == name)
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
        if (mod.signature == signature)
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

    const QRegularExpression expression(pattern);
    if (!expression.isValid()) {
        *errorMessage = QLatin1String("Invalid message pattern \"") + warning
            + QLatin1String("\": ") + expression.errorString();
        return false;
    }

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

QString TypeDatabase::modifiedTypesystemFilepath(const QString& tsFile) const
{
    const QFileInfo tsFi(tsFile);
    if (tsFi.isAbsolute()) // No point in further lookups
        return tsFi.absoluteFilePath();
    if (tsFi.isFile()) // Make path absolute
        return tsFi.absoluteFilePath();
    const QString fileName = tsFi.fileName();
    for (const QString &path : m_typesystemPaths) {
        const QFileInfo fi(path + QLatin1Char('/') + fileName);
        if (fi.isFile())
            return fi.absoluteFilePath();
    }
    return tsFile;
}

bool TypeDatabase::parseFile(const QString &filename, bool generate)
{
    QString filepath = modifiedTypesystemFilepath(filename);
    if (m_parsedTypesystemFiles.contains(filepath))
        return m_parsedTypesystemFiles[filepath];

    m_parsedTypesystemFiles[filepath] = true; // Prevent recursion when including self.

    QFile file(filepath);
    if (!file.exists()) {
        m_parsedTypesystemFiles[filepath] = false;
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find " << filename << ", typesystem paths: " << m_typesystemPaths.join(QLatin1String(", "));
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
    Handler handler(this, generate);
    return handler.parse(reader);
}

PrimitiveTypeEntry *TypeDatabase::findPrimitiveType(const QString& name) const
{
    const TypeEntryList &entries = findTypes(name);

    for (TypeEntry *entry : entries) {
        if (entry && entry->isPrimitive() && static_cast<PrimitiveTypeEntry*>(entry)->preferredTargetLangType())
            return static_cast<PrimitiveTypeEntry*>(entry);
    }

    return 0;
}

ComplexTypeEntry* TypeDatabase::findComplexType(const QString& name) const
{
    const TypeEntryList &entries = findTypes(name);
    for (TypeEntry *entry : entries) {
        if (entry && entry->isComplex())
            return static_cast<ComplexTypeEntry*>(entry);
    }
    return 0;
}

ObjectTypeEntry* TypeDatabase::findObjectType(const QString& name) const
{
    const TypeEntryList &entries = findTypes(name);
    for (TypeEntry *entry : entries) {
        if (entry && entry->isObject())
            return static_cast<ObjectTypeEntry*>(entry);
    }
    return 0;
}

NamespaceTypeEntry* TypeDatabase::findNamespaceType(const QString& name) const
{
    const TypeEntryList &entries = findTypes(name);
    for (TypeEntry *entry : entries) {
        if (entry && entry->isNamespace())
            return static_cast<NamespaceTypeEntry*>(entry);
    }
    return 0;
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

// Using std::pair to save some memory
// the pair means (revision, typeIndex)
// This global variable exists only because we can't break the ABI
typedef QHash<const TypeEntry*, std::pair<int, int> > TypeRevisionMap;
Q_GLOBAL_STATIC(TypeRevisionMap, typeEntryFields);
static bool computeTypeIndexes = true;
static int maxTypeIndex;

int getTypeRevision(const TypeEntry* typeEntry)
{
    return typeEntryFields()->value(typeEntry).first;
}

void setTypeRevision(TypeEntry* typeEntry, int revision)
{
    (*typeEntryFields())[typeEntry].first = revision;
    computeTypeIndexes = true;
}

static bool compareTypeEntriesByName(const TypeEntry* t1, const TypeEntry* t2)
{
    return t1->qualifiedCppName() < t2->qualifiedCppName();
}

static void _computeTypeIndexes()
{
    TypeDatabase* tdb = TypeDatabase::instance();
    typedef QMap<int, TypeEntryList> GroupedTypeEntries;
    GroupedTypeEntries groupedEntries;

    // Group type entries by revision numbers
    const TypeEntryHash &allEntries = tdb->allEntries();
    for (TypeEntryHash::const_iterator tit = allEntries.cbegin(), end = allEntries.cend(); tit != end; ++tit) {
        for (TypeEntry *entry : tit.value()) {
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
            groupedEntries[getTypeRevision(entry)] << entry;
        }
    }

    maxTypeIndex = 0;
    GroupedTypeEntries::iterator it = groupedEntries.begin();
    for (; it != groupedEntries.end(); ++it) {
        // Remove duplicates
        TypeEntryList::iterator newEnd = std::unique(it.value().begin(), it.value().end());
        it.value().erase(newEnd, it.value().end());
        // Sort the type entries by name
        qSort(it.value().begin(), newEnd, compareTypeEntriesByName);

        for (TypeEntry *entry : qAsConst(it.value())) {
            (*typeEntryFields())[entry].second = maxTypeIndex++;
        }
    }
    computeTypeIndexes = false;
}

int getTypeIndex(const TypeEntry* typeEntry)
{
    if (computeTypeIndexes)
        _computeTypeIndexes();
    return typeEntryFields()->value(typeEntry).second;
}

int getMaxTypeIndex()
{
    if (computeTypeIndexes)
        _computeTypeIndexes();
    return maxTypeIndex;
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

bool TypeDatabase::checkApiVersion(const QString& package, const QString& version) const
{
    const QVersionNumber versionNumber = QVersionNumber::fromString(version);
    if (versionNumber.isNull()) {
        qCWarning(lcShiboken).noquote().nospace()
            << "checkApiVersion: Invalid version \"" << version << "\" specified for package "
            << package << '.';
        return false;
    }
    const ApiVersions &versions = *apiVersions();
    for (int i = 0, size = versions.size(); i < size; ++i) {
        if (versions.at(i).first.match(package).hasMatch())
            return versions.at(i).second >= versionNumber;
    }
    return false;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeEntry *te)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TypeEntry(";
    if (te) {
        const QString name = te->name();
        const QString cppName = te->qualifiedCppName();
        d << '"' << name << '"';
        if (name != cppName)
            d << "\", cppName=\"" << cppName << '"';
        d << ", type=" << te->type();
        if (te->include().isValid())
            d << ", include=" << te->include();
        const IncludeList &extraIncludes = te->extraIncludes();
        if (const int count = extraIncludes.size()) {
            d << ", extraIncludes[" << count << "]=";
            for (int i = 0; i < count; ++i) {
                if (i)
                    d << ", ";
                d << extraIncludes.at(i);
            }
        }
    } else {
        d << '0';
    }
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
        d << '"' << te->name() << "\", version=" << te->version();
    } else {
        d << '0';
    }
    d << ')';
    return d;
}

void TypeDatabase::formatDebug(QDebug &d) const
{
    typedef TypeEntryHash::ConstIterator Eit;
    typedef SingleTypeEntryHash::ConstIterator Sit;
    typedef TemplateEntryHash::ConstIterator TplIt;
    d << "TypeDatabase("
      << "entries[" << m_entries.size() << "]=";
    for (Eit it = m_entries.cbegin(), end = m_entries.cend(); it != end; ++it) {
        const int count = it.value().size();
        d << '"' << it.key() << "\" [" << count << "]: (";
        for (int t = 0; t < count; ++t) {
            if (t)
                d << ", ";
            d << it.value().at(t);
        }
        d << ")\n";
    }
    if (!m_templates.isEmpty()) {
        d << "templates[" << m_templates.size() << "]=(";
        const TplIt begin = m_templates.cbegin();
        for (TplIt it = begin, end = m_templates.cend(); it != end; ++it) {
            if (it != begin)
                d << ", ";
            d << it.value();
        }
        d << ")\n";
    }
    if (!m_flagsEntries.isEmpty()) {
        d << "flags[" << m_flagsEntries.size() << "]=(";
        const Sit begin = m_flagsEntries.cbegin();
        for (Sit it = begin, end = m_flagsEntries.cend(); it != end; ++it) {
            if (it != begin)
                d << ", ";
            d << it.value();
        }
        d << ")\n";
    }
    d <<"\nglobalUserFunctions=" << m_globalUserFunctions << ')';
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
