/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "clangbuilder.h"
#include "compilersupport.h"
#include "clangutils.h"

#include <codemodel.h>
#include <reporthandler.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtCore/QVector>

#include <cstring>
#include <ctype.h>

#if QT_VERSION < 0x050800
#  define Q_FALLTHROUGH()  (void)0
#endif

namespace clang {

static inline QString colonColon() { return QStringLiteral("::"); }
static inline QString templateBrackets() { return QStringLiteral("<>"); }

static inline bool isClassCursor(const CXCursor &c)
{
    return c.kind == CXCursor_ClassDecl || c.kind == CXCursor_StructDecl
        || c.kind == CXCursor_ClassTemplate
        || c.kind == CXCursor_ClassTemplatePartialSpecialization;
}

static inline bool withinClassDeclaration(const CXCursor &cursor)
{
    return isClassCursor(clang_getCursorLexicalParent(cursor));
}

static QString fixTypeName(QString t)
{
    // Fix "Foo &" -> "Foo&", similarly "Bar **" -> "Bar**"
    int pos = t.size() - 1;
    for (; pos >= 0 && (t.at(pos) == QLatin1Char('&') || t.at(pos) == QLatin1Char('*')); --pos) {}
    if (pos > 0 && t.at(pos) == QLatin1Char(' '))
        t.remove(pos, 1);
    return t;
}

// Insert template parameter to class name: "Foo<>" -> "Foo<T1>" -> "Foo<T1,T2>"
// This needs to be done immediately when template parameters are encountered since
// the class name "Foo<T1,T2>" is the scope for nested items.
static bool insertTemplateParameterIntoClassName(const QString &parmName, QString *name)
{
     if (Q_UNLIKELY(!name->endsWith(QLatin1Char('>'))))
        return false;
     const bool needsComma = name->at(name->size() - 2) != QLatin1Char('<');
     const int insertionPos = name->size() - 1;
     name->insert(insertionPos, parmName);
     if (needsComma)
         name->insert(insertionPos, QLatin1Char(','));
     return true;
}

static inline bool insertTemplateParameterIntoClassName(const QString &parmName,
                                                        const ClassModelItem &item)
{
     QString name = item->name();
     const bool result = insertTemplateParameterIntoClassName(parmName, &name);
     item->setName(name);
     return result;
}

static inline CodeModel::AccessPolicy accessPolicy(CX_CXXAccessSpecifier access)
{
    CodeModel::AccessPolicy result = CodeModel::Public;
    switch (access) {
    case CX_CXXProtected:
        result = CodeModel::Protected;
        break;
    case CX_CXXPrivate:
        result = CodeModel::Private;
        break;
    default:
        break;
    }
    return result;
}

static bool isSigned(CXTypeKind kind)
{
    switch (kind) {
    case CXType_UChar:
    case CXType_Char16:
    case CXType_Char32:
    case CXType_UShort:
    case CXType_UInt:
    case CXType_ULong:
    case CXType_ULongLong:
    case CXType_UInt128:
        return false;
    default:
        break;
    }
    return true;
}

class BuilderPrivate {
public:
    using CursorClassHash = QHash<CXCursor, ClassModelItem>;
    using CursorTypedefHash = QHash<CXCursor, TypeDefModelItem>;
    using TypeInfoHash = QHash<CXType, TypeInfo>;

    explicit BuilderPrivate(BaseVisitor *bv) : m_baseVisitor(bv), m_model(new CodeModel)
    {
       m_scopeStack.push(NamespaceModelItem(new _FileModelItem(m_model)));
    }

    // Determine scope from top item. Note that the scope list does not necessarily
    // match the scope stack in case of forward-declared inner classes whose definition
    // appears in the translation unit while the scope is the outer class.
    void updateScope()
    {
        if (m_scopeStack.size() <= 1)
            m_scope.clear();
        else
            m_scope = m_scopeStack.back()->scope() << m_scopeStack.back()->name();
    }

    void pushScope(const ScopeModelItem &i)
    {
        m_scopeStack.push(i);
        updateScope();
    }

    void popScope()
    {
        m_scopeStack.back()->purgeClassDeclarations();
        m_scopeStack.pop();
        updateScope();
    }

    bool addClass(const CXCursor &cursor, CodeModel::ClassType t);
    FunctionModelItem createFunction(const CXCursor &cursor,
                                     CodeModel::FunctionType t = CodeModel::Normal,
                                     bool isTemplateCode = false);
    FunctionModelItem createMemberFunction(const CXCursor &cursor,
                                           bool isTemplateCode = false);
    void qualifyConstructor(const CXCursor &cursor);
    TypeInfo createTypeInfoHelper(const CXType &type) const; // uncashed
    TypeInfo createTypeInfo(const CXType &type) const;
    TypeInfo createTypeInfo(const CXCursor &cursor) const
    { return createTypeInfo(clang_getCursorType(cursor)); }
    void addTemplateInstantiations(const CXType &type,
                                   QString *typeName,
                                   TypeInfo *t) const;
    bool addTemplateInstantiationsRecursion(const CXType &type, TypeInfo *t) const;

    void addTypeDef(const CXCursor &cursor, const CXType &cxType);
    void startTemplateTypeAlias(const CXCursor &cursor);
    void endTemplateTypeAlias(const CXCursor &typeAliasCursor);

    TemplateParameterModelItem createTemplateParameter(const CXCursor &cursor) const;
    TemplateParameterModelItem createNonTypeTemplateParameter(const CXCursor &cursor) const;
    void addField(const CXCursor &cursor);

    QString cursorValueExpression(BaseVisitor *bv, const CXCursor &cursor) const;
    void addBaseClass(const CXCursor &cursor);

    template <class Item>
    void qualifyTypeDef(const CXCursor &typeRefCursor, const QSharedPointer<Item> &item) const;

    bool visitHeader(const char *cFileName) const;

    void setFileName(const CXCursor &cursor, _CodeModelItem *item);

    BaseVisitor *m_baseVisitor;
    CodeModel *m_model;

    QStack<ScopeModelItem> m_scopeStack;
    QStringList m_scope;
    // Store all classes by cursor so that base classes can be found and inner
    // classes can be correctly parented in case of forward-declared inner classes
    // (QMetaObject::Connection)
    CursorClassHash m_cursorClassHash;
    CursorTypedefHash m_cursorTypedefHash;

    mutable TypeInfoHash m_typeInfoHash; // Cache type information
    mutable QHash<QString, TemplateTypeAliasModelItem> m_templateTypeAliases;

    ClassModelItem m_currentClass;
    EnumModelItem m_currentEnum;
    FunctionModelItem m_currentFunction;
    ArgumentModelItem m_currentArgument;
    VariableModelItem m_currentField;
    TemplateTypeAliasModelItem m_currentTemplateTypeAlias;
    QByteArrayList m_systemIncludes; // files, like "memory"
    QByteArrayList m_systemIncludePaths; // paths, like "/usr/include/Qt/"

    int m_anonymousEnumCount = 0;
    CodeModel::FunctionType m_currentFunctionType = CodeModel::Normal;
};

bool BuilderPrivate::addClass(const CXCursor &cursor, CodeModel::ClassType t)
{
    QString className = getCursorSpelling(cursor);
    m_currentClass.reset(new _ClassModelItem(m_model, className));
    setFileName(cursor, m_currentClass.data());
    m_currentClass->setClassType(t);
    // Some inner class? Note that it does not need to be (lexically) contained in a
    // class since it is possible to forward declare an inner class:
    // class QMetaObject { class Connection; }
    // class QMetaObject::Connection {}
    const CXCursor semPar = clang_getCursorSemanticParent(cursor);
    if (isClassCursor(semPar)) {
        const CursorClassHash::const_iterator it = m_cursorClassHash.constFind(semPar);
        if (it == m_cursorClassHash.constEnd()) {
            const QString message = QStringLiteral("Unable to find parent of inner class ") + className;
            const Diagnostic d(message, cursor, CXDiagnostic_Error);
            qWarning() << d;
            m_baseVisitor->appendDiagnostic(d);
            return false;
        }
        const ClassModelItem &containingClass = it.value();
        containingClass->addClass(m_currentClass);
        m_currentClass->setScope(containingClass->scope() << containingClass->name());
    } else {
        m_currentClass->setScope(m_scope);
        m_scopeStack.back()->addClass(m_currentClass);
    }
    pushScope(m_currentClass);
    m_cursorClassHash.insert(cursor, m_currentClass);
    return true;
}

static QString msgCannotDetermineException(const std::string_view &snippetV)
{
    const auto newLine = snippetV.find('\n'); // Multiline noexcept specifications have been found in Qt
    const bool truncate = newLine != std::string::npos;
    const qsizetype length = qsizetype(truncate ? newLine : snippetV.size());
    QString snippet = QString::fromUtf8(snippetV.data(), length);
    if (truncate)
        snippet += QStringLiteral("...");

    return QLatin1String("Cannot determine exception specification: \"")
        + snippet + QLatin1Char('"');
}

// Return whether noexcept(<value>) throws. noexcept() takes a constexpr value.
// Try to determine the simple cases (true|false) via code snippet.
static ExceptionSpecification computedExceptionSpecificationFromClang(BaseVisitor *bv,
                                                                      const CXCursor &cursor,
                                                                      bool isTemplateCode)
{
    const std::string_view snippet = bv->getCodeSnippet(cursor);
    if (snippet.empty())
        return ExceptionSpecification::Unknown; // Macro expansion, cannot tell
    if (snippet.find("noexcept(false)") != std::string::npos)
        return ExceptionSpecification::Throws;
    if (snippet.find("noexcept(true)") != std::string::npos)
        return ExceptionSpecification::NoExcept;
    // Warn about it unless it is some form of template code where it is common
    // to have complicated code, which is of no concern to shiboken, like:
    // "QList::emplace(T) noexcept(is_pod<T>)".
    if (!isTemplateCode && ReportHandler::isDebug(ReportHandler::FullDebug)) {
        const Diagnostic d(msgCannotDetermineException(snippet), cursor, CXDiagnostic_Warning);
        qWarning() << d;
        bv->appendDiagnostic(d);
    }
    return ExceptionSpecification::Unknown;
}

static ExceptionSpecification exceptionSpecificationFromClang(BaseVisitor *bv,
                                                              const CXCursor &cursor,
                                                              bool isTemplateCode)
{
    const auto ce = clang_getCursorExceptionSpecificationType(cursor);
    switch (ce) {
    case CXCursor_ExceptionSpecificationKind_ComputedNoexcept:
        return computedExceptionSpecificationFromClang(bv, cursor, isTemplateCode);
    case CXCursor_ExceptionSpecificationKind_BasicNoexcept:
    case CXCursor_ExceptionSpecificationKind_DynamicNone: // throw()
    case CXCursor_ExceptionSpecificationKind_NoThrow:
        return ExceptionSpecification::NoExcept;
    case CXCursor_ExceptionSpecificationKind_Dynamic: // throw(t1..)
    case CXCursor_ExceptionSpecificationKind_MSAny: // throw(...)
        return ExceptionSpecification::Throws;
    default:
        // CXCursor_ExceptionSpecificationKind_None,
        // CXCursor_ExceptionSpecificationKind_Unevaluated,
        // CXCursor_ExceptionSpecificationKind_Uninstantiated
        break;
    }
    return ExceptionSpecification::Unknown;
}

FunctionModelItem BuilderPrivate::createFunction(const CXCursor &cursor,
                                                 CodeModel::FunctionType t,
                                                 bool isTemplateCode)
{
    QString name = getCursorSpelling(cursor);
    // Apply type fixes to "operator X &" -> "operator X&"
    if (name.startsWith(QLatin1String("operator ")))
        name = fixTypeName(name);
    FunctionModelItem result(new _FunctionModelItem(m_model, name));
    setFileName(cursor, result.data());
    result->setType(createTypeInfoHelper(clang_getCursorResultType(cursor)));
    result->setFunctionType(t);
    result->setScope(m_scope);
    result->setStatic(clang_Cursor_getStorageClass(cursor) == CX_SC_Static);
    result->setExceptionSpecification(exceptionSpecificationFromClang(m_baseVisitor, cursor, isTemplateCode));
    switch (clang_getCursorAvailability(cursor)) {
    case CXAvailability_Available:
        break;
    case CXAvailability_Deprecated:
        result->setDeprecated(true);
        break;
    case CXAvailability_NotAvailable: // "Foo(const Foo&) = delete;"
        result->setDeleted(true);
        break;
    case CXAvailability_NotAccessible:
        break;
    }
    return result;
}

static inline CodeModel::FunctionType functionTypeFromCursor(const CXCursor &cursor)
{
    CodeModel::FunctionType result = CodeModel::Normal;
    switch (cursor.kind) {
    case CXCursor_Constructor:
        if (clang_CXXConstructor_isCopyConstructor(cursor) != 0)
            result = CodeModel::CopyConstructor;
        else if (clang_CXXConstructor_isMoveConstructor(cursor) != 0)
            result = CodeModel::MoveConstructor;
        else
            result = CodeModel::Constructor;
        break;
    case CXCursor_Destructor:
        result = CodeModel::Destructor;
        break;
    default:
        break;
    }
    return result;
}

FunctionModelItem BuilderPrivate::createMemberFunction(const CXCursor &cursor,
                                                       bool isTemplateCode)
{
    const CodeModel::FunctionType functionType =
        m_currentFunctionType == CodeModel::Signal || m_currentFunctionType == CodeModel::Slot
        ? m_currentFunctionType // by annotation
        : functionTypeFromCursor(cursor);
    isTemplateCode |= m_currentClass->name().endsWith(QLatin1Char('>'));
    auto result = createFunction(cursor, functionType, isTemplateCode);
    result->setAccessPolicy(accessPolicy(clang_getCXXAccessSpecifier(cursor)));
    result->setConstant(clang_CXXMethod_isConst(cursor) != 0);
    result->setStatic(clang_CXXMethod_isStatic(cursor) != 0);
    result->setVirtual(clang_CXXMethod_isVirtual(cursor) != 0);
    result->setAbstract(clang_CXXMethod_isPureVirtual(cursor) != 0);
    return result;
}

// For CXCursor_Constructor, on endToken().
void BuilderPrivate::qualifyConstructor(const CXCursor &cursor)
{
    // Clang does not tell us whether a constructor is explicit, preventing it
    // from being used for implicit conversions. Try to guess whether a
    // constructor is explicit in the C++99 sense (1 parameter) by checking for
    // isConvertingConstructor() == 0. Fixme: The notion of "isConvertingConstructor"
    // should be used in the code model instead of "explicit"
    if (clang_CXXConstructor_isDefaultConstructor(cursor) == 0
        && m_currentFunction->arguments().size() == 1
        && clang_CXXConstructor_isCopyConstructor(cursor) == 0
        && clang_CXXConstructor_isMoveConstructor(cursor) == 0) {
        m_currentFunction->setExplicit(clang_CXXConstructor_isConvertingConstructor(cursor) == 0);
    }
}

TemplateParameterModelItem BuilderPrivate::createTemplateParameter(const CXCursor &cursor) const
{
    return TemplateParameterModelItem(new _TemplateParameterModelItem(m_model, getCursorSpelling(cursor)));
}

TemplateParameterModelItem BuilderPrivate::createNonTypeTemplateParameter(const CXCursor &cursor) const
{
    TemplateParameterModelItem result = createTemplateParameter(cursor);
    result->setType(createTypeInfoHelper(clang_getCursorType(cursor)));
    return result;
}

// CXCursor_VarDecl, CXCursor_FieldDecl cursors
void BuilderPrivate::addField(const CXCursor &cursor)
{
    VariableModelItem field(new _VariableModelItem(m_model, getCursorSpelling(cursor)));
    field->setAccessPolicy(accessPolicy(clang_getCXXAccessSpecifier(cursor)));
    field->setScope(m_scope);
    field->setType(createTypeInfo(cursor));
    field->setMutable(clang_CXXField_isMutable(cursor) != 0);
    m_currentField = field;
    m_scopeStack.back()->addVariable(field);
}

// Create qualified name "std::list<std::string>" -> ("std", "list<std::string>")
static QStringList qualifiedName(const QString &t)
{
    QStringList result;
    int end = t.indexOf(QLatin1Char('<'));
    if (end == -1)
        end = t.indexOf(QLatin1Char('('));
    if (end == -1)
        end = t.size();
    int lastPos = 0;
    while (true) {
        const int nextPos = t.indexOf(colonColon(), lastPos);
        if (nextPos < 0 || nextPos >= end)
            break;
        result.append(t.mid(lastPos, nextPos - lastPos));
        lastPos = nextPos + 2;
    }
    result.append(t.right(t.size() - lastPos));
    return result;
}

static bool isArrayType(CXTypeKind k)
{
    return k == CXType_ConstantArray || k == CXType_IncompleteArray
        || k == CXType_VariableArray || k == CXType_DependentSizedArray;
}

static bool isPointerType(CXTypeKind k)
{
    return k == CXType_Pointer || k == CXType_LValueReference || k == CXType_RValueReference;
}

bool BuilderPrivate::addTemplateInstantiationsRecursion(const CXType &type, TypeInfo *t) const
{
    // Template arguments
    switch (type.kind) {
    case CXType_Elaborated:
    case CXType_Record:
    case CXType_Unexposed:
        if (const int numTemplateArguments = qMax(0, clang_Type_getNumTemplateArguments(type))) {
            for (unsigned tpl = 0; tpl < unsigned(numTemplateArguments); ++tpl) {
                const CXType argType = clang_Type_getTemplateArgumentAsType(type, tpl);
                // CXType_Invalid is returned when hitting on a specialization
                // of a non-type template (template <int v>).
                if (argType.kind == CXType_Invalid)
                    return false;
                t->addInstantiation(createTypeInfoHelper(argType));
            }
        }
        break;
    default:
        break;
    }
    return true;
}

static void dummyTemplateArgumentHandler(int, QStringView) {}

void BuilderPrivate::addTemplateInstantiations(const CXType &type,
                                               QString *typeName,
                                               TypeInfo *t) const
{
    // In most cases, for templates like "Vector<A>", Clang will give us the
    // arguments by recursing down the type. However this will fail for example
    // within template classes (for functions like the copy constructor):
    // template <class T>
    // class Vector {
    //    Vector(const Vector&);
    // };
    // In that case, have TypeInfo parse the list from the spelling.
    // Finally, remove the list "<>" from the type name.
    const bool parsed = addTemplateInstantiationsRecursion(type, t)
        && !t->instantiations().isEmpty();
    if (!parsed)
        t->setInstantiations({});
    const QPair<int, int> pos = parsed
        ? parseTemplateArgumentList(*typeName, dummyTemplateArgumentHandler)
        : t->parseTemplateArgumentList(*typeName);
    if (pos.first != -1 && pos.second != -1 && pos.second > pos.first)
        typeName->remove(pos.first, pos.second - pos.first);
}

TypeInfo BuilderPrivate::createTypeInfoHelper(const CXType &type) const
{
    if (type.kind == CXType_Pointer) { // Check for function pointers, first.
        const CXType pointeeType = clang_getPointeeType(type);
        const int argCount = clang_getNumArgTypes(pointeeType);
        if (argCount >= 0) {
            TypeInfo result = createTypeInfoHelper(clang_getResultType(pointeeType));
            result.setFunctionPointer(true);
            for (int a = 0; a < argCount; ++a)
                result.addArgument(createTypeInfoHelper(clang_getArgType(pointeeType, unsigned(a))));
            return result;
        }
    }

    TypeInfo typeInfo;

    CXType nestedType = type;
    for (; isArrayType(nestedType.kind); nestedType = clang_getArrayElementType(nestedType)) {
         const long long size = clang_getArraySize(nestedType);
         typeInfo.addArrayElement(size >= 0 ? QString::number(size) : QString());
    }

    TypeInfo::Indirections indirections;
    for (; isPointerType(nestedType.kind); nestedType = clang_getPointeeType(nestedType)) {
        switch (nestedType.kind) {
        case CXType_Pointer:
            indirections.prepend(clang_isConstQualifiedType(nestedType) != 0
                                 ? Indirection::ConstPointer : Indirection::Pointer);
            break;
        case CXType_LValueReference:
            typeInfo.setReferenceType(LValueReference);
            break;
        case CXType_RValueReference:
            typeInfo.setReferenceType(RValueReference);
            break;
        default:
            break;
        }
    }
    typeInfo.setIndirectionsV(indirections);

    typeInfo.setConstant(clang_isConstQualifiedType(nestedType) != 0);
    typeInfo.setVolatile(clang_isVolatileQualifiedType(nestedType) != 0);

    QString typeName = getTypeName(nestedType);
    while (TypeInfo::stripLeadingConst(&typeName)
           || TypeInfo::stripLeadingVolatile(&typeName)) {
    }

    // Obtain template instantiations if the name has '<' (thus excluding
    // typedefs like "std::string".
    if (typeName.contains(QLatin1Char('<')))
        addTemplateInstantiations(nestedType, &typeName, &typeInfo);

    typeInfo.setQualifiedName(qualifiedName(typeName));
    // 3320:CINDEX_LINKAGE int clang_getNumArgTypes(CXType T); function ptr types?
    typeInfo.simplifyStdType();
    return typeInfo;
}

TypeInfo BuilderPrivate::createTypeInfo(const CXType &type) const
{
    TypeInfoHash::iterator it = m_typeInfoHash.find(type);
    if (it == m_typeInfoHash.end())
        it = m_typeInfoHash.insert(type, createTypeInfoHelper(type));
    return it.value();
}

void BuilderPrivate::addTypeDef(const CXCursor &cursor, const CXType &cxType)
{
    const QString target = getCursorSpelling(cursor);
    TypeDefModelItem item(new _TypeDefModelItem(m_model, target));
    setFileName(cursor, item.data());
    item->setType(createTypeInfo(cxType));
    item->setScope(m_scope);
    m_scopeStack.back()->addTypeDef(item);
    m_cursorTypedefHash.insert(cursor, item);
}

void BuilderPrivate::startTemplateTypeAlias(const CXCursor &cursor)
{
    const QString target = getCursorSpelling(cursor);
    m_currentTemplateTypeAlias.reset(new _TemplateTypeAliasModelItem(m_model, target));
    setFileName(cursor, m_currentTemplateTypeAlias.data());
    m_currentTemplateTypeAlias->setScope(m_scope);
}

void BuilderPrivate::endTemplateTypeAlias(const CXCursor &typeAliasCursor)
{
    CXType type = clang_getTypedefDeclUnderlyingType(typeAliasCursor);
    // Usually "<elaborated>std::list<T>" or "<unexposed>Container1<T>",
    // as obtained with parser of PYSIDE-323
    if (type.kind == CXType_Unexposed || type.kind == CXType_Elaborated) {
        m_currentTemplateTypeAlias->setType(createTypeInfo(type));
        m_scopeStack.back()->addTemplateTypeAlias(m_currentTemplateTypeAlias);
    }
    m_currentTemplateTypeAlias.reset();
}

// extract an expression from the cursor via source
// CXCursor_EnumConstantDecl, ParmDecl (a = Flag1 | Flag2)
QString BuilderPrivate::cursorValueExpression(BaseVisitor *bv, const CXCursor &cursor) const
{
    const std::string_view snippet = bv->getCodeSnippet(cursor);
    auto equalSign = snippet.find('=');
    if (equalSign == std::string::npos)
        return QString();
    ++equalSign;
    return QString::fromLocal8Bit(snippet.data() + equalSign,
                                  qsizetype(snippet.size() - equalSign)).trimmed();
}

// Resolve declaration and type of a base class

struct TypeDeclaration
{
    CXType type;
    CXCursor declaration;
};

static TypeDeclaration resolveBaseSpecifier(const CXCursor &cursor)
{
    Q_ASSERT(clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier);
    CXType inheritedType = clang_getCursorType(cursor);
    CXCursor decl = clang_getTypeDeclaration(inheritedType);
    if (inheritedType.kind != CXType_Unexposed) {
        while (true) {
            auto kind = clang_getCursorKind(decl);
            if (kind != CXCursor_TypeAliasDecl && kind != CXCursor_TypedefDecl)
                break;
            inheritedType = clang_getTypedefDeclUnderlyingType(decl);
            decl = clang_getTypeDeclaration(inheritedType);
        }
    }
    return {inheritedType, decl};
}

// Add a base class to the current class from CXCursor_CXXBaseSpecifier
void BuilderPrivate::addBaseClass(const CXCursor &cursor)
{
    Q_ASSERT(clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier);
    // Note: spelling has "struct baseClass", use type
    QString baseClassName;
    const auto decl = resolveBaseSpecifier(cursor);
    if (decl.type.kind == CXType_Unexposed) {
        // The type is unexposed when the base class is a template type alias:
        // "class QItemSelection : public QList<X>" where QList is aliased to QVector.
        // Try to resolve via code model.
        TypeInfo info = createTypeInfo(decl.type);
        auto parentScope = m_scopeStack.at(m_scopeStack.size() - 2); // Current is class.
        auto resolved = TypeInfo::resolveType(info, parentScope);
        if (resolved != info)
            baseClassName = resolved.toString();
    }
    if (baseClassName.isEmpty())
        baseClassName = getTypeName(decl.type);

    auto it = m_cursorClassHash.constFind(decl.declaration);
    const CodeModel::AccessPolicy access = accessPolicy(clang_getCXXAccessSpecifier(cursor));
    if (it == m_cursorClassHash.constEnd()) {
        // Set unqualified name. This happens in cases like "class X : public std::list<...>"
        // "template<class T> class Foo : public T" and standard types like true_type, false_type.
        m_currentClass->addBaseClass(baseClassName, access);
        return;
    }
    // Completely qualify the class name by looking it up and taking its scope
    // plus the actual baseClass stripped off any scopes. Consider:
    //   namespace std {
    //   template <class T> class vector {};
    //   namespace n {
    //      class Foo : public vector<int> {};
    //   }
    //   }
    // should have "std::vector<int>" as base class (whereas the type of the base class is
    // "std::vector<T>").
    const QStringList &baseScope = it.value()->scope();
    if (!baseScope.isEmpty()) {
        const int lastSep = baseClassName.lastIndexOf(colonColon());
        if (lastSep >= 0)
            baseClassName.remove(0, lastSep + colonColon().size());
        baseClassName.prepend(colonColon());
        baseClassName.prepend(baseScope.join(colonColon()));
    }
    m_currentClass->addBaseClass(baseClassName, access);
}

static inline CXCursor definitionFromTypeRef(const CXCursor &typeRefCursor)
{
    Q_ASSERT(typeRefCursor.kind == CXCursor_TypeRef);
    return clang_getTypeDeclaration(clang_getCursorType(typeRefCursor));
}

// Qualify function arguments or fields that are typedef'ed from another scope:
// enum ConversionFlag {};
// typedef QFlags<ConversionFlag> ConversionFlags;
// class QTextCodec {
//      enum ConversionFlag {};
//      typedef QFlags<ConversionFlag> ConversionFlags;
//      struct ConverterState {
//          explicit ConverterState(ConversionFlags);
//                                  ^^ qualify to QTextCodec::ConversionFlags
//          ConversionFlags m_flags;
//                          ^^ ditto

template <class Item> // ArgumentModelItem, VariableModelItem
void BuilderPrivate::qualifyTypeDef(const CXCursor &typeRefCursor, const QSharedPointer<Item> &item) const
{
    TypeInfo type = item->type();
    if (type.qualifiedName().size() == 1) { // item's type is unqualified.
        const auto it = m_cursorTypedefHash.constFind(definitionFromTypeRef(typeRefCursor));
        if (it != m_cursorTypedefHash.constEnd() && !it.value()->scope().isEmpty()) {
            type.setQualifiedName(it.value()->scope() + type.qualifiedName());
            item->setType(type);
        }
    }
}

void BuilderPrivate::setFileName(const CXCursor &cursor, _CodeModelItem *item)
{
    const SourceRange range = getCursorRange(cursor);
    QString file = m_baseVisitor->getFileName(range.first.file);
    if (!file.isEmpty()) { // Has been observed to be 0 for invalid locations
        item->setFileName(QDir::cleanPath(file));
        item->setStartPosition(int(range.first.line), int(range.first.column));
        item->setEndPosition(int(range.second.line), int(range.second.column));
    }
}

Builder::Builder()
{
    d = new BuilderPrivate(this);
}

Builder::~Builder()
{
    delete d;
}

static const char *cBaseName(const char *fileName)
{
    const char *lastSlash = std::strrchr(fileName, '/');
#ifdef Q_OS_WIN
    if (lastSlash == nullptr)
        lastSlash = std::strrchr(fileName, '\\');
#endif
    return lastSlash != nullptr ? (lastSlash + 1) : fileName;
}

static inline bool cCompareFileName(const char *f1, const char *f2)
{
#ifdef Q_OS_WIN
   return _stricmp(f1, f2) == 0;
#else
    return std::strcmp(f1, f2) == 0;
#endif
}

#ifdef Q_OS_UNIX
template<size_t N>
static bool cStringStartsWith(const char *str, const char (&prefix)[N])
{
    return std::strncmp(prefix, str, N - 1) == 0;
}
#endif

static bool cStringStartsWith(const char *str, const QByteArray &prefix)
{
    return std::strncmp(prefix.constData(), str, int(prefix.size())) == 0;
}

bool BuilderPrivate::visitHeader(const char *cFileName) const
{
    // Resolve OpenGL typedefs although the header is considered a system header.
    const char *baseName = cBaseName(cFileName);
    if (cCompareFileName(baseName, "gl.h"))
        return true;
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    if (cStringStartsWith(cFileName, "/usr/include/stdint.h"))
        return true;
#endif
#ifdef Q_OS_LINUX
    if (cStringStartsWith(cFileName, "/usr/include/stdlib.h")
        || cStringStartsWith(cFileName, "/usr/include/sys/types.h")) {
        return true;
    }
#endif // Q_OS_LINUX
#ifdef Q_OS_MACOS
    // Parse the following system headers to get the correct typdefs for types like
    // int32_t, which are used in the macOS implementation of OpenGL framework.
    if (cCompareFileName(baseName, "gltypes.h")
        || cStringStartsWith(cFileName, "/usr/include/_types")
        || cStringStartsWith(cFileName, "/usr/include/_types")
        || cStringStartsWith(cFileName, "/usr/include/sys/_types")) {
        return true;
    }
#endif // Q_OS_MACOS
    if (baseName) {
        for (const auto &systemInclude : m_systemIncludes) {
            if (systemInclude == baseName)
                return true;
        }
    }
    for (const auto &systemIncludePath : m_systemIncludePaths) {
        if (cStringStartsWith(cFileName, systemIncludePath))
            return true;
    }
    return false;
}

bool Builder::visitLocation(const CXSourceLocation &location) const
{
    if (clang_Location_isInSystemHeader(location) == 0)
        return true;
    CXFile file; // void *
    unsigned line;
    unsigned column;
    unsigned offset;
    clang_getExpansionLocation(location, &file, &line, &column, &offset);
    const CXString cxFileName = clang_getFileName(file);
    // Has been observed to be 0 for invalid locations
    bool result = false;
    if (const char *cFileName = clang_getCString(cxFileName)) {
        result = d->visitHeader(cFileName);
        clang_disposeString(cxFileName);
    }
    return result;
}

void Builder::setSystemIncludes(const QByteArrayList &systemIncludes)
{
    for (const auto &i : systemIncludes) {
        if (i.endsWith('/'))
            d->m_systemIncludePaths.append(i);
        else
            d->m_systemIncludes.append(i);
    }
}

FileModelItem Builder::dom() const
{
    Q_ASSERT(!d->m_scopeStack.isEmpty());
    auto rootScope = d->m_scopeStack.constFirst();
    rootScope->purgeClassDeclarations();
    return qSharedPointerDynamicCast<_FileModelItem>(rootScope);
}

static QString msgOutOfOrder(const CXCursor &cursor, const char *expectedScope)
{
    return getCursorKindName(cursor.kind) + QLatin1Char(' ')
        + getCursorSpelling(cursor) + QLatin1String(" encountered outside ")
        + QLatin1String(expectedScope) + QLatin1Char('.');
}

static CodeModel::ClassType codeModelClassTypeFromCursor(CXCursorKind kind)
{
    CodeModel::ClassType result = CodeModel::Class;
    if (kind == CXCursor_UnionDecl)
        result = CodeModel::Union;
    else if (kind == CXCursor_StructDecl)
        result = CodeModel::Struct;
    return result;
}

static NamespaceType namespaceType(const CXCursor &cursor)
{
    if (clang_Cursor_isAnonymous(cursor))
        return NamespaceType::Anonymous;
#if CINDEX_VERSION_MAJOR > 0 || CINDEX_VERSION_MINOR >= 59
    if (clang_Cursor_isInlineNamespace(cursor))
        return NamespaceType::Inline;
#endif
    return NamespaceType::Default;
}

static QString enumType(const CXCursor &cursor)
{
    QString name = getCursorSpelling(cursor); // "enum Foo { v1, v2 };"
    if (name.isEmpty()) {
        // PYSIDE-1228: For "typedef enum { v1, v2 } Foo;", type will return
        // "Foo" as expected. Care must be taken to exclude real anonymous enums.
        name = getTypeName(clang_getCursorType(cursor));
        if (name.contains(QLatin1String("(anonymous")))
            name.clear();
    }
    return name;
}

BaseVisitor::StartTokenResult Builder::startToken(const CXCursor &cursor)
{
    switch (cursor.kind) {
    case CXCursor_CXXAccessSpecifier:
        d->m_currentFunctionType = CodeModel::Normal;
        break;
    case CXCursor_AnnotateAttr: {
        const QString annotation = getCursorSpelling(cursor);
        if (annotation == QLatin1String("qt_slot"))
            d->m_currentFunctionType = CodeModel::Slot;
        else if (annotation == QLatin1String("qt_signal"))
            d->m_currentFunctionType = CodeModel::Signal;
        else
            d->m_currentFunctionType = CodeModel::Normal;
    }
        break;
    case CXCursor_CXXBaseSpecifier:
        if (d->m_currentClass.isNull()) {
            const Diagnostic d(msgOutOfOrder(cursor, "class"), cursor, CXDiagnostic_Error);
            qWarning() << d;
            appendDiagnostic(d);
            return Error;
        }
        d->addBaseClass(cursor);
        break;
    case CXCursor_ClassDecl:
    case CXCursor_UnionDecl:
    case CXCursor_StructDecl:
        if (clang_isCursorDefinition(cursor) == 0)
            return Skip;
        if (!d->addClass(cursor, codeModelClassTypeFromCursor(cursor.kind)))
            return Error;
        break;
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
        if (clang_isCursorDefinition(cursor) == 0)
            return Skip;
        d->addClass(cursor, CodeModel::Class);
        d->m_currentClass->setName(d->m_currentClass->name() + templateBrackets());
        d->m_scope.back() += templateBrackets();
        break;
    case CXCursor_EnumDecl: {
        QString name = enumType(cursor);
        EnumKind kind = CEnum;
        if (name.isEmpty()) {
            kind = AnonymousEnum;
            name = QStringLiteral("enum_") + QString::number(++d->m_anonymousEnumCount);
#if !CLANG_NO_ENUMDECL_ISSCOPED
        } else if (clang_EnumDecl_isScoped(cursor) != 0) {
#else
        } else if (clang_EnumDecl_isScoped4(this, cursor) != 0) {
#endif
            kind = EnumClass;
        }
        d->m_currentEnum.reset(new _EnumModelItem(d->m_model, name));
        d->setFileName(cursor, d->m_currentEnum.data());
        d->m_currentEnum->setScope(d->m_scope);
        d->m_currentEnum->setEnumKind(kind);
        d->m_currentEnum->setSigned(isSigned(clang_getEnumDeclIntegerType(cursor).kind));
        if (!qSharedPointerDynamicCast<_ClassModelItem>(d->m_scopeStack.back()).isNull())
            d->m_currentEnum->setAccessPolicy(accessPolicy(clang_getCXXAccessSpecifier(cursor)));
    }
        break;
    case CXCursor_EnumConstantDecl: {
        const QString name = getCursorSpelling(cursor);
        if (d->m_currentEnum.isNull()) {
            const Diagnostic d(msgOutOfOrder(cursor, "enum"), cursor, CXDiagnostic_Error);
            qWarning() << d;
            appendDiagnostic(d);
            return Error;
        }
        EnumValue enumValue;
        if (d->m_currentEnum->isSigned())
            enumValue.setValue(clang_getEnumConstantDeclValue(cursor));
        else
            enumValue.setUnsignedValue(clang_getEnumConstantDeclUnsignedValue(cursor));
        EnumeratorModelItem enumConstant(new _EnumeratorModelItem(d->m_model, name));
        enumConstant->setStringValue(d->cursorValueExpression(this, cursor));
        enumConstant->setValue(enumValue);
        d->m_currentEnum->addEnumerator(enumConstant);
    }
        break;
    case CXCursor_VarDecl:
        // static class members are seen as CXCursor_VarDecl
        if (!d->m_currentClass.isNull() && isClassCursor(clang_getCursorSemanticParent(cursor))) {
             d->addField(cursor);
             d->m_currentField->setStatic(true);
        }
        break;
    case CXCursor_FieldDecl:
        d->addField(cursor);
        break;
#if CINDEX_VERSION_MAJOR > 0 || CINDEX_VERSION_MINOR >= 37 // Clang 4.0
    case CXCursor_FriendDecl:
        return Skip;
#endif
    case CXCursor_Constructor:
    case CXCursor_Destructor: // Note: Also use clang_CXXConstructor_is..Constructor?
    case CXCursor_CXXMethod:
    case CXCursor_ConversionFunction:
        // Skip inline member functions outside class, only go by declarations inside class
        if (!withinClassDeclaration(cursor))
            return Skip;
        d->m_currentFunction = d->createMemberFunction(cursor, false);
        d->m_scopeStack.back()->addFunction(d->m_currentFunction);
        break;
    // Not fully supported, currently, seen as normal function
    // Note: May appear inside class (member template) or outside (free template).
    case CXCursor_FunctionTemplate: {
        const CXCursor semParent = clang_getCursorSemanticParent(cursor);
        if (isClassCursor(semParent)) {
            if (semParent == clang_getCursorLexicalParent(cursor)) {
                d->m_currentFunction = d->createMemberFunction(cursor, true);
                d->m_scopeStack.back()->addFunction(d->m_currentFunction);
                break;
            }
            return Skip; // inline member functions outside class
        }
    }
        d->m_currentFunction = d->createFunction(cursor, CodeModel::Normal, true);
        d->m_scopeStack.back()->addFunction(d->m_currentFunction);
        break;
    case CXCursor_FunctionDecl:
        d->m_currentFunction = d->createFunction(cursor, CodeModel::Normal, false);
        d->m_scopeStack.back()->addFunction(d->m_currentFunction);
        break;
    case CXCursor_Namespace: {
        const auto type = namespaceType(cursor);
        if (type == NamespaceType::Anonymous)
            return Skip;
        const QString name = getCursorSpelling(cursor);
        const NamespaceModelItem parentNamespaceItem = qSharedPointerDynamicCast<_NamespaceModelItem>(d->m_scopeStack.back());
        if (parentNamespaceItem.isNull()) {
            const QString message = msgOutOfOrder(cursor, "namespace")
                + QLatin1String(" (current scope: ") + d->m_scopeStack.back()->name() + QLatin1Char(')');
            const Diagnostic d(message, cursor, CXDiagnostic_Error);
            qWarning() << d;
            appendDiagnostic(d);
            return Error;
        }
        // Treat namespaces separately to allow for extending namespaces
        // in subsequent modules.
        NamespaceModelItem namespaceItem = parentNamespaceItem->findNamespace(name);
        namespaceItem.reset(new _NamespaceModelItem(d->m_model, name));
        d->setFileName(cursor, namespaceItem.data());
        namespaceItem->setScope(d->m_scope);
        namespaceItem->setType(type);
        parentNamespaceItem->addNamespace(namespaceItem);
        d->pushScope(namespaceItem);
    }
        break;
    case CXCursor_ParmDecl:
        // Skip in case of nested CXCursor_ParmDecls in case one parameter is a function pointer
        // and function pointer typedefs.
        if (d->m_currentArgument.isNull() && !d->m_currentFunction.isNull()) {
            const QString name = getCursorSpelling(cursor);
            d->m_currentArgument.reset(new _ArgumentModelItem(d->m_model, name));
            d->m_currentArgument->setType(d->createTypeInfo(cursor));
            d->m_currentFunction->addArgument(d->m_currentArgument);
            QString defaultValueExpression = d->cursorValueExpression(this, cursor);
            if (!defaultValueExpression.isEmpty()) {
                d->m_currentArgument->setDefaultValueExpression(defaultValueExpression);
                d->m_currentArgument->setDefaultValue(true);
            }
        } else {
            return Skip;
        }
        break;
    case CXCursor_TemplateTypeParameter:
    case CXCursor_NonTypeTemplateParameter: {
        const TemplateParameterModelItem tItem = cursor.kind == CXCursor_TemplateTemplateParameter
            ? d->createTemplateParameter(cursor) : d->createNonTypeTemplateParameter(cursor);
        // Apply to function/member template?
        if (!d->m_currentFunction.isNull()) {
            d->m_currentFunction->setTemplateParameters(d->m_currentFunction->templateParameters() << tItem);
        } else if (!d->m_currentTemplateTypeAlias.isNull()) {
            d->m_currentTemplateTypeAlias->addTemplateParameter(tItem);
        } else if (!d->m_currentClass.isNull()) { // Apply to class
            const QString &tplParmName = tItem->name();
            if (Q_UNLIKELY(!insertTemplateParameterIntoClassName(tplParmName, d->m_currentClass)
                           || !insertTemplateParameterIntoClassName(tplParmName, &d->m_scope.back()))) {
                const QString message = QStringLiteral("Error inserting template parameter \"") + tplParmName
                        + QStringLiteral("\" into ") + d->m_currentClass->name();
                const Diagnostic d(message, cursor, CXDiagnostic_Error);
                qWarning() << d;
                appendDiagnostic(d);
                return Error;
            }
            d->m_currentClass->setTemplateParameters(d->m_currentClass->templateParameters() << tItem);
        }
    }
        break;
    case CXCursor_TypeAliasTemplateDecl:
        d->startTemplateTypeAlias(cursor);
        break;
    case CXCursor_TypeAliasDecl: // May contain nested CXCursor_TemplateTypeParameter
        if (d->m_currentTemplateTypeAlias.isNull()) {
            const CXType type = clang_getCanonicalType(clang_getCursorType(cursor));
            if (type.kind > CXType_Unexposed)
                d->addTypeDef(cursor, type);
            return Skip;
        } else {
            d->endTemplateTypeAlias(cursor);
        }
        break;
    case CXCursor_TypedefDecl: {
        auto underlyingType = clang_getTypedefDeclUnderlyingType(cursor);
        d->addTypeDef(cursor, underlyingType);
        // For "typedef enum/struct {} Foo;", skip the enum/struct
        // definition nested into the typedef (PYSIDE-1228).
        if (underlyingType.kind == CXType_Elaborated)
            return Skip;
    }
        break;
    case CXCursor_TypeRef:
        if (!d->m_currentFunction.isNull()) {
            if (d->m_currentArgument.isNull())
                d->qualifyTypeDef(cursor, d->m_currentFunction); // return type
            else
                d->qualifyTypeDef(cursor, d->m_currentArgument);
        } else if (!d->m_currentField.isNull()) {
            d->qualifyTypeDef(cursor, d->m_currentField);
        }
        break;
    case CXCursor_CXXFinalAttr:
         if (!d->m_currentFunction.isNull())
             d->m_currentFunction->setFinal(true);
         else if (!d->m_currentClass.isNull())
             d->m_currentClass->setFinal(true);
        break;
    case CXCursor_CXXOverrideAttr:
        if (!d->m_currentFunction.isNull())
            d->m_currentFunction->setOverride(true);
        break;
    case CXCursor_StaticAssert:
        // Check for Q_PROPERTY() (see PySide2/global.h.in for an explanation
        // how it is defined, and qdoc).
        if (clang_isDeclaration(cursor.kind) && !d->m_currentClass.isNull()) {
            auto snippet = getCodeSnippet(cursor);
            const auto length = snippet.size();
            if (length > 12 && *snippet.rbegin() == ')'
                && snippet.compare(0, 11, "Q_PROPERTY(") == 0) {
                const QString qProperty = QString::fromUtf8(snippet.data() + 11, length - 12);
                d->m_currentClass->addPropertyDeclaration(qProperty);
            }
        }
        break;
    default:
        break;
    }
    return BaseVisitor::Recurse;
}

bool Builder::endToken(const CXCursor &cursor)
{
    switch (cursor.kind) {
    case CXCursor_UnionDecl:
    case CXCursor_ClassDecl:
    case CXCursor_StructDecl:
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
        d->popScope();
        // Continue in outer class after leaving inner class?
        if (ClassModelItem lastClass = qSharedPointerDynamicCast<_ClassModelItem>(d->m_scopeStack.back()))
            d->m_currentClass = lastClass;
        else
            d->m_currentClass.clear();
        d->m_currentFunctionType = CodeModel::Normal;
        break;
    case CXCursor_EnumDecl:
        // Add enum only if values were encountered, otherwise assume it
        // is a forward declaration of an enum class.
        if (!d->m_currentEnum.isNull() && d->m_currentEnum->hasValues())
            d->m_scopeStack.back()->addEnum(d->m_currentEnum);
        d->m_currentEnum.clear();
        break;
    case CXCursor_VarDecl:
    case CXCursor_FieldDecl:
        d->m_currentField.clear();
        break;
    case CXCursor_Constructor:
        d->qualifyConstructor(cursor);
        d->m_currentFunction.clear();
        break;
    case CXCursor_Destructor:
    case CXCursor_CXXMethod:
    case CXCursor_FunctionDecl:
    case CXCursor_FunctionTemplate:
        d->m_currentFunction.clear();
        break;
    case CXCursor_Namespace:
        d->popScope();
        break;
    case CXCursor_ParmDecl:
        d->m_currentArgument.clear();
        break;
    case CXCursor_TypeAliasTemplateDecl:
        d->m_currentTemplateTypeAlias.reset();
        break;
    default:
        break;
    }
    return true;
}

} // namespace clang
