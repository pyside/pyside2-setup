/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "shibokengenerator.h"
#include "ctypenames.h"
#include <abstractmetalang.h>
#include <messages.h>
#include "overloaddata.h"
#include "propertyspec.h"
#include <reporthandler.h>
#include <typedatabase.h>
#include <abstractmetabuilder.h>
#include <iostream>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <limits>
#include <memory>

static const char AVOID_PROTECTED_HACK[] = "avoid-protected-hack";
static const char PARENT_CTOR_HEURISTIC[] = "enable-parent-ctor-heuristic";
static const char RETURN_VALUE_HEURISTIC[] = "enable-return-value-heuristic";
static const char ENABLE_PYSIDE_EXTENSIONS[] = "enable-pyside-extensions";
static const char DISABLE_VERBOSE_ERROR_MESSAGES[] = "disable-verbose-error-messages";
static const char USE_ISNULL_AS_NB_NONZERO[] = "use-isnull-as-nb_nonzero";
static const char WRAPPER_DIAGNOSTICS[] = "wrapper-diagnostics";

const char *CPP_ARG = "cppArg";
const char *CPP_ARG_REMOVED = "removed_cppArg";
const char *CPP_RETURN_VAR = "cppResult";
const char *CPP_SELF_VAR = "cppSelf";
const char *NULL_PTR = "nullptr";
const char *PYTHON_ARG = "pyArg";
const char *PYTHON_ARGS = "pyArgs";
const char *PYTHON_OVERRIDE_VAR = "pyOverride";
const char *PYTHON_RETURN_VAR = "pyResult";
const char *PYTHON_TO_CPP_VAR = "pythonToCpp";
const char *SMART_POINTER_GETTER = "kSmartPointerGetter";

const char *CONV_RULE_OUT_VAR_SUFFIX = "_out";
const char *BEGIN_ALLOW_THREADS =
    "PyThreadState *_save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS";
const char *END_ALLOW_THREADS = "PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS";

//static void dumpFunction(AbstractMetaFunctionList lst);

QHash<QString, QString> ShibokenGenerator::m_pythonPrimitiveTypeName = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_pythonOperators = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_formatUnits = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_tpFuncs = QHash<QString, QString>();
QStringList ShibokenGenerator::m_knownPythonTypes = QStringList();

static QRegularExpression placeHolderRegex(int index)
{
    return QRegularExpression(QLatin1Char('%') + QString::number(index) + QStringLiteral("\\b"));
}

// Return a prefix to fully qualify value, eg:
// resolveScopePrefix("Class::NestedClass::Enum::Value1", "Enum::Value1")
//     -> "Class::NestedClass::")
static QString resolveScopePrefix(const QStringList &scopeList, const QString &value)
{
    QString name;
    for (int i = scopeList.size() - 1 ; i >= 0; --i) {
        const QString prefix = scopeList.at(i) + QLatin1String("::");
        if (value.startsWith(prefix))
            name.clear();
        else
            name.prepend(prefix);
    }
    return name;
}

static inline QStringList splitClassScope(const AbstractMetaClass *scope)
{
    return scope->qualifiedCppName().split(QLatin1String("::"), Qt::SkipEmptyParts);
}

static QString resolveScopePrefix(const AbstractMetaClass *scope, const QString &value)
{
    return scope
        ? resolveScopePrefix(splitClassScope(scope), value)
        : QString();
}

static QString resolveScopePrefix(const AbstractMetaEnum *metaEnum,
                                  const QString &value)
{
    QStringList parts;
    if (const AbstractMetaClass *scope = metaEnum->enclosingClass())
        parts.append(splitClassScope(scope));
    // Fully qualify the value which is required for C++ 11 enum classes.
    if (!metaEnum->isAnonymous())
        parts.append(metaEnum->name());
    return resolveScopePrefix(parts, value);
}

struct GeneratorClassInfoCacheEntry
{
    ShibokenGenerator::FunctionGroups functionGroups;
    bool needsGetattroFunction = false;
};

using GeneratorClassInfoCache = QHash<const AbstractMetaClass *, GeneratorClassInfoCacheEntry>;

Q_GLOBAL_STATIC(GeneratorClassInfoCache, generatorClassInfoCache)

ShibokenGenerator::ShibokenGenerator()
{
    if (m_pythonPrimitiveTypeName.isEmpty())
        ShibokenGenerator::initPrimitiveTypesCorrespondences();

    if (m_tpFuncs.isEmpty())
        ShibokenGenerator::clearTpFuncs();

    if (m_knownPythonTypes.isEmpty())
        ShibokenGenerator::initKnownPythonTypes();

    m_metaTypeFromStringCache = AbstractMetaTypeCache();

    m_typeSystemConvName[TypeSystemCheckFunction]         = QLatin1String("checkType");
    m_typeSystemConvName[TypeSystemIsConvertibleFunction] = QLatin1String("isConvertible");
    m_typeSystemConvName[TypeSystemToCppFunction]         = QLatin1String("toCpp");
    m_typeSystemConvName[TypeSystemToPythonFunction]      = QLatin1String("toPython");

    const char CHECKTYPE_REGEX[] = R"(%CHECKTYPE\[([^\[]*)\]\()";
    const char ISCONVERTIBLE_REGEX[] = R"(%ISCONVERTIBLE\[([^\[]*)\]\()";
    const char CONVERTTOPYTHON_REGEX[] = R"(%CONVERTTOPYTHON\[([^\[]*)\]\()";
    // Capture a '*' leading the variable name into the target
    // so that "*valuePtr = %CONVERTTOCPP..." works as expected.
    const char CONVERTTOCPP_REGEX[] =
        R"((\*?%?[a-zA-Z_][\w\.]*(?:\[[^\[^<^>]+\])*)(?:\s+)=(?:\s+)%CONVERTTOCPP\[([^\[]*)\]\()";
    m_typeSystemConvRegEx[TypeSystemCheckFunction]         = QRegularExpression(QLatin1String(CHECKTYPE_REGEX));
    m_typeSystemConvRegEx[TypeSystemIsConvertibleFunction] = QRegularExpression(QLatin1String(ISCONVERTIBLE_REGEX));
    m_typeSystemConvRegEx[TypeSystemToPythonFunction]      = QRegularExpression(QLatin1String(CONVERTTOPYTHON_REGEX));
    m_typeSystemConvRegEx[TypeSystemToCppFunction]         = QRegularExpression(QLatin1String(CONVERTTOCPP_REGEX));
}

ShibokenGenerator::~ShibokenGenerator() = default;

void ShibokenGenerator::clearTpFuncs()
{
    m_tpFuncs.insert(QLatin1String("__str__"), QString());
    m_tpFuncs.insert(QLatin1String("__repr__"), QString());
    m_tpFuncs.insert(QLatin1String("__iter__"), QString());
    m_tpFuncs.insert(QLatin1String("__next__"), QString());
}

void ShibokenGenerator::initPrimitiveTypesCorrespondences()
{
    // Python primitive types names
    m_pythonPrimitiveTypeName.clear();

    // PyBool
    m_pythonPrimitiveTypeName.insert(QLatin1String("bool"), QLatin1String("PyBool"));

    const char *charTypes[] = {
        "char", "signed char", "unsigned char"
    };
    for (const char *charType : charTypes)
        m_pythonPrimitiveTypeName.insert(QLatin1String(charType), QStringLiteral("SbkChar"));

    // PyInt
    const char *intTypes[] = {
        "int", "signed int", "uint", "unsigned int",
        "short", "ushort", "signed short", "signed short int",
        "unsigned short", "unsigned short", "unsigned short int",
        "long"
    };
    for (const char *intType : intTypes)
        m_pythonPrimitiveTypeName.insert(QLatin1String(intType), QStringLiteral("PyInt"));

    // PyFloat
    m_pythonPrimitiveTypeName.insert(doubleT(), QLatin1String("PyFloat"));
    m_pythonPrimitiveTypeName.insert(floatT(), QLatin1String("PyFloat"));

    // PyLong
    const char *longTypes[] = {
        "unsigned long", "signed long", "ulong", "unsigned long int",
        "long long", "__int64",
        "unsigned long long", "unsigned __int64", "size_t"
    };
    for (const char *longType : longTypes)
        m_pythonPrimitiveTypeName.insert(QLatin1String(longType), QStringLiteral("PyLong"));

    // Python operators
    m_pythonOperators.clear();

    // call operator
    m_pythonOperators.insert(QLatin1String("operator()"), QLatin1String("call"));

    // Arithmetic operators
    m_pythonOperators.insert(QLatin1String("operator+"), QLatin1String("add"));
    m_pythonOperators.insert(QLatin1String("operator-"), QLatin1String("sub"));
    m_pythonOperators.insert(QLatin1String("operator*"), QLatin1String("mul"));
    m_pythonOperators.insert(QLatin1String("operator/"), QLatin1String("div"));
    m_pythonOperators.insert(QLatin1String("operator%"), QLatin1String("mod"));

    // Inplace arithmetic operators
    m_pythonOperators.insert(QLatin1String("operator+="), QLatin1String("iadd"));
    m_pythonOperators.insert(QLatin1String("operator-="), QLatin1String("isub"));
    m_pythonOperators.insert(QLatin1String("operator++"), QLatin1String("iadd"));
    m_pythonOperators.insert(QLatin1String("operator--"), QLatin1String("isub"));
    m_pythonOperators.insert(QLatin1String("operator*="), QLatin1String("imul"));
    m_pythonOperators.insert(QLatin1String("operator/="), QLatin1String("idiv"));
    m_pythonOperators.insert(QLatin1String("operator%="), QLatin1String("imod"));

    // Bitwise operators
    m_pythonOperators.insert(QLatin1String("operator&"), QLatin1String("and"));
    m_pythonOperators.insert(QLatin1String("operator^"), QLatin1String("xor"));
    m_pythonOperators.insert(QLatin1String("operator|"), QLatin1String("or"));
    m_pythonOperators.insert(QLatin1String("operator<<"), QLatin1String("lshift"));
    m_pythonOperators.insert(QLatin1String("operator>>"), QLatin1String("rshift"));
    m_pythonOperators.insert(QLatin1String("operator~"), QLatin1String("invert"));

    // Inplace bitwise operators
    m_pythonOperators.insert(QLatin1String("operator&="), QLatin1String("iand"));
    m_pythonOperators.insert(QLatin1String("operator^="), QLatin1String("ixor"));
    m_pythonOperators.insert(QLatin1String("operator|="), QLatin1String("ior"));
    m_pythonOperators.insert(QLatin1String("operator<<="), QLatin1String("ilshift"));
    m_pythonOperators.insert(QLatin1String("operator>>="), QLatin1String("irshift"));

    // Comparison operators
    m_pythonOperators.insert(QLatin1String("operator=="), QLatin1String("eq"));
    m_pythonOperators.insert(QLatin1String("operator!="), QLatin1String("ne"));
    m_pythonOperators.insert(QLatin1String("operator<"), QLatin1String("lt"));
    m_pythonOperators.insert(QLatin1String("operator>"), QLatin1String("gt"));
    m_pythonOperators.insert(QLatin1String("operator<="), QLatin1String("le"));
    m_pythonOperators.insert(QLatin1String("operator>="), QLatin1String("ge"));

    // Initialize format units for C++->Python->C++ conversion
    m_formatUnits.clear();
    m_formatUnits.insert(QLatin1String("char"), QLatin1String("b"));
    m_formatUnits.insert(QLatin1String("unsigned char"), QLatin1String("B"));
    m_formatUnits.insert(intT(), QLatin1String("i"));
    m_formatUnits.insert(QLatin1String("unsigned int"), QLatin1String("I"));
    m_formatUnits.insert(shortT(), QLatin1String("h"));
    m_formatUnits.insert(unsignedShortT(), QLatin1String("H"));
    m_formatUnits.insert(longT(), QLatin1String("l"));
    m_formatUnits.insert(unsignedLongLongT(), QLatin1String("k"));
    m_formatUnits.insert(longLongT(), QLatin1String("L"));
    m_formatUnits.insert(QLatin1String("__int64"), QLatin1String("L"));
    m_formatUnits.insert(unsignedLongLongT(), QLatin1String("K"));
    m_formatUnits.insert(QLatin1String("unsigned __int64"), QLatin1String("K"));
    m_formatUnits.insert(doubleT(), QLatin1String("d"));
    m_formatUnits.insert(floatT(), QLatin1String("f"));
}

void ShibokenGenerator::initKnownPythonTypes()
{
    m_knownPythonTypes.clear();
    m_knownPythonTypes << QLatin1String("PyBool") << QLatin1String("PyInt")
        << QLatin1String("PyFloat") << QLatin1String("PyLong") << QLatin1String("PyObject")
        << QLatin1String("PyString") << QLatin1String("PyBuffer") << QLatin1String("PySequence")
        << QLatin1String("PyTuple") << QLatin1String("PyList") << QLatin1String("PyDict")
        << QLatin1String("PyObject*") << QLatin1String("PyObject *") << QLatin1String("PyTupleObject*");
}

QString ShibokenGenerator::translateTypeForWrapperMethod(const AbstractMetaType *cType,
                                                         const AbstractMetaClass *context,
                                                         Options options) const
{
    if (cType->isArray())
        return translateTypeForWrapperMethod(cType->arrayElementType(), context, options) + QLatin1String("[]");

    if (avoidProtectedHack() && cType->isEnum()) {
        const AbstractMetaEnum *metaEnum = findAbstractMetaEnum(cType);
        if (metaEnum && metaEnum->isProtected())
            return protectedEnumSurrogateName(metaEnum);
    }

    return translateType(cType, context, options);
}

bool ShibokenGenerator::shouldGenerateCppWrapper(const AbstractMetaClass *metaClass) const
{
    if (metaClass->isNamespace() || (metaClass->attributes() & AbstractMetaAttributes::FinalCppClass))
        return false;
    bool result = metaClass->isPolymorphic() || metaClass->hasVirtualDestructor();
    if (avoidProtectedHack()) {
        result = result || metaClass->hasProtectedFields() || metaClass->hasProtectedDestructor();
        if (!result && metaClass->hasProtectedFunctions()) {
            int protectedFunctions = 0;
            int protectedOperators = 0;
            const AbstractMetaFunctionList &funcs = metaClass->functions();
            for (const AbstractMetaFunction *func : funcs) {
                if (!func->isProtected() || func->isSignal() || func->isModifiedRemoved())
                    continue;
                if (func->isOperatorOverload())
                    protectedOperators++;
                else
                    protectedFunctions++;
            }
            result = result || (protectedFunctions > protectedOperators);
        }
    } else {
        result = result && !metaClass->hasPrivateDestructor();
    }
    return result;
}

bool ShibokenGenerator::shouldWriteVirtualMethodNative(const AbstractMetaFunction *func)
{
    // PYSIDE-803: Extracted this because it is used multiple times.
    const AbstractMetaClass *metaClass = func->ownerClass();
    return (!avoidProtectedHack() || !metaClass->hasPrivateDestructor())
            && ((func->isVirtual() || func->isAbstract())
            && (func->attributes() & AbstractMetaAttributes::FinalCppMethod) == 0);
}

QString ShibokenGenerator::wrapperName(const AbstractMetaClass *metaClass) const
{
    Q_ASSERT(shouldGenerateCppWrapper(metaClass));
    QString result = metaClass->name();
    if (metaClass->enclosingClass()) // is a inner class
        result.replace(QLatin1String("::"), QLatin1String("_"));
    return result + QLatin1String("Wrapper");
}

QString ShibokenGenerator::fullPythonClassName(const AbstractMetaClass *metaClass)
{
    QString fullClassName = metaClass->name();
    const AbstractMetaClass *enclosing = metaClass->enclosingClass();
    while (enclosing) {
        if (NamespaceTypeEntry::isVisibleScope(enclosing->typeEntry()))
            fullClassName.prepend(enclosing->name() + QLatin1Char('.'));
        enclosing = enclosing->enclosingClass();
    }
    fullClassName.prepend(packageName() + QLatin1Char('.'));
    return fullClassName;
}

QString ShibokenGenerator::fullPythonFunctionName(const AbstractMetaFunction *func)
{
    QString funcName;
    if (func->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(func);
    else
       funcName = func->name();
    if (func->ownerClass()) {
        QString fullClassName = fullPythonClassName(func->ownerClass());
        if (func->isConstructor())
            funcName = fullClassName;
        else
            funcName.prepend(fullClassName + QLatin1Char('.'));
    }
    else {
        funcName = packageName() + QLatin1Char('.') + func->name();
    }
    return funcName;
}

QString ShibokenGenerator::protectedEnumSurrogateName(const AbstractMetaEnum *metaEnum)
{
    return metaEnum->fullName().replace(QLatin1Char('.'), QLatin1Char('_')).replace(QLatin1String("::"), QLatin1String("_")) + QLatin1String("_Surrogate");
}

QString ShibokenGenerator::protectedFieldGetterName(const AbstractMetaField *field)
{
    return QStringLiteral("protected_%1_getter").arg(field->name());
}

QString ShibokenGenerator::protectedFieldSetterName(const AbstractMetaField *field)
{
    return QStringLiteral("protected_%1_setter").arg(field->name());
}

QString ShibokenGenerator::cpythonFunctionName(const AbstractMetaFunction *func)
{
    QString result;

    // PYSIDE-331: For inherited functions, we need to find the same labels.
    // Therefore we use the implementing class.
    if (func->implementingClass()) {
        result = cpythonBaseName(func->implementingClass()->typeEntry());
        if (func->isConstructor()) {
            result += QLatin1String("_Init");
        } else {
            result += QLatin1String("Func_");
            if (func->isOperatorOverload())
                result += ShibokenGenerator::pythonOperatorFunctionName(func);
            else
                result += func->name();
        }
    } else {
        result = QLatin1String("Sbk") + moduleName() + QLatin1String("Module_") + func->name();
    }

    return result;
}

QString ShibokenGenerator::cpythonMethodDefinitionName(const AbstractMetaFunction *func)
{
    if (!func->ownerClass())
        return QString();
    return QStringLiteral("%1Method_%2").arg(cpythonBaseName(func->ownerClass()->typeEntry()), func->name());
}

QString ShibokenGenerator::cpythonGettersSettersDefinitionName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_getsetlist");
}

QString ShibokenGenerator::cpythonSetattroFunctionName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_setattro");
}


QString ShibokenGenerator::cpythonGetattroFunctionName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_getattro");
}

QString ShibokenGenerator::cpythonGetterFunctionName(const QString &name,
                                                     const AbstractMetaClass *enclosingClass)
{
    return cpythonBaseName(enclosingClass) + QStringLiteral("_get_") + name;
}

QString ShibokenGenerator::cpythonSetterFunctionName(const QString &name,
                                                     const AbstractMetaClass *enclosingClass)
{
    return cpythonBaseName(enclosingClass) + QStringLiteral("_set_") + name;
}

QString ShibokenGenerator::cpythonGetterFunctionName(const AbstractMetaField *metaField)
{
    return cpythonGetterFunctionName(metaField->name(), metaField->enclosingClass());
}

QString ShibokenGenerator::cpythonSetterFunctionName(const AbstractMetaField *metaField)
{
    return cpythonSetterFunctionName(metaField->name(), metaField->enclosingClass());
}

QString ShibokenGenerator::cpythonGetterFunctionName(const QPropertySpec *property,
                                                     const AbstractMetaClass *metaClass)
{
    return cpythonGetterFunctionName(property->name(), metaClass);
}

QString ShibokenGenerator::cpythonSetterFunctionName(const QPropertySpec *property,
                                                     const AbstractMetaClass *metaClass)
{
    return cpythonSetterFunctionName(property->name(), metaClass);
}

static QString cpythonEnumFlagsName(const QString &moduleName,
                                    const QString &qualifiedCppName)
{
    QString result = QStringLiteral("Sbk%1_%2").arg(moduleName, qualifiedCppName);
    result.replace(QLatin1String("::"), QLatin1String("_"));
    return result;
}

// Return the scope for fully qualifying the enumeration including trailing "::".
static QString searchForEnumScope(const AbstractMetaClass *metaClass, const QString &value)
{
    if (!metaClass)
        return QString();
    const AbstractMetaEnumList &enums = metaClass->enums();
    for (const AbstractMetaEnum *metaEnum : enums) {
        if (metaEnum->findEnumValue(value))
            return resolveScopePrefix(metaEnum, value);
    }
    // PYSIDE-331: We need to also search the base classes.
    QString ret = searchForEnumScope(metaClass->enclosingClass(), value);
    if (ret.isEmpty())
        ret = searchForEnumScope(metaClass->baseClass(), value);
    return ret;
}

// Handle QFlags<> for guessScopeForDefaultValue()
QString ShibokenGenerator::guessScopeForDefaultFlagsValue(const AbstractMetaFunction *func,
                                                          const AbstractMetaArgument *arg,
                                                          const QString &value) const
{
    // Numeric values -> "Options(42)"
    static const QRegularExpression numberRegEx(QStringLiteral("^\\d+$")); // Numbers to flags
    Q_ASSERT(numberRegEx.isValid());
    if (numberRegEx.match(value).hasMatch()) {
        QString typeName = translateTypeForWrapperMethod(arg->type(), func->implementingClass());
        if (arg->type()->isConstant())
            typeName.remove(0, sizeof("const ") / sizeof(char) - 1);
        switch (arg->type()->referenceType()) {
        case NoReference:
            break;
        case LValueReference:
            typeName.chop(1);
            break;
        case RValueReference:
            typeName.chop(2);
            break;
        }
        return typeName + QLatin1Char('(') + value + QLatin1Char(')');
    }

    // "Options(Option1 | Option2)" -> "Options(Class::Enum::Option1 | Class::Enum::Option2)"
    static const QRegularExpression enumCombinationRegEx(QStringLiteral("^([A-Za-z_][\\w:]*)\\(([^,\\(\\)]*)\\)$")); // FlagName(EnumItem|EnumItem|...)
    Q_ASSERT(enumCombinationRegEx.isValid());
    const QRegularExpressionMatch match = enumCombinationRegEx.match(value);
    if (match.hasMatch()) {
        const QString expression = match.captured(2).trimmed();
        if (expression.isEmpty())
            return value;
        const QStringList enumItems = expression.split(QLatin1Char('|'));
        const QString scope = searchForEnumScope(func->implementingClass(),
                                                 enumItems.constFirst().trimmed());
        if (scope.isEmpty())
            return value;
        QString result;
        QTextStream str(&result);
        str << match.captured(1) << '('; // Flag name
        for (int i = 0, size = enumItems.size(); i < size; ++i) {
            if (i)
                str << '|';
            str << scope << enumItems.at(i).trimmed();
        }
        str << ')';
        return result;
    }
    // A single flag "Option1" -> "Class::Enum::Option1"
    return searchForEnumScope(func->implementingClass(), value) + value;
}

/*
 * This function uses some heuristics to find out the scope for a given
 * argument default value since they must be fully qualified when used outside the class:
 * class A {
 *     enum Enum { e1, e1 };
 *     void foo(Enum e = e1);
 * }
 * should be qualified to:
 * A::Enum cppArg0 = A::Enum::e1;
 *
 * New situations may arise in the future and
 * this method should be updated, do it with care.
 */
QString ShibokenGenerator::guessScopeForDefaultValue(const AbstractMetaFunction *func,
                                                     const AbstractMetaArgument *arg) const
{
    QString value = arg->defaultValueExpression();

    if (value.isEmpty() || value == QLatin1String("{}")
        || arg->hasModifiedDefaultValueExpression()
        || isPointer(arg->type())) {
        return value;
    }

    static const QRegularExpression enumValueRegEx(QStringLiteral("^([A-Za-z_]\\w*)?$"));
    Q_ASSERT(enumValueRegEx.isValid());
    // Do not qualify macros by class name, eg QSGGeometry(..., int t = GL_UNSIGNED_SHORT);
    static const QRegularExpression macroRegEx(QStringLiteral("^[A-Z_][A-Z0-9_]*$"));
    Q_ASSERT(macroRegEx.isValid());
    if (arg->type()->isPrimitive() && macroRegEx.match(value).hasMatch())
        return value;

    QString prefix;
    if (arg->type()->isEnum()) {
        if (const AbstractMetaEnum *metaEnum = findAbstractMetaEnum(arg->type()))
            prefix = resolveScopePrefix(metaEnum, value);
    } else if (arg->type()->isFlags()) {
        value = guessScopeForDefaultFlagsValue(func, arg, value);
    } else if (arg->type()->typeEntry()->isValue()) {
        const AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes(), arg->type()->typeEntry());
        if (enumValueRegEx.match(value).hasMatch() && value != QLatin1String("NULL"))
            prefix = resolveScopePrefix(metaClass, value);
    } else if (arg->type()->isPrimitive() && arg->type()->name() == intT()) {
        if (enumValueRegEx.match(value).hasMatch() && func->implementingClass())
            prefix = resolveScopePrefix(func->implementingClass(), value);
    } else if(arg->type()->isPrimitive()) {
        static const QRegularExpression unknowArgumentRegEx(QStringLiteral("^(?:[A-Za-z_][\\w:]*\\()?([A-Za-z_]\\w*)(?:\\))?$")); // [PrimitiveType(] DESIREDNAME [)]
        Q_ASSERT(unknowArgumentRegEx.isValid());
        const QRegularExpressionMatch match = unknowArgumentRegEx.match(value);
        if (match.hasMatch() && func->implementingClass()) {
            const AbstractMetaFieldList &fields = func->implementingClass()->fields();
            for (const AbstractMetaField *field : fields) {
                if (match.captured(1).trimmed() == field->name()) {
                    QString fieldName = field->name();
                    if (field->isStatic()) {
                        prefix = resolveScopePrefix(func->implementingClass(), value);
                        fieldName.prepend(prefix);
                        prefix.clear();
                    } else {
                        fieldName.prepend(QLatin1String(CPP_SELF_VAR) + QLatin1String("->"));
                    }
                    value.replace(match.captured(1), fieldName);
                    break;
                }
            }
        }
    }

    if (!prefix.isEmpty())
        value.prepend(prefix);
    return value;
}

QString ShibokenGenerator::cpythonEnumName(const EnumTypeEntry *enumEntry)
{
    QString p = enumEntry->targetLangPackage();
    p.replace(QLatin1Char('.'), QLatin1Char('_'));
    return cpythonEnumFlagsName(p, enumEntry->qualifiedCppName());
}

QString ShibokenGenerator::cpythonEnumName(const AbstractMetaEnum *metaEnum)
{
    return cpythonEnumName(metaEnum->typeEntry());
}

QString ShibokenGenerator::cpythonFlagsName(const FlagsTypeEntry *flagsEntry)
{
    QString p = flagsEntry->targetLangPackage();
    p.replace(QLatin1Char('.'), QLatin1Char('_'));
    return cpythonEnumFlagsName(p, flagsEntry->originalName());
}

QString ShibokenGenerator::cpythonFlagsName(const AbstractMetaEnum *metaEnum)
{
    const FlagsTypeEntry *flags = metaEnum->typeEntry()->flags();
    if (!flags)
        return QString();
    return cpythonFlagsName(flags);
}

QString ShibokenGenerator::cpythonSpecialCastFunctionName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass->typeEntry()) + QLatin1String("SpecialCastFunction");
}

QString ShibokenGenerator::cpythonWrapperCPtr(const AbstractMetaClass *metaClass,
                                              const QString &argName) const
{
    return cpythonWrapperCPtr(metaClass->typeEntry(), argName);
}

QString ShibokenGenerator::cpythonWrapperCPtr(const AbstractMetaType *metaType,
                                              const QString &argName) const
{
    if (!ShibokenGenerator::isWrapperType(metaType->typeEntry()))
        return QString();
    return QLatin1String("reinterpret_cast< ::") + metaType->cppSignature()
        + QLatin1String(" *>(Shiboken::Conversions::cppPointer(") + cpythonTypeNameExt(metaType)
        + QLatin1String(", reinterpret_cast<SbkObject *>(") + argName + QLatin1String(")))");
}

QString ShibokenGenerator::cpythonWrapperCPtr(const TypeEntry *type,
                                              const QString &argName) const
{
    if (!ShibokenGenerator::isWrapperType(type))
        return QString();
    return QLatin1String("reinterpret_cast< ::") + type->qualifiedCppName()
        + QLatin1String(" *>(Shiboken::Conversions::cppPointer(") + cpythonTypeNameExt(type)
        + QLatin1String(", reinterpret_cast<SbkObject *>(") + argName + QLatin1String(")))");
}

void ShibokenGenerator::writeToPythonConversion(QTextStream & s, const AbstractMetaType *type,
                                                const AbstractMetaClass * /* context */,
                                                const QString &argumentName)
{
    s << cpythonToPythonConversionFunction(type) << argumentName << ')';
}

void ShibokenGenerator::writeToCppConversion(QTextStream &s, const AbstractMetaClass *metaClass,
                                             const QString &inArgName, const QString &outArgName)
{
    s << cpythonToCppConversionFunction(metaClass) << inArgName << ", &" << outArgName << ')';
}

void ShibokenGenerator::writeToCppConversion(QTextStream &s, const AbstractMetaType *type, const AbstractMetaClass *context,
                                             const QString &inArgName, const QString &outArgName)
{
    s << cpythonToCppConversionFunction(type, context) << inArgName << ", &" << outArgName << ')';
}

bool ShibokenGenerator::shouldRejectNullPointerArgument(const AbstractMetaFunction *func, int argIndex)
{
    if (argIndex < 0 || argIndex >= func->arguments().count())
        return false;

    const AbstractMetaArgument *arg = func->arguments().at(argIndex);
    if (isValueTypeWithCopyConstructorOnly(arg->type()))
        return true;

    // Argument type is not a pointer, a None rejection should not be
    // necessary because the type checking would handle that already.
    if (!isPointer(arg->type()))
        return false;
    if (func->argumentRemoved(argIndex + 1))
        return false;
    const FunctionModificationList &mods = func->modifications();
    for (const FunctionModification &funcMod : mods) {
        for (const ArgumentModification &argMod : funcMod.argument_mods) {
            if (argMod.index == argIndex + 1 && argMod.noNullPointers)
                return true;
        }
    }
    return false;
}

QString ShibokenGenerator::getFormatUnitString(const AbstractMetaFunction *func, bool incRef) const
{
    QString result;
    const char objType = (incRef ? 'O' : 'N');
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument *arg : arguments) {
        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        if (!func->typeReplaced(arg->argumentIndex() + 1).isEmpty()) {
            result += QLatin1Char(objType);
        } else if (arg->type()->isObject()
            || arg->type()->isValue()
            || arg->type()->isValuePointer()
            || arg->type()->isNativePointer()
            || arg->type()->isEnum()
            || arg->type()->isFlags()
            || arg->type()->isContainer()
            || arg->type()->isSmartPointer()
            || arg->type()->referenceType() == LValueReference) {
            result += QLatin1Char(objType);
        } else if (arg->type()->isPrimitive()) {
            const auto *ptype =
                static_cast<const PrimitiveTypeEntry *>(arg->type()->typeEntry());
            if (ptype->basicReferencedTypeEntry())
                ptype = ptype->basicReferencedTypeEntry();
            if (m_formatUnits.contains(ptype->name()))
                result += m_formatUnits[ptype->name()];
            else
                result += QLatin1Char(objType);
        } else if (isCString(arg->type())) {
            result += QLatin1Char('z');
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << "Method: " << func->ownerClass()->qualifiedCppName()
                << "::" << func->signature() << " => Arg:"
                << arg->name() << "index: " << arg->argumentIndex()
                << " - cannot be handled properly. Use an inject-code to fix it!";
            result += QLatin1Char('?');
        }
    }
    return result;
}

QString ShibokenGenerator::cpythonBaseName(const AbstractMetaType *type)
{
    if (isCString(type))
        return QLatin1String("PyString");
    return cpythonBaseName(type->typeEntry());
}

QString ShibokenGenerator::cpythonBaseName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonBaseName(const TypeEntry *type)
{
    QString baseName;
    if (ShibokenGenerator::isWrapperType(type) || type->isNamespace()) { // && type->referenceType() == NoReference) {
        baseName = QLatin1String("Sbk_") + type->name();
    } else if (type->isPrimitive()) {
        const auto *ptype = static_cast<const PrimitiveTypeEntry *>(type);
        while (ptype->basicReferencedTypeEntry())
            ptype = ptype->basicReferencedTypeEntry();
        if (ptype->targetLangApiName() == ptype->name())
            baseName = pythonPrimitiveTypeName(ptype->name());
        else
            baseName = ptype->targetLangApiName();
    } else if (type->isEnum()) {
        baseName = cpythonEnumName(static_cast<const EnumTypeEntry *>(type));
    } else if (type->isFlags()) {
        baseName = cpythonFlagsName(static_cast<const FlagsTypeEntry *>(type));
    } else if (type->isContainer()) {
        const auto *ctype = static_cast<const ContainerTypeEntry *>(type);
        switch (ctype->containerKind()) {
            case ContainerTypeEntry::ListContainer:
            case ContainerTypeEntry::StringListContainer:
            case ContainerTypeEntry::LinkedListContainer:
            case ContainerTypeEntry::VectorContainer:
            case ContainerTypeEntry::StackContainer:
            case ContainerTypeEntry::QueueContainer:
                //baseName = "PyList";
                //break;
            case ContainerTypeEntry::PairContainer:
                //baseName = "PyTuple";
                baseName = QLatin1String("PySequence");
                break;
            case ContainerTypeEntry::SetContainer:
                baseName = QLatin1String("PySet");
                break;
            case ContainerTypeEntry::MapContainer:
            case ContainerTypeEntry::MultiMapContainer:
            case ContainerTypeEntry::HashContainer:
            case ContainerTypeEntry::MultiHashContainer:
                baseName = QLatin1String("PyDict");
                break;
            default:
                Q_ASSERT(false);
        }
    } else {
        baseName = QLatin1String("PyObject");
    }
    return baseName.replace(QLatin1String("::"), QLatin1String("_"));
}

QString ShibokenGenerator::cpythonTypeName(const AbstractMetaClass *metaClass)
{
    return cpythonTypeName(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonTypeName(const TypeEntry *type)
{
    return cpythonBaseName(type) + QLatin1String("_TypeF()");
}

QString ShibokenGenerator::cpythonTypeNameExt(const TypeEntry *type) const
{
    return cppApiVariableName(type->targetLangPackage()) + QLatin1Char('[')
            + getTypeIndexVariableName(type) + QLatin1Char(']');
}

QString ShibokenGenerator::converterObject(const AbstractMetaType *type)
{
    if (isCString(type))
        return QLatin1String("Shiboken::Conversions::PrimitiveTypeConverter<const char *>()");
    if (isVoidPointer(type))
        return QLatin1String("Shiboken::Conversions::PrimitiveTypeConverter<void *>()");
    const AbstractMetaTypeCList nestedArrayTypes = type->nestedArrayTypes();
    if (!nestedArrayTypes.isEmpty() && nestedArrayTypes.constLast()->isCppPrimitive()) {
        return QStringLiteral("Shiboken::Conversions::ArrayTypeConverter<")
            + nestedArrayTypes.constLast()->minimalSignature()
            + QLatin1String(">(") + QString::number(nestedArrayTypes.size())
            + QLatin1Char(')');
    }
    if (type->typeEntry()->isContainer()) {
        return convertersVariableName(type->typeEntry()->targetLangPackage())
               + QLatin1Char('[') + getTypeIndexVariableName(type) + QLatin1Char(']');
    }
    return converterObject(type->typeEntry());
}

QString ShibokenGenerator::converterObject(const TypeEntry *type)
{
    if (isCppPrimitive(type))
        return QString::fromLatin1("Shiboken::Conversions::PrimitiveTypeConverter<%1>()").arg(type->qualifiedCppName());
    if (isWrapperType(type) || type->isEnum() || type->isFlags())
        return QString::fromLatin1("*PepType_SGTP(%1)->converter").arg(cpythonTypeNameExt(type));

    if (type->isArray()) {
        qDebug() << "Warning: no idea how to handle the Qt5 type " << type->qualifiedCppName();
        return QString();
    }

    /* the typedef'd primitive types case */
    const auto *pte = dynamic_cast<const PrimitiveTypeEntry *>(type);
    if (!pte) {
        qDebug() << "Warning: the Qt5 primitive type is unknown" << type->qualifiedCppName();
        return QString();
    }
    if (pte->basicReferencedTypeEntry())
        pte = pte->basicReferencedTypeEntry();
    if (pte->isPrimitive() && !pte->isCppPrimitive() && !pte->customConversion())
        return QString::fromLatin1("Shiboken::Conversions::PrimitiveTypeConverter<%1>()").arg(pte->qualifiedCppName());

    return convertersVariableName(type->targetLangPackage())
           + QLatin1Char('[') + getTypeIndexVariableName(type) + QLatin1Char(']');
}

QString ShibokenGenerator::cpythonTypeNameExt(const AbstractMetaType *type) const
{
    return cppApiVariableName(type->typeEntry()->targetLangPackage()) + QLatin1Char('[')
           + getTypeIndexVariableName(type) + QLatin1Char(']');
}

static inline QString unknownOperator() { return QStringLiteral("__UNKNOWN_OPERATOR__"); }

QString ShibokenGenerator::fixedCppTypeName(const CustomConversion::TargetToNativeConversion *toNative)
{
    if (toNative->sourceType())
        return fixedCppTypeName(toNative->sourceType());
    return toNative->sourceTypeName();
}
QString ShibokenGenerator::fixedCppTypeName(const AbstractMetaType *type)
{
    return fixedCppTypeName(type->typeEntry(), type->cppSignature());
}

static QString _fixedCppTypeName(QString typeName)
{
    typeName.remove(QLatin1Char(' '));
    typeName.replace(QLatin1Char('.'),  QLatin1Char('_'));
    typeName.replace(QLatin1Char(','),  QLatin1Char('_'));
    typeName.replace(QLatin1Char('<'),  QLatin1Char('_'));
    typeName.replace(QLatin1Char('>'),  QLatin1Char('_'));
    typeName.replace(QLatin1String("::"), QLatin1String("_"));
    typeName.replace(QLatin1String("*"),  QLatin1String("PTR"));
    typeName.replace(QLatin1String("&"),  QLatin1String("REF"));
    return typeName;
}
QString ShibokenGenerator::fixedCppTypeName(const TypeEntry *type, QString typeName)
{
    if (typeName.isEmpty())
        typeName = type->qualifiedCppName();
    if (!type->generateCode()) {
        typeName.prepend(QLatin1Char('_'));
        typeName.prepend(type->targetLangPackage());
    }
    return _fixedCppTypeName(typeName);
}

QString ShibokenGenerator::pythonPrimitiveTypeName(const QString &cppTypeName)
{
    QString rv = ShibokenGenerator::m_pythonPrimitiveTypeName.value(cppTypeName, QString());
    if (rv.isEmpty()) {
        // activate this when some primitive types are missing,
        // i.e. when shiboken itself fails to build.
        // In general, this is valid while just called by isNumeric()
        // used on Qt5, 2015-09-20
        if (false) {
            std::cerr << "primitive type not found: " << qPrintable(cppTypeName) << std::endl;
            abort();
        }
    }
    return rv;
}

QString ShibokenGenerator::pythonPrimitiveTypeName(const PrimitiveTypeEntry *type)
{
    while (type->basicReferencedTypeEntry())
        type = type->basicReferencedTypeEntry();
    return pythonPrimitiveTypeName(type->name());
}

QString ShibokenGenerator::pythonOperatorFunctionName(const QString &cppOpFuncName)
{
    QString value = m_pythonOperators.value(cppOpFuncName);
    if (value.isEmpty())
        return unknownOperator();
    value.prepend(QLatin1String("__"));
    value.append(QLatin1String("__"));
    return value;
}

QString ShibokenGenerator::pythonOperatorFunctionName(const AbstractMetaFunction *func)
{
    QString op = pythonOperatorFunctionName(func->originalName());
    if (op == unknownOperator())
        qCWarning(lcShiboken).noquote().nospace() << msgUnknownOperator(func);
    if (func->arguments().isEmpty()) {
        if (op == QLatin1String("__sub__"))
            op = QLatin1String("__neg__");
        else if (op == QLatin1String("__add__"))
            op = QLatin1String("__pos__");
    } else if (func->isStatic() && func->arguments().size() == 2) {
        // If a operator overload function has 2 arguments and
        // is static we assume that it is a reverse operator.
        op = op.insert(2, QLatin1Char('r'));
    }
    return op;
}

QString ShibokenGenerator::pythonRichCompareOperatorId(const QString &cppOpFuncName)
{
    return QLatin1String("Py_") + m_pythonOperators.value(cppOpFuncName).toUpper();
}

QString ShibokenGenerator::pythonRichCompareOperatorId(const AbstractMetaFunction *func)
{
    return pythonRichCompareOperatorId(func->originalName());
}

bool ShibokenGenerator::isNumber(const QString &cpythonApiName)
{
    return cpythonApiName == QLatin1String("PyInt")
            || cpythonApiName == QLatin1String("PyFloat")
            || cpythonApiName == QLatin1String("PyLong")
            || cpythonApiName == QLatin1String("PyBool");
}

bool ShibokenGenerator::isNumber(const TypeEntry *type)
{
    if (!type->isPrimitive())
        return false;
    return isNumber(pythonPrimitiveTypeName(static_cast<const PrimitiveTypeEntry *>(type)));
}

bool ShibokenGenerator::isNumber(const AbstractMetaType *type)
{
    return isNumber(type->typeEntry());
}

bool ShibokenGenerator::isPyInt(const TypeEntry *type)
{
    if (!type->isPrimitive())
        return false;
    return pythonPrimitiveTypeName(static_cast<const PrimitiveTypeEntry *>(type))
        == QLatin1String("PyInt");
}

bool ShibokenGenerator::isPyInt(const AbstractMetaType *type)
{
    return isPyInt(type->typeEntry());
}

bool ShibokenGenerator::isWrapperType(const TypeEntry *type)
{
    if (type->isComplex())
        return ShibokenGenerator::isWrapperType(static_cast<const ComplexTypeEntry *>(type));
    return type->isObject() || type->isValue() || type->isSmartPointer();
}
bool ShibokenGenerator::isWrapperType(const ComplexTypeEntry *type)
{
    return isObjectType(type) || type->isValue() || type->isSmartPointer();
}
bool ShibokenGenerator::isWrapperType(const AbstractMetaType *metaType)
{
    return isObjectType(metaType)
            || metaType->typeEntry()->isValue()
            || metaType->typeEntry()->isSmartPointer();
}

bool ShibokenGenerator::isPointerToWrapperType(const AbstractMetaType *type)
{
    return (isObjectType(type) && type->indirections() == 1) || type->isValuePointer();
}

bool ShibokenGenerator::isObjectTypeUsedAsValueType(const AbstractMetaType *type)
{
    return type->typeEntry()->isObject() && type->referenceType() == NoReference && type->indirections() == 0;
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const AbstractMetaClass *metaClass)
{
    if (!metaClass || !metaClass->typeEntry()->isValue())
        return false;
    if (metaClass->attributes().testFlag(AbstractMetaAttributes::HasRejectedDefaultConstructor))
        return false;
    const AbstractMetaFunctionList ctors =
        metaClass->queryFunctions(AbstractMetaClass::Constructors);
    bool copyConstructorFound = false;
    for (auto ctor : ctors) {
        switch (ctor->functionType()) {
        case AbstractMetaFunction::ConstructorFunction:
            return false;
        case AbstractMetaFunction::CopyConstructorFunction:
            copyConstructorFound = true;
            break;
        case AbstractMetaFunction::MoveConstructorFunction:
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    return copyConstructorFound;
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const TypeEntry *type) const
{
    if (!type || !type->isValue())
        return false;
    return isValueTypeWithCopyConstructorOnly(AbstractMetaClass::findClass(classes(), type));
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const AbstractMetaType *type) const
{
    if (!type || !type->typeEntry()->isValue())
        return false;
    return isValueTypeWithCopyConstructorOnly(type->typeEntry());
}

bool ShibokenGenerator::isUserPrimitive(const TypeEntry *type)
{
    if (!type->isPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(type);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->isPrimitive() && !trueType->isCppPrimitive()
           && trueType->qualifiedCppName() != QLatin1String("std::string");
}

bool ShibokenGenerator::isUserPrimitive(const AbstractMetaType *type)
{
    if (type->indirections() != 0)
        return false;
    return isUserPrimitive(type->typeEntry());
}

bool ShibokenGenerator::isCppPrimitive(const TypeEntry *type)
{
    if (type->isCppPrimitive())
        return true;
    if (!type->isPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(type);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->qualifiedCppName() == QLatin1String("std::string");
}

bool ShibokenGenerator::isCppPrimitive(const AbstractMetaType *type)
{
    if (isCString(type) || isVoidPointer(type))
        return true;
    if (type->indirections() != 0)
        return false;
    return isCppPrimitive(type->typeEntry());
}

bool ShibokenGenerator::shouldDereferenceArgumentPointer(const AbstractMetaArgument *arg)
{
    return shouldDereferenceAbstractMetaTypePointer(arg->type());
}

bool ShibokenGenerator::shouldDereferenceAbstractMetaTypePointer(const AbstractMetaType *metaType)
{
    return metaType->referenceType() == LValueReference && isWrapperType(metaType) && !isPointer(metaType);
}

bool ShibokenGenerator::visibilityModifiedToPrivate(const AbstractMetaFunction *func)
{
    const FunctionModificationList &mods = func->modifications();
    for (const FunctionModification &mod : mods) {
        if (mod.modifiers & Modification::Private)
            return true;
    }
    return false;
}

bool ShibokenGenerator::isNullPtr(const QString &value)
{
    return value == QLatin1String("0") || value == QLatin1String("nullptr")
        || value == QLatin1String("NULLPTR") || value == QLatin1String("{}");
}

QString ShibokenGenerator::cpythonCheckFunction(const AbstractMetaType *metaType, bool genericNumberType)
{
    QString customCheck;
    if (metaType->typeEntry()->isCustom()) {
        AbstractMetaType *type;
        customCheck = guessCPythonCheckFunction(metaType->typeEntry()->name(), &type);
        if (type)
            metaType = type;
        if (!customCheck.isEmpty())
            return customCheck;
    }

    if (isCppPrimitive(metaType)) {
        if (isCString(metaType))
            return QLatin1String("Shiboken::String::check");
        if (isVoidPointer(metaType))
            return QLatin1String("PyObject_Check");
        return cpythonCheckFunction(metaType->typeEntry(), genericNumberType);
    }
    auto typeEntry = metaType->typeEntry();
    if (typeEntry->isContainer()) {
        QString typeCheck = QLatin1String("Shiboken::Conversions::");
        ContainerTypeEntry::ContainerKind type =
            static_cast<const ContainerTypeEntry *>(typeEntry)->containerKind();
        if (type == ContainerTypeEntry::ListContainer
            || type == ContainerTypeEntry::StringListContainer
            || type == ContainerTypeEntry::LinkedListContainer
            || type == ContainerTypeEntry::VectorContainer
            || type == ContainerTypeEntry::StackContainer
            || type == ContainerTypeEntry::SetContainer
            || type == ContainerTypeEntry::QueueContainer) {
            const AbstractMetaType *type = metaType->instantiations().constFirst();
            if (isPointerToWrapperType(type)) {
                typeCheck += QString::fromLatin1("checkSequenceTypes(%1, ").arg(cpythonTypeNameExt(type));
            } else if (isWrapperType(type)) {
                typeCheck += QLatin1String("convertibleSequenceTypes(reinterpret_cast<SbkObjectType *>(");
                typeCheck += cpythonTypeNameExt(type);
                typeCheck += QLatin1String("), ");
            } else {
                typeCheck += QString::fromLatin1("convertibleSequenceTypes(%1, ").arg(converterObject(type));
            }
        } else if (type == ContainerTypeEntry::MapContainer
            || type == ContainerTypeEntry::MultiMapContainer
            || type == ContainerTypeEntry::HashContainer
            || type == ContainerTypeEntry::MultiHashContainer
            || type == ContainerTypeEntry::PairContainer) {
            QString pyType = (type == ContainerTypeEntry::PairContainer) ? QLatin1String("Pair") : QLatin1String("Dict");
            const AbstractMetaType *firstType = metaType->instantiations().constFirst();
            const AbstractMetaType *secondType = metaType->instantiations().constLast();
            if (isPointerToWrapperType(firstType) && isPointerToWrapperType(secondType)) {
                typeCheck += QString::fromLatin1("check%1Types(%2, %3, ")
                             .arg(pyType, cpythonTypeNameExt(firstType), cpythonTypeNameExt(secondType));
            } else {
                typeCheck += QString::fromLatin1("convertible%1Types(%2, %3, %4, %5, ")
                                .arg(pyType, converterObject(firstType),
                                     isPointerToWrapperType(firstType) ? QLatin1String("true") : QLatin1String("false"),
                                     converterObject(secondType),
                                     isPointerToWrapperType(secondType) ? QLatin1String("true") : QLatin1String("false"));
            }
        }
        return typeCheck;
    }
    return cpythonCheckFunction(typeEntry, genericNumberType);
}

QString ShibokenGenerator::cpythonCheckFunction(const TypeEntry *type, bool genericNumberType)
{
    QString customCheck;
    if (type->isCustom()) {
        AbstractMetaType *metaType;
        customCheck = guessCPythonCheckFunction(type->name(), &metaType);
        if (metaType)
            return cpythonCheckFunction(metaType, genericNumberType);
        return customCheck;
    }

    if (type->isEnum() || type->isFlags() || isWrapperType(type))
        return QString::fromLatin1("SbkObject_TypeCheck(%1, ").arg(cpythonTypeNameExt(type));
    if (isCppPrimitive(type)) {
        return pythonPrimitiveTypeName(static_cast<const PrimitiveTypeEntry *>(type))
                                       + QLatin1String("_Check");
    }
    QString typeCheck;
    if (type->targetLangApiName() == type->name())
        typeCheck = cpythonIsConvertibleFunction(type);
    else if (type->targetLangApiName() == QLatin1String("PyUnicode"))
        typeCheck = QLatin1String("Shiboken::String::check");
    else
        typeCheck = type->targetLangApiName() + QLatin1String("_Check");
    return typeCheck;
}

QString ShibokenGenerator::guessCPythonCheckFunction(const QString &type, AbstractMetaType **metaType)
{
    *metaType = nullptr;
    // PYSIDE-795: We abuse PySequence for iterables.
    // This part handles the overrides in the XML files.
    if (type == QLatin1String("PySequence"))
        return QLatin1String("Shiboken::String::checkIterable");

    if (type == QLatin1String("PyTypeObject"))
        return QLatin1String("PyType_Check");

    if (type == QLatin1String("PyBuffer"))
        return QLatin1String("Shiboken::Buffer::checkType");

    if (type == QLatin1String("str"))
        return QLatin1String("Shiboken::String::check");

    *metaType = buildAbstractMetaTypeFromString(type);
    if (*metaType && !(*metaType)->typeEntry()->isCustom())
        return QString();

    return type + QLatin1String("_Check");
}

QString ShibokenGenerator::cpythonIsConvertibleFunction(const TypeEntry *type,
                                                        bool /* genericNumberType */,
                                                        bool /* checkExact */)
{
    if (isWrapperType(type)) {
        QString result = QLatin1String("Shiboken::Conversions::");
        result += (type->isValue() && !isValueTypeWithCopyConstructorOnly(type))
                         ? QLatin1String("isPythonToCppValueConvertible")
                         : QLatin1String("isPythonToCppPointerConvertible");
        result += QLatin1String("(reinterpret_cast<SbkObjectType *>(")
            + cpythonTypeNameExt(type) + QLatin1String("), ");
        return result;
    }
    return QString::fromLatin1("Shiboken::Conversions::isPythonToCppConvertible(%1, ")
              .arg(converterObject(type));
}
QString ShibokenGenerator::cpythonIsConvertibleFunction(const AbstractMetaType *metaType,
                                                        bool /* genericNumberType */)
{
    QString customCheck;
    if (metaType->typeEntry()->isCustom()) {
        AbstractMetaType *type;
        customCheck = guessCPythonCheckFunction(metaType->typeEntry()->name(), &type);
        if (type)
            metaType = type;
        if (!customCheck.isEmpty())
            return customCheck;
    }

    QString result = QLatin1String("Shiboken::Conversions::");
    if (isWrapperType(metaType)) {
        if (isPointer(metaType) || isValueTypeWithCopyConstructorOnly(metaType))
            result += QLatin1String("isPythonToCppPointerConvertible");
        else if (metaType->referenceType() == LValueReference)
            result += QLatin1String("isPythonToCppReferenceConvertible");
        else
            result += QLatin1String("isPythonToCppValueConvertible");
        result += QLatin1String("(reinterpret_cast<SbkObjectType *>(")
            + cpythonTypeNameExt(metaType) + QLatin1String("), ");
        return result;
    }
    result += QLatin1String("isPythonToCppConvertible(") + converterObject(metaType);
    // Write out array sizes if known
    const AbstractMetaTypeCList nestedArrayTypes = metaType->nestedArrayTypes();
    if (!nestedArrayTypes.isEmpty() && nestedArrayTypes.constLast()->isCppPrimitive()) {
        const int dim1 = metaType->arrayElementCount();
        const int dim2 = nestedArrayTypes.constFirst()->isArray()
            ? nestedArrayTypes.constFirst()->arrayElementCount() : -1;
        result += QLatin1String(", ") + QString::number(dim1)
            + QLatin1String(", ") + QString::number(dim2);
    }
    result += QLatin1String(", ");
    return result;
}

QString ShibokenGenerator::cpythonIsConvertibleFunction(const AbstractMetaArgument *metaArg, bool genericNumberType)
{
    return cpythonIsConvertibleFunction(metaArg->type(), genericNumberType);
}

QString ShibokenGenerator::cpythonToCppConversionFunction(const AbstractMetaClass *metaClass)
{
    return QLatin1String("Shiboken::Conversions::pythonToCppPointer(reinterpret_cast<SbkObjectType *>(")
        + cpythonTypeNameExt(metaClass->typeEntry()) + QLatin1String("), ");
}

QString ShibokenGenerator::cpythonToCppConversionFunction(const AbstractMetaType *type,
                                                          const AbstractMetaClass * /* context */)
{
    if (isWrapperType(type)) {
        return QLatin1String("Shiboken::Conversions::pythonToCpp")
            + (isPointer(type) ? QLatin1String("Pointer") : QLatin1String("Copy"))
            + QLatin1String("(reinterpret_cast<SbkObjectType *>(")
            + cpythonTypeNameExt(type) + QLatin1String("), ");
    }
    return QStringLiteral("Shiboken::Conversions::pythonToCppCopy(%1, ")
              .arg(converterObject(type));
}

QString ShibokenGenerator::cpythonToPythonConversionFunction(const AbstractMetaType *type,
                                                             const AbstractMetaClass * /* context */)
{
    if (isWrapperType(type)) {
        QString conversion;
        if (type->referenceType() == LValueReference && !(type->isValue() && type->isConstant()) && !isPointer(type))
            conversion = QLatin1String("reference");
        else if (type->isValue() || type->isSmartPointer())
            conversion = QLatin1String("copy");
        else
            conversion = QLatin1String("pointer");
        QString result = QLatin1String("Shiboken::Conversions::") + conversion
            + QLatin1String("ToPython(reinterpret_cast<SbkObjectType *>(")
            + cpythonTypeNameExt(type) + QLatin1String("), ");
        if (conversion != QLatin1String("pointer"))
            result += QLatin1Char('&');
        return result;
    }
    return QStringLiteral("Shiboken::Conversions::copyToPython(%1, %2")
              .arg(converterObject(type),
                   (isCString(type) || isVoidPointer(type)) ? QString() : QLatin1String("&"));
}

QString ShibokenGenerator::cpythonToPythonConversionFunction(const AbstractMetaClass *metaClass)
{
    return cpythonToPythonConversionFunction(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonToPythonConversionFunction(const TypeEntry *type)
{
    if (isWrapperType(type)) {
        const QString conversion = type->isValue() ? QLatin1String("copy") : QLatin1String("pointer");
         QString result = QLatin1String("Shiboken::Conversions::") + conversion
             + QLatin1String("ToPython(reinterpret_cast<SbkObjectType *>(") + cpythonTypeNameExt(type)
             + QLatin1String("), ");
         if (conversion != QLatin1String("pointer"))
             result += QLatin1Char('&');
        return result;
    }

    return QStringLiteral("Shiboken::Conversions::copyToPython(%1, &").arg(converterObject(type));
}

QString ShibokenGenerator::argumentString(const AbstractMetaFunction *func,
                                          const AbstractMetaArgument *argument,
                                          Options options) const
{
    QString modified_type;
    if (!(options & OriginalTypeDescription))
        modified_type = func->typeReplaced(argument->argumentIndex() + 1);
    QString arg;

    if (modified_type.isEmpty())
        arg = translateType(argument->type(), func->implementingClass(), options);
    else
        arg = modified_type.replace(QLatin1Char('$'), QLatin1Char('.'));

    if (!(options & Generator::SkipName)) {
        // "int a", "int a[]"
        const int arrayPos = arg.indexOf(QLatin1Char('['));
        if (arrayPos != -1)
            arg.insert(arrayPos, QLatin1Char(' ') + argument->name());
        else
            arg.append(QLatin1Char(' ') + argument->name());
    }

    if ((options & Generator::SkipDefaultValues) != Generator::SkipDefaultValues &&
        !argument->originalDefaultValueExpression().isEmpty())
    {
        QString default_value = argument->originalDefaultValueExpression();
        if (default_value == QLatin1String("NULL"))
            default_value = QLatin1String(NULL_PTR);

        //WORKAROUND: fix this please
        if (default_value.startsWith(QLatin1String("new ")))
            default_value.remove(0, 4);

        arg += QLatin1String(" = ") + default_value;
    }

    return arg;
}

void ShibokenGenerator::writeArgument(QTextStream &s,
                                      const AbstractMetaFunction *func,
                                      const AbstractMetaArgument *argument,
                                      Options options) const
{
    s << argumentString(func, argument, options);
}

void ShibokenGenerator::writeFunctionArguments(QTextStream &s,
                                               const AbstractMetaFunction *func,
                                               Options options) const
{
    AbstractMetaArgumentList arguments = func->arguments();

    if (options & Generator::WriteSelf) {
        s << func->implementingClass()->name() << '&';
        if (!(options & SkipName))
            s << " self";
    }

    int argUsed = 0;
    for (int i = 0; i < arguments.size(); ++i) {
        if ((options & Generator::SkipRemovedArguments) && func->argumentRemoved(i+1))
            continue;

        if ((options & Generator::WriteSelf) || argUsed != 0)
            s << ", ";
        writeArgument(s, func, arguments[i], options);
        argUsed++;
    }
}

GeneratorContext ShibokenGenerator::contextForClass(const AbstractMetaClass *c) const
{
    GeneratorContext result = Generator::contextForClass(c);
    if (shouldGenerateCppWrapper(c)) {
        result.m_type = GeneratorContext::WrappedClass;
        result.m_wrappername = wrapperName(c);
    }
    return result;
}

QString ShibokenGenerator::functionReturnType(const AbstractMetaFunction *func, Options options) const
{
    QString modifiedReturnType = QString(func->typeReplaced(0));
    if (!modifiedReturnType.isEmpty() && !(options & OriginalTypeDescription))
        return modifiedReturnType;
    return translateType(func->type(), func->implementingClass(), options);
}

QString ShibokenGenerator::functionSignature(const AbstractMetaFunction *func,
                                             const QString &prepend,
                                             const QString &append,
                                             Options options,
                                             int /* argCount */) const
{
    QString result;
    QTextStream s(&result);
    // The actual function
    if (!(func->isEmptyFunction() ||
          func->isNormal() ||
          func->isSignal())) {
        options |= Generator::SkipReturnType;
    } else {
        s << functionReturnType(func, options) << ' ';
    }

    // name
    QString name(func->originalName());
    if (func->isConstructor())
        name = wrapperName(func->ownerClass());

    s << prepend << name << append << '(';
    writeFunctionArguments(s, func, options);
    s << ')';

    if (func->isConstant() && !(options & Generator::ExcludeMethodConst))
        s << " const";

    if (func->exceptionSpecification() == ExceptionSpecification::NoExcept)
        s << " noexcept";

    return result;
}

void ShibokenGenerator::writeArgumentNames(QTextStream &s,
                                           const AbstractMetaFunction *func,
                                           Options options) const
{
    const AbstractMetaArgumentList arguments = func->arguments();
    int argCount = 0;
    for (auto argument : arguments) {
        const int index = argument->argumentIndex() + 1;
        if ((options & Generator::SkipRemovedArguments) && (func->argumentRemoved(index)))
            continue;

        s << ((argCount > 0) ? ", " : "") << argument->name();

        if (((options & Generator::VirtualCall) == 0)
            && (!func->conversionRule(TypeSystem::NativeCode, index).isEmpty()
                || !func->conversionRule(TypeSystem::TargetLangCode, index).isEmpty())
            && !func->isConstructor()) {
           s << CONV_RULE_OUT_VAR_SUFFIX;
        }

        argCount++;
    }
}

void ShibokenGenerator::writeFunctionCall(QTextStream &s,
                                          const AbstractMetaFunction *func,
                                          Options options) const
{
    if (!(options & Generator::SkipName))
        s << (func->isConstructor() ? func->ownerClass()->qualifiedCppName() : func->originalName());
    s << '(';
    writeArgumentNames(s, func, options);
    s << ')';
}

void ShibokenGenerator::writeUnusedVariableCast(QTextStream &s, const QString &variableName)
{
    s << INDENT << "SBK_UNUSED(" << variableName<< ")\n";
}

static bool filterFunction(const AbstractMetaFunction *func, bool avoidProtectedHack)
{
    switch (func->functionType()) {
    case AbstractMetaFunction::DestructorFunction:
    case AbstractMetaFunction::SignalFunction:
    case AbstractMetaFunction::GetAttroFunction:
    case AbstractMetaFunction::SetAttroFunction:
        return false;
    default:
        break;
    }
    if (func->usesRValueReferences())
        return false;
    if (func->isModifiedRemoved() && !func->isAbstract()
        && (!avoidProtectedHack || !func->isProtected())) {
        return false;
    }
    return true;
}

AbstractMetaFunctionList ShibokenGenerator::filterFunctions(const AbstractMetaClass *metaClass)
{
    AbstractMetaFunctionList result;
    const AbstractMetaFunctionList &funcs = metaClass->functions();
    result.reserve(funcs.size());
    for (AbstractMetaFunction *func : funcs) {
        if (filterFunction(func, avoidProtectedHack()))
            result.append(func);
    }
    return result;
}

ShibokenGenerator::ExtendedConverterData ShibokenGenerator::getExtendedConverters() const
{
    ExtendedConverterData extConvs;
    for (const AbstractMetaClass *metaClass : classes()) {
        // Use only the classes for the current module.
        if (!shouldGenerate(metaClass))
            continue;
        const AbstractMetaFunctionList &overloads = metaClass->operatorOverloads(AbstractMetaClass::ConversionOp);
        for (AbstractMetaFunction *convOp : overloads) {
            // Get only the conversion operators that return a type from another module,
            // that are value-types and were not removed in the type system.
            const TypeEntry *convType = convOp->type()->typeEntry();
            if (convType->generateCode() || !convType->isValue()
                || convOp->isModifiedRemoved())
                continue;
            extConvs[convType].append(convOp->ownerClass());
        }
    }
    return extConvs;
}

QVector<const CustomConversion *> ShibokenGenerator::getPrimitiveCustomConversions()
{
    QVector<const CustomConversion *> conversions;
    const PrimitiveTypeEntryList &primitiveTypeList = primitiveTypes();
    for (const PrimitiveTypeEntry *type : primitiveTypeList) {
        if (!shouldGenerateTypeEntry(type) || !isUserPrimitive(type) || !type->customConversion())
            continue;

        conversions << type->customConversion();
    }
    return conversions;
}

static QString getArgumentsFromMethodCall(const QString &str)
{
    // It would be way nicer to be able to use a Perl like
    // regular expression that accepts temporary variables
    // to count the parenthesis.
    // For more information check this:
    // http://perl.plover.com/yak/regex/samples/slide083.html
    static QLatin1String funcCall("%CPPSELF.%FUNCTION_NAME");
    int pos = str.indexOf(funcCall);
    if (pos == -1)
        return QString();
    pos = pos + funcCall.size();
    while (str.at(pos) == QLatin1Char(' ') || str.at(pos) == QLatin1Char('\t'))
        ++pos;
    if (str.at(pos) == QLatin1Char('('))
        ++pos;
    int begin = pos;
    int counter = 1;
    while (counter != 0) {
        if (str.at(pos) == QLatin1Char('('))
            ++counter;
        else if (str.at(pos) == QLatin1Char(')'))
            --counter;
        ++pos;
    }
    return str.mid(begin, pos-begin-1);
}

QString ShibokenGenerator::getCodeSnippets(const CodeSnipList &codeSnips,
                                           TypeSystem::CodeSnipPosition position,
                                           TypeSystem::Language language)
{
    QString code;
    QTextStream c(&code);
    for (const CodeSnip &snip : codeSnips) {
        if ((position != TypeSystem::CodeSnipPositionAny && snip.position != position) || !(snip.language & language))
            continue;
        QString snipCode;
        QTextStream sc(&snipCode);
        formatCode(sc, snip.code(), INDENT);
        c << snipCode;
    }
    return code;
}

void ShibokenGenerator::processClassCodeSnip(QString &code, const GeneratorContext &context)
{
    auto metaClass = context.metaClass();
    // Replace template variable by the Python Type object
    // for the class context in which the variable is used.
    code.replace(QLatin1String("%PYTHONTYPEOBJECT"),
                 cpythonTypeName(metaClass) + QLatin1String("->type"));
    const QString className = context.useWrapper()
        ? context.wrapperName() : metaClass->qualifiedCppName();
    code.replace(QLatin1String("%TYPE"), className);
    code.replace(QLatin1String("%CPPTYPE"), metaClass->name());

    processCodeSnip(code);
}

void ShibokenGenerator::processCodeSnip(QString &code)
{
    // replace "toPython" converters
    replaceConvertToPythonTypeSystemVariable(code);

    // replace "toCpp" converters
    replaceConvertToCppTypeSystemVariable(code);

    // replace "isConvertible" check
    replaceIsConvertibleToCppTypeSystemVariable(code);

    // replace "checkType" check
    replaceTypeCheckTypeSystemVariable(code);
}

ShibokenGenerator::ArgumentVarReplacementList ShibokenGenerator::getArgumentReplacement(const AbstractMetaFunction *func,
                                                                                        bool usePyArgs, TypeSystem::Language language,
                                                                                        const AbstractMetaArgument *lastArg)
{
    ArgumentVarReplacementList argReplacements;
    TypeSystem::Language convLang = (language == TypeSystem::TargetLangCode)
                                    ? TypeSystem::NativeCode : TypeSystem::TargetLangCode;
    int removed = 0;
    for (int i = 0; i < func->arguments().size(); ++i) {
        const AbstractMetaArgument *arg = func->arguments().at(i);
        QString argValue;
        if (language == TypeSystem::TargetLangCode) {
            bool hasConversionRule = !func->conversionRule(convLang, i+1).isEmpty();
            const bool argRemoved = func->argumentRemoved(i+1);
            if (argRemoved)
                ++removed;
            if (argRemoved && hasConversionRule)
                argValue = arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
            else if (argRemoved || (lastArg && arg->argumentIndex() > lastArg->argumentIndex()))
                argValue = QLatin1String(CPP_ARG_REMOVED) + QString::number(i);
            if (!argRemoved && argValue.isEmpty()) {
                int argPos = i - removed;
                const AbstractMetaType *type = arg->type();
                QString typeReplaced = func->typeReplaced(arg->argumentIndex() + 1);
                if (!typeReplaced.isEmpty()) {
                    AbstractMetaType *builtType = buildAbstractMetaTypeFromString(typeReplaced);
                    if (builtType)
                        type = builtType;
                }
                if (type->typeEntry()->isCustom()) {
                    argValue = usePyArgs
                               ? pythonArgsAt(argPos) : QLatin1String(PYTHON_ARG);
                } else {
                    argValue = hasConversionRule
                               ? arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX)
                               : QLatin1String(CPP_ARG) + QString::number(argPos);
                    if (isWrapperType(type)) {
                        if (type->referenceType() == LValueReference && !isPointer(type))
                            argValue.prepend(QLatin1Char('*'));
                    }
                }
            }
        } else {
            argValue = arg->name();
        }
        if (!argValue.isEmpty())
            argReplacements << ArgumentVarReplacementPair(arg, argValue);

    }
    return argReplacements;
}

void ShibokenGenerator::writeClassCodeSnips(QTextStream &s,
                                       const CodeSnipList &codeSnips,
                                       TypeSystem::CodeSnipPosition position,
                                       TypeSystem::Language language,
                                       const GeneratorContext &context)
{
    QString code = getCodeSnippets(codeSnips, position, language);
    if (code.isEmpty())
        return;
    processClassCodeSnip(code, context);
    s << INDENT << "// Begin code injection\n";
    s << code;
    s << INDENT << "// End of code injection\n\n";
}

void ShibokenGenerator::writeCodeSnips(QTextStream &s,
                                       const CodeSnipList &codeSnips,
                                       TypeSystem::CodeSnipPosition position,
                                       TypeSystem::Language language)
{
    QString code = getCodeSnippets(codeSnips, position, language);
    if (code.isEmpty())
        return;
    processCodeSnip(code);
    s << INDENT << "// Begin code injection\n";
    s << code;
    s << INDENT << "// End of code injection\n\n";
}

void ShibokenGenerator::writeCodeSnips(QTextStream &s,
                                       const CodeSnipList &codeSnips,
                                       TypeSystem::CodeSnipPosition position,
                                       TypeSystem::Language language,
                                       const AbstractMetaFunction *func,
                                       const AbstractMetaArgument *lastArg)
{
    QString code = getCodeSnippets(codeSnips, position, language);
    if (code.isEmpty())
        return;

    // Calculate the real number of arguments.
    int argsRemoved = 0;
    for (int i = 0; i < func->arguments().size(); i++) {
        if (func->argumentRemoved(i+1))
            argsRemoved++;
    }

    const auto &groups = func->implementingClass()
        ? getFunctionGroups(func->implementingClass())
        : getGlobalFunctionGroups();
    OverloadData od(groups[func->name()], this);
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(od);

    // Replace %PYARG_# variables.
    code.replace(QLatin1String("%PYARG_0"), QLatin1String(PYTHON_RETURN_VAR));

    static const QRegularExpression pyArgsRegex(QStringLiteral("%PYARG_(\\d+)"));
    Q_ASSERT(pyArgsRegex.isValid());
    if (language == TypeSystem::TargetLangCode) {
        if (usePyArgs) {
            code.replace(pyArgsRegex, QLatin1String(PYTHON_ARGS) + QLatin1String("[\\1-1]"));
        } else {
            static const QRegularExpression pyArgsRegexCheck(QStringLiteral("%PYARG_([2-9]+)"));
            Q_ASSERT(pyArgsRegexCheck.isValid());
            const QRegularExpressionMatch match = pyArgsRegexCheck.match(code);
            if (match.hasMatch()) {
                qCWarning(lcShiboken).noquote().nospace()
                    << msgWrongIndex("%PYARG", match.captured(1), func);
                return;
            }
            code.replace(QLatin1String("%PYARG_1"), QLatin1String(PYTHON_ARG));
        }
    } else {
        // Replaces the simplest case of attribution to a
        // Python argument on the binding virtual method.
        static const QRegularExpression pyArgsAttributionRegex(QStringLiteral("%PYARG_(\\d+)\\s*=[^=]\\s*([^;]+)"));
        Q_ASSERT(pyArgsAttributionRegex.isValid());
        code.replace(pyArgsAttributionRegex, QLatin1String("PyTuple_SET_ITEM(")
                     + QLatin1String(PYTHON_ARGS) + QLatin1String(", \\1-1, \\2)"));
        code.replace(pyArgsRegex, QLatin1String("PyTuple_GET_ITEM(")
                     + QLatin1String(PYTHON_ARGS) + QLatin1String(", \\1-1)"));
    }

    // Replace %ARG#_TYPE variables.
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument *arg : arguments) {
        QString argTypeVar = QStringLiteral("%ARG%1_TYPE").arg(arg->argumentIndex() + 1);
        QString argTypeVal = arg->type()->cppSignature();
        code.replace(argTypeVar, argTypeVal);
    }

    static const QRegularExpression cppArgTypeRegexCheck(QStringLiteral("%ARG(\\d+)_TYPE"));
    Q_ASSERT(cppArgTypeRegexCheck.isValid());
    QRegularExpressionMatchIterator rit = cppArgTypeRegexCheck.globalMatch(code);
    while (rit.hasNext()) {
        QRegularExpressionMatch match = rit.next();
        qCWarning(lcShiboken).noquote().nospace()
            << msgWrongIndex("%ARG#_TYPE", match.captured(1), func);
    }

    // Replace template variable for return variable name.
    if (func->isConstructor()) {
        code.replace(QLatin1String("%0."), QLatin1String("cptr->"));
        code.replace(QLatin1String("%0"), QLatin1String("cptr"));
    } else if (!func->isVoid()) {
        QString returnValueOp = isPointerToWrapperType(func->type())
            ? QLatin1String("%1->") : QLatin1String("%1.");
        if (ShibokenGenerator::isWrapperType(func->type()))
            code.replace(QLatin1String("%0."), returnValueOp.arg(QLatin1String(CPP_RETURN_VAR)));
        code.replace(QLatin1String("%0"), QLatin1String(CPP_RETURN_VAR));
    }

    // Replace template variable for self Python object.
    QString pySelf = language == TypeSystem::NativeCode
        ? QLatin1String("pySelf") : QLatin1String("self");
    code.replace(QLatin1String("%PYSELF"), pySelf);

    // Replace template variable for a pointer to C++ of this object.
    if (func->implementingClass()) {
        QString replacement = func->isStatic() ? QLatin1String("%1::") : QLatin1String("%1->");
        QString cppSelf;
        if (func->isStatic())
            cppSelf = func->ownerClass()->qualifiedCppName();
        else if (language == TypeSystem::NativeCode)
            cppSelf = QLatin1String("this");
        else
            cppSelf = QLatin1String(CPP_SELF_VAR);

        // On comparison operator CPP_SELF_VAR is always a reference.
        if (func->isComparisonOperator())
            replacement = QLatin1String("%1.");

        if (func->isVirtual() && !func->isAbstract() && (!avoidProtectedHack() || !func->isProtected())) {
            QString methodCallArgs = getArgumentsFromMethodCall(code);
            if (!methodCallArgs.isEmpty()) {
                const QString pattern = QStringLiteral("%CPPSELF.%FUNCTION_NAME(%1)").arg(methodCallArgs);
                if (func->name() == QLatin1String("metaObject")) {
                    QString wrapperClassName = wrapperName(func->ownerClass());
                    QString cppSelfVar = avoidProtectedHack()
                                         ? QLatin1String("%CPPSELF")
                                         : QStringLiteral("reinterpret_cast<%1 *>(%CPPSELF)").arg(wrapperClassName);
                    code.replace(pattern,
                                 QString::fromLatin1("(Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject *>(%1))"
                                         " ? %2->::%3::%FUNCTION_NAME(%4)"
                                         " : %CPPSELF.%FUNCTION_NAME(%4))").arg(pySelf, cppSelfVar, wrapperClassName, methodCallArgs));
                } else {
                    code.replace(pattern,
                                 QString::fromLatin1("(Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject *>(%1))"
                                         " ? %CPPSELF->::%TYPE::%FUNCTION_NAME(%2)"
                                         " : %CPPSELF.%FUNCTION_NAME(%2))").arg(pySelf, methodCallArgs));
                }
            }
        }

        code.replace(QLatin1String("%CPPSELF."), replacement.arg(cppSelf));
        code.replace(QLatin1String("%CPPSELF"), cppSelf);

        if (code.indexOf(QLatin1String("%BEGIN_ALLOW_THREADS")) > -1) {
            if (code.count(QLatin1String("%BEGIN_ALLOW_THREADS")) == code.count(QLatin1String("%END_ALLOW_THREADS"))) {
                code.replace(QLatin1String("%BEGIN_ALLOW_THREADS"), QLatin1String(BEGIN_ALLOW_THREADS));
                code.replace(QLatin1String("%END_ALLOW_THREADS"), QLatin1String(END_ALLOW_THREADS));
            } else {
                qCWarning(lcShiboken) << "%BEGIN_ALLOW_THREADS and %END_ALLOW_THREADS mismatch";
            }
        }

        // replace template variable for the Python Type object for the
        // class implementing the method in which the code snip is written
        if (func->isStatic()) {
            code.replace(QLatin1String("%PYTHONTYPEOBJECT"),
                                       cpythonTypeName(func->implementingClass()) + QLatin1String("->type"));
        } else {
            code.replace(QLatin1String("%PYTHONTYPEOBJECT."), pySelf + QLatin1String("->ob_type->"));
            code.replace(QLatin1String("%PYTHONTYPEOBJECT"), pySelf + QLatin1String("->ob_type"));
        }
    }

    // Replaces template %ARGUMENT_NAMES and %# variables by argument variables and values.
    // Replaces template variables %# for individual arguments.
    const ArgumentVarReplacementList &argReplacements = getArgumentReplacement(func, usePyArgs, language, lastArg);

    QStringList args;
    for (const ArgumentVarReplacementPair &pair : argReplacements) {
        if (pair.second.startsWith(QLatin1String(CPP_ARG_REMOVED)))
            continue;
        args << pair.second;
    }
    code.replace(QLatin1String("%ARGUMENT_NAMES"), args.join(QLatin1String(", ")));

    for (const ArgumentVarReplacementPair &pair : argReplacements) {
        const AbstractMetaArgument *arg = pair.first;
        int idx = arg->argumentIndex() + 1;
        AbstractMetaType *type = arg->type();
        QString typeReplaced = func->typeReplaced(arg->argumentIndex() + 1);
        if (!typeReplaced.isEmpty()) {
            AbstractMetaType *builtType = buildAbstractMetaTypeFromString(typeReplaced);
            if (builtType)
                type = builtType;
        }
        if (isWrapperType(type)) {
            QString replacement = pair.second;
            if (type->referenceType() == LValueReference && !isPointer(type))
                replacement.remove(0, 1);
            if (type->referenceType() == LValueReference || isPointer(type))
                code.replace(QString::fromLatin1("%%1.").arg(idx), replacement + QLatin1String("->"));
        }
        code.replace(placeHolderRegex(idx), pair.second);
    }

    if (language == TypeSystem::NativeCode) {
        // Replaces template %PYTHON_ARGUMENTS variable with a pointer to the Python tuple
        // containing the converted virtual method arguments received from C++ to be passed
        // to the Python override.
        code.replace(QLatin1String("%PYTHON_ARGUMENTS"), QLatin1String(PYTHON_ARGS));

        // replace variable %PYTHON_METHOD_OVERRIDE for a pointer to the Python method
        // override for the C++ virtual method in which this piece of code was inserted
        code.replace(QLatin1String("%PYTHON_METHOD_OVERRIDE"), QLatin1String(PYTHON_OVERRIDE_VAR));
    }

    if (avoidProtectedHack()) {
        // If the function being processed was added by the user via type system,
        // Shiboken needs to find out if there are other overloads for the same method
        // name and if any of them is of the protected visibility. This is used to replace
        // calls to %FUNCTION_NAME on user written custom code for calls to the protected
        // dispatcher.
        bool hasProtectedOverload = false;
        if (func->isUserAdded()) {
            const AbstractMetaFunctionList &funcs = getFunctionOverloads(func->ownerClass(), func->name());
            for (const AbstractMetaFunction *f : funcs)
                hasProtectedOverload |= f->isProtected();
        }

        if (func->isProtected() || hasProtectedOverload) {
            code.replace(QLatin1String("%TYPE::%FUNCTION_NAME"),
                         QStringLiteral("%1::%2_protected")
                         .arg(wrapperName(func->ownerClass()), func->originalName()));
            code.replace(QLatin1String("%FUNCTION_NAME"),
                         func->originalName() + QLatin1String("_protected"));
        }
    }

    if (func->isConstructor() && shouldGenerateCppWrapper(func->ownerClass()))
        code.replace(QLatin1String("%TYPE"), wrapperName(func->ownerClass()));

    if (func->ownerClass())
        code.replace(QLatin1String("%CPPTYPE"), func->ownerClass()->name());

    replaceTemplateVariables(code, func);

    processCodeSnip(code);
    s << INDENT << "// Begin code injection\n";
    s << code;
    s << INDENT << "// End of code injection\n\n";
}

// Returns true if the string is an expression,
// and false if it is a variable.
static bool isVariable(const QString &code)
{
    static const QRegularExpression expr(QStringLiteral("^\\s*\\*?\\s*[A-Za-z_][A-Za-z_0-9.]*\\s*(?:\\[[^\\[]+\\])*$"));
    Q_ASSERT(expr.isValid());
    return expr.match(code.trimmed()).hasMatch();
}

// A miniature normalizer that puts a type string into a format
// suitable for comparison with AbstractMetaType::cppSignature()
// result.
static QString miniNormalizer(const QString &varType)
{
    QString normalized = varType.trimmed();
    if (normalized.isEmpty())
        return normalized;
    if (normalized.startsWith(QLatin1String("::")))
        normalized.remove(0, 2);
    QString suffix;
    while (normalized.endsWith(QLatin1Char('*')) || normalized.endsWith(QLatin1Char('&'))) {
        suffix.prepend(normalized.at(normalized.count() - 1));
        normalized.chop(1);
        normalized = normalized.trimmed();
    }
    const QString result = normalized + QLatin1Char(' ') + suffix;
    return result.trimmed();
}
// The position must indicate the first character after the opening '('.
// ATTENTION: do not modify this function to trim any resulting string!
// This must be done elsewhere.
static QString getConverterTypeSystemVariableArgument(const QString &code, int pos)
{
    QString arg;
    int parenthesisDepth = 0;
    int count = 0;
    while (pos + count < code.count()) {
        char c = code.at(pos+count).toLatin1(); // toAscii is gone
        if (c == '(') {
            ++parenthesisDepth;
        } else if (c == ')') {
            if (parenthesisDepth == 0) {
                arg = code.mid(pos, count).trimmed();
                break;
            }
            --parenthesisDepth;
        }
        ++count;
    }
    if (parenthesisDepth != 0)
        qFatal("Unbalanced parenthesis on type system converter variable call.");
    return arg;
}
using StringPair = QPair<QString, QString>;

void ShibokenGenerator::replaceConverterTypeSystemVariable(TypeSystemConverterVariable converterVariable, QString &code)
{
    QVector<StringPair> replacements;
    QRegularExpressionMatchIterator rit = m_typeSystemConvRegEx[converterVariable].globalMatch(code);
    while (rit.hasNext()) {
        const QRegularExpressionMatch match = rit.next();
        const QStringList list = match.capturedTexts();
        QString conversionString = list.constFirst();
        const QString &conversionTypeName = list.constLast();
        QString message;
        const AbstractMetaType *conversionType = buildAbstractMetaTypeFromString(conversionTypeName, &message);
        if (!conversionType) {
            qFatal("%s", qPrintable(msgCannotFindType(conversionTypeName,
                                                      m_typeSystemConvName[converterVariable],
                                                      message)));
        }
        QString conversion;
        QTextStream c(&conversion);
        switch (converterVariable) {
            case TypeSystemToCppFunction: {
                int end = match.capturedStart();
                int start = end;
                while (start > 0 && code.at(start) != QLatin1Char('\n'))
                    --start;
                while (code.at(start).isSpace())
                    ++start;
                QString varType = code.mid(start, end - start);
                conversionString = varType + list.constFirst();
                varType = miniNormalizer(varType);
                QString varName = list.at(1).trimmed();
                if (!varType.isEmpty()) {
                    const QString conversionSignature = conversionType->cppSignature();
                    if (varType != QLatin1String("auto") && varType != conversionSignature)
                        qFatal("%s", qPrintable(msgConversionTypesDiffer(varType, conversionSignature)));
                    c << getFullTypeName(conversionType) << ' ' << varName;
                    writeMinimalConstructorExpression(c, conversionType);
                    c << ";\n";
                    Indentation indent(INDENT);
                    c << INDENT;
                }
                c << cpythonToCppConversionFunction(conversionType);
                QString prefix;
                if (varName.startsWith(QLatin1Char('*'))) {
                    varName.remove(0, 1);
                    varName = varName.trimmed();
                } else {
                    prefix = QLatin1Char('&');
                }
                QString arg = getConverterTypeSystemVariableArgument(code, match.capturedEnd());
                conversionString += arg;
                c << arg << ", " << prefix << '(' << varName << ')';
                break;
            }
            case TypeSystemCheckFunction:
                conversion = cpythonCheckFunction(conversionType);
                if (conversionType->typeEntry()->isPrimitive()
                    && (conversionType->typeEntry()->name() == QLatin1String("PyObject")
                        || !conversion.endsWith(QLatin1Char(' ')))) {
                    c << '(';
                    break;
                }
            Q_FALLTHROUGH();
            case TypeSystemIsConvertibleFunction:
                if (conversion.isEmpty())
                    conversion = cpythonIsConvertibleFunction(conversionType);
            Q_FALLTHROUGH();
            case TypeSystemToPythonFunction:
                if (conversion.isEmpty())
                    conversion = cpythonToPythonConversionFunction(conversionType);
            Q_FALLTHROUGH();
            default: {
                QString arg = getConverterTypeSystemVariableArgument(code, match.capturedEnd());
                conversionString += arg;
                if (converterVariable == TypeSystemToPythonFunction && !isVariable(arg)) {
                    qFatal("Only variables are acceptable as argument to %%CONVERTTOPYTHON type system variable on code snippet: '%s'",
                           qPrintable(code));
                }
                if (conversion.contains(QLatin1String("%in"))) {
                    conversion.prepend(QLatin1Char('('));
                    conversion.replace(QLatin1String("%in"), arg);
                } else {
                    c << arg;
                }
            }
        }
        replacements.append(qMakePair(conversionString, conversion));
    }
    for (const StringPair &rep : qAsConst(replacements))
        code.replace(rep.first, rep.second);
}

bool ShibokenGenerator::injectedCodeUsesPySelf(const AbstractMetaFunction *func)
{
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode);
    for (const CodeSnip &snip : qAsConst(snips)) {
        if (snip.code().contains(QLatin1String("%PYSELF")))
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeCallsCppFunction(const GeneratorContext &context,
                                                     const AbstractMetaFunction *func)
{
    QString funcCall = func->originalName() + QLatin1Char('(');
    QString wrappedCtorCall;
    if (func->isConstructor()) {
        funcCall.prepend(QLatin1String("new "));
        const auto owner = func->ownerClass();
        const QString className = context.useWrapper()
            ? context.wrapperName() : owner->qualifiedCppName();
        wrappedCtorCall = QLatin1String("new ") + className + QLatin1Char('(');
    }
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
    for (const CodeSnip &snip : qAsConst(snips)) {
        if (snip.code().contains(QLatin1String("%FUNCTION_NAME(")) || snip.code().contains(funcCall)
            || (func->isConstructor()
                && ((func->ownerClass()->isPolymorphic() && snip.code().contains(wrappedCtorCall))
                    || snip.code().contains(QLatin1String("new %TYPE("))))
            )
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeCallsPythonOverride(const AbstractMetaFunction *func)
{
    static const QRegularExpression overrideCallRegexCheck(QStringLiteral("PyObject_Call\\s*\\(\\s*%PYTHON_METHOD_OVERRIDE\\s*,"));
    Q_ASSERT(overrideCallRegexCheck.isValid());
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode);
    for (const CodeSnip &snip : qAsConst(snips)) {
        if (snip.code().contains(overrideCallRegexCheck))
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeHasReturnValueAttribution(const AbstractMetaFunction *func, TypeSystem::Language language)
{
    static const QRegularExpression retValAttributionRegexCheck_native(QStringLiteral("%0\\s*=[^=]\\s*.+"));
    Q_ASSERT(retValAttributionRegexCheck_native.isValid());
    static const QRegularExpression retValAttributionRegexCheck_target(QStringLiteral("%PYARG_0\\s*=[^=]\\s*.+"));
    Q_ASSERT(retValAttributionRegexCheck_target.isValid());
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, language);
    for (const CodeSnip &snip : qAsConst(snips)) {
        if (language == TypeSystem::TargetLangCode) {
            if (snip.code().contains(retValAttributionRegexCheck_target))
                return true;
        } else {
            if (snip.code().contains(retValAttributionRegexCheck_native))
                return true;
        }
    }
    return false;
}

bool ShibokenGenerator::injectedCodeUsesArgument(const AbstractMetaFunction *func, int argumentIndex)
{
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny);
    const QRegularExpression argRegEx = placeHolderRegex(argumentIndex + 1);
    for (const CodeSnip &snip : qAsConst(snips)) {
        QString code = snip.code();
        if (code.contains(QLatin1String("%ARGUMENT_NAMES")) || code.contains(argRegEx))
            return true;
    }
    return false;
}

bool ShibokenGenerator::useOverrideCaching(const AbstractMetaClass *metaClass)
{
    return metaClass->isPolymorphic();
}

ShibokenGenerator::AttroCheck ShibokenGenerator::checkAttroFunctionNeeds(const AbstractMetaClass *metaClass) const
{
    AttroCheck result;
    if (metaClass->typeEntry()->isSmartPointer()) {
        result |= AttroCheckFlag::GetattroSmartPointer | AttroCheckFlag::SetattroSmartPointer;
    } else {
        if (getGeneratorClassInfo(metaClass).needsGetattroFunction)
            result |= AttroCheckFlag::GetattroOverloads;
        if (metaClass->queryFirstFunction(metaClass->functions(),
                                          AbstractMetaClass::GetAttroFunction)) {
            result |= AttroCheckFlag::GetattroUser;
        }
        if (usePySideExtensions() && metaClass->qualifiedCppName() == qObjectT())
            result |= AttroCheckFlag::SetattroQObject;
        if (useOverrideCaching(metaClass))
            result |= AttroCheckFlag::SetattroMethodOverride;
        if (metaClass->queryFirstFunction(metaClass->functions(),
                                          AbstractMetaClass::SetAttroFunction)) {
            result |= AttroCheckFlag::SetattroUser;
        }
        // PYSIDE-1255: If setattro is generated for a class inheriting
        // QObject, the property code needs to be generated, too.
        if ((result & AttroCheckFlag::SetattroMask) != 0
            && !result.testFlag(AttroCheckFlag::SetattroQObject)
            && metaClass->isQObject()) {
            result |= AttroCheckFlag::SetattroQObject;
        }
    }
    return result;
}

bool ShibokenGenerator::classNeedsGetattroFunctionImpl(const AbstractMetaClass *metaClass)
{
    if (!metaClass)
        return false;
    if (metaClass->typeEntry()->isSmartPointer())
        return true;
    const auto &functionGroup = getFunctionGroups(metaClass);
    for (auto it = functionGroup.cbegin(), end = functionGroup.cend(); it != end; ++it) {
        AbstractMetaFunctionList overloads;
        for (AbstractMetaFunction *func : qAsConst(it.value())) {
            if (func->isAssignmentOperator() || func->isCastOperator() || func->isModifiedRemoved()
                || func->isPrivate() || func->ownerClass() != func->implementingClass()
                || func->isConstructor() || func->isOperatorOverload())
                continue;
            overloads.append(func);
        }
        if (overloads.isEmpty())
            continue;
        if (OverloadData::hasStaticAndInstanceFunctions(overloads))
            return true;
    }
    return false;
}

AbstractMetaFunctionList ShibokenGenerator::getMethodsWithBothStaticAndNonStaticMethods(const AbstractMetaClass *metaClass)
{
    AbstractMetaFunctionList methods;
    if (metaClass) {
        const auto &functionGroups = getFunctionGroups(metaClass);
        for (auto it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
            AbstractMetaFunctionList overloads;
            for (AbstractMetaFunction *func : qAsConst(it.value())) {
                if (func->isAssignmentOperator() || func->isCastOperator() || func->isModifiedRemoved()
                    || func->isPrivate() || func->ownerClass() != func->implementingClass()
                    || func->isConstructor() || func->isOperatorOverload())
                    continue;
                overloads.append(func);
            }
            if (overloads.isEmpty())
                continue;
            if (OverloadData::hasStaticAndInstanceFunctions(overloads))
                methods.append(overloads.constFirst());
        }
    }
    return methods;
}

AbstractMetaClassList ShibokenGenerator::getBaseClasses(const AbstractMetaClass *metaClass) const
{
    AbstractMetaClassList baseClasses;
    if (metaClass) {
        QStringList baseClassNames(metaClass->baseClassNames());
        const QString defaultSuperclass = metaClass->typeEntry()->defaultSuperclass();
        if (!defaultSuperclass.isEmpty()) {
            int index = baseClassNames.indexOf(defaultSuperclass);
            if (index >= 0)
                baseClassNames.move(index, 0);
        }
        for (const QString &parent : baseClassNames) {
            AbstractMetaClass *clazz = AbstractMetaClass::findClass(classes(), parent);
            if (clazz)
                baseClasses << clazz;
        }
    }
    return baseClasses;
}

const AbstractMetaClass *ShibokenGenerator::getMultipleInheritingClass(const AbstractMetaClass *metaClass)
{
    if (!metaClass || metaClass->baseClassNames().isEmpty())
        return nullptr;
    if (metaClass->baseClassNames().size() > 1)
        return metaClass;
    return getMultipleInheritingClass(metaClass->baseClass());
}

AbstractMetaClassList ShibokenGenerator::getAllAncestors(const AbstractMetaClass *metaClass) const
{
    AbstractMetaClassList result;
    if (metaClass) {
        AbstractMetaClassList baseClasses = getBaseClasses(metaClass);
        for (AbstractMetaClass *base : qAsConst(baseClasses)) {
            result.append(base);
            result.append(getAllAncestors(base));
        }
    }
    return result;
}

QString ShibokenGenerator::getModuleHeaderFileName(const QString &moduleName) const
{
    return moduleCppPrefix(moduleName).toLower() + QLatin1String("_python.h");
}

bool ShibokenGenerator::isCopyable(const AbstractMetaClass *metaClass)

{
    if (metaClass->isNamespace() || isObjectType(metaClass))
        return false;
    if (metaClass->typeEntry()->copyable() == ComplexTypeEntry::Unknown)
        return metaClass->hasCloneOperator();

    return metaClass->typeEntry()->copyable() == ComplexTypeEntry::CopyableSet;
}

AbstractMetaType *ShibokenGenerator::buildAbstractMetaTypeFromString(QString typeSignature,
                                                                     QString *errorMessage)
{
    typeSignature = typeSignature.trimmed();
    if (typeSignature.startsWith(QLatin1String("::")))
        typeSignature.remove(0, 2);

    auto it = m_metaTypeFromStringCache.find(typeSignature);
    if (it == m_metaTypeFromStringCache.end()) {
        AbstractMetaType *metaType =
              AbstractMetaBuilder::translateType(typeSignature, nullptr, {}, errorMessage);
        if (Q_UNLIKELY(!metaType)) {
            if (errorMessage)
                errorMessage->prepend(msgCannotBuildMetaType(typeSignature));
            return nullptr;
        }
        it = m_metaTypeFromStringCache.insert(typeSignature, metaType);
    }
    return it.value();
}

AbstractMetaType *ShibokenGenerator::buildAbstractMetaTypeFromTypeEntry(const TypeEntry *typeEntry)
{
    QString typeName = typeEntry->qualifiedCppName();
    if (typeName.startsWith(QLatin1String("::")))
        typeName.remove(0, 2);
    if (m_metaTypeFromStringCache.contains(typeName))
        return m_metaTypeFromStringCache.value(typeName);
    auto *metaType = new AbstractMetaType(typeEntry);
    metaType->clearIndirections();
    metaType->setReferenceType(NoReference);
    metaType->setConstant(false);
    metaType->decideUsagePattern();
    m_metaTypeFromStringCache.insert(typeName, metaType);
    return metaType;
}
AbstractMetaType *ShibokenGenerator::buildAbstractMetaTypeFromAbstractMetaClass(const AbstractMetaClass *metaClass)
{
    return ShibokenGenerator::buildAbstractMetaTypeFromTypeEntry(metaClass->typeEntry());
}

/*
static void dumpFunction(AbstractMetaFunctionList lst)
{
    qDebug() << "DUMP FUNCTIONS: ";
    for (AbstractMetaFunction *func : qAsConst(lst))
        qDebug() << "*" << func->ownerClass()->name()
                        << func->signature()
                        << "Private: " << func->isPrivate()
                        << "Empty: " << func->isEmptyFunction()
                        << "Static:" << func->isStatic()
                        << "Signal:" << func->isSignal()
                        << "ClassImplements: " <<  (func->ownerClass() != func->implementingClass())
                        << "is operator:" << func->isOperatorOverload()
                        << "is global:" << func->isInGlobalScope();
}
*/

static bool isGroupable(const AbstractMetaFunction *func)
{
    switch (func->functionType()) {
    case AbstractMetaFunction::DestructorFunction:
    case AbstractMetaFunction::SignalFunction:
    case AbstractMetaFunction::GetAttroFunction:
    case AbstractMetaFunction::SetAttroFunction:
        return false;
    default:
        break;
    }
    if (func->isModifiedRemoved() && !func->isAbstract())
        return false;
    // weird operator overloads
    if (func->name() == QLatin1String("operator[]") || func->name() == QLatin1String("operator->"))  // FIXME: what about cast operators?
        return false;
    return true;
}

static void insertIntoFunctionGroups(const AbstractMetaFunctionList &lst,
                                     ShibokenGenerator::FunctionGroups *results)
{
    for (AbstractMetaFunction *func : lst) {
        if (isGroupable(func))
            (*results)[func->name()].append(func);
    }
}

ShibokenGenerator::FunctionGroups ShibokenGenerator::getGlobalFunctionGroups() const
{
    FunctionGroups results;
    insertIntoFunctionGroups(globalFunctions(), &results);
    for (auto nsp : invisibleTopNamespaces())
        insertIntoFunctionGroups(nsp->functions(), &results);
    return results;
}

const GeneratorClassInfoCacheEntry &ShibokenGenerator::getGeneratorClassInfo(const AbstractMetaClass *scope)
{
    auto cache = generatorClassInfoCache();
    auto it = cache->find(scope);
    if (it == cache->end()) {
        it = cache->insert(scope, {});
        it.value().functionGroups = getFunctionGroupsImpl(scope);
        it.value().needsGetattroFunction = classNeedsGetattroFunctionImpl(scope);
    }
    return it.value();
}

ShibokenGenerator::FunctionGroups ShibokenGenerator::getFunctionGroups(const AbstractMetaClass *scope)
{
    Q_ASSERT(scope);
    return getGeneratorClassInfo(scope).functionGroups;
}

ShibokenGenerator::FunctionGroups ShibokenGenerator::getFunctionGroupsImpl(const AbstractMetaClass *scope)
{
    AbstractMetaFunctionList lst = scope->functions();
    scope->getFunctionsFromInvisibleNamespacesToBeGenerated(&lst);

    FunctionGroups results;
    for (AbstractMetaFunction *func : lst) {
        if (isGroupable(func)) {
            auto it = results.find(func->name());
            if (it == results.end()) {
                results.insert(func->name(), AbstractMetaFunctionList(1, func));
            } else {
                // If there are virtuals methods in the mix (PYSIDE-570,
                // QFileSystemModel::index(QString,int) and
                // QFileSystemModel::index(int,int,QModelIndex)) override, make sure
                // the overriding method of the most-derived class is seen first
                // and inserted into the "seenSignatures" set.
                if (func->isVirtual())
                    it.value().prepend(func);
                else
                    it.value().append(func);
            }
        }
    }
    return results;
}

AbstractMetaFunctionList ShibokenGenerator::getInheritedOverloads(const AbstractMetaFunction *func, QSet<QString> *seen)
{
    AbstractMetaFunctionList results;
    AbstractMetaClass *basis;
    if (func->ownerClass() && (basis = func->ownerClass()->baseClass())) {
        for (; basis; basis = basis->baseClass()) {
            const AbstractMetaFunction *inFunc = basis->findFunction(func->name());
            if (inFunc && !seen->contains(inFunc->minimalSignature())) {
                seen->insert(inFunc->minimalSignature());
                AbstractMetaFunction *newFunc = inFunc->copy();
                newFunc->setImplementingClass(func->implementingClass());
                results << newFunc;
            }
        }
    }
    return results;
}

AbstractMetaFunctionList ShibokenGenerator::getFunctionAndInheritedOverloads(const AbstractMetaFunction *func, QSet<QString> *seen)
{
    AbstractMetaFunctionList results;
    seen->insert(func->minimalSignature());
    results << const_cast<AbstractMetaFunction *>(func) << getInheritedOverloads(func, seen);
    return results;
}

AbstractMetaFunctionList ShibokenGenerator::getFunctionOverloads(const AbstractMetaClass *scope, const QString &functionName)
{
    AbstractMetaFunctionList lst = scope ? scope->functions() : globalFunctions();

    AbstractMetaFunctionList results;
    QSet<QString> seenSignatures;
    for (AbstractMetaFunction *func : qAsConst(lst)) {
        if (func->name() != functionName)
            continue;
        if (isGroupable(func)) {
            // PYSIDE-331: look also into base classes.
            results << getFunctionAndInheritedOverloads(func, &seenSignatures);
        }
    }
    return results;
}

Generator::OptionDescriptions ShibokenGenerator::options() const
{
    return OptionDescriptions()
        << qMakePair(QLatin1String(AVOID_PROTECTED_HACK),
                     QLatin1String("Avoid the use of the '#define protected public' hack."))
        << qMakePair(QLatin1String(DISABLE_VERBOSE_ERROR_MESSAGES),
                     QLatin1String("Disable verbose error messages. Turn the python code hard to debug\n"
                                   "but safe few kB on the generated bindings."))
        << qMakePair(QLatin1String(PARENT_CTOR_HEURISTIC),
                     QLatin1String("Enable heuristics to detect parent relationship on constructors."))
        << qMakePair(QLatin1String(ENABLE_PYSIDE_EXTENSIONS),
                     QLatin1String("Enable PySide extensions, such as support for signal/slots,\n"
                                   "use this if you are creating a binding for a Qt-based library."))
        << qMakePair(QLatin1String(RETURN_VALUE_HEURISTIC),
                     QLatin1String("Enable heuristics to detect parent relationship on return values\n"
                                   "(USE WITH CAUTION!)"))
        << qMakePair(QLatin1String(USE_ISNULL_AS_NB_NONZERO),
                     QLatin1String("If a class have an isNull() const method, it will be used to compute\n"
                                   "the value of boolean casts"))
        << qMakePair(QLatin1String(WRAPPER_DIAGNOSTICS),
                     QLatin1String("Generate diagnostic code around wrappers"));
}

bool ShibokenGenerator::handleOption(const QString &key, const QString & /* value */)
{
    if (key == QLatin1String(PARENT_CTOR_HEURISTIC))
        return (m_useCtorHeuristic = true);
    if (key == QLatin1String(ENABLE_PYSIDE_EXTENSIONS))
        return (m_usePySideExtensions = true);
    if (key == QLatin1String(RETURN_VALUE_HEURISTIC))
        return (m_userReturnValueHeuristic = true);
    if (key == QLatin1String(DISABLE_VERBOSE_ERROR_MESSAGES))
        return (m_verboseErrorMessagesDisabled = true);
    if (key == QLatin1String(USE_ISNULL_AS_NB_NONZERO))
        return (m_useIsNullAsNbNonZero = true);
    if (key == QLatin1String(AVOID_PROTECTED_HACK))
        return (m_avoidProtectedHack = true);
    if (key == QLatin1String(WRAPPER_DIAGNOSTICS))
        return (m_wrapperDiagnostics = true);
    return false;
}

static void getCode(QStringList &code, const CodeSnipList &codeSnips)
{
    for (const CodeSnip &snip : qAsConst(codeSnips))
        code.append(snip.code());
}

static void getCode(QStringList &code, const TypeEntry *type)
{
    getCode(code, type->codeSnips());

    CustomConversion *customConversion = type->customConversion();
    if (!customConversion)
        return;

    if (!customConversion->nativeToTargetConversion().isEmpty())
        code.append(customConversion->nativeToTargetConversion());

    const CustomConversion::TargetToNativeConversions &toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;

    for (CustomConversion::TargetToNativeConversion *toNative : qAsConst(toCppConversions))
        code.append(toNative->conversion());
}

bool ShibokenGenerator::doSetup()
{
    QStringList snips;
    const PrimitiveTypeEntryList &primitiveTypeList = primitiveTypes();
    for (const PrimitiveTypeEntry *type : primitiveTypeList)
        getCode(snips, type);
    const ContainerTypeEntryList &containerTypeList = containerTypes();
    for (const ContainerTypeEntry *type : containerTypeList)
        getCode(snips, type);
    for (const AbstractMetaClass *metaClass : classes())
        getCode(snips, metaClass->typeEntry());

    const TypeSystemTypeEntry *moduleEntry = TypeDatabase::instance()->defaultTypeSystemType();
    Q_ASSERT(moduleEntry);
    getCode(snips, moduleEntry);

    const auto &functionGroups = getGlobalFunctionGroups();
    for (auto it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        for (AbstractMetaFunction *func : it.value())
            getCode(snips, func->injectedCodeSnips());
    }

    for (const QString &code : qAsConst(snips)) {
        collectContainerTypesFromConverterMacros(code, true);
        collectContainerTypesFromConverterMacros(code, false);
    }

    return true;
}

void ShibokenGenerator::collectContainerTypesFromConverterMacros(const QString &code, bool toPythonMacro)
{
    QString convMacro = toPythonMacro ? QLatin1String("%CONVERTTOPYTHON[") : QLatin1String("%CONVERTTOCPP[");
    int offset = toPythonMacro ? sizeof("%CONVERTTOPYTHON") : sizeof("%CONVERTTOCPP");
    int start = 0;
    QString errorMessage;
    while ((start = code.indexOf(convMacro, start)) != -1) {
        int end = code.indexOf(QLatin1Char(']'), start);
        start += offset;
        if (code.at(start) != QLatin1Char('%')) {
            QString typeString = code.mid(start, end - start);
            if (AbstractMetaType *type =
                    buildAbstractMetaTypeFromString(typeString, &errorMessage)) {
                addInstantiatedContainersAndSmartPointers(type, type->originalTypeDescription());
            } else {
                qFatal("%s: Cannot translate type \"%s\": %s", __FUNCTION__,
                       qPrintable(typeString), qPrintable(errorMessage));
            }
        }
        start = end;
    }
}

bool ShibokenGenerator::useCtorHeuristic() const
{
    return m_useCtorHeuristic;
}

bool ShibokenGenerator::useReturnValueHeuristic() const
{
    return m_userReturnValueHeuristic;
}

bool ShibokenGenerator::usePySideExtensions() const
{
    return m_usePySideExtensions;
}

bool ShibokenGenerator::useIsNullAsNbNonZero() const
{
    return m_useIsNullAsNbNonZero;
}

bool ShibokenGenerator::avoidProtectedHack() const
{
    return m_avoidProtectedHack;
}

QString ShibokenGenerator::moduleCppPrefix(const QString &moduleName) const
 {
    QString result = moduleName.isEmpty() ? packageName() : moduleName;
    result.replace(QLatin1Char('.'), QLatin1Char('_'));
    return result;
}

QString ShibokenGenerator::cppApiVariableName(const QString &moduleName) const
{
    return QLatin1String("Sbk") + moduleCppPrefix(moduleName)
        + QLatin1String("Types");
}

QString ShibokenGenerator::pythonModuleObjectName(const QString &moduleName) const
{
    return QLatin1String("Sbk") + moduleCppPrefix(moduleName)
        + QLatin1String("ModuleObject");
}

QString ShibokenGenerator::convertersVariableName(const QString &moduleName) const
{
    QString result = cppApiVariableName(moduleName);
    result.chop(1);
    result.append(QLatin1String("Converters"));
    return result;
}

static QString processInstantiationsVariableName(const AbstractMetaType *type)
{
    QString res = QLatin1Char('_') + _fixedCppTypeName(type->typeEntry()->qualifiedCppName()).toUpper();
    for (const auto *instantiation : type->instantiations()) {
        res += instantiation->isContainer()
               ? processInstantiationsVariableName(instantiation)
               : QLatin1Char('_') + _fixedCppTypeName(instantiation->cppSignature()).toUpper();
    }
    return res;
}

static void appendIndexSuffix(QString *s)
{
    if (!s->endsWith(QLatin1Char('_')))
        s->append(QLatin1Char('_'));
    s->append(QStringLiteral("IDX"));
}

QString ShibokenGenerator::getTypeIndexVariableName(const AbstractMetaClass *metaClass,
                                                    bool alternativeTemplateName) const
{
    if (alternativeTemplateName) {
        const AbstractMetaClass *templateBaseClass = metaClass->templateBaseClass();
        if (!templateBaseClass)
            return QString();
        QString result = QLatin1String("SBK_")
            + _fixedCppTypeName(templateBaseClass->typeEntry()->qualifiedCppName()).toUpper();
        for (const auto *instantiation : metaClass->templateBaseClassInstantiations())
            result += processInstantiationsVariableName(instantiation);
        appendIndexSuffix(&result);
        return result;
    }
    return getTypeIndexVariableName(metaClass->typeEntry());
}
QString ShibokenGenerator::getTypeIndexVariableName(const TypeEntry *type) const
{
    if (type->isCppPrimitive()) {
        const auto *trueType = static_cast<const PrimitiveTypeEntry *>(type);
        if (trueType->basicReferencedTypeEntry())
            type = trueType->basicReferencedTypeEntry();
    }
    QString result = QLatin1String("SBK_");
    // Disambiguate namespaces per module to allow for extending them.
    if (type->isNamespace()) {
        QString package = type->targetLangPackage();
        const int dot = package.lastIndexOf(QLatin1Char('.'));
        result += package.rightRef(package.size() - (dot + 1));
    }
    result += _fixedCppTypeName(type->qualifiedCppName()).toUpper();
    appendIndexSuffix(&result);
    return result;
}
QString ShibokenGenerator::getTypeIndexVariableName(const AbstractMetaType *type) const
{
    QString result = QLatin1String("SBK");
    if (type->typeEntry()->isContainer())
        result += QLatin1Char('_') + moduleName().toUpper();
    result += processInstantiationsVariableName(type);
    appendIndexSuffix(&result);
    return result;
}

bool ShibokenGenerator::verboseErrorMessagesDisabled() const
{
    return m_verboseErrorMessagesDisabled;
}

bool ShibokenGenerator::pythonFunctionWrapperUsesListOfArguments(const OverloadData &overloadData)
{
    if (overloadData.referenceFunction()->isCallOperator())
        return true;
    if (overloadData.referenceFunction()->isOperatorOverload())
        return false;
    int maxArgs = overloadData.maxArgs();
    int minArgs = overloadData.minArgs();
    return (minArgs != maxArgs)
           || (maxArgs > 1)
           || overloadData.referenceFunction()->isConstructor()
           || overloadData.hasArgumentWithDefaultValue();
}

void ShibokenGenerator::writeMinimalConstructorExpression(QTextStream &s, const AbstractMetaType *type, const QString &defaultCtor)
{
    if (!defaultCtor.isEmpty()) {
         s << " = " << defaultCtor;
         return;
    }
    if (isCppPrimitive(type) || type->isSmartPointer())
        return;
    const auto ctor = minimalConstructor(type);
    if (ctor.isValid()) {
        s << ctor.initialization();
    } else {
        const QString message = msgCouldNotFindMinimalConstructor(QLatin1String(__FUNCTION__), type->cppSignature());
        qCWarning(lcShiboken()).noquote() << message;
        s << ";\n#error " << message << '\n';
    }
}

void ShibokenGenerator::writeMinimalConstructorExpression(QTextStream &s, const TypeEntry *type, const QString &defaultCtor)
{
    if (!defaultCtor.isEmpty()) {
         s << " = " << defaultCtor;
         return;
    }
    if (isCppPrimitive(type))
        return;
    const auto ctor = minimalConstructor(type);
    if (ctor.isValid()) {
        s << ctor.initialization();
    } else {
        const QString message = msgCouldNotFindMinimalConstructor(QLatin1String(__FUNCTION__), type->qualifiedCppName());
        qCWarning(lcShiboken()).noquote() << message;
        s << ";\n#error " << message << Qt::endl;
    }
}

bool ShibokenGenerator::isCppIntegralPrimitive(const TypeEntry *type)
{
    if (!type->isCppPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(type);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    QString typeName = trueType->qualifiedCppName();
    return !typeName.contains(QLatin1String("double"))
        && !typeName.contains(QLatin1String("float"))
        && !typeName.contains(QLatin1String("wchar"));
}
bool ShibokenGenerator::isCppIntegralPrimitive(const AbstractMetaType *type)
{
    return isCppIntegralPrimitive(type->typeEntry());
}

QString ShibokenGenerator::pythonArgsAt(int i)
{
    return QLatin1String(PYTHON_ARGS) + QLatin1Char('[')
           + QString::number(i) + QLatin1Char(']');
}
