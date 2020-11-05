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

#ifndef ABSTRACTMETAFUNCTION_H
#define ABSTRACTMETAFUNCTION_H

#include "abstractmetalang_typedefs.h"
#include "abstractmetaargument.h"
#include "abstractmetaattributes.h"
#include "abstractmetatype.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "sourcelocation.h"

QT_FORWARD_DECLARE_CLASS(QDebug)

class FunctionTypeEntry;
class QPropertySpec;

struct ArgumentOwner;
struct FieldModification;
struct FunctionModification;
struct ReferenceCount;

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

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString originalName() const
    {
        return m_originalName.isEmpty() ? name() : m_originalName;
    }

    void setOriginalName(const QString &name) { m_originalName = name; }

    bool isReverseOperator() const { return m_reverse; }
    void setReverseOperator(bool reverse) { m_reverse = reverse; }

    /**
     *  Returns true if this is a operator and the "self" operand is a pointer.
     *  e.g. class Foo {}; operator+(SomeEnum, Foo*);
     */
    bool isPointerOperator() const { return m_pointerOperator; }
    void setPointerOperator(bool value) { m_pointerOperator = value; }


    /**
    *   Says if the function (a constructor) was declared as explicit in C++.
    *   \return true if the function was declared as explicit in C++
    */
    bool isExplicit() const { return m_explicit; }
    void setExplicit(bool isExplicit) { m_explicit = isExplicit; }

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

    const AbstractMetaType &type() const { return m_type; }
    void setType(const AbstractMetaType &type) { m_type = type; }

    // The class that has this function as a member.
    const AbstractMetaClass *ownerClass() const { return m_class; }
    void setOwnerClass(const AbstractMetaClass *cls) { m_class = cls; }

    // Owner excluding invisible namespaces
    const AbstractMetaClass *targetLangOwner() const;

    // The first class in a hierarchy that declares the function
    const AbstractMetaClass *declaringClass() const { return m_declaringClass; }
    void setDeclaringClass(const AbstractMetaClass *cls) { m_declaringClass = cls; }

    // The class that actually implements this function
    const AbstractMetaClass *implementingClass() const { return m_implementingClass; }
    void setImplementingClass(const AbstractMetaClass *cls) { m_implementingClass = cls; }

    const AbstractMetaArgumentList &arguments() const { return m_arguments; }
    AbstractMetaArgumentList &arguments() { return m_arguments; }
    void setArguments(const AbstractMetaArgumentList &arguments) { m_arguments = arguments; }
    void addArgument(const AbstractMetaArgument &argument)
    {
        m_arguments << argument;
    }
    int actualMinimumArgumentCount() const;

    bool isDeprecated() const;
    bool isDestructor() const { return functionType() == DestructorFunction; }
    bool isConstructor() const
    {
        return m_functionType == ConstructorFunction || m_functionType == CopyConstructorFunction
            || m_functionType == MoveConstructorFunction;
    }
    bool isNormal() const
    {
        return functionType() == NormalFunction || isSlot() || isInGlobalScope();
    }
    bool isInGlobalScope() const { return functionType() == GlobalScopeFunction; }
    bool isSignal() const { return functionType() == SignalFunction; }
    bool isSlot() const { return functionType() == SlotFunction; }
    bool isEmptyFunction() const { return functionType() == EmptyFunction; }
    FunctionType functionType() const { return m_functionType; }
    void setFunctionType(FunctionType type) { m_functionType = type; }

    bool usesRValueReferences() const;
    QStringList introspectionCompatibleSignatures(const QStringList &resolvedArguments = QStringList()) const;
    QString signature() const;

    bool isConstant() const { return m_constant; }
    void setConstant(bool constant) { m_constant = constant; }

    /// Returns true if the AbstractMetaFunction was added by the user via the type system description.
    bool isUserAdded() const;

    QString toString() const { return m_name; }

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

    QPropertySpec *propertySpec() const { return m_propertySpec; }
    void setPropertySpec(QPropertySpec *spec) { m_propertySpec = spec; }

    FunctionTypeEntry* typeEntry() const { return m_typeEntry; }

    void setTypeEntry(FunctionTypeEntry* typeEntry) { m_typeEntry = typeEntry; }

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
    void formatDebugBrief(QDebug &d) const;
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

#endif // ABSTRACTMETAFUNCTION_H
