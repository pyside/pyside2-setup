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

QString TypeDatabase::normalizedSignature(const char* signature)
{
    QString normalized = QLatin1String(QMetaObject::normalizedSignature(signature));

    if (!instance() || !QByteArray(signature).contains("unsigned"))
        return normalized;

    QStringList types;
    types << QLatin1String("char") << QLatin1String("short")
        << QLatin1String("int") << QLatin1String("long");
    foreach (const QString& type, types) {
        if (instance()->findType(QLatin1Char('u') + type))
            continue;
        const QString pattern = QLatin1String("\\bu") + type + QLatin1String("\\b");
        normalized.replace(QRegExp(pattern), QLatin1String("unsigned ") + type);
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
    QList<TypeEntry *> entries = findTypes(name);
    foreach (TypeEntry *entry, entries) {
        if (entry &&
            (!entry->isPrimitive() || static_cast<PrimitiveTypeEntry *>(entry)->preferredTargetLangType())) {
            return entry;
        }
    }
    return 0;
}

QList<TypeEntry *> TypeDatabase::findTypes(const QString &name) const
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

QList<const PrimitiveTypeEntry*> TypeDatabase::primitiveTypes() const
{
    TypeEntryHash entries = allEntries();
    QList<const PrimitiveTypeEntry*> returned;
    for (TypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        foreach (TypeEntry *typeEntry, it.value()) {
            if (typeEntry->isPrimitive())
                returned.append(static_cast<PrimitiveTypeEntry *>(typeEntry));
        }
    }
    return returned;
}

QList<const ContainerTypeEntry*> TypeDatabase::containerTypes() const
{
    TypeEntryHash entries = allEntries();
    QList<const ContainerTypeEntry*> returned;
    for (TypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        foreach (TypeEntry *typeEntry, it.value()) {
            if (typeEntry->isContainer())
                returned.append(static_cast<ContainerTypeEntry *>(typeEntry));
        }
    }
    return returned;
}
void TypeDatabase::addRejection(const QString& className, const QString& functionName,
                                const QString& fieldName, const QString& enumName)
{
    TypeRejection r;
    r.class_name = className;
    r.function_name = functionName;
    r.field_name = fieldName;
    r.enum_name = enumName;

    m_rejections << r;
}

bool TypeDatabase::isClassRejected(const QString& className) const
{
    foreach (const TypeRejection& r, m_rejections) {
        if (r.class_name == className && r.function_name == QLatin1String("*")
            && r.field_name == QLatin1String("*") && r.enum_name == QLatin1String("*")) {
            return true;
        }
    }
    return false;
}

bool TypeDatabase::isEnumRejected(const QString& className, const QString& enumName) const
{
    foreach (const TypeRejection& r, m_rejections) {
        if (r.enum_name == enumName
            && (r.class_name == className || r.class_name == QLatin1String("*"))) {
            return true;
        }
    }

    return false;
}

void TypeDatabase::addType(TypeEntry *e)
{
    m_entries[e->qualifiedCppName()].append(e);
}

bool TypeDatabase::isFunctionRejected(const QString& className, const QString& functionName) const
{
    foreach (const TypeRejection& r, m_rejections)
    if (r.function_name == functionName &&
        (r.class_name == className || r.class_name == QLatin1String("*")))
        return true;
    return false;
}


bool TypeDatabase::isFieldRejected(const QString& className, const QString& fieldName) const
{
    foreach (const TypeRejection& r, m_rejections)
    if (r.field_name == fieldName &&
        (r.class_name == className || r.class_name == QLatin1String("*")))
        return true;
    return false;
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
    foreach (const AddedFunction &func, m_globalUserFunctions) {
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

void TypeDatabase::addSuppressedWarning(const QString &s)
{
    m_suppressedWarnings.append(s);
}

bool TypeDatabase::isSuppressedWarning(const QString& s) const
{
    if (!m_suppressWarnings)
        return false;

    foreach (const QString &_warning, m_suppressedWarnings) {
        QString warning = _warning;
        warning.replace(QLatin1String("\\*"), QLatin1String("&place_holder_for_asterisk;"));

        QStringList segs = warning.split(QLatin1Char('*'), QString::SkipEmptyParts);
        if (!segs.size())
            continue;

        int i = 0;
        int pos = s.indexOf(QString(segs.at(i++)).replace(QLatin1String("&place_holder_for_asterisk;"), QLatin1String("*")));
        //qDebug() << "s == " << s << ", warning == " << segs;
        while (pos != -1) {
            if (i == segs.size())
                return true;
            pos = s.indexOf(QString(segs.at(i++)).replace(QLatin1String("&place_holder_for_asterisk;"), QLatin1String("*")), pos);
        }
    }

    return false;
}

QString TypeDatabase::modifiedTypesystemFilepath(const QString& tsFile) const
{
    if (!QFile::exists(tsFile)) {
        int idx = tsFile.lastIndexOf(QLatin1Char('/'));
        QString fileName = idx >= 0 ? tsFile.right(tsFile.length() - idx - 1) : tsFile;
        foreach (const QString &path, m_typesystemPaths) {
            QString filepath(path + QLatin1Char('/') + fileName);
            if (QFile::exists(filepath))
                return filepath;
        }
    }
    return tsFile;
}

bool TypeDatabase::parseFile(const QString &filename, bool generate)
{
    QString filepath = modifiedTypesystemFilepath(filename);
    if (m_parsedTypesystemFiles.contains(filepath))
        return m_parsedTypesystemFiles[filepath];

    QFile file(filepath);
    if (!file.exists()) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find " << filename << ", typesystem paths: " << m_typesystemPaths.join(QLatin1String(", "));
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
    QList<TypeEntry*> entries = findTypes(name);

    foreach (TypeEntry* entry, entries) {
        if (entry && entry->isPrimitive() && static_cast<PrimitiveTypeEntry*>(entry)->preferredTargetLangType())
            return static_cast<PrimitiveTypeEntry*>(entry);
    }

    return 0;
}

ComplexTypeEntry* TypeDatabase::findComplexType(const QString& name) const
{
    QList<TypeEntry*> entries = findTypes(name);
    foreach (TypeEntry* entry, entries) {
        if (entry && entry->isComplex())
            return static_cast<ComplexTypeEntry*>(entry);
    }
    return 0;
}

ObjectTypeEntry* TypeDatabase::findObjectType(const QString& name) const
{
    QList<TypeEntry*> entries = findTypes(name);
    foreach (TypeEntry* entry, entries) {
        if (entry && entry->isObject())
            return static_cast<ObjectTypeEntry*>(entry);
    }
    return 0;
}

NamespaceTypeEntry* TypeDatabase::findNamespaceType(const QString& name) const
{
    QList<TypeEntry*> entries = findTypes(name);
    foreach (TypeEntry* entry, entries) {
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
    typedef QMap<int, QList<TypeEntry*> > GroupedTypeEntries;
    GroupedTypeEntries groupedEntries;

    // Group type entries by revision numbers
    TypeEntryHash allEntries = tdb->allEntries();
    foreach (QList<TypeEntry*> entryList, allEntries) {
        foreach (TypeEntry* entry, entryList) {
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
        QList<TypeEntry*>::iterator newEnd = std::unique(it.value().begin(), it.value().end());
        it.value().erase(newEnd, it.value().end());
        // Sort the type entries by name
        qSort(it.value().begin(), newEnd, compareTypeEntriesByName);

        foreach (TypeEntry* entry, it.value()) {
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
        d << '"' << te->qualifiedCppName() << "\", type=" << te->type();
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
