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

#include "abstractmetalang_typedefs.h"
#include "abstractmetaattributes.h"
#include "abstractmetafield.h"
#include "enclosingclassmixin.h"
#include "documentation.h"
#include "sourcelocation.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include <QtCore/qobjectdefs.h>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QDebug)

class QPropertySpec;
class ComplexTypeEntry;
class EnumTypeEntry;

class AbstractMetaClass : public AbstractMetaAttributes, public EnclosingClassMixin
{
    Q_GADGET
public:
    enum FunctionQueryOption {
        Constructors                 = 0x0000001, // Only constructors
        //Destructors                  = 0x0000002, // Only destructors. Not included in class.
        FinalInTargetLangFunctions   = 0x0000008, // Only functions that are non-virtual in TargetLang
        ClassImplements              = 0x0000020, // Only functions implemented by the current class
        StaticFunctions              = 0x0000080, // Only static functions
        Signals                      = 0x0000100, // Only signals
        NormalFunctions              = 0x0000200, // Only functions that aren't signals
        Visible                      = 0x0000400, // Only public and protected functions
        WasPublic                    = 0x0001000, // Only functions that were originally public
        NonStaticFunctions           = 0x0004000, // No static functions
        Empty                        = 0x0008000, // Empty overrides of abstract functions
        Invisible                    = 0x0010000, // Only private functions
        VirtualInCppFunctions        = 0x0020000, // Only functions that are virtual in C++
        VirtualInTargetLangFunctions = 0x0080000, // Only functions which are virtual in TargetLang
        NotRemovedFromTargetLang     = 0x0400000, // Only functions that have not been removed from TargetLang
        OperatorOverloads            = 0x2000000, // Only functions that are operator overloads
        GenerateExceptionHandling    = 0x4000000,
        GetAttroFunction             = 0x8000000,
        SetAttroFunction            = 0x10000000
    };
    Q_DECLARE_FLAGS(FunctionQueryOptions, FunctionQueryOption)
    Q_FLAG(FunctionQueryOption)

    enum OperatorQueryOption {
        ArithmeticOp   = 0x01, // Arithmetic: +, -, *, /, %, +=, -=, *=, /=, %=, ++, --, unary+, unary-
        BitwiseOp      = 0x02, // Bitwise: <<, <<=, >>, >>=, ~, &, &=, |, |=, ^, ^=
        ComparisonOp   = 0x04, // Comparison: <, <=, >, >=, !=, ==
        LogicalOp      = 0x08, // Logical: !, &&, ||
        ConversionOp   = 0x10, // Conversion: operator [const] TYPE()
        SubscriptionOp = 0x20, // Subscription: []
        AssignmentOp   = 0x40, // Assignment: =
        OtherOp        = 0x80, // The remaining operators: call(), etc
        AllOperators   = ArithmeticOp | BitwiseOp | ComparisonOp
                        | LogicalOp | ConversionOp | SubscriptionOp
                        | AssignmentOp | OtherOp
    };
    Q_DECLARE_FLAGS(OperatorQueryOptions, OperatorQueryOption)
    Q_FLAG(OperatorQueryOption)

    AbstractMetaClass();
    ~AbstractMetaClass();

    void fixFunctions();

    const AbstractMetaFunctionList &functions() const { return m_functions; }

    void setFunctions(const AbstractMetaFunctionList &functions);
    void addFunction(AbstractMetaFunction *function);
    bool hasFunction(const AbstractMetaFunction *f) const;
    bool hasFunction(const QString &str) const;
    const AbstractMetaFunction* findFunction(const QString& functionName) const;
    bool hasSignal(const AbstractMetaFunction *f) const;

    bool hasConstructors() const;
    const AbstractMetaFunction *copyConstructor() const;
    bool hasCopyConstructor() const { return copyConstructor() != nullptr; }
    bool hasPrivateCopyConstructor() const;

    void addDefaultConstructor();
    void addDefaultCopyConstructor(bool isPrivate = false);

    bool hasNonPrivateConstructor() const { return m_hasNonPrivateConstructor; }
    void setHasNonPrivateConstructor(bool value) { m_hasNonPrivateConstructor = value; }

    bool hasPrivateConstructor() const { return m_hasPrivateConstructor; }
    void setHasPrivateConstructor(bool value) { m_hasPrivateConstructor = value; }

    bool hasPrivateDestructor() const { return m_hasPrivateDestructor; }
    void setHasPrivateDestructor(bool value) { m_hasPrivateDestructor = value; }

    bool hasProtectedDestructor() const { return m_hasProtectedDestructor; }
    void setHasProtectedDestructor(bool value) { m_hasProtectedDestructor = value; }

    bool hasVirtualDestructor() const { return m_hasVirtualDestructor; }
    void setHasVirtualDestructor(bool value);

    bool isConstructible() const
    {
        return (hasNonPrivateConstructor() || !hasPrivateConstructor()) && !hasPrivateDestructor();
    }

    bool generateExceptionHandling() const;

    AbstractMetaFunctionList queryFunctionsByName(const QString &name) const;
    static bool queryFunction(const AbstractMetaFunction *f, FunctionQueryOptions query);
    static AbstractMetaFunctionList queryFunctionList(const AbstractMetaFunctionList &list,
                                                      FunctionQueryOptions query);
    static const AbstractMetaFunction *queryFirstFunction(const AbstractMetaFunctionList &list,
                                                          FunctionQueryOptions query);

    AbstractMetaFunctionList queryFunctions(FunctionQueryOptions query) const;
    AbstractMetaFunctionList functionsInTargetLang() const;
    AbstractMetaFunctionList cppSignalFunctions() const;
    AbstractMetaFunctionList implicitConversions() const;

    /**
     *   Retrieves all class' operator overloads that meet
     *   query criteria defined with the OperatorQueryOption
     *   enum.
     *   /param query composition of OperatorQueryOption enum values
     *   /return list of operator overload methods that meet the
     *   query criteria
     */
    AbstractMetaFunctionList operatorOverloads(OperatorQueryOptions query = AllOperators) const;

    bool hasArithmeticOperatorOverload() const;
    bool hasBitwiseOperatorOverload() const;
    bool hasComparisonOperatorOverload() const;
    bool hasLogicalOperatorOverload() const;

    const AbstractMetaFieldList &fields() const { return m_fields; }
    AbstractMetaFieldList &fields() { return m_fields; }
    void setFields(const AbstractMetaFieldList &fields) { m_fields = fields; }
    void addField(const AbstractMetaField &field) { m_fields << field; }

    std::optional<AbstractMetaField> findField(const QString &name) const;

    const AbstractMetaEnumList &enums() const { return m_enums; }
    void setEnums(const AbstractMetaEnumList &enums) { m_enums = enums; }
    void addEnum(AbstractMetaEnum *e) { m_enums << e; }

    AbstractMetaEnum *findEnum(const QString &enumName);
    AbstractMetaEnumValue *findEnumValue(const QString &enumName);
    void getEnumsToBeGenerated(AbstractMetaEnumList *enumList) const;
    void getEnumsFromInvisibleNamespacesToBeGenerated(AbstractMetaEnumList *enumList) const;

    void getFunctionsFromInvisibleNamespacesToBeGenerated(AbstractMetaFunctionList *funcList) const;

    QString fullName() const
    {
        return package() + QLatin1Char('.') + name();
    }

    /**
     *   Retrieves the class name without any namespace/scope information.
     *   /return the class name without scope information
     */
    QString name() const;

    const Documentation &documentation() const { return m_doc; }
    void setDocumentation(const Documentation& doc) { m_doc = doc; }

    QString baseClassName() const
    {
        return m_baseClasses.isEmpty() ? QString() : m_baseClasses.constFirst()->name();
    }

    AbstractMetaClass *baseClass() const
    {
        return m_baseClasses.value(0, nullptr);
    }
    const AbstractMetaClassList &baseClasses() const { return m_baseClasses; }

    void addBaseClass(AbstractMetaClass *base_class);
    void setBaseClass(AbstractMetaClass *base_class);

    /**
     *   \return the namespace from another package which this namespace extends.
     */
    AbstractMetaClass *extendedNamespace() const { return m_extendedNamespace; }
    void setExtendedNamespace(AbstractMetaClass *e) { m_extendedNamespace = e; }

    const AbstractMetaClassList& innerClasses() const { return m_innerClasses; }
    void addInnerClass(AbstractMetaClass* cl) { m_innerClasses << cl; }
    void setInnerClasses(const AbstractMetaClassList &innerClasses) { m_innerClasses = innerClasses; }

    QString package() const;

    bool isNamespace() const;
    bool isInvisibleNamespace() const;

    bool isQObject() const;

    bool isQtNamespace() const
    {
        return isNamespace() && name() == QLatin1String("Qt");
    }

    QString qualifiedCppName() const;

    bool hasSignals() const;
    bool inheritsFrom(const AbstractMetaClass *other) const;

    /**
    *   Says if the class that declares or inherits a virtual function.
    *   \return true if the class implements or inherits any virtual methods
    */
    bool isPolymorphic() const { return m_isPolymorphic; }

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


    const QVector<TypeEntry *> &templateArguments() const { return m_templateArgs; }
    void setTemplateArguments(const QVector<TypeEntry *> &args) { m_templateArgs = args; }

    // only valid during metabuilder's run
    const QStringList &baseClassNames() const { return m_baseClassNames; }
    void setBaseClassNames(const QStringList &names) { m_baseClassNames = names; }

    const ComplexTypeEntry *typeEntry() const { return m_typeEntry; }
    ComplexTypeEntry *typeEntry() { return m_typeEntry; }
    void setTypeEntry(ComplexTypeEntry *type) { m_typeEntry = type; }

    void setHasHashFunction(bool on) { m_hasHashFunction = on; }

    bool hasHashFunction() const { return m_hasHashFunction; }

    bool hasDefaultToStringFunction() const;

    bool hasEqualsOperator() const { return m_hasEqualsOperator; }
    void setHasEqualsOperator(bool on) { m_hasEqualsOperator = on; }

    bool hasCloneOperator() const { return m_hasCloneOperator; }
    void setHasCloneOperator(bool on) { m_hasCloneOperator = on; }

    const QVector<QPropertySpec *> &propertySpecs() const { return m_propertySpecs; }
    void addPropertySpec(QPropertySpec *spec) { m_propertySpecs << spec; }

    QPropertySpec *propertySpecByName(const QString &name) const;
    QPropertySpec *propertySpecForRead(const QString &name) const;
    QPropertySpec *propertySpecForWrite(const QString &name) const;
    QPropertySpec *propertySpecForReset(const QString &name) const;

    /// Returns a list of conversion operators for this class. The conversion operators are defined in other classes of the same module.
    AbstractMetaFunctionList externalConversionOperators() const
    {
        return m_externalConversionOperators;
    }
    /// Adds a converter operator for this class.
    void addExternalConversionOperator(AbstractMetaFunction* conversionOp)
    {
        if (!m_externalConversionOperators.contains(conversionOp))
            m_externalConversionOperators.append(conversionOp);
    }
    /// Returns true if this class has any converter operators defined elsewhere.
    bool hasExternalConversionOperators() const
    {
        return !m_externalConversionOperators.isEmpty();
    }

    void sortFunctions();

    const AbstractMetaClass *templateBaseClass() const { return m_templateBaseClass; }
    void setTemplateBaseClass(const AbstractMetaClass *cls) { m_templateBaseClass = cls; }

    bool hasTemplateBaseClassInstantiations() const;
    const AbstractMetaTypeList &templateBaseClassInstantiations() const;
    void setTemplateBaseClassInstantiations(const AbstractMetaTypeList& instantiations);

    void setTypeDef(bool typeDef) { m_isTypeDef = typeDef; }
    bool isTypeDef() const { return m_isTypeDef; }

    bool isStream() const { return m_stream; }
    void setStream(bool stream) { m_stream = stream; }

    bool hasToStringCapability() const { return m_hasToStringCapability; }
    void setToStringCapability(bool value, uint indirections = 0)
    {
        m_hasToStringCapability = value;
        m_toStringCapabilityIndirections = indirections;
    }

    uint toStringCapabilityIndirections() const { return m_toStringCapabilityIndirections; }

    bool deleteInMainThread() const;

    static AbstractMetaClass *findClass(const AbstractMetaClassList &classes,
                                        const QString &name);
    static AbstractMetaClass *findClass(const AbstractMetaClassList &classes,
                                        const TypeEntry* typeEntry);
    static AbstractMetaEnumValue *findEnumValue(const AbstractMetaClassList &classes,
                                                const QString &string);
    static AbstractMetaEnum *findEnum(const AbstractMetaClassList &classes,
                                      const EnumTypeEntry *entry);

    SourceLocation sourceLocation() const;
    void setSourceLocation(const SourceLocation &sourceLocation);

    template <class Function>
    void invisibleNamespaceRecursion(Function f) const;

private:
#ifndef QT_NO_DEBUG_STREAM
    void format(QDebug &d) const;
    void formatMembers(QDebug &d) const;
    friend QDebug operator<<(QDebug d, const AbstractMetaClass *ac);
#endif
    uint m_hasVirtuals : 1;
    uint m_isPolymorphic : 1;
    uint m_hasNonpublic : 1;
    uint m_hasNonPrivateConstructor : 1;
    uint m_hasPrivateConstructor : 1;
    uint m_functionsFixed : 1;
    uint m_hasPrivateDestructor : 1;
    uint m_hasProtectedDestructor : 1;
    uint m_hasVirtualDestructor : 1;
    uint m_hasHashFunction : 1;
    uint m_hasEqualsOperator : 1;
    uint m_hasCloneOperator : 1;
    uint m_isTypeDef : 1;
    uint m_hasToStringCapability : 1;

    Documentation m_doc;

    const AbstractMetaClass *m_enclosingClass = nullptr;
    AbstractMetaClassList m_baseClasses; // Real base classes after setting up inheritance
    AbstractMetaTypeList m_baseTemplateInstantiations;
    AbstractMetaClass *m_extendedNamespace = nullptr;

    const AbstractMetaClass *m_templateBaseClass = nullptr;
    AbstractMetaFunctionList m_functions;
    AbstractMetaFieldList m_fields;
    AbstractMetaEnumList m_enums;
    QVector<QPropertySpec *> m_propertySpecs;
    AbstractMetaClassList m_innerClasses;

    AbstractMetaFunctionList m_externalConversionOperators;

    QStringList m_baseClassNames;  // Base class names from C++, including rejected
    QVector<TypeEntry *> m_templateArgs;
    ComplexTypeEntry *m_typeEntry = nullptr;
    SourceLocation m_sourceLocation;
//     FunctionModelItem m_qDebugStreamFunction;

    bool m_stream = false;
    uint m_toStringCapabilityIndirections = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaClass::FunctionQueryOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaClass::OperatorQueryOptions)

template <class Function>
void AbstractMetaClass::invisibleNamespaceRecursion(Function f) const
{
    for (auto ic : m_innerClasses) {
        if (ic->isInvisibleNamespace()) {
            f(ic);
            ic->invisibleNamespaceRecursion(f);
        }
    }
}

#endif // ABSTRACTMETALANG_H
