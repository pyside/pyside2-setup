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

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtCore/QVector>

#include <string.h>
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

static void setFileName(const CXCursor &cursor, _CodeModelItem *item)
{
    const SourceRange range = getCursorRange(cursor);
    if (!range.first.file.isEmpty()) { // Has been observed to be 0 for invalid locations
        item->setFileName(QDir::cleanPath(range.first.file));
        item->setStartPosition(int(range.first.line), int(range.first.column));
        item->setEndPosition(int(range.second.line), int(range.second.column));
    }
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
    typedef QHash<CXCursor, ClassModelItem> CursorClassHash;
    typedef QHash<CXCursor, TypeDefModelItem> CursorTypedefHash;

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
        m_scopeStack.pop();
        updateScope();
    }

    bool addClass(const CXCursor &cursor, CodeModel::ClassType t);
    FunctionModelItem createFunction(const CXCursor &cursor,
                                     CodeModel::FunctionType t = CodeModel::Normal) const;
    FunctionModelItem createMemberFunction(const CXCursor &cursor) const;
    void qualifyConstructor(const CXCursor &cursor);
    TypeInfo createTypeInfo(const CXType &type) const;
    TypeInfo createTypeInfo(const CXCursor &cursor) const
    { return createTypeInfo(clang_getCursorType(cursor)); }

    TemplateParameterModelItem createTemplateParameter(const CXCursor &cursor) const;
    TemplateParameterModelItem createNonTypeTemplateParameter(const CXCursor &cursor) const;
    void addField(const CXCursor &cursor);

    QString cursorValueExpression(BaseVisitor *bv, const CXCursor &cursor) const;
    void addBaseClass(const CXCursor &cursor);

    template <class Item>
    void qualifyTypeDef(const CXCursor &typeRefCursor, const QSharedPointer<Item> &item) const;

    BaseVisitor *m_baseVisitor;
    CodeModel *m_model;

    QStack<ScopeModelItem> m_scopeStack;
    QStringList m_scope;
    // Store all classes by cursor so that base classes can be found and inner
    // classes can be correctly parented in case of forward-declared inner classes
    // (QMetaObject::Connection)
    CursorClassHash m_cursorClassHash;
    CursorTypedefHash m_cursorTypedefHash;

    ClassModelItem m_currentClass;
    EnumModelItem m_currentEnum;
    FunctionModelItem m_currentFunction;
    ArgumentModelItem m_currentArgument;
    VariableModelItem m_currentField;

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

FunctionModelItem BuilderPrivate::createFunction(const CXCursor &cursor,
                                                 CodeModel::FunctionType t) const
{
    QString name = getCursorSpelling(cursor);
    // Apply type fixes to "operator X &" -> "operator X&"
    if (name.startsWith(QLatin1String("operator ")))
        name = fixTypeName(name);
    FunctionModelItem result(new _FunctionModelItem(m_model, name));
    setFileName(cursor, result.data());
    result->setType(createTypeInfo(clang_getCursorResultType(cursor)));
    result->setFunctionType(t);
    result->setScope(m_scope);
    result->setStatic(clang_Cursor_getStorageClass(cursor) == CX_SC_Static);
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

FunctionModelItem BuilderPrivate::createMemberFunction(const CXCursor &cursor) const
{
    const CodeModel::FunctionType functionType =
        m_currentFunctionType == CodeModel::Signal || m_currentFunctionType == CodeModel::Slot
        ? m_currentFunctionType // by annotation
        : functionTypeFromCursor(cursor);
    FunctionModelItem result = createFunction(cursor, functionType);
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
    result->setType(createTypeInfo(cursor));
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

// Array helpers: Parse "a[2][4]" into a list of dimensions

struct ArrayDimensionResult
{
    QVector<QStringRef> dimensions;
    int position;
};

static ArrayDimensionResult arrayDimensions(const QString &typeName)
{
    ArrayDimensionResult result;
    result.position = typeName.indexOf(QLatin1Char('['));
    for (int openingPos = result.position; openingPos != -1; ) {
        const int closingPos = typeName.indexOf(QLatin1Char(']'), openingPos + 1);
        if (closingPos == -1)
            break;
        result.dimensions.append(typeName.midRef(openingPos + 1, closingPos - openingPos - 1));
        openingPos = typeName.indexOf(QLatin1Char('['), closingPos + 1);
    }
    return result;
}

// Array helpers: Parse "a[2][4]" into a list of dimensions or "" for none
static QStringList parseArrayArgs(const CXType &type, QString *typeName)
{
    const ArrayDimensionResult dimensions = arrayDimensions(*typeName);
    Q_ASSERT(!dimensions.dimensions.isEmpty());

    QStringList result;
    // get first dimension from clang, preferably.
    // "a[]" is seen as pointer by Clang, set special indicator ""
    const long long size = clang_getArraySize(type);
    result.append(size >= 0 ? QString::number(size) : QString());
    // Parse out remaining dimensions
    for (int i = 1, count = dimensions.dimensions.size(); i < count; ++i)
        result.append(dimensions.dimensions.at(i).toString());
    typeName->truncate(dimensions.position);
    return result;
}

TypeInfo BuilderPrivate::createTypeInfo(const CXType &type) const
{
    if (type.kind == CXType_Pointer) { // Check for function pointers, first.
        const CXType pointeeType = clang_getPointeeType(type);
        const int argCount = clang_getNumArgTypes(pointeeType);
        if (argCount >= 0) {
            TypeInfo result = createTypeInfo(clang_getResultType(pointeeType));
            result.setFunctionPointer(true);
            for (int a = 0; a < argCount; ++a)
                result.addArgument(createTypeInfo(clang_getArgType(pointeeType, unsigned(a))));
            return result;
        }
    }

    TypeInfo typeInfo;
    QString typeName = fixTypeName(getTypeName(type));

    int indirections = 0;
    // "int **"
    for ( ; typeName.endsWith(QLatin1Char('*')) ; ++indirections)
        typeName.chop(1);
    typeInfo.setIndirections(indirections);
    // "int &&"
    if (typeName.endsWith(QLatin1String("&&"))) {
        typeName.chop(2);
        typeInfo.setReferenceType(RValueReference);
    } else if (typeName.endsWith(QLatin1Char('&'))) { // "int &"
        typeName.chop(1);
        typeInfo.setReferenceType(LValueReference);
    }

    // "int [3], int[]"
    if (type.kind == CXType_ConstantArray || type.kind == CXType_IncompleteArray
        || type.kind == CXType_VariableArray || type.kind == CXType_DependentSizedArray) {
         typeInfo.setArrayElements(parseArrayArgs(type, &typeName));
    }

    bool isConstant = clang_isConstQualifiedType(type) != 0;
    // A "char *const" parameter, is considered to be const-qualified by Clang, but
    // not in the TypeInfo sense (corresponds to "char *" and not "const char *").
    if (type.kind == CXType_Pointer && isConstant && typeName.endsWith(QLatin1String("const"))) {
        typeName.chop(5);
        typeName = typeName.trimmed();
        isConstant = false;
    }
    // Clang has been observed to return false for "const int .."
    if (!isConstant && typeName.startsWith(QLatin1String("const "))) {
        typeName.remove(0, 6);
        isConstant = true;
    }
    typeInfo.setConstant(isConstant);

    // clang_isVolatileQualifiedType() returns true for "volatile int", but not for "volatile int *"
    if (typeName.startsWith(QLatin1String("volatile "))) {
        typeName.remove(0, 9);
        typeInfo.setVolatile(true);
    }

    typeName = typeName.trimmed();

    typeInfo.setQualifiedName(typeName.split(colonColon()));
    // 3320:CINDEX_LINKAGE int clang_getNumArgTypes(CXType T); function ptr types?
    return typeInfo;
}

// extract an expression from the cursor via source
// CXCursor_EnumConstantDecl, ParmDecl (a = Flag1 | Flag2)
QString BuilderPrivate::cursorValueExpression(BaseVisitor *bv, const CXCursor &cursor) const
{
    BaseVisitor::CodeSnippet snippet = bv->getCodeSnippet(cursor);
    const char *equalSign = std::find(snippet.first, snippet.second, '=');
    if (equalSign == snippet.second)
        return QString();
    ++equalSign;
    return QString::fromLocal8Bit(equalSign, int(snippet.second - equalSign)).trimmed();
}

// A hacky reimplementation of clang_EnumDecl_isScoped() for Clang < 5.0
// which simply checks for a blank-delimited " class " keyword in the enum snippet.

#define CLANG_NO_ENUMDECL_ISSCOPED \
    (CINDEX_VERSION_MAJOR == 0 && CINDEX_VERSION_MINOR < 43)

#if CLANG_NO_ENUMDECL_ISSCOPED
static const char *indexOf(const BaseVisitor::CodeSnippet &snippet, const char *needle)
{
    const size_t snippetLength = snippet.first ? size_t(snippet.second - snippet.first) : 0;
    const size_t needleLength = strlen(needle);
    if (needleLength > snippetLength)
        return nullptr;
    for (const char *c = snippet.first, *end = snippet.second - needleLength; c < end; ++c) {
        if (memcmp(c, needle, needleLength) == 0)
            return c;
    }
    return nullptr;
}

long clang_EnumDecl_isScoped4(BaseVisitor *bv, const CXCursor &cursor)
{
    BaseVisitor::CodeSnippet snippet = bv->getCodeSnippet(cursor);
    const char *classSpec = indexOf(snippet, "class");
    const bool isClass = classSpec && classSpec > snippet.first
        && isspace(*(classSpec - 1)) && isspace(*(classSpec + 5));
    return isClass ? 1 : 0;
}
#endif // CLANG_NO_ENUMDECL_ISSCOPED

// Add a base class to the current class from CXCursor_CXXBaseSpecifier
void BuilderPrivate::addBaseClass(const CXCursor &cursor)
{
    const CXType inheritedType = clang_getCursorType(cursor); // Note spelling has "struct baseClass",
    QString baseClassName = getTypeName(inheritedType);       // use type.
    const CXCursor declCursor = clang_getTypeDeclaration(inheritedType);
    const CursorClassHash::const_iterator it = m_cursorClassHash.constFind(declCursor);
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
    typedef typename CursorTypedefHash::const_iterator ConstIt;

    TypeInfo type = item->type();
    if (type.qualifiedName().size() == 1) { // item's type is unqualified.
        const ConstIt it = m_cursorTypedefHash.constFind(definitionFromTypeRef(typeRefCursor));
        if (it != m_cursorTypedefHash.constEnd() && !it.value()->scope().isEmpty()) {
            type.setQualifiedName(it.value()->scope() + type.qualifiedName());
            item->setType(type);
        }
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

static inline bool compareHeaderName(const char *haystack, const char *needle)
{
    const char *lastSlash = strrchr(haystack, '/');
#ifdef Q_OS_WIN
    if (!lastSlash)
        lastSlash = strrchr(haystack, '\\');
#endif
    if (!lastSlash)
        lastSlash = haystack;
    else
        ++lastSlash;
#ifdef Q_OS_WIN
   return _stricmp(lastSlash, needle) == 0;
#else
    return strcmp(lastSlash, needle) == 0;
#endif
}

#ifdef Q_OS_UNIX
static bool cStringStartsWith(const char *prefix, const char *str)
{
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
#endif

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
    if (const char *cFileName = clang_getCString(cxFileName)) {
        // Resolve OpenGL typedefs although the header is considered a system header.
        const bool visitHeader = compareHeaderName(cFileName, "gl.h")
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
                || cStringStartsWith("/usr/include/stdint.h", cFileName)
#endif
#if defined(Q_OS_LINUX)
                || cStringStartsWith("/usr/include/stdlib.h", cFileName)
                || cStringStartsWith("/usr/include/sys/types.h", cFileName)
#elif defined(Q_OS_MACOS)
                // Parse the following system headers to get the correct typdefs for types like
                // int32_t, which are used in the macOS implementation of OpenGL framework.
                || compareHeaderName(cFileName, "gltypes.h")
                || cStringStartsWith("/usr/include/_types", cFileName)
                || cStringStartsWith("/usr/include/sys/_types", cFileName)
#endif
                ;
        clang_disposeString(cxFileName);
        if (visitHeader)
            return true;
    }
    return false;
}

FileModelItem Builder::dom() const
{
    Q_ASSERT(!d->m_scopeStack.isEmpty());
    return qSharedPointerDynamicCast<_FileModelItem>(d->m_scopeStack.constFirst());
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
        QString name = getCursorSpelling(cursor);
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
        setFileName(cursor, d->m_currentEnum.data());
        d->m_currentEnum->setScope(d->m_scope);
        d->m_currentEnum->setEnumKind(kind);
        d->m_currentEnum->setSigned(isSigned(clang_getEnumDeclIntegerType(cursor).kind));
        if (!qSharedPointerDynamicCast<_ClassModelItem>(d->m_scopeStack.back()).isNull())
            d->m_currentEnum->setAccessPolicy(accessPolicy(clang_getCXXAccessSpecifier(cursor)));
        d->m_scopeStack.back()->addEnum(d->m_currentEnum);
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
        d->m_currentFunction = d->createMemberFunction(cursor);
        d->m_scopeStack.back()->addFunction(d->m_currentFunction);
        break;
    // Not fully supported, currently, seen as normal function
    // Note: May appear inside class (member template) or outside (free template).
    case CXCursor_FunctionTemplate: {
        const CXCursor semParent = clang_getCursorSemanticParent(cursor);
        if (isClassCursor(semParent)) {
            if (semParent == clang_getCursorLexicalParent(cursor)) {
                d->m_currentFunction = d->createMemberFunction(cursor);
                d->m_scopeStack.back()->addFunction(d->m_currentFunction);
                break;
            } else {
                return Skip; // inline member functions outside class
            }
        }
    }
        Q_FALLTHROUGH(); // fall through to free template function.
    case CXCursor_FunctionDecl:
        d->m_currentFunction = d->createFunction(cursor);
        d->m_scopeStack.back()->addFunction(d->m_currentFunction);
        break;
    case CXCursor_Namespace: {
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
        // If possible, continue existing namespace (as otherwise, all headers
        // where a namespace is continued show up in the type database).
        NamespaceModelItem namespaceItem = parentNamespaceItem->findNamespace(name);
        if (namespaceItem.isNull()) {
            namespaceItem.reset(new _NamespaceModelItem(d->m_model, name));
            setFileName(cursor, namespaceItem.data());
            namespaceItem->setScope(d->m_scope);
            parentNamespaceItem->addNamespace(namespaceItem);
        }
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
    case CXCursor_TypeAliasDecl:
    case CXCursor_TypeAliasTemplateDecl: // May contain nested CXCursor_TemplateTypeParameter
        return Skip;
    case CXCursor_TypedefDecl: {
        const QString name = getCursorSpelling(cursor);
        TypeDefModelItem item(new _TypeDefModelItem(d->m_model, name));
        setFileName(cursor, item.data());
        item->setType(d->createTypeInfo(clang_getTypedefDeclUnderlyingType(cursor)));
        item->setScope(d->m_scope);
        d->m_scopeStack.back()->addTypeDef(item);
        d->m_cursorTypedefHash.insert(cursor, item);
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
    default:
        break;
    }
    return true;
}

} // namespace clang
