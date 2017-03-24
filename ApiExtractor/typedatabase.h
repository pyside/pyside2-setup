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

#ifndef TYPEDATABASE_H
#define TYPEDATABASE_H

#include "apiextractormacros.h"
#include "include.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QIODevice)

class ComplexTypeEntry;
class ContainerTypeEntry;
class FlagsTypeEntry;
class FunctionTypeEntry;
class NamespaceTypeEntry;
class ObjectTypeEntry;
class TemplateEntry;
class TypeEntry;

struct TypeRejection;

QT_FORWARD_DECLARE_CLASS(QDebug)

void setTypeRevision(TypeEntry* typeEntry, int revision);
int getTypeRevision(const TypeEntry* typeEntry);
int getTypeIndex(const TypeEntry* typeEntry);
int getMaxTypeIndex();

class ContainerTypeEntry;
class PrimitiveTypeEntry;
class TypeDatabase
{
    TypeDatabase();
    Q_DISABLE_COPY(TypeDatabase)
public:
    ~TypeDatabase();

    /**
    * Return the type system instance.
    * \param newInstance This parameter is useful just for unit testing, because singletons causes
    *                    too many side effects on unit testing.
    */
    static TypeDatabase* instance(bool newInstance = false);

    static QString normalizedSignature(const QString &signature);

    QStringList requiredTargetImports() const;

    void addRequiredTargetImport(const QString& moduleName);

    void addTypesystemPath(const QString& typesystem_paths);

    IncludeList extraIncludes(const QString& className) const;

    PrimitiveTypeEntry* findPrimitiveType(const QString& name) const;
    ComplexTypeEntry* findComplexType(const QString& name) const;
    ObjectTypeEntry* findObjectType(const QString& name) const;
    NamespaceTypeEntry* findNamespaceType(const QString& name) const;
    ContainerTypeEntry* findContainerType(const QString& name) const;
    FunctionTypeEntry* findFunctionType(const QString& name) const;

    TypeEntry* findType(const QString& name) const;

    TypeEntryHash allEntries() const { return m_entries; }

    SingleTypeEntryHash entries() const;

    QList<const PrimitiveTypeEntry*> primitiveTypes() const;

    QList<const ContainerTypeEntry*> containerTypes() const;

    void addRejection(const QString& className, const QString& functionName,
                        const QString& fieldName, const QString& enumName);
    bool isClassRejected(const QString& className) const;
    bool isFunctionRejected(const QString& className, const QString& functionName) const;
    bool isFieldRejected(const QString& className, const QString& fieldName) const;
    bool isEnumRejected(const QString& className, const QString& enumName) const;

    void addType(TypeEntry* e);

    FlagsTypeEntry* findFlagsType(const QString& name) const;
    void addFlagsType(FlagsTypeEntry* fte);

    TemplateEntry *findTemplate(const QString& name) const { return m_templates[name]; }

    void addTemplate(TemplateEntry* t);

    AddedFunctionList globalUserFunctions() const { return m_globalUserFunctions; }

    void addGlobalUserFunctions(const AddedFunctionList &functions);

    AddedFunctionList findGlobalUserFunctions(const QString& name) const;

    void addGlobalUserFunctionModifications(const FunctionModificationList &functionModifications);

    FunctionModificationList functionModifications(const QString& signature) const;

    void setSuppressWarnings(bool on) { m_suppressWarnings = on; }

    void addSuppressedWarning(const QString &s);

    bool isSuppressedWarning(const QString& s) const;

    static QString globalNamespaceClassName(const TypeEntry *te);

    bool parseFile(const QString &filename, bool generate = true);
    bool parseFile(QIODevice* device, bool generate = true);

    bool setApiVersion(const QString& package, const QString& version);

    bool checkApiVersion(const QString& package, const QString &version) const;

    bool hasDroppedTypeEntries() const { return !m_dropTypeEntries.isEmpty(); }

    bool shouldDropTypeEntry(const QString& fullTypeName) const;

    void setDropTypeEntries(QStringList dropTypeEntries);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif
private:
    QList<TypeEntry *> findTypes(const QString &name) const;
    QString modifiedTypesystemFilepath(const QString &tsFile) const;

    bool m_suppressWarnings;
    TypeEntryHash m_entries;
    SingleTypeEntryHash m_flagsEntries;
    TemplateEntryHash m_templates;
    QStringList m_suppressedWarnings;

    AddedFunctionList m_globalUserFunctions;
    FunctionModificationList m_functionMods;

    QStringList m_requiredTargetImports;

    QStringList m_typesystemPaths;
    QHash<QString, bool> m_parsedTypesystemFiles;

    QList<TypeRejection> m_rejections;

    QStringList m_dropTypeEntries;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeEntry *te);
QDebug operator<<(QDebug d, const TypeDatabase &db);
#endif
#endif // TYPEDATABASE_H
