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

    const AbstractMetaFunctionList &functions() const;
    void setFunctions(const AbstractMetaFunctionList &functions);
    void addFunction(AbstractMetaFunction *function);
    bool hasFunction(const AbstractMetaFunction *f) const;
    bool hasFunction(const QString &str) const;
    const AbstractMetaFunction* findFunction(const QString& functionName) const;
    bool hasSignal(const AbstractMetaFunction *f) const;

    bool hasConstructors() const;
    const AbstractMetaFunction *copyConstructor() const;
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

    void getFunctionsFromInvisibleNamespacesToBeGenerated(AbstractMetaFunctionList *funcList) const;

    QString fullName() const;

    /**
     *   Retrieves the class name without any namespace/scope information.
     *   /return the class name without scope information
     */
    QString name() const;

    const Documentation &documentation() const;
    void setDocumentation(const Documentation& doc);

    QString baseClassName() const;

    AbstractMetaClass *baseClass() const;
    const AbstractMetaClassList &baseClasses() const;

    void addBaseClass(AbstractMetaClass *base_class);
    void setBaseClass(AbstractMetaClass *base_class);

    /**
     *   \return the namespace from another package which this namespace extends.
     */
    AbstractMetaClass *extendedNamespace() const;
    void setExtendedNamespace(AbstractMetaClass *e);

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


    const QVector<TypeEntry *> &templateArguments() const;
    void setTemplateArguments(const QVector<TypeEntry *> &args);

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
    AbstractMetaFunctionList externalConversionOperators() const;
    /// Adds a converter operator for this class.
    void addExternalConversionOperator(AbstractMetaFunction* conversionOp);
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

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaClass::FunctionQueryOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaClass::OperatorQueryOptions)

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
