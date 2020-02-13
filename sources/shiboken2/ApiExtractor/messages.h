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

#ifndef MESSAGES_H
#define MESSAGES_H

#include "abstractmetalang_typedefs.h"
#include "parser/codemodel_fwd.h"
#include "typesystem_typedefs.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVector>

class TypeEntry;
class TypeInfo;
struct TypeRejection;

QT_FORWARD_DECLARE_CLASS(QDir)
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)

QString msgNoFunctionForModification(const QString &signature,
                                     const QString &originalSignature,
                                     const QString &className,
                                     const QStringList &possibleSignatures,
                                     const AbstractMetaFunctionList &allFunctions);

QString msgNoEnumTypeEntry(const EnumModelItem &enumItem,
                           const QString &className);


QString msgNoEnumTypeConflict(const EnumModelItem &enumItem,
                              const QString &className,
                              const TypeEntry *t);

QString msgAmbiguousVaryingTypesFound(const QString &qualifiedName, const TypeEntries &te);
QString msgAmbiguousTypesFound(const QString &qualifiedName, const TypeEntries &te);

QString msgUnmatchedParameterType(const ArgumentModelItem &arg, int n,
                                  const QString &why);

QString msgUnmatchedReturnType(const FunctionModelItem &functionItem,
                               const QString &why);

QString msgSkippingFunction(const FunctionModelItem &functionItem,
                            const QString &signature, const QString &why);

QString msgCannotResolveEntity(const QString &name, const QString &reason);

QString msgCannotSetArrayUsage(const QString &function, int i, const QString &reason);

QString msgUnableToTranslateType(const QString &t, const QString &why);

QString msgUnableToTranslateType(const TypeInfo &typeInfo,
                                 const QString &why);

QString msgCannotFindTypeEntry(const QString &t);

QString msgCannotFindTypeEntryForSmartPointer(const QString &t, const QString &smartPointerType);
QString msgInvalidSmartPointerType(const TypeInfo &i);
QString msgCannotFindSmartPointerInstantion(const TypeInfo &i);

QString msgCannotTranslateTemplateArgument(int i,
                                           const TypeInfo &typeInfo,
                                           const QString &why);

QString msgDisallowThread(const AbstractMetaFunction *f);

QString msgNamespaceToBeExtendedNotFound(const QString &namespaceName, const QString &packageName);

QString msgCannotFindDocumentation(const QString &fileName,
                                   const char *what, const QString &name,
                                   const QString &query);

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaFunction *function,
                                   const QString &query);

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaEnum *e,
                                   const QString &query);

QString msgCannotFindDocumentation(const QString &fileName,
                                   const AbstractMetaClass *metaClass,
                                   const AbstractMetaField *f,
                                   const QString &query);

QString msgXpathDocModificationError(const DocModificationList& mods,
                                     const QString &what);

QString msgCannotOpenForReading(const QFile &f);

QString msgCannotOpenForWriting(const QFile &f);

QString msgCannotUseEnumAsInt(const QString &name);

QString msgConversionTypesDiffer(const QString &varType, const QString &conversionType);

QString msgCannotFindSmartPointer(const QString &instantiationType,
                                  const AbstractMetaClassList &pointers);

QString msgLeftOverArguments(const QMap<QString, QString> &remainingArgs);

QString msgInvalidVersion(const QString &package, const QString &version);

QString msgCannotFindNamespaceToExtend(const QString &name,
                                       const QStringRef &extendsPackage);

QString msgExtendingNamespaceRequiresPattern(const QString &name);

QString msgInvalidRegularExpression(const QString &pattern, const QString &why);

QString msgNoRootTypeSystemEntry();

QString msgIncorrectlyNestedName(const QString &name);

QString msgCyclicDependency(const QString &funcName, const QString &graphName,
                            const QVector<const AbstractMetaFunction *> &involvedConversions);

QString msgUnknownOperator(const AbstractMetaFunction* func);

QString msgWrongIndex(const char *varName, const QString &capture,
                      const AbstractMetaFunction *func);

QString msgCannotFindType(const QString &type, const QString &variable,
                          const QString &why);

QString msgCannotBuildMetaType(const QString &s);

QString msgCouldNotFindMinimalConstructor(const QString &where, const QString &type);

QString msgRejectReason(const TypeRejection &r, const QString &needle = QString());

QString msgTagWarning(const QXmlStreamReader &reader, const QString &context,
                      const QString &tag, const QString &message);

QString msgFallbackWarning(const QXmlStreamReader &reader, const QString &context,
                           const QString &tag, const QString &location,
                           const QString &identifier, const QString &fallback);

#endif // MESSAGES_H
