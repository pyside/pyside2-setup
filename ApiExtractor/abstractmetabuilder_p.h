/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef ABSTRACTMETABUILDER_P_H
#define ABSTRACTMETABUILDER_P_H

#include "abstractmetabuilder.h"
#include "parser/codemodel_fwd.h"
#include "abstractmetalang.h"
#include "typesystem.h"
#include "typeparser.h"

#include <QSet>
#include <QFileInfo>

class TypeDatabase;

class AbstractMetaBuilderPrivate
{
public:
    AbstractMetaBuilderPrivate();
    ~AbstractMetaBuilderPrivate();

    static FileModelItem buildDom(const QByteArrayList &arguments, unsigned clangFlags);
    void traverseDom(const FileModelItem &dom);

    void dumpLog() const;
    AbstractMetaClassList classesTopologicalSorted(const AbstractMetaClass *cppClass = Q_NULLPTR,
                                                   const Dependencies &additionalDependencies = Dependencies()) const;
    ScopeModelItem popScope() { return m_scopes.takeLast(); }

    void pushScope(ScopeModelItem item) { m_scopes << item; }

    ScopeModelItem currentScope() const { return m_scopes.last(); }

    AbstractMetaClass *argumentToClass(ArgumentModelItem);

    void figureOutEnumValuesForClass(AbstractMetaClass *metaClass, QSet<AbstractMetaClass *> *classes);
    int figureOutEnumValue(const QString &name, int value, AbstractMetaEnum *meta_enum, AbstractMetaFunction *metaFunction = 0);
    void figureOutEnumValues();
    void figureOutDefaultEnumArguments();

    void addAbstractMetaClass(AbstractMetaClass *cls);
    AbstractMetaClass *traverseTypeDef(const FileModelItem &dom,
                                       const TypeDefModelItem &typeDef);
    AbstractMetaClass *traverseClass(const FileModelItem &dom,
                                     const ClassModelItem &item);
    AbstractMetaClass *currentTraversedClass(ScopeModelItem item);
    void traverseScopeMembers(ScopeModelItem item, AbstractMetaClass *metaClass);
    void traverseClassMembers(ClassModelItem scopeItem);
    void traverseNamespaceMembers(NamespaceModelItem scopeItem);
    bool setupInheritance(AbstractMetaClass *metaClass);
    AbstractMetaClass *traverseNamespace(const FileModelItem &dom,
                                         const NamespaceModelItem &item);
    AbstractMetaEnum *traverseEnum(EnumModelItem item, AbstractMetaClass *enclosing,
                                   const QSet<QString> &enumsDeclarations);
    void traverseEnums(ScopeModelItem item, AbstractMetaClass *parent,
                       const QStringList &enumsDeclarations);
    AbstractMetaFunctionList classFunctionList(const ScopeModelItem &scopeItem);
    AbstractMetaFunctionList templateClassFunctionList(const ScopeModelItem &scopeItem,
                                                       AbstractMetaClass *metaClass);
    void traverseFunctions(ScopeModelItem item, AbstractMetaClass *parent);
    void applyFunctionModifications(AbstractMetaFunction* func);
    void traverseFields(ScopeModelItem item, AbstractMetaClass *parent);
    void traverseStreamOperator(FunctionModelItem functionItem);
    void traverseOperatorFunction(FunctionModelItem item);
    AbstractMetaFunction* traverseFunction(const AddedFunction &addedFunc);
    AbstractMetaFunction* traverseFunction(const AddedFunction &addedFunc,
                                           AbstractMetaClass *metaClass);
    AbstractMetaFunction *traverseFunction(FunctionModelItem function);
    AbstractMetaField *traverseField(VariableModelItem field,
                                     const AbstractMetaClass *cls);
    void checkFunctionModifications();
    void registerHashFunction(FunctionModelItem functionItem);
    void registerToStringCapability(FunctionModelItem functionItem);

    /**
     *   A conversion operator function should not have its owner class as
     *   its return type, but unfortunately it does. This function fixes the
     *   return type of operator functions of this kind making the return type
     *   be the same as it is supposed to generate when used in C++.
     *   If the returned type is a wrapped C++ class, this method also adds the
     *   conversion operator to the collection of external conversions of the
     *   said class.
     *   \param metaFunction conversion operator function to be fixed.
     */
    void fixReturnTypeOfConversionOperator(AbstractMetaFunction *metaFunction);

    void parseQ_Property(AbstractMetaClass *metaClass, const QStringList &declarations);
    void setupEquals(AbstractMetaClass *metaClass);
    void setupComparable(AbstractMetaClass *metaClass);
    void setupClonable(AbstractMetaClass *cls);
    void setupExternalConversion(AbstractMetaClass *cls);
    void setupFunctionDefaults(AbstractMetaFunction *metaFunction,
                               AbstractMetaClass *metaClass);

    QString fixDefaultValue(ArgumentModelItem item, AbstractMetaType *type,
                            AbstractMetaFunction *fnc, AbstractMetaClass *,
                            int argumentIndex);
    AbstractMetaType* translateType(double vr, const AddedFunction::TypeInfo &typeInfo);
    AbstractMetaType *translateType(const TypeInfo &type, bool *ok,
                                    bool resolveType = true,
                                    bool resolveScope = true);

    int findOutValueFromString(const QString &stringValue, bool &ok);

    void decideUsagePattern(AbstractMetaType *type);

    AbstractMetaClass *findTemplateClass(const QString& name, const AbstractMetaClass *context,
                                         TypeParser::Info *info = Q_NULLPTR,
                                         ComplexTypeEntry **baseContainerType = Q_NULLPTR) const;
    AbstractMetaClassList getBaseClasses(const AbstractMetaClass *metaClass) const;
    bool ancestorHasPrivateCopyConstructor(const AbstractMetaClass *metaClass) const;

    bool inheritTemplate(AbstractMetaClass *subclass,
                         const AbstractMetaClass *templateClass,
                         const TypeParser::Info &info);
    AbstractMetaType *inheritTemplateType(const QList<AbstractMetaType *> &templateTypes,
                                          const AbstractMetaType *metaType,
                                          bool *ok = Q_NULLPTR);

    bool isQObject(const FileModelItem &dom, const QString &qualifiedName);
    bool isEnum(const FileModelItem &dom, const QStringList &qualifiedName);

    void fixQObjectForScope(const FileModelItem &dom, const TypeDatabase *types,
                            const NamespaceModelItem &item);

    void sortLists();
    AbstractMetaArgumentList reverseList(const AbstractMetaArgumentList &list);
    void setInclude(TypeEntry *te, const QString &fileName) const;
    void fixArgumentNames(AbstractMetaFunction *func);
    void fillAddedFunctions(AbstractMetaClass *metaClass);

    AbstractMetaBuilder *q;
    AbstractMetaClassList m_metaClasses;
    AbstractMetaClassList m_templates;
    AbstractMetaClassList m_smartPointers;
    AbstractMetaFunctionList m_globalFunctions;
    AbstractMetaEnumList m_globalEnums;

    QSet<const TypeEntry *> m_usedTypes;

    typedef QMap<QString, AbstractMetaBuilder::RejectReason> RejectMap;

    RejectMap m_rejectedClasses;
    RejectMap m_rejectedEnums;
    RejectMap m_rejectedFunctions;
    RejectMap m_rejectedFields;

    QList<AbstractMetaEnum *> m_enums;

    QList<QPair<AbstractMetaArgument *, AbstractMetaFunction *> > m_enumDefaultArguments;

    QHash<QString, AbstractMetaEnumValue *> m_enumValues;

    AbstractMetaClass *m_currentClass;
    QList<ScopeModelItem> m_scopes;
    QString m_namespacePrefix;

    QSet<AbstractMetaClass *> m_setupInheritanceDone;

    // QtScript
    QSet<QString> m_qmetatypeDeclaredTypenames;

    QString m_logDirectory;
    QFileInfo m_globalHeader;
};

#endif // ABSTRACTMETBUILDER_P_H
