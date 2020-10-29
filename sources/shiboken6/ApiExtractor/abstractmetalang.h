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
#include "abstractmetaargument.h"
#include "abstractmetatype.h"
#include "documentation.h"
#include "sourcelocation.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include "parser/codemodel_enums.h"
#include "parser/enumvalue.h"

#include <QtCore/qobjectdefs.h>
#include <QtCore/QStringList>
#include <QtCore/QSharedDataPointer>

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMeta;
class AbstractMetaClass;
class AbstractMetaField;
class AbstractMetaFunction;
class AbstractMetaVariable;
class AbstractMetaArgument;
class AbstractMetaEnumValue;
class AbstractMetaEnum;
class QPropertySpec;

class CodeSnip;
class ComplexTypeEntry;
class EnumTypeEntry;
class FlagsTypeEntry;
class FunctionTypeEntry;
class TypeEntry;

struct ArgumentOwner;
struct FieldModification;
struct FunctionModification;
struct ReferenceCount;

class AbstractMetaAttributes
{
    Q_GADGET
public:
    Q_DISABLE_COPY(AbstractMetaAttributes)

    AbstractMetaAttributes();
    virtual ~AbstractMetaAttributes();

    enum Attribute {
        None                        = 0x00000000,

        Private                     = 0x00000001,
        Protected                   = 0x00000002,
        Public                      = 0x00000004,
        Friendly                    = 0x00000008,
        Visibility                  = 0x0000000f,

        Abstract                    = 0x00000020,
        Static                      = 0x00000040,

        FinalInTargetLang           = 0x00000080,

        GetterFunction              = 0x00000400,
        SetterFunction              = 0x00000800,

        PropertyReader              = 0x00004000,
        PropertyWriter              = 0x00008000,
        PropertyResetter            = 0x00010000,

        Invokable                   = 0x00040000,

        HasRejectedConstructor      = 0x00080000,
        HasRejectedDefaultConstructor = 0x00100000,

        FinalCppClass               = 0x00200000,
        VirtualCppMethod            = 0x00400000,
        OverriddenCppMethod         = 0x00800000,
        FinalCppMethod              = 0x01000000,
        // Add by meta builder (implicit constructors, inherited methods, etc)
        AddedMethod                 = 0x02000000,
        Deprecated                  = 0x04000000
    };
    Q_DECLARE_FLAGS(Attributes, Attribute)
    Q_FLAG(Attribute)

    Attributes attributes() const
    {
        return m_attributes;
    }

    void setAttributes(Attributes attributes)
    {
        m_attributes = attributes;
    }

    Attributes originalAttributes() const
    {
        return m_originalAttributes;
    }

    void setOriginalAttributes(Attributes attributes)
    {
        m_originalAttributes = attributes;
    }

    Attributes visibility() const
    {
        return m_attributes & Visibility;
    }

    void setVisibility(Attributes visi)
    {
        m_attributes = (m_attributes & ~Visibility) | visi;
    }

    void operator+=(Attribute attribute)
    {
        m_attributes |= attribute;
    }

    void operator-=(Attribute attribute)
    {
        m_attributes &= ~attribute;
    }

    bool isFinalInTargetLang() const
    {
        return m_attributes & FinalInTargetLang;
    }

    bool isAbstract() const
    {
        return m_attributes & Abstract;
    }

    bool isStatic() const
    {
        return m_attributes & Static;
    }

    bool isInvokable() const
    {
        return m_attributes & Invokable;
    }

    bool isPropertyReader() const
    {
        return m_attributes & PropertyReader;
    }

    bool isPropertyWriter() const
    {
        return m_attributes & PropertyWriter;
    }

    bool isPropertyResetter() const
    {
        return m_attributes & PropertyResetter;
    }

    bool isPrivate() const
    {
        return m_attributes & Private;
    }

    bool isProtected() const
    {
        return m_attributes & Protected;
    }

    bool isPublic() const
    {
        return m_attributes & Public;
    }

    bool isFriendly() const
    {
        return m_attributes & Friendly;
    }

    bool wasPrivate() const
    {
        return m_originalAttributes & Private;
    }

    bool wasPublic() const
    {
        return m_originalAttributes & Public;
    }

    void setDocumentation(const Documentation& doc)
    {
        m_doc = doc;
    }

    Documentation documentation() const
    {
        return m_doc;
    }

protected:
    void assignMetaAttributes(const AbstractMetaAttributes &other);

private:
    Attributes m_attributes;
    Attributes m_originalAttributes;
    Documentation m_doc;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaAttributes::Attributes)

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaAttributes *aa);
#endif

class AbstractMetaVariable
{
    Q_DISABLE_COPY(AbstractMetaVariable)
public:
    AbstractMetaVariable();

    virtual ~AbstractMetaVariable();

    const AbstractMetaType &type() const
    {
        return m_type;
    }
    void setType(const AbstractMetaType &type)
    {
        m_type = type;
    }

    QString name() const
    {
        return m_name;
    }
    void setName(const QString &name, bool realName = true)
    {
        m_name = name;
        m_hasName = realName;
    }
    bool hasName() const
    {
        return m_hasName;
    }
    QString originalName() const
    {
        return m_originalName;
    }
    void setOriginalName(const QString& name)
    {
        m_originalName = name;
    }
    void setDocumentation(const Documentation& doc)
    {
        m_doc = doc;
    }
    Documentation documentation() const
    {
        return m_doc;
    }

protected:
    void assignMetaVariable(const AbstractMetaVariable &other);

private:
    QString m_originalName;
    QString m_name;
    AbstractMetaType m_type;
    bool m_hasName = false;

    Documentation m_doc;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaVariable *av);
#endif

class EnclosingClassMixin {
public:
    const AbstractMetaClass *enclosingClass() const { return m_enclosingClass; }
    void setEnclosingClass(const AbstractMetaClass *cls) { m_enclosingClass = cls; }
    const AbstractMetaClass *targetLangEnclosingClass() const;

private:
     const AbstractMetaClass *m_enclosingClass = nullptr;
};

class AbstractMetaField : public AbstractMetaVariable, public AbstractMetaAttributes, public EnclosingClassMixin
{
public:
    AbstractMetaField();

    FieldModificationList modifications() const;

    bool isModifiedRemoved(int types = TypeSystem::All) const;

    using AbstractMetaVariable::setDocumentation;
    using AbstractMetaVariable::documentation;

    AbstractMetaField *copy() const;

    static AbstractMetaField *
        find(const AbstractMetaFieldList &haystack, const QString &needle);
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaField *af);
#endif

class AbstractMetaFunction : public AbstractMetaAttributes
{
    Q_GADGET
public:
    enum FunctionType {
        ConstructorFunction,
        CopyConstructorFunction,
        MoveConstructorFunction,
        AssignmentOperatorFunction,
        MoveAssignmentOperatorFunction,
        DestructorFunction,
        NormalFunction,
        SignalFunction,
        EmptyFunction,
        SlotFunction,
        GlobalScopeFunction,
        GetAttroFunction,
        SetAttroFunction
    };
    Q_ENUM(FunctionType)

    enum CompareResultFlag {
        EqualName                   = 0x00000001,
        EqualArguments              = 0x00000002,
        EqualAttributes             = 0x00000004,
        EqualImplementor            = 0x00000008,
        EqualReturnType             = 0x00000010,
        EqualDefaultValueOverload   = 0x00000020,
        EqualModifiedName           = 0x00000040,

        NameLessThan                = 0x00001000,

        PrettySimilar               = EqualName | EqualArguments,
        Equal                       = 0x0000001f,
        NotEqual                    = 0x00001000
    };
    Q_DECLARE_FLAGS(CompareResult, CompareResultFlag)
    Q_FLAG(CompareResultFlag)

    AbstractMetaFunction();
    explicit AbstractMetaFunction(const AddedFunctionPtr &addedFunc);
    ~AbstractMetaFunction();

    QString name() const
    {
        return m_name;
    }

    void setName(const QString &name)
    {
        m_name = name;
    }

    QString originalName() const
    {
        return m_originalName.isEmpty() ? name() : m_originalName;
    }

    void setOriginalName(const QString &name)
    {
        m_originalName = name;
    }

    void setReverseOperator(bool reverse)
    {
        m_reverse = reverse;
    }

    bool isReverseOperator() const
    {
        return m_reverse;
    }

    /**
     *  Returns true if this is a operator and the "self" operand is a pointer.
     *  e.g. class Foo {}; operator+(SomeEnum, Foo*);
     */
    bool isPointerOperator() const
    {
        return m_pointerOperator;
    }

    void setPointerOperator(bool value)
    {
        m_pointerOperator = value;
    }

    void setExplicit(bool isExplicit)
    {
        m_explicit = isExplicit;
    }
    /**
    *   Says if the function (a constructor) was declared as explicit in C++.
    *   \return true if the function was declared as explicit in C++
    */
    bool isExplicit() const
    {
        return m_explicit;
    }

    static bool isConversionOperator(const QString& funcName);

    ExceptionSpecification exceptionSpecification() const;
    void setExceptionSpecification(ExceptionSpecification e);

    bool generateExceptionHandling() const;

    bool isConversionOperator() const
    {
        return isConversionOperator(originalName());
    }

    static bool isOperatorOverload(const QString& funcName);
    bool isOperatorOverload() const
    {
        return isOperatorOverload(originalName());
    }
    bool isCastOperator() const;

    bool isArithmeticOperator() const;
    bool isBitwiseOperator() const;
    bool isComparisonOperator() const;
    bool isLogicalOperator() const;
    bool isSubscriptOperator() const;
    bool isAssignmentOperator() const; // Assignment or move assignment
    bool isOtherOperator() const;

    /**
     * Informs the arity of the operator or -1 if the function is not
     * an operator overload.
     * /return the arity of the operator or -1
     */
    int arityOfOperator() const;
    bool isUnaryOperator() const { return arityOfOperator() == 1; }
    bool isBinaryOperator() const { return arityOfOperator() == 2; }
    bool isInplaceOperator() const;

    bool isVirtual() const;
    bool allowThread() const;
    QString modifiedName() const;

    QString minimalSignature() const;
    QString debugSignature() const; // including virtual/override/final, etc., for debugging only.

    bool isModifiedRemoved(int types = TypeSystem::All) const;

    bool isVoid() const { return m_type.isVoid(); }
    const AbstractMetaType &type() const
    {
        return m_type;
    }
    void setType(const AbstractMetaType &type)
    {
        m_type = type;
    }

    // The class that has this function as a member.
    const AbstractMetaClass *ownerClass() const
    {
        return m_class;
    }
    void setOwnerClass(const AbstractMetaClass *cls)
    {
        m_class = cls;
    }

    // Owner excluding invisible namespaces
    const AbstractMetaClass *targetLangOwner() const;

    // The first class in a hierarchy that declares the function
    const AbstractMetaClass *declaringClass() const
    {
        return m_declaringClass;
    }
    void setDeclaringClass(const AbstractMetaClass *cls)
    {
        m_declaringClass = cls;
    }

    // The class that actually implements this function
    const AbstractMetaClass *implementingClass() const
    {
        return m_implementingClass;
    }
    void setImplementingClass(const AbstractMetaClass *cls)
    {
        m_implementingClass = cls;
    }

    const AbstractMetaArgumentList &arguments() const
    {
        return m_arguments;
    }
    AbstractMetaArgumentList &arguments()
    {
        return m_arguments;
    }
    void setArguments(const AbstractMetaArgumentList &arguments)
    {
        m_arguments = arguments;
    }
    void addArgument(const AbstractMetaArgument &argument)
    {
        m_arguments << argument;
    }
    int actualMinimumArgumentCount() const;

    bool isDeprecated() const;
    bool isDestructor() const
    {
        return functionType() == DestructorFunction;
    }
    bool isConstructor() const
    {
        return m_functionType == ConstructorFunction || m_functionType == CopyConstructorFunction
            || m_functionType == MoveConstructorFunction;
    }
    bool isNormal() const
    {
        return functionType() == NormalFunction || isSlot() || isInGlobalScope();
    }
    bool isInGlobalScope() const
    {
        return functionType() == GlobalScopeFunction;
    }
    bool isSignal() const
    {
        return functionType() == SignalFunction;
    }
    bool isSlot() const
    {
        return functionType() == SlotFunction;
    }
    bool isEmptyFunction() const
    {
        return functionType() == EmptyFunction;
    }
    FunctionType functionType() const
    {
        return m_functionType;
    }
    void setFunctionType(FunctionType type)
    {
        m_functionType = type;
    }

    bool usesRValueReferences() const;
    QStringList introspectionCompatibleSignatures(const QStringList &resolvedArguments = QStringList()) const;
    QString signature() const;

    bool isConstant() const
    {
        return m_constant;
    }
    void setConstant(bool constant)
    {
        m_constant = constant;
    }

    /// Returns true if the AbstractMetaFunction was added by the user via the type system description.
    bool isUserAdded() const;

    QString toString() const
    {
        return m_name;
    }

    CompareResult compareTo(const AbstractMetaFunction *other) const;

    bool operator <(const AbstractMetaFunction &a) const;

    AbstractMetaFunction *copy() const;

    QString conversionRule(TypeSystem::Language language, int idx) const;
    QVector<ReferenceCount> referenceCounts(const AbstractMetaClass *cls, int idx = -2) const;
    ArgumentOwner argumentOwner(const AbstractMetaClass *cls, int idx) const;

    // Returns the ownership rules for the given argument in the given context
    TypeSystem::Ownership ownership(const AbstractMetaClass *cls, TypeSystem::Language language, int idx) const;

    QString typeReplaced(int argument_index) const;
    bool isModifiedToArray(int argumentIndex) const;
    bool isRemovedFromAllLanguages(const AbstractMetaClass *) const;
    bool isRemovedFrom(const AbstractMetaClass *, TypeSystem::Language language) const;
    bool argumentRemoved(int) const;
    /**
    *   Verifies if any modification to the function is an inject code.
    *   \return true if there is inject code modifications to the function.
    */
    bool hasInjectedCode() const;
    /**
    *   Returns a list of code snips for this function.
    *   The code snips can be filtered by position and language.
    *   \return list of code snips
    */
    CodeSnipList injectedCodeSnips(TypeSystem::CodeSnipPosition position = TypeSystem::CodeSnipPositionAny,
                                   TypeSystem::Language language = TypeSystem::All) const;

    /**
    *   Verifies if any modification to the function alters/removes its
    *   arguments types or default values.
    *   \return true if there is some modification to function signature
    */
    bool hasSignatureModifications() const;
    FunctionModificationList modifications(const AbstractMetaClass* implementor = nullptr) const;

    /**
     * Return the argument name if there is a modification the renamed value will be returned
     */
    QString argumentName(int index, bool create = true, const AbstractMetaClass *cl = nullptr) const;

    void setPropertySpec(QPropertySpec *spec)
    {
        m_propertySpec = spec;
    }

    QPropertySpec *propertySpec() const
    {
        return m_propertySpec;
    }

    FunctionTypeEntry* typeEntry() const
    {
        return m_typeEntry;
    }

    void setTypeEntry(FunctionTypeEntry* typeEntry)
    {
        m_typeEntry = typeEntry;
    }

    bool isCallOperator() const;

    static AbstractMetaFunction *
        find(const AbstractMetaFunctionList &haystack, const QString &needle);

    // for the meta builder only
    void setAllowThreadModification(TypeSystem::AllowThread am)
    { m_allowThreadModification = am; }
    void setExceptionHandlingModification(TypeSystem::ExceptionHandling em)
    { m_exceptionHandlingModification = em;  }

     int overloadNumber() const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebugVerbose(QDebug &d) const;
#endif

    SourceLocation sourceLocation() const;
    void setSourceLocation(const SourceLocation &sourceLocation);

private:
    bool autoDetectAllowThread() const;

    QString m_name;
    QString m_originalName;
    mutable QString m_cachedMinimalSignature;
    mutable QString m_cachedSignature;
    mutable QString m_cachedModifiedName;

    FunctionTypeEntry* m_typeEntry = nullptr;
    FunctionType m_functionType = NormalFunction;
    AbstractMetaType m_type;
    const AbstractMetaClass *m_class = nullptr;
    const AbstractMetaClass *m_implementingClass = nullptr;
    const AbstractMetaClass *m_declaringClass = nullptr;
    QPropertySpec *m_propertySpec = nullptr;
    AbstractMetaArgumentList m_arguments;
    AddedFunctionPtr m_addedFunction;
    SourceLocation m_sourceLocation;
    uint m_constant                 : 1;
    uint m_reverse                  : 1;
    uint m_explicit                 : 1;
    uint m_pointerOperator          : 1;
    uint m_isCallOperator           : 1;
    mutable int m_cachedOverloadNumber = TypeSystem::OverloadNumberUnset;
    ExceptionSpecification m_exceptionSpecification = ExceptionSpecification::Unknown;
    TypeSystem::AllowThread m_allowThreadModification = TypeSystem::AllowThread::Unspecified;
    TypeSystem::ExceptionHandling m_exceptionHandlingModification = TypeSystem::ExceptionHandling::Unspecified;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaFunction::CompareResult)

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaFunction *af);
#endif

class AbstractMetaEnumValue
{
public:
    AbstractMetaEnumValue() = default;

    EnumValue value() const
    {
        return m_value;
    }

    void setValue(EnumValue value)
    {
        m_value = value;
    }

    QString stringValue() const
    {
        return m_stringValue;
    }

    void setStringValue(const QString &v)
    {
        m_stringValue = v;
    }

    QString name() const
    {
        return m_name;
    }

    void setName(const QString &name)
    {
        m_name = name;
    }

    void setDocumentation(const Documentation& doc)
    {
        m_doc = doc;
    }

    Documentation documentation() const
    {
        return m_doc;
    }

private:
    QString m_name;
    QString m_stringValue;

    EnumValue m_value;

    Documentation m_doc;
};

class AbstractMetaEnum : public AbstractMetaAttributes, public EnclosingClassMixin
{
public:
    AbstractMetaEnum();
    ~AbstractMetaEnum();

    AbstractMetaEnumValueList values() const
    {
        return m_enumValues;
    }

    void addEnumValue(AbstractMetaEnumValue *enumValue)
    {
        m_enumValues << enumValue;
    }

    AbstractMetaEnumValue *findEnumValue(const QString &value) const;

    QString name() const;

    QString qualifier() const;

    QString package() const;

    QString fullName() const
    {
        return package() + QLatin1Char('.') + qualifier()  + QLatin1Char('.') + name();
    }

    EnumKind enumKind() const { return m_enumKind; }
    void setEnumKind(EnumKind kind) { m_enumKind = kind; }

    bool isAnonymous() const { return m_enumKind == AnonymousEnum; }

    // Has the enum been declared inside a Q_ENUMS() macro in its enclosing class?
    void setHasQEnumsDeclaration(bool on)
    {
        m_hasQenumsDeclaration = on;
    }

    bool hasQEnumsDeclaration() const
    {
        return m_hasQenumsDeclaration;
    }

    EnumTypeEntry *typeEntry() const
    {
        return m_typeEntry;
    }

    void setTypeEntry(EnumTypeEntry *entry)
    {
        m_typeEntry = entry;
    }

    bool isSigned() const { return m_signed; }
    void setSigned(bool s) { m_signed = s; }

private:
    AbstractMetaEnumValueList m_enumValues;
    EnumTypeEntry *m_typeEntry = nullptr;

    EnumKind m_enumKind = CEnum;
    uint m_hasQenumsDeclaration : 1;
    uint m_signed : 1;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaEnum *ae);
#endif

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

    AbstractMetaFunctionList functions() const
    {
        return m_functions;
    }

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

    bool hasNonPrivateConstructor() const
    {
        return m_hasNonPrivateConstructor;
    }

    void setHasNonPrivateConstructor(bool value)
    {
        m_hasNonPrivateConstructor = value;
    }

    bool hasPrivateConstructor() const
    {
        return m_hasPrivateConstructor;
    }

    void setHasPrivateConstructor(bool value)
    {
        m_hasPrivateConstructor = value;
    }

    bool hasPrivateDestructor() const
    {
        return m_hasPrivateDestructor;
    }

    void setHasPrivateDestructor(bool value)
    {
        m_hasPrivateDestructor = value;
    }

    bool hasProtectedDestructor() const
    {
        return m_hasProtectedDestructor;
    }

    void setHasProtectedDestructor(bool value)
    {
        m_hasProtectedDestructor = value;
    }

    bool hasVirtualDestructor() const
    {
        return m_hasVirtualDestructor;
    }

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

    AbstractMetaFieldList fields() const
    {
        return m_fields;
    }

    void setFields(const AbstractMetaFieldList &fields)
    {
        m_fields = fields;
    }

    void addField(AbstractMetaField *field)
    {
        m_fields << field;
    }

    AbstractMetaField *findField(const QString &name) const;

    const AbstractMetaEnumList &enums() const { return m_enums; }
    void setEnums(const AbstractMetaEnumList &enums)
    {
        m_enums = enums;
    }

    void addEnum(AbstractMetaEnum *e)
    {
        m_enums << e;
    }

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

    const AbstractMetaClassList& innerClasses() const
    {
        return m_innerClasses;
    }

    void addInnerClass(AbstractMetaClass* cl)
    {
        m_innerClasses << cl;
    }

    void setInnerClasses(const AbstractMetaClassList &innerClasses)
    {
        m_innerClasses = innerClasses;
    }

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
    bool isPolymorphic() const
    {
        return m_isPolymorphic;
    }

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


    QVector<TypeEntry *> templateArguments() const
    {
        return m_templateArgs;
    }

    void setTemplateArguments(const QVector<TypeEntry *> &args)
    {
        m_templateArgs = args;
    }

    // only valid during metabuilder's run
    QStringList baseClassNames() const
    {
        return m_baseClassNames;
    }

    void setBaseClassNames(const QStringList &names)
    {
        m_baseClassNames = names;
    }

    const ComplexTypeEntry *typeEntry() const
    {
        return m_typeEntry;
    }

    ComplexTypeEntry *typeEntry()
    {
        return m_typeEntry;
    }

    void setTypeEntry(ComplexTypeEntry *type)
    {
        m_typeEntry = type;
    }

    void setHasHashFunction(bool on)
    {
        m_hasHashFunction = on;
    }

    bool hasHashFunction() const
    {
        return m_hasHashFunction;
    }
    virtual bool hasDefaultToStringFunction() const;

    void setHasEqualsOperator(bool on)
    {
        m_hasEqualsOperator = on;
    }

    bool hasEqualsOperator() const
    {
        return m_hasEqualsOperator;
    }

    void setHasCloneOperator(bool on)
    {
        m_hasCloneOperator = on;
    }

    bool hasCloneOperator() const
    {
        return m_hasCloneOperator;
    }

    void addPropertySpec(QPropertySpec *spec)
    {
        m_propertySpecs << spec;
    }

    const QVector<QPropertySpec *> &propertySpecs() const { return m_propertySpecs; }

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

    const AbstractMetaClass *templateBaseClass() const
    {
        return m_templateBaseClass;
    }

    void setTemplateBaseClass(const AbstractMetaClass *cls)
    {
        m_templateBaseClass = cls;
    }

    bool hasTemplateBaseClassInstantiations() const;
    const AbstractMetaTypeList &templateBaseClassInstantiations() const;
    void setTemplateBaseClassInstantiations(const AbstractMetaTypeList& instantiations);

    void setTypeDef(bool typeDef) { m_isTypeDef = typeDef; }
    bool isTypeDef() const { return m_isTypeDef; }

    void setStream(bool stream)
    {
        m_stream = stream;
    }

    bool isStream() const
    {
        return m_stream;
    }

    void setToStringCapability(bool value, uint indirections = 0)
    {
        m_hasToStringCapability = value;
        m_toStringCapabilityIndirections = indirections;
    }

    bool hasToStringCapability() const
    {
        return m_hasToStringCapability;
    }

    uint toStringCapabilityIndirections() const
    {
        return m_toStringCapabilityIndirections;
    }

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
