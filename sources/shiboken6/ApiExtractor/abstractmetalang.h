/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef ABSTRACTMETALANG_H
#define ABSTRACTMETALANG_H

#include "abstractmetalang_enums.h"
#include "abstractmetalang_typedefs.h"
#include "abstractmetaattributes.h"
#include "enclosingclassmixin.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include <QtCore/qobjectdefs.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMetaClassPrivate;
class ComplexTypeEntry;
class Documentation;
class EnumTypeEntry;
class QPropertySpec;
class SourceLocation;

class AbstractMetaClass : public AbstractMetaAttributes, public EnclosingClassMixin
{
    Q_GADGET
public:
    Q_DISABLE_COPY_MOVE(AbstractMetaClass)

    AbstractMetaClass();
    ~AbstractMetaClass();

    const AbstractMetaFunctionCList &functions() const;
    void setFunctions(const AbstractMetaFunctionCList &functions);
    void addFunction(const AbstractMetaFunctionCPtr &function);
    bool hasFunction(const QString &str) const;
    AbstractMetaFunctionCPtr findFunction(const QString& functionName) const;
    bool hasSignal(const AbstractMetaFunction *f) const;

    bool hasConstructors() const;
    AbstractMetaFunctionCPtr copyConstructor() const;
    bool hasCopyConstructor() const;
    bool hasPrivateCopyConstructor() const;

    void addDefaultConstructor();
    void addDefaultCopyConstructor(bool isPrivate = false);

    bool hasNonPrivateConstructor() const;
    void setHasNonPrivateConstructor(bool value);

    bool hasPrivateConstructor() const;
    void setHasPrivateConstructor(bool value);

    bool hasPrivateDestructor() const;
    void setHasPrivateDestructor(bool value);

    bool hasProtectedDestructor() const;
    void setHasProtectedDestructor(bool value);

    bool hasVirtualDestructor() const;
    void setHasVirtualDestructor(bool value);

    bool isConstructible() const;

    bool generateExceptionHandling() const;

    AbstractMetaFunctionCList queryFunctionsByName(const QString &name) const;
    static bool queryFunction(const AbstractMetaFunction *f, FunctionQueryOptions query);
    static AbstractMetaFunctionCList queryFunctionList(const AbstractMetaFunctionCList &list,
                                                      FunctionQueryOptions query);
    static AbstractMetaFunctionCPtr queryFirstFunction(const AbstractMetaFunctionCList &list,
                                                       FunctionQueryOptions query);

    AbstractMetaFunctionCList queryFunctions(FunctionQueryOptions query) const;
    AbstractMetaFunctionCList functionsInTargetLang() const;
    AbstractMetaFunctionCList cppSignalFunctions() const;
    AbstractMetaFunctionCList implicitConversions() const;

    /**
     *   Retrieves all class' operator overloads that meet
     *   query criteria defined with the OperatorQueryOption
     *   enum.
     *   /param query composition of OperatorQueryOption enum values
     *   /return list of operator overload methods that meet the
     *   query criteria
     */
    AbstractMetaFunctionCList operatorOverloads(OperatorQueryOptions query) const;

    bool hasArithmeticOperatorOverload() const;
    bool hasBitwiseOperatorOverload() const;
    bool hasComparisonOperatorOverload() const;
    bool hasLogicalOperatorOverload() const;

    const AbstractMetaFieldList &fields() const;
    AbstractMetaFieldList &fields();
    void setFields(const AbstractMetaFieldList &fields);
    void addField(const AbstractMetaField &field);

    std::optional<AbstractMetaField> findField(const QString &name) const;

    const AbstractMetaEnumList &enums() const;
    AbstractMetaEnumList &enums();
    void setEnums(const AbstractMetaEnumList &enums);
    void addEnum(const AbstractMetaEnum &e);

    std::optional<AbstractMetaEnum> findEnum(const QString &enumName) const;
    std::optional<AbstractMetaEnumValue> findEnumValue(const QString &enumName) const;
    void getEnumsToBeGenerated(AbstractMetaEnumList *enumList) const;
    void getEnumsFromInvisibleNamespacesToBeGenerated(AbstractMetaEnumList *enumList) const;

    void getFunctionsFromInvisibleNamespacesToBeGenerated(AbstractMetaFunctionCList *funcList) const;

    QString fullName() const;

    /**
     *   Retrieves the class name without any namespace/scope information.
     *   /return the class name without scope information
     */
    QString name() const;

    const Documentation &documentation() const;
    void setDocumentation(const Documentation& doc);

    QString baseClassName() const;

    AbstractMetaClass *defaultSuperclass() const; // Attribute "default-superclass"
    void setDefaultSuperclass(AbstractMetaClass *s);

    AbstractMetaClass *baseClass() const;
    const AbstractMetaClassList &baseClasses() const;
    // base classes including defaultSuperclass
    AbstractMetaClassList typeSystemBaseClasses() const;
    // Recursive list of all base classes including defaultSuperclass
    AbstractMetaClassList allTypeSystemAncestors() const;

    void addBaseClass(AbstractMetaClass *base_class);
    void setBaseClass(AbstractMetaClass *base_class);

    /**
     *   \return the namespace from another package which this namespace extends.
     */
    const AbstractMetaClass *extendedNamespace() const;
    void setExtendedNamespace(const AbstractMetaClass *e);

    const AbstractMetaClassList &innerClasses() const;
    void addInnerClass(AbstractMetaClass* cl);
    void setInnerClasses(const AbstractMetaClassList &innerClasses);

    QString package() const;

    bool isNamespace() const;
    bool isInvisibleNamespace() const;

    bool isQObject() const;

    bool isQtNamespace() const;

    QString qualifiedCppName() const;

    bool hasSignals() const;
    bool inheritsFrom(const AbstractMetaClass *other) const;

    /**
    *   Says if the class that declares or inherits a virtual function.
    *   \return true if the class implements or inherits any virtual methods
    */
    bool isPolymorphic() const;

    /**
     * Tells if this class has one or more functions that are protected.
     * \return true if the class has protected functions.
     */
    bool hasProtectedFunctions() const;

    /**
     * Tells if this class has one or more fields (member variables) that are protected.
     * \return true if the class has protected fields.
     */
    bool hasProtectedFields() const;

    /**
     * Tells if this class has one or more members (functions or fields) that are protected.
     * \return true if the class has protected members.
     */
    bool hasProtectedMembers() const;


    const TypeEntries &templateArguments() const;
    void setTemplateArguments(const TypeEntries &);

    // only valid during metabuilder's run
    const QStringList &baseClassNames() const;
    void setBaseClassNames(const QStringList &names);

    const ComplexTypeEntry *typeEntry() const;
    ComplexTypeEntry *typeEntry();
    void setTypeEntry(ComplexTypeEntry *type);

    void setHasHashFunction(bool on);

    bool hasHashFunction() const;

    bool hasDefaultToStringFunction() const;

    bool hasEqualsOperator() const;
    void setHasEqualsOperator(bool on);

    bool hasCloneOperator() const;
    void setHasCloneOperator(bool on);

    const QList<QPropertySpec> &propertySpecs() const;
    void addPropertySpec(const QPropertySpec &spec);

    // Helpers to search whether a functions is a property setter/getter/reset
    enum class PropertyFunction
    {
        Read,
        Write,
        Reset
    };
    struct PropertyFunctionSearchResult
    {
        int index;
        PropertyFunction function;
    };

    PropertyFunctionSearchResult searchPropertyFunction(const QString &name) const;

    std::optional<QPropertySpec> propertySpecByName(const QString &name) const;

    /// Returns a list of conversion operators for this class. The conversion
    /// operators are defined in other classes of the same module.
    const AbstractMetaFunctionCList &externalConversionOperators() const;
    /// Adds a converter operator for this class.
    void addExternalConversionOperator(const AbstractMetaFunctionCPtr &conversionOp);
    /// Returns true if this class has any converter operators defined elsewhere.
    bool hasExternalConversionOperators() const;

    void sortFunctions();

    const AbstractMetaClass *templateBaseClass() const;
    void setTemplateBaseClass(const AbstractMetaClass *cls);

    bool hasTemplateBaseClassInstantiations() const;
    const AbstractMetaTypeList &templateBaseClassInstantiations() const;
    void setTemplateBaseClassInstantiations(const AbstractMetaTypeList& instantiations);

    void setTypeDef(bool typeDef);
    bool isTypeDef() const;

    bool isStream() const;
    void setStream(bool stream);

    bool hasToStringCapability() const;
    void setToStringCapability(bool value, uint indirections = 0);

    uint toStringCapabilityIndirections() const;

    bool deleteInMainThread() const;

    // Query functions for generators
    bool isObjectType() const;
    bool isCopyable() const;
    bool isValueTypeWithCopyConstructorOnly() const;

    static AbstractMetaClass *findClass(const AbstractMetaClassList &classes,
                                        const QString &name);
    static AbstractMetaClass *findClass(const AbstractMetaClassList &classes,
                                        const TypeEntry* typeEntry);
    static std::optional<AbstractMetaEnumValue> findEnumValue(const AbstractMetaClassList &classes,
                                                              const QString &string);
    static  std::optional<AbstractMetaEnum> findEnum(const AbstractMetaClassList &classes,
                                                     const EnumTypeEntry *entry);

    SourceLocation sourceLocation() const;
    void setSourceLocation(const SourceLocation &sourceLocation);

    // For AbstractMetaBuilder
    void fixFunctions();
    bool needsInheritanceSetup() const;
    void setInheritanceDone(bool b);
    bool inheritanceDone() const;

    template <class Function>
    void invisibleNamespaceRecursion(Function f) const;

private:
#ifndef QT_NO_DEBUG_STREAM
    void format(QDebug &d) const;
    void formatMembers(QDebug &d) const;
    friend QDebug operator<<(QDebug d, const AbstractMetaClass *ac);
#endif

    QScopedPointer<AbstractMetaClassPrivate> d;
};

template <class Function>
void AbstractMetaClass::invisibleNamespaceRecursion(Function f) const
{
    for (auto ic : innerClasses()) {
        if (ic->isInvisibleNamespace()) {
            f(ic);
            ic->invisibleNamespaceRecursion(f);
        }
    }
}

#endif // ABSTRACTMETALANG_H
