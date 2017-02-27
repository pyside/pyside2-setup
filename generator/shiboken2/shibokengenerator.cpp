/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "shibokengenerator.h"
#include <abstractmetalang.h>
#include "overloaddata.h"
#include <reporthandler.h>
#include <typedatabase.h>
#include <iostream>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <limits>
#include <memory>

#define NULL_VALUE "NULL"
#define AVOID_PROTECTED_HACK "avoid-protected-hack"
#define PARENT_CTOR_HEURISTIC "enable-parent-ctor-heuristic"
#define RETURN_VALUE_HEURISTIC "enable-return-value-heuristic"
#define ENABLE_PYSIDE_EXTENSIONS "enable-pyside-extensions"
#define DISABLE_VERBOSE_ERROR_MESSAGES "disable-verbose-error-messages"
#define USE_ISNULL_AS_NB_NONZERO "use-isnull-as-nb_nonzero"

//static void dumpFunction(AbstractMetaFunctionList lst);

QHash<QString, QString> ShibokenGenerator::m_pythonPrimitiveTypeName = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_pythonOperators = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_formatUnits = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_tpFuncs = QHash<QString, QString>();
QStringList ShibokenGenerator::m_knownPythonTypes = QStringList();

static QString resolveScopePrefix(const AbstractMetaClass* scope, const QString& value)
{
    if (!scope)
        return QString();

    QString name;
    QStringList parts = scope->qualifiedCppName().split(QLatin1String("::"), QString::SkipEmptyParts);
    for(int i = (parts.size() - 1) ; i >= 0; i--) {
        if (!value.startsWith(parts[i] + QLatin1String("::")))
            name = parts[i] + QLatin1String("::") + name;
        else
            name.clear();
    }

    return name;
}
ShibokenGenerator::ShibokenGenerator() : Generator()
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
    m_typeSystemConvRegEx[TypeSystemCheckFunction]         = QRegExp(QLatin1String(CHECKTYPE_REGEX));
    m_typeSystemConvRegEx[TypeSystemIsConvertibleFunction] = QRegExp(QLatin1String(ISCONVERTIBLE_REGEX));
    m_typeSystemConvRegEx[TypeSystemToPythonFunction]      = QRegExp(QLatin1String(CONVERTTOPYTHON_REGEX));
    m_typeSystemConvRegEx[TypeSystemToCppFunction]         = QRegExp(QLatin1String(CONVERTTOCPP_REGEX));
}

ShibokenGenerator::~ShibokenGenerator()
{
    // TODO-CONVERTER: it must be caching types that were not created here.
    //qDeleteAll(m_metaTypeFromStringCache.values());
}

void ShibokenGenerator::clearTpFuncs()
{
    m_tpFuncs.insert(QLatin1String("__str__"), QLatin1String("0"));
    m_tpFuncs.insert(QLatin1String("__repr__"), QLatin1String("0"));
    m_tpFuncs.insert(QLatin1String("__iter__"), QLatin1String("0"));
    m_tpFuncs.insert(QLatin1String("__next__"), QLatin1String("0"));
}

void ShibokenGenerator::initPrimitiveTypesCorrespondences()
{
    // Python primitive types names
    m_pythonPrimitiveTypeName.clear();

    // PyBool
    m_pythonPrimitiveTypeName.insert(QLatin1String("bool"), QLatin1String("PyBool"));

    // PyInt
    m_pythonPrimitiveTypeName.insert(QLatin1String("char"), QLatin1String("SbkChar"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("signed char"), QLatin1String("SbkChar"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned char"), QLatin1String("SbkChar"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("int"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("signed int"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("uint"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned int"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("short"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("ushort"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("signed short"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("signed short int"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned short"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned short int"), QLatin1String("PyInt"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("long"), QLatin1String("PyInt"));

    // PyFloat
    m_pythonPrimitiveTypeName.insert(QLatin1String("double"), QLatin1String("PyFloat"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("float"), QLatin1String("PyFloat"));

    // PyLong
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned long"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("signed long"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("ulong"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned long int"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("long long"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("__int64"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned long long"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("unsigned __int64"), QLatin1String("PyLong"));
    m_pythonPrimitiveTypeName.insert(QLatin1String("size_t"), QLatin1String("PyLong"));

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
    m_formatUnits.insert(QLatin1String("int"), QLatin1String("i"));
    m_formatUnits.insert(QLatin1String("unsigned int"), QLatin1String("I"));
    m_formatUnits.insert(QLatin1String("short"), QLatin1String("h"));
    m_formatUnits.insert(QLatin1String("unsigned short"), QLatin1String("H"));
    m_formatUnits.insert(QLatin1String("long"), QLatin1String("l"));
    m_formatUnits.insert(QLatin1String("unsigned long"), QLatin1String("k"));
    m_formatUnits.insert(QLatin1String("long long"), QLatin1String("L"));
    m_formatUnits.insert(QLatin1String("__int64"), QLatin1String("L"));
    m_formatUnits.insert(QLatin1String("unsigned long long"), QLatin1String("K"));
    m_formatUnits.insert(QLatin1String("unsigned __int64"), QLatin1String("K"));
    m_formatUnits.insert(QLatin1String("double"), QLatin1String("d"));
    m_formatUnits.insert(QLatin1String("float"), QLatin1String("f"));
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

QString ShibokenGenerator::translateTypeForWrapperMethod(const AbstractMetaType* cType,
                                                         const AbstractMetaClass* context,
                                                         Options options) const
{
    if (cType->isArray())
        return translateTypeForWrapperMethod(cType->arrayElementType(), context, options) + QLatin1String("[]");

    if (avoidProtectedHack() && cType->isEnum()) {
        const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(cType);
        if (metaEnum && metaEnum->isProtected())
            return protectedEnumSurrogateName(metaEnum);
    }

    return translateType(cType, context, options);
}

bool ShibokenGenerator::shouldGenerateCppWrapper(const AbstractMetaClass* metaClass) const
{
    bool result = metaClass->isPolymorphic() || metaClass->hasVirtualDestructor();
    if (avoidProtectedHack()) {
        result = result || metaClass->hasProtectedFields() || metaClass->hasProtectedDestructor();
        if (!result && metaClass->hasProtectedFunctions()) {
            int protectedFunctions = 0;
            int protectedOperators = 0;
            foreach (const AbstractMetaFunction* func, metaClass->functions()) {
                if (!func->isProtected() || func->isSignal() || func->isModifiedRemoved())
                    continue;
                else if (func->isOperatorOverload())
                    protectedOperators++;
                else
                    protectedFunctions++;
            }
            result = result || (protectedFunctions > protectedOperators);
        }
    } else {
        result = result && !metaClass->hasPrivateDestructor();
    }
    return result && !metaClass->isNamespace();
}

void ShibokenGenerator::lookForEnumsInClassesNotToBeGenerated(AbstractMetaEnumList& enumList, const AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return;

    if (metaClass->typeEntry()->codeGeneration() == TypeEntry::GenerateForSubclass) {
        foreach (const AbstractMetaEnum* metaEnum, metaClass->enums()) {
            if (metaEnum->isPrivate() || metaEnum->typeEntry()->codeGeneration() == TypeEntry::GenerateForSubclass)
                continue;
            if (!enumList.contains(const_cast<AbstractMetaEnum*>(metaEnum)))
                enumList.append(const_cast<AbstractMetaEnum*>(metaEnum));
        }
        lookForEnumsInClassesNotToBeGenerated(enumList, metaClass->enclosingClass());
    }
}

static const AbstractMetaClass* getProperEnclosingClass(const AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return 0;

    if (metaClass->typeEntry()->codeGeneration() != TypeEntry::GenerateForSubclass)
        return metaClass;

    return getProperEnclosingClass(metaClass->enclosingClass());
}

const AbstractMetaClass* ShibokenGenerator::getProperEnclosingClassForEnum(const AbstractMetaEnum* metaEnum)
{
    return getProperEnclosingClass(metaEnum->enclosingClass());
}

QString ShibokenGenerator::wrapperName(const AbstractMetaClass* metaClass) const
{
    if (shouldGenerateCppWrapper(metaClass)) {
        QString result = metaClass->name();
        if (metaClass->enclosingClass()) // is a inner class
            result.replace(QLatin1String("::"), QLatin1String("_"));

        result += QLatin1String("Wrapper");
        return result;
    } else {
        return metaClass->qualifiedCppName();
    }
}

QString ShibokenGenerator::fullPythonFunctionName(const AbstractMetaFunction* func)
{
    QString funcName;
    if (func->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(func);
    else
       funcName = func->name();
    if (func->ownerClass()) {
        QString fullName = func->ownerClass()->fullName();
        if (func->isConstructor())
            funcName = fullName;
        else
            funcName.prepend(fullName + QLatin1Char('.'));
    }
    return funcName;
}

QString ShibokenGenerator::protectedEnumSurrogateName(const AbstractMetaEnum* metaEnum)
{
    return metaEnum->fullName().replace(QLatin1Char('.'), QLatin1Char('_')).replace(QLatin1String("::"), QLatin1String("_")) + QLatin1String("_Surrogate");
}

QString ShibokenGenerator::protectedFieldGetterName(const AbstractMetaField* field)
{
    return QStringLiteral("protected_%1_getter").arg(field->name());
}

QString ShibokenGenerator::protectedFieldSetterName(const AbstractMetaField* field)
{
    return QStringLiteral("protected_%1_setter").arg(field->name());
}

QString ShibokenGenerator::cpythonFunctionName(const AbstractMetaFunction* func)
{
    QString result;

    if (func->ownerClass()) {
        result = cpythonBaseName(func->ownerClass()->typeEntry());
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

QString ShibokenGenerator::cpythonMethodDefinitionName(const AbstractMetaFunction* func)
{
    if (!func->ownerClass())
        return QString();
    return QStringLiteral("%1Method_%2").arg(cpythonBaseName(func->ownerClass()->typeEntry()), func->name());
}

QString ShibokenGenerator::cpythonGettersSettersDefinitionName(const AbstractMetaClass* metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_getsetlist");
}

QString ShibokenGenerator::cpythonSetattroFunctionName(const AbstractMetaClass* metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_setattro");
}


QString ShibokenGenerator::cpythonGetattroFunctionName(const AbstractMetaClass* metaClass)
{
    return cpythonBaseName(metaClass) + QLatin1String("_getattro");
}

QString ShibokenGenerator::cpythonGetterFunctionName(const AbstractMetaField* metaField)
{
    return QStringLiteral("%1_get_%2").arg(cpythonBaseName(metaField->enclosingClass()), metaField->name());
}

QString ShibokenGenerator::cpythonSetterFunctionName(const AbstractMetaField* metaField)
{
    return QStringLiteral("%1_set_%2").arg(cpythonBaseName(metaField->enclosingClass()), metaField->name());
}

static QString cpythonEnumFlagsName(QString moduleName, QString qualifiedCppName)
{
    QString result = QStringLiteral("Sbk%1_%2").arg(moduleName, qualifiedCppName);
    result.replace(QLatin1String("::"), QLatin1String("_"));
    return result;
}

static QString searchForEnumScope(const AbstractMetaClass* metaClass, const QString& value)
{
    QString enumValueName = value.trimmed();

    if (!metaClass)
        return QString();

    foreach (const AbstractMetaEnum* metaEnum, metaClass->enums()) {
        foreach (const AbstractMetaEnumValue* enumValue, metaEnum->values()) {
            if (enumValueName == enumValue->name())
                return metaClass->qualifiedCppName();
        }
    }

    return searchForEnumScope(metaClass->enclosingClass(), enumValueName);
}

/*
 * This function uses some heuristics to find out the scope for a given
 * argument default value. New situations may arise in the future and
 * this method should be updated, do it with care.
 */
QString ShibokenGenerator::guessScopeForDefaultValue(const AbstractMetaFunction* func, const AbstractMetaArgument* arg)
{
    QString value = getDefaultValue(func, arg);

    if (value.isEmpty())
        return QString();

    if (isPointer(arg->type()))
        return value;

    static QRegExp enumValueRegEx(QLatin1String("^([A-Za-z_]\\w*)?$"));
    QString prefix;
    QString suffix;

    if (arg->type()->isEnum()) {
        const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(arg->type());
        if (metaEnum)
            prefix = resolveScopePrefix(metaEnum->enclosingClass(), value);
    } else if (arg->type()->isFlags()) {
        static QRegExp numberRegEx(QLatin1String("^\\d+$")); // Numbers to flags
        if (numberRegEx.exactMatch(value)) {
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
            prefix = typeName + QLatin1Char('(');
            suffix = QLatin1Char(')');
        }

        static QRegExp enumCombinationRegEx(QLatin1String("^([A-Za-z_][\\w:]*)\\(([^,\\(\\)]*)\\)$")); // FlagName(EnumItem|EnumItem|...)
        if (prefix.isEmpty() && enumCombinationRegEx.indexIn(value) != -1) {
            QString flagName = enumCombinationRegEx.cap(1);
            QStringList enumItems = enumCombinationRegEx.cap(2).split(QLatin1Char('|'));
            QString scope = searchForEnumScope(func->implementingClass(), enumItems.first());
            if (!scope.isEmpty())
                scope.append(QLatin1String("::"));

            QStringList fixedEnumItems;
            foreach (const QString& enumItem, enumItems)
                fixedEnumItems << QString(scope + enumItem);

            if (!fixedEnumItems.isEmpty()) {
                prefix = flagName + QLatin1Char('(');
                value = fixedEnumItems.join(QLatin1Char('|'));
                suffix = QLatin1Char(')');
            }
        }
    } else if (arg->type()->typeEntry()->isValue()) {
        const AbstractMetaClass* metaClass = classes().findClass(arg->type()->typeEntry());
        if (enumValueRegEx.exactMatch(value)&& value != QLatin1String("NULL"))
            prefix = resolveScopePrefix(metaClass, value);
    } else if (arg->type()->isPrimitive() && arg->type()->name() == QLatin1String("int")) {
        if (enumValueRegEx.exactMatch(value) && func->implementingClass())
            prefix = resolveScopePrefix(func->implementingClass(), value);
    } else if(arg->type()->isPrimitive()) {
        static QRegExp unknowArgumentRegEx(QLatin1String("^(?:[A-Za-z_][\\w:]*\\()?([A-Za-z_]\\w*)(?:\\))?$")); // [PrimitiveType(] DESIREDNAME [)]
        if (unknowArgumentRegEx.indexIn(value) != -1 && func->implementingClass()) {
            foreach (const AbstractMetaField* field, func->implementingClass()->fields()) {
                if (unknowArgumentRegEx.cap(1).trimmed() == field->name()) {
                    QString fieldName = field->name();
                    if (field->isStatic()) {
                        prefix = resolveScopePrefix(func->implementingClass(), value);
                        fieldName.prepend(prefix);
                        prefix.clear();
                    } else {
                        fieldName.prepend(QLatin1String(CPP_SELF_VAR "->"));
                    }
                    value.replace(unknowArgumentRegEx.cap(1), fieldName);
                    break;
                }
            }
        }
    }

    if (!prefix.isEmpty())
        value.prepend(prefix);
    if (!suffix.isEmpty())
        value.append(suffix);

    return value;
}

QString ShibokenGenerator::cpythonEnumName(const EnumTypeEntry* enumEntry)
{
    QString p = enumEntry->targetLangPackage();
    p.replace(QLatin1Char('.'), QLatin1Char('_'));
    return cpythonEnumFlagsName(p, enumEntry->qualifiedCppName());
}

QString ShibokenGenerator::cpythonEnumName(const AbstractMetaEnum *metaEnum)
{
    return cpythonEnumName(metaEnum->typeEntry());
}

QString ShibokenGenerator::cpythonFlagsName(const FlagsTypeEntry* flagsEntry)
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

QString ShibokenGenerator::cpythonSpecialCastFunctionName(const AbstractMetaClass* metaClass)
{
    return cpythonBaseName(metaClass->typeEntry()) + QLatin1String("SpecialCastFunction");
}

QString ShibokenGenerator::cpythonWrapperCPtr(const AbstractMetaClass* metaClass, QString argName)
{
    return cpythonWrapperCPtr(metaClass->typeEntry(), argName);
}

QString ShibokenGenerator::cpythonWrapperCPtr(const AbstractMetaType* metaType, QString argName)
{
    return cpythonWrapperCPtr(metaType->typeEntry(), argName);
}

QString ShibokenGenerator::cpythonWrapperCPtr(const TypeEntry* type, QString argName)
{
    if (!ShibokenGenerator::isWrapperType(type))
        return QString();
    return QStringLiteral("((::%1*)Shiboken::Conversions::cppPointer(%2, (SbkObject*)%3))")
              .arg(type->qualifiedCppName(), cpythonTypeNameExt(type), argName);
}

QString ShibokenGenerator::getFunctionReturnType(const AbstractMetaFunction* func, Options) const
{
    if (func->ownerClass() && func->isConstructor())
        return func->ownerClass()->qualifiedCppName() + QLatin1Char('*');

    return translateTypeForWrapperMethod(func->type(), func->implementingClass());
}

void ShibokenGenerator::writeToPythonConversion(QTextStream & s, const AbstractMetaType* type,
                                                const AbstractMetaClass * /* context */,
                                                const QString& argumentName)
{
    s << cpythonToPythonConversionFunction(type) << argumentName << ')';
}

void ShibokenGenerator::writeToCppConversion(QTextStream& s, const AbstractMetaClass* metaClass,
                                             const QString& inArgName, const QString& outArgName)
{
    s << cpythonToCppConversionFunction(metaClass) << inArgName << ", &" << outArgName << ')';
}

void ShibokenGenerator::writeToCppConversion(QTextStream& s, const AbstractMetaType* type, const AbstractMetaClass* context,
                                             const QString& inArgName, const QString& outArgName)
{
    s << cpythonToCppConversionFunction(type, context) << inArgName << ", &" << outArgName << ')';
}

bool ShibokenGenerator::shouldRejectNullPointerArgument(const AbstractMetaFunction* func, int argIndex)
{
    if (argIndex < 0 || argIndex >= func->arguments().count())
        return false;

    const AbstractMetaArgument* arg = func->arguments().at(argIndex);
    if (isValueTypeWithCopyConstructorOnly(arg->type()))
        return true;

    // Argument type is not a pointer, a None rejection should not be
    // necessary because the type checking would handle that already.
    if (!isPointer(arg->type()))
        return false;
    if (func->argumentRemoved(argIndex + 1))
        return false;
    foreach (const FunctionModification &funcMod, func->modifications()) {
        foreach (const ArgumentModification &argMod, funcMod.argument_mods) {
            if (argMod.index == argIndex + 1 && argMod.noNullPointers)
                return true;
        }
    }
    return false;
}

QString ShibokenGenerator::getFormatUnitString(const AbstractMetaFunction* func, bool incRef) const
{
    QString result;
    const char objType = (incRef ? 'O' : 'N');
    foreach (const AbstractMetaArgument* arg, func->arguments()) {
        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        if (!func->typeReplaced(arg->argumentIndex() + 1).isEmpty()) {
            result += QLatin1Char(objType);
        } else if (arg->type()->isQObject()
            || arg->type()->isObject()
            || arg->type()->isValue()
            || arg->type()->isValuePointer()
            || arg->type()->isNativePointer()
            || arg->type()->isEnum()
            || arg->type()->isFlags()
            || arg->type()->isContainer()
            || arg->type()->referenceType() == LValueReference) {
            result += QLatin1Char(objType);
        } else if (arg->type()->isPrimitive()) {
            const PrimitiveTypeEntry* ptype = (const PrimitiveTypeEntry*) arg->type()->typeEntry();
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

QString ShibokenGenerator::cpythonBaseName(const AbstractMetaType* type)
{
    if (isCString(type))
        return QLatin1String("PyString");
    return cpythonBaseName(type->typeEntry());
}

QString ShibokenGenerator::cpythonBaseName(const AbstractMetaClass* metaClass)
{
    return cpythonBaseName(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonBaseName(const TypeEntry* type)
{
    QString baseName;
    if (ShibokenGenerator::isWrapperType(type) || type->isNamespace()) { // && type->referenceType() == NoReference) {
        baseName = QLatin1String("Sbk_") + type->name();
    } else if (type->isPrimitive()) {
        const PrimitiveTypeEntry* ptype = (const PrimitiveTypeEntry*) type;
        while (ptype->basicReferencedTypeEntry())
            ptype = ptype->basicReferencedTypeEntry();
        if (ptype->targetLangApiName() == ptype->name())
            baseName = pythonPrimitiveTypeName(ptype->name());
        else
            baseName = ptype->targetLangApiName();
    } else if (type->isEnum()) {
        baseName = cpythonEnumName((const EnumTypeEntry*) type);
    } else if (type->isFlags()) {
        baseName = cpythonFlagsName((const FlagsTypeEntry*) type);
    } else if (type->isContainer()) {
        const ContainerTypeEntry* ctype = (const ContainerTypeEntry*) type;
        switch (ctype->type()) {
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

QString ShibokenGenerator::cpythonTypeName(const AbstractMetaClass* metaClass)
{
    return cpythonTypeName(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonTypeName(const TypeEntry* type)
{
    return cpythonBaseName(type) + QLatin1String("_Type");
}

QString ShibokenGenerator::cpythonTypeNameExt(const TypeEntry* type)
{
    return cppApiVariableName(type->targetLangPackage()) + QLatin1Char('[')
            + getTypeIndexVariableName(type) + QLatin1Char(']');
}

QString ShibokenGenerator::converterObject(const AbstractMetaType* type)
{
    if (isCString(type))
        return QLatin1String("Shiboken::Conversions::PrimitiveTypeConverter<const char*>()");
    if (isVoidPointer(type))
        return QLatin1String("Shiboken::Conversions::PrimitiveTypeConverter<void*>()");
    if (type->typeEntry()->isContainer()) {
        return convertersVariableName(type->typeEntry()->targetLangPackage())
               + QLatin1Char('[') + getTypeIndexVariableName(type) + QLatin1Char(']');
    }
    return converterObject(type->typeEntry());
}

QString ShibokenGenerator::converterObject(const TypeEntry* type)
{
    if (isCppPrimitive(type))
        return QString::fromLatin1("Shiboken::Conversions::PrimitiveTypeConverter<%1>()").arg(type->qualifiedCppName());
    if (isWrapperType(type) || type->isEnum() || type->isFlags())
        return QString::fromLatin1("SBK_CONVERTER(%1)").arg(cpythonTypeNameExt(type));
    
    if (type->isArray()) {
        qDebug() << "Warning: no idea how to handle the Qt5 type " << type->qualifiedCppName();
        return QString::null;
    }
        
    /* the typedef'd primitive types case */
    const PrimitiveTypeEntry* pte = dynamic_cast<const PrimitiveTypeEntry*>(type);
    if (!pte) {
        qDebug() << "Warning: the Qt5 primitive type is unknown" << type->qualifiedCppName();
        return QString::null;
    }
    if (pte->basicReferencedTypeEntry())
        pte = pte->basicReferencedTypeEntry();
    if (pte->isPrimitive() && !pte->isCppPrimitive() && !pte->customConversion())
        return QString::fromLatin1("Shiboken::Conversions::PrimitiveTypeConverter<%1>()").arg(pte->qualifiedCppName());

    return convertersVariableName(type->targetLangPackage())
           + QLatin1Char('[') + getTypeIndexVariableName(type) + QLatin1Char(']');
}

QString ShibokenGenerator::cpythonTypeNameExt(const AbstractMetaType* type)
{
    return cppApiVariableName(type->typeEntry()->targetLangPackage()) + QLatin1Char('[')
           + getTypeIndexVariableName(type) + QLatin1Char(']');
}

QString ShibokenGenerator::cpythonOperatorFunctionName(const AbstractMetaFunction* func)
{
    if (!func->isOperatorOverload())
        return QString();
    return QLatin1String("Sbk") + func->ownerClass()->name()
            + QLatin1Char('_') + pythonOperatorFunctionName(func->originalName());
}

QString ShibokenGenerator::fixedCppTypeName(const CustomConversion::TargetToNativeConversion* toNative)
{
    if (toNative->sourceType())
        return fixedCppTypeName(toNative->sourceType());
    return toNative->sourceTypeName();
}
QString ShibokenGenerator::fixedCppTypeName(const AbstractMetaType* type)
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
QString ShibokenGenerator::fixedCppTypeName(const TypeEntry* type, QString typeName)
{
    if (typeName.isEmpty())
        typeName = type->qualifiedCppName();
    if (!(type->codeGeneration() & TypeEntry::GenerateTargetLang)) {
        typeName.prepend(QLatin1Char('_'));
        typeName.prepend(type->targetLangPackage());
    }
    return _fixedCppTypeName(typeName);
}

QString ShibokenGenerator::pythonPrimitiveTypeName(const QString& cppTypeName)
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

QString ShibokenGenerator::pythonPrimitiveTypeName(const PrimitiveTypeEntry* type)
{
    while (type->basicReferencedTypeEntry())
        type = type->basicReferencedTypeEntry();
    return pythonPrimitiveTypeName(type->name());
}

QString ShibokenGenerator::pythonOperatorFunctionName(QString cppOpFuncName)
{
    QString value = m_pythonOperators.value(cppOpFuncName);
    if (value.isEmpty()) {
        qCWarning(lcShiboken).noquote().nospace() << "Unknown operator:  " << cppOpFuncName;
        value = QLatin1String("UNKNOWN_OPERATOR");
    }
    value.prepend(QLatin1String("__"));
    value.append(QLatin1String("__"));
    return value;
}

QString ShibokenGenerator::pythonOperatorFunctionName(const AbstractMetaFunction* func)
{
    QString op = pythonOperatorFunctionName(func->originalName());
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

QString ShibokenGenerator::pythonRichCompareOperatorId(QString cppOpFuncName)
{
    return QLatin1String("Py_") + m_pythonOperators.value(cppOpFuncName).toUpper();
}

QString ShibokenGenerator::pythonRichCompareOperatorId(const AbstractMetaFunction* func)
{
    return pythonRichCompareOperatorId(func->originalName());
}

bool ShibokenGenerator::isNumber(QString cpythonApiName)
{
    return cpythonApiName == QLatin1String("PyInt")
            || cpythonApiName == QLatin1String("PyFloat")
            || cpythonApiName == QLatin1String("PyLong")
            || cpythonApiName == QLatin1String("PyBool");
}

bool ShibokenGenerator::isNumber(const TypeEntry* type)
{
    if (!type->isPrimitive())
        return false;
    return isNumber(pythonPrimitiveTypeName((const PrimitiveTypeEntry*) type));
}

bool ShibokenGenerator::isNumber(const AbstractMetaType* type)
{
    return isNumber(type->typeEntry());
}

bool ShibokenGenerator::isPyInt(const TypeEntry* type)
{
    if (!type->isPrimitive())
        return false;
    return pythonPrimitiveTypeName((const PrimitiveTypeEntry*) type) == QLatin1String("PyInt");
}

bool ShibokenGenerator::isPyInt(const AbstractMetaType* type)
{
    return isPyInt(type->typeEntry());
}

bool ShibokenGenerator::isPairContainer(const AbstractMetaType* type)
{
    return type->isContainer()
        && static_cast<const ContainerTypeEntry *>(type->typeEntry())->type() == ContainerTypeEntry::PairContainer;
}

bool ShibokenGenerator::isWrapperType(const TypeEntry* type)
{
    if (type->isComplex())
        return ShibokenGenerator::isWrapperType((const ComplexTypeEntry*)type);
    return type->isObject() || type->isValue();
}
bool ShibokenGenerator::isWrapperType(const ComplexTypeEntry* type)
{
    return isObjectType(type) || type->isValue();
}
bool ShibokenGenerator::isWrapperType(const AbstractMetaType* metaType)
{
    return isObjectType(metaType)
            || metaType->typeEntry()->isValue();
}

bool ShibokenGenerator::isPointerToWrapperType(const AbstractMetaType* type)
{
    return (isObjectType(type) && type->indirections() == 1) || type->isValuePointer();
}

bool ShibokenGenerator::isObjectTypeUsedAsValueType(const AbstractMetaType* type)
{
    return type->typeEntry()->isObject() && type->referenceType() == NoReference && type->indirections() == 0;
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const AbstractMetaClass* metaClass)
{
    if (!metaClass || !metaClass->typeEntry()->isValue())
        return false;
    AbstractMetaFunctionList ctors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
    if (ctors.count() != 1)
        return false;
    return ctors.first()->functionType() == AbstractMetaFunction::CopyConstructorFunction;
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const TypeEntry* type) const
{
    if (!type || !type->isValue())
        return false;
    return isValueTypeWithCopyConstructorOnly(classes().findClass(type));
}

bool ShibokenGenerator::isValueTypeWithCopyConstructorOnly(const AbstractMetaType* type) const
{
    if (!type || !type->typeEntry()->isValue())
        return false;
    return isValueTypeWithCopyConstructorOnly(type->typeEntry());
}

bool ShibokenGenerator::isUserPrimitive(const TypeEntry* type)
{
    if (!type->isPrimitive())
        return false;
    const PrimitiveTypeEntry* trueType = (const PrimitiveTypeEntry*) type;
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->isPrimitive() && !trueType->isCppPrimitive()
           && trueType->qualifiedCppName() != QLatin1String("std::string");
}

bool ShibokenGenerator::isUserPrimitive(const AbstractMetaType* type)
{
    if (type->indirections() != 0)
        return false;
    return isUserPrimitive(type->typeEntry());
}

bool ShibokenGenerator::isCppPrimitive(const TypeEntry* type)
{
    if (type->isCppPrimitive())
        return true;
    if (!type->isPrimitive())
        return false;
    const PrimitiveTypeEntry* trueType = (const PrimitiveTypeEntry*) type;
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->qualifiedCppName() == QLatin1String("std::string");
}

bool ShibokenGenerator::isCppPrimitive(const AbstractMetaType* type)
{
    if (isCString(type) || isVoidPointer(type))
        return true;
    if (type->indirections() != 0)
        return false;
    return isCppPrimitive(type->typeEntry());
}

bool ShibokenGenerator::shouldDereferenceArgumentPointer(const AbstractMetaArgument* arg)
{
    return shouldDereferenceAbstractMetaTypePointer(arg->type());
}

bool ShibokenGenerator::shouldDereferenceAbstractMetaTypePointer(const AbstractMetaType* metaType)
{
    return metaType->referenceType() == LValueReference && isWrapperType(metaType) && !isPointer(metaType);
}

bool ShibokenGenerator::visibilityModifiedToPrivate(const AbstractMetaFunction* func)
{
    foreach (const FunctionModification &mod, func->modifications()) {
        if (mod.modifiers & Modification::Private)
            return true;
    }
    return false;
}

QString ShibokenGenerator::cpythonCheckFunction(const AbstractMetaType* metaType, bool genericNumberType)
{
    QString customCheck;
    if (metaType->typeEntry()->isCustom()) {
        AbstractMetaType* type;
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
    } else if (metaType->typeEntry()->isContainer()) {
        QString typeCheck = QLatin1String("Shiboken::Conversions::");
        ContainerTypeEntry::Type type = ((const ContainerTypeEntry*)metaType->typeEntry())->type();
        if (type == ContainerTypeEntry::ListContainer
            || type == ContainerTypeEntry::StringListContainer
            || type == ContainerTypeEntry::LinkedListContainer
            || type == ContainerTypeEntry::VectorContainer
            || type == ContainerTypeEntry::StackContainer
            || type == ContainerTypeEntry::SetContainer
            || type == ContainerTypeEntry::QueueContainer) {
            const AbstractMetaType* type = metaType->instantiations().first();
            if (isPointerToWrapperType(type))
                typeCheck += QString::fromLatin1("checkSequenceTypes(%1, ").arg(cpythonTypeNameExt(type));
            else if (isWrapperType(type))
                typeCheck += QString::fromLatin1("convertibleSequenceTypes((SbkObjectType*)%1, ").arg(cpythonTypeNameExt(type));
            else
                typeCheck += QString::fromLatin1("convertibleSequenceTypes(%1, ").arg(converterObject(type));
        } else if (type == ContainerTypeEntry::MapContainer
            || type == ContainerTypeEntry::MultiMapContainer
            || type == ContainerTypeEntry::HashContainer
            || type == ContainerTypeEntry::MultiHashContainer
            || type == ContainerTypeEntry::PairContainer) {
            QString pyType = (type == ContainerTypeEntry::PairContainer) ? QLatin1String("Pair") : QLatin1String("Dict");
            const AbstractMetaType* firstType = metaType->instantiations().first();
            const AbstractMetaType* secondType = metaType->instantiations().last();
            if (isPointerToWrapperType(firstType) && isPointerToWrapperType(secondType)) {
                typeCheck += QString::fromLatin1("check%1Types(%2, %3, ").arg(pyType)
                                .arg(cpythonTypeNameExt(firstType), cpythonTypeNameExt(secondType));
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
    return cpythonCheckFunction(metaType->typeEntry(), genericNumberType);
}

QString ShibokenGenerator::cpythonCheckFunction(const TypeEntry* type, bool genericNumberType)
{
    QString customCheck;
    if (type->isCustom()) {
        AbstractMetaType* metaType;
        customCheck = guessCPythonCheckFunction(type->name(), &metaType);
        if (metaType)
            return cpythonCheckFunction(metaType, genericNumberType);
        return customCheck;
    }

    if (type->isEnum() || type->isFlags() || isWrapperType(type))
        return QString::fromLatin1("SbkObject_TypeCheck(%1, ").arg(cpythonTypeNameExt(type));
    else if (isCppPrimitive(type))
        return pythonPrimitiveTypeName((const PrimitiveTypeEntry*)type) + QLatin1String("_Check");
    QString typeCheck;
    if (type->targetLangApiName() == type->name())
        typeCheck = cpythonIsConvertibleFunction(type);
    else if (type->targetLangApiName() == QLatin1String("PyUnicode"))
        typeCheck = QLatin1String("Shiboken::String::check");
    else
        typeCheck = type->targetLangApiName() + QLatin1String("_Check");
    return typeCheck;
}

QString ShibokenGenerator::guessCPythonCheckFunction(const QString& type, AbstractMetaType** metaType)
{
    *metaType = 0;
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

QString ShibokenGenerator::guessCPythonIsConvertible(const QString& type)
{
    if (type == QLatin1String("PyTypeObject"))
        return QLatin1String("PyType_Check");

    AbstractMetaType* metaType = buildAbstractMetaTypeFromString(type);
    if (metaType && !metaType->typeEntry()->isCustom())
        return cpythonIsConvertibleFunction(metaType);

    return type + QLatin1String("_Check");
}

QString ShibokenGenerator::cpythonIsConvertibleFunction(const TypeEntry* type,
                                                        bool /* genericNumberType */,
                                                        bool /* checkExact */)
{
    if (isWrapperType(type)) {
        QString isConv = (type->isValue() && !isValueTypeWithCopyConstructorOnly(type))
                         ? QLatin1String("isPythonToCppValueConvertible")
                         : QLatin1String("isPythonToCppPointerConvertible");
        return QString::fromLatin1("Shiboken::Conversions::%1((SbkObjectType*)%2, ")
                  .arg(isConv, cpythonTypeNameExt(type));
    }
    return QString::fromLatin1("Shiboken::Conversions::isPythonToCppConvertible(%1, ")
              .arg(converterObject(type));
}
QString ShibokenGenerator::cpythonIsConvertibleFunction(const AbstractMetaType* metaType,
                                                        bool /* genericNumberType */)
{
    QString customCheck;
    if (metaType->typeEntry()->isCustom()) {
        AbstractMetaType* type;
        customCheck = guessCPythonCheckFunction(metaType->typeEntry()->name(), &type);
        if (type)
            metaType = type;
        if (!customCheck.isEmpty())
            return customCheck;
    }

    if (isWrapperType(metaType)) {
        QString isConv;
        if (isPointer(metaType) || isValueTypeWithCopyConstructorOnly(metaType))
            isConv = QLatin1String("isPythonToCppPointerConvertible");
        else if (metaType->referenceType() == LValueReference)
            isConv = QLatin1String("isPythonToCppReferenceConvertible");
        else
            isConv = QLatin1String("isPythonToCppValueConvertible");
        return QStringLiteral("Shiboken::Conversions::%1((SbkObjectType*)%2, ")
                  .arg(isConv, cpythonTypeNameExt(metaType));
    }
    return QStringLiteral("Shiboken::Conversions::isPythonToCppConvertible(%1, ")
              .arg(converterObject(metaType));
}

QString ShibokenGenerator::cpythonIsConvertibleFunction(const AbstractMetaArgument *metaArg, bool genericNumberType)
{
    return cpythonIsConvertibleFunction(metaArg->type(), genericNumberType);
}

QString ShibokenGenerator::cpythonToCppConversionFunction(const AbstractMetaClass* metaClass)
{
    return QStringLiteral("Shiboken::Conversions::pythonToCppPointer((SbkObjectType*)%1, ")
              .arg(cpythonTypeNameExt(metaClass->typeEntry()));
}
QString ShibokenGenerator::cpythonToCppConversionFunction(const AbstractMetaType *type,
                                                          const AbstractMetaClass * /* context */)
{
    if (isWrapperType(type)) {
        return QStringLiteral("Shiboken::Conversions::pythonToCpp%1((SbkObjectType*)%2, ")
                  .arg(isPointer(type) ? QLatin1String("Pointer") : QLatin1String("Copy"))
                  .arg(cpythonTypeNameExt(type));
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
        else if (type->isValue())
            conversion = QLatin1String("copy");
        else
            conversion = QLatin1String("pointer");
        return QStringLiteral("Shiboken::Conversions::%1ToPython((SbkObjectType*)%2, %3")
                  .arg(conversion, cpythonTypeNameExt(type),
                       conversion == QLatin1String("pointer") ? QString() : QLatin1String("&"));
    }
    return QStringLiteral("Shiboken::Conversions::copyToPython(%1, %2")
              .arg(converterObject(type),
                   (isCString(type) || isVoidPointer(type)) ? QString() : QLatin1String("&"));
}

QString ShibokenGenerator::cpythonToPythonConversionFunction(const AbstractMetaClass* metaClass)
{
    return cpythonToPythonConversionFunction(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonToPythonConversionFunction(const TypeEntry* type)
{
    if (isWrapperType(type)) {
        QString conversion;
        if (type->isValue())
            conversion = QLatin1String("copy");
        else
            conversion = QLatin1String("pointer");
        return QStringLiteral("Shiboken::Conversions::%1ToPython((SbkObjectType*)%2, %3")
                  .arg(conversion, cpythonTypeNameExt(type),
                       conversion == QLatin1String("pointer") ? QString() : QLatin1String("&"));
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
        arg += QLatin1Char(' ');
        arg += argument->name();
    }

    QList<ReferenceCount> referenceCounts;
    referenceCounts = func->referenceCounts(func->implementingClass(), argument->argumentIndex() + 1);
    if ((options & Generator::SkipDefaultValues) != Generator::SkipDefaultValues &&
        !argument->originalDefaultValueExpression().isEmpty())
    {
        QString default_value = argument->originalDefaultValueExpression();
        if (default_value == QLatin1String("NULL"))
            default_value = QLatin1String(NULL_VALUE);

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
            s << " " PYTHON_SELF_VAR;
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

QString ShibokenGenerator::functionReturnType(const AbstractMetaFunction* func, Options options) const
{
    QString modifiedReturnType = QString(func->typeReplaced(0));
    if (!modifiedReturnType.isNull() && !(options & OriginalTypeDescription))
        return modifiedReturnType;
    else
        return translateType(func->type(), func->implementingClass(), options);
}

QString ShibokenGenerator::functionSignature(const AbstractMetaFunction *func,
                                             QString prepend,
                                             QString append,
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

    return result;
}

void ShibokenGenerator::writeArgumentNames(QTextStream &s,
                                           const AbstractMetaFunction *func,
                                           Options options) const
{
    AbstractMetaArgumentList arguments = func->arguments();
    int argCount = 0;
    for (int j = 0, max = arguments.size(); j < max; j++) {

        if ((options & Generator::SkipRemovedArguments) && (func->argumentRemoved(arguments.at(j)->argumentIndex()+1)))
            continue;

        s << ((argCount > 0) ? ", " : "") << arguments.at(j)->name();

        if (((options & Generator::VirtualCall) == 0)
            && (!func->conversionRule(TypeSystem::NativeCode, arguments.at(j)->argumentIndex() + 1).isEmpty()
                || !func->conversionRule(TypeSystem::TargetLangCode, arguments.at(j)->argumentIndex() + 1).isEmpty())
            && !func->isConstructor()) {
           s << CONV_RULE_OUT_VAR_SUFFIX;
        }

        argCount++;
    }
}

void ShibokenGenerator::writeFunctionCall(QTextStream& s,
                                          const AbstractMetaFunction* func,
                                          Options options) const
{
    if (!(options & Generator::SkipName))
        s << (func->isConstructor() ? func->ownerClass()->qualifiedCppName() : func->originalName());
    s << '(';
    writeArgumentNames(s, func, options);
    s << ')';
}

void ShibokenGenerator::writeUnusedVariableCast(QTextStream& s, const QString& variableName)
{
    s << INDENT << "SBK_UNUSED(" << variableName<< ')' << endl;
}

AbstractMetaFunctionList ShibokenGenerator::filterFunctions(const AbstractMetaClass* metaClass)
{
    AbstractMetaFunctionList result;
    foreach (AbstractMetaFunction *func, metaClass->functions()) {
        //skip signals
        if (func->isSignal() || func->isDestructor()
            || (func->isModifiedRemoved() && !func->isAbstract()
                && (!avoidProtectedHack() || !func->isProtected())))
            continue;
        result << func;
    }
    return result;
}

ShibokenGenerator::ExtendedConverterData ShibokenGenerator::getExtendedConverters() const
{
    ExtendedConverterData extConvs;
    foreach (const AbstractMetaClass* metaClass, classes()) {
        // Use only the classes for the current module.
        if (!shouldGenerate(metaClass))
            continue;
        foreach (AbstractMetaFunction* convOp, metaClass->operatorOverloads(AbstractMetaClass::ConversionOp)) {
            // Get only the conversion operators that return a type from another module,
            // that are value-types and were not removed in the type system.
            const TypeEntry* convType = convOp->type()->typeEntry();
            if ((convType->codeGeneration() & TypeEntry::GenerateTargetLang)
                || !convType->isValue()
                || convOp->isModifiedRemoved())
                continue;
            extConvs[convType].append(convOp->ownerClass());
        }
    }
    return extConvs;
}

QList<const CustomConversion*> ShibokenGenerator::getPrimitiveCustomConversions()
{
    QList<const CustomConversion*> conversions;
    foreach (const PrimitiveTypeEntry* type, primitiveTypes()) {
        if (!shouldGenerateTypeEntry(type) || !isUserPrimitive(type) || !type->customConversion())
            continue;

        conversions << type->customConversion();
    }
    return conversions;
}

static QString getArgumentsFromMethodCall(const QString& str)
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

QString ShibokenGenerator::getCodeSnippets(const CodeSnipList& codeSnips,
                                           TypeSystem::CodeSnipPosition position,
                                           TypeSystem::Language language)
{
    QString code;
    QTextStream c(&code);
    foreach (const CodeSnip &snip, codeSnips) {
        if ((position != TypeSystem::CodeSnipPositionAny && snip.position != position) || !(snip.language & language))
            continue;
        QString snipCode;
        QTextStream sc(&snipCode);
        formatCode(sc, snip.code(), INDENT);
        c << snipCode;
    }
    return code;
}
void ShibokenGenerator::processCodeSnip(QString& code, const AbstractMetaClass* context)
{
    if (context) {
        // Replace template variable by the Python Type object
        // for the class context in which the variable is used.
        code.replace(QLatin1String("%PYTHONTYPEOBJECT"),
                     cpythonTypeName(context) + QLatin1String(".super.ht_type"));
        code.replace(QLatin1String("%TYPE"), wrapperName(context));
        code.replace(QLatin1String("%CPPTYPE"), context->name());
    }

    // replace "toPython" converters
    replaceConvertToPythonTypeSystemVariable(code);

    // replace "toCpp" converters
    replaceConvertToCppTypeSystemVariable(code);

    // replace "isConvertible" check
    replaceIsConvertibleToCppTypeSystemVariable(code);

    // replace "checkType" check
    replaceTypeCheckTypeSystemVariable(code);
}

ShibokenGenerator::ArgumentVarReplacementList ShibokenGenerator::getArgumentReplacement(const AbstractMetaFunction* func,
                                                                                        bool usePyArgs, TypeSystem::Language language,
                                                                                        const AbstractMetaArgument* lastArg)
{
    ArgumentVarReplacementList argReplacements;
    TypeSystem::Language convLang = (language == TypeSystem::TargetLangCode)
                                    ? TypeSystem::NativeCode : TypeSystem::TargetLangCode;
    int removed = 0;
    for (int i = 0; i < func->arguments().size(); ++i) {
        const AbstractMetaArgument* arg = func->arguments().at(i);
        QString argValue;
        if (language == TypeSystem::TargetLangCode) {
            bool hasConversionRule = !func->conversionRule(convLang, i+1).isEmpty();
            bool argRemoved = func->argumentRemoved(i+1);
            removed = removed + (int) argRemoved;
            if (argRemoved && hasConversionRule)
                argValue = arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
            else if (argRemoved || (lastArg && arg->argumentIndex() > lastArg->argumentIndex()))
                argValue = QLatin1String(CPP_ARG_REMOVED) + QString::number(i);
            if (!argRemoved && argValue.isEmpty()) {
                int argPos = i - removed;
                const AbstractMetaType* type = arg->type();
                QString typeReplaced = func->typeReplaced(arg->argumentIndex() + 1);
                if (!typeReplaced.isEmpty()) {
                    AbstractMetaType* builtType = buildAbstractMetaTypeFromString(typeReplaced);
                    if (builtType)
                        type = builtType;
                }
                if (type->typeEntry()->isCustom()) {
                    argValue = usePyArgs
                               ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(argPos)
                               : QLatin1String(PYTHON_ARG);
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

void ShibokenGenerator::writeCodeSnips(QTextStream& s,
                                       const CodeSnipList& codeSnips,
                                       TypeSystem::CodeSnipPosition position,
                                       TypeSystem::Language language,
                                       const AbstractMetaClass* context)
{
    QString code = getCodeSnippets(codeSnips, position, language);
    if (code.isEmpty())
        return;
    processCodeSnip(code, context);
    s << INDENT << "// Begin code injection" << endl;
    s << code;
    s << INDENT << "// End of code injection" << endl;
}

void ShibokenGenerator::writeCodeSnips(QTextStream& s,
                                       const CodeSnipList& codeSnips,
                                       TypeSystem::CodeSnipPosition position,
                                       TypeSystem::Language language,
                                       const AbstractMetaFunction* func,
                                       const AbstractMetaArgument* lastArg)
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

    OverloadData od(getFunctionGroups(func->implementingClass())[func->name()], this);
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(od);

    // Replace %PYARG_# variables.
    code.replace(QLatin1String("%PYARG_0"), QLatin1String(PYTHON_RETURN_VAR));

    static QRegExp pyArgsRegex(QLatin1String("%PYARG_(\\d+)"));
    if (language == TypeSystem::TargetLangCode) {
        if (usePyArgs) {
            code.replace(pyArgsRegex, QLatin1String(PYTHON_ARGS"[\\1-1]"));
        } else {
            static QRegExp pyArgsRegexCheck(QLatin1String("%PYARG_([2-9]+)"));
            if (pyArgsRegexCheck.indexIn(code) != -1) {
                qCWarning(lcShiboken).noquote().nospace()
                    << "Wrong index for %PYARG variable (" << pyArgsRegexCheck.cap(1) << ") on " << func->signature();
                return;
            }
            code.replace(QLatin1String("%PYARG_1"), QLatin1String(PYTHON_ARG));
        }
    } else {
        // Replaces the simplest case of attribution to a
        // Python argument on the binding virtual method.
        static QRegExp pyArgsAttributionRegex(QLatin1String("%PYARG_(\\d+)\\s*=[^=]\\s*([^;]+)"));
        code.replace(pyArgsAttributionRegex, QLatin1String("PyTuple_SET_ITEM(" PYTHON_ARGS ", \\1-1, \\2)"));
        code.replace(pyArgsRegex, QLatin1String("PyTuple_GET_ITEM(" PYTHON_ARGS ", \\1-1)"));
    }

    // Replace %ARG#_TYPE variables.
    foreach (const AbstractMetaArgument* arg, func->arguments()) {
        QString argTypeVar = QStringLiteral("%ARG%1_TYPE").arg(arg->argumentIndex() + 1);
        QString argTypeVal = arg->type()->cppSignature();
        code.replace(argTypeVar, argTypeVal);
    }

    int pos = 0;
    static QRegExp cppArgTypeRegexCheck(QLatin1String("%ARG(\\d+)_TYPE"));
    while ((pos = cppArgTypeRegexCheck.indexIn(code, pos)) != -1) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Wrong index for %ARG#_TYPE variable (" << cppArgTypeRegexCheck.cap(1)
            << ") on " << func->signature();
        pos += cppArgTypeRegexCheck.matchedLength();
    }

    // Replace template variable for return variable name.
    if (func->isConstructor()) {
        code.replace(QLatin1String("%0."), QLatin1String("cptr->"));
        code.replace(QLatin1String("%0"), QLatin1String("cptr"));
    } else if (func->type()) {
        QString returnValueOp = isPointerToWrapperType(func->type())
            ? QLatin1String("%1->") : QLatin1String("%1.");
        if (ShibokenGenerator::isWrapperType(func->type()))
            code.replace(QLatin1String("%0."), returnValueOp.arg(QLatin1String(CPP_RETURN_VAR)));
        code.replace(QLatin1String("%0"), QLatin1String(CPP_RETURN_VAR));
    }

    // Replace template variable for self Python object.
    QString pySelf = (language == TypeSystem::NativeCode) ? QLatin1String("pySelf") : QLatin1String(PYTHON_SELF_VAR);
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
            if (!methodCallArgs.isNull()) {
                const QString pattern = QStringLiteral("%CPPSELF.%FUNCTION_NAME(%1)").arg(methodCallArgs);
                if (func->name() == QLatin1String("metaObject")) {
                    QString wrapperClassName = wrapperName(func->ownerClass());
                    QString cppSelfVar = avoidProtectedHack()
                                         ? QLatin1String("%CPPSELF")
                                         : QStringLiteral("reinterpret_cast<%1*>(%CPPSELF)").arg(wrapperClassName);
                    code.replace(pattern,
                                 QString::fromLatin1("(Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject*>(%1))"
                                         " ? %2->::%3::%FUNCTION_NAME(%4)"
                                         " : %CPPSELF.%FUNCTION_NAME(%4))").arg(pySelf, cppSelfVar, wrapperClassName, methodCallArgs));
                } else {
                    code.replace(pattern,
                                 QString::fromLatin1("(Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject*>(%1))"
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
                                       cpythonTypeName(func->implementingClass()) + QLatin1String(".super.ht_type"));
        } else {
            code.replace(QLatin1String("%PYTHONTYPEOBJECT."), pySelf + QLatin1String("->ob_type->"));
            code.replace(QLatin1String("%PYTHONTYPEOBJECT"), pySelf + QLatin1String("->ob_type"));
        }
    }

    // Replaces template %ARGUMENT_NAMES and %# variables by argument variables and values.
    // Replaces template variables %# for individual arguments.
    ArgumentVarReplacementList argReplacements = getArgumentReplacement(func, usePyArgs, language, lastArg);

    QStringList args;
    foreach (const ArgumentVarReplacementPair &pair, argReplacements) {
        if (pair.second.startsWith(QLatin1String(CPP_ARG_REMOVED)))
            continue;
        args << pair.second;
    }
    code.replace(QLatin1String("%ARGUMENT_NAMES"), args.join(QLatin1String(", ")));

    foreach (const ArgumentVarReplacementPair &pair, argReplacements) {
        const AbstractMetaArgument* arg = pair.first;
        int idx = arg->argumentIndex() + 1;
        AbstractMetaType* type = arg->type();
        QString typeReplaced = func->typeReplaced(arg->argumentIndex() + 1);
        if (!typeReplaced.isEmpty()) {
            AbstractMetaType* builtType = buildAbstractMetaTypeFromString(typeReplaced);
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
        code.replace(QRegExp(QString::fromLatin1("%%1\\b").arg(idx)), pair.second);
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
            foreach (const AbstractMetaFunction* f, getFunctionOverloads(func->ownerClass(), func->name()))
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
    s << INDENT << "// Begin code injection" << endl;
    s << code;
    s << INDENT << "// End of code injection" << endl;
}

// Returns true if the string is an expression,
// and false if it is a variable.
static bool isVariable(const QString& code)
{
    static QRegExp expr(QLatin1String("\\s*\\*?\\s*[A-Za-z_][A-Za-z_0-9.]*\\s*(?:\\[[^\\[]+\\])*"));
    return expr.exactMatch(code.trimmed());
}

// A miniature normalizer that puts a type string into a format
// suitable for comparison with AbstractMetaType::cppSignature()
// result.
static QString miniNormalizer(const QString& varType)
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
static QString getConverterTypeSystemVariableArgument(const QString& code, int pos)
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
typedef QPair<QString, QString> StringPair;
void ShibokenGenerator::replaceConverterTypeSystemVariable(TypeSystemConverterVariable converterVariable, QString& code)
{
    QRegExp& regex = m_typeSystemConvRegEx[converterVariable];
    int pos = 0;
    QList<StringPair> replacements;
    while ((pos = regex.indexIn(code, pos)) != -1) {
        pos += regex.matchedLength();
        QStringList list = regex.capturedTexts();
        QString conversionString = list.first();
        QString conversionTypeName = list.last();
        const AbstractMetaType* conversionType = buildAbstractMetaTypeFromString(conversionTypeName);
        if (!conversionType) {
            qFatal(qPrintable(QString::fromLatin1("Could not find type '%1' for use in '%2' conversion. "
                                      "Make sure to use the full C++ name, e.g. 'Namespace::Class'.")
                                 .arg(conversionTypeName).arg(m_typeSystemConvName[converterVariable])), NULL);

        }
        QString conversion;
        QTextStream c(&conversion);
        switch (converterVariable) {
            case TypeSystemToCppFunction: {
                int end = pos - list.first().count();
                int start = end;
                while (start > 0 && code.at(start) != QLatin1Char('\n'))
                    --start;
                while (code.at(start).isSpace())
                    ++start;
                QString varType = code.mid(start, end - start);
                conversionString = varType + list.first();
                varType = miniNormalizer(varType);
                QString varName = list.at(1).trimmed();
                if (!varType.isEmpty()) {
                    if (varType != conversionType->cppSignature()) {
                        qFatal(qPrintable(QString::fromLatin1("Types of receiver variable ('%1') and %CONVERTTOCPP type system variable ('%2') differ.")
                                                              .arg(varType, conversionType->cppSignature())), NULL);
                    }
                    c << getFullTypeName(conversionType) << ' ' << varName;
                    writeMinimalConstructorExpression(c, conversionType);
                    c << ';' << endl;
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
                QString arg = getConverterTypeSystemVariableArgument(code, pos);
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
            case TypeSystemIsConvertibleFunction:
                if (conversion.isEmpty())
                    conversion = cpythonIsConvertibleFunction(conversionType);
            case TypeSystemToPythonFunction:
                if (conversion.isEmpty())
                    conversion = cpythonToPythonConversionFunction(conversionType);
            default: {
                QString arg = getConverterTypeSystemVariableArgument(code, pos);
                conversionString += arg;
                if (converterVariable == TypeSystemToPythonFunction && !isVariable(arg)) {
                    qFatal(qPrintable(QString::fromLatin1("Only variables are acceptable as argument to %%CONVERTTOPYTHON type system variable on code snippet: '%1'")
                                         .arg(code)), NULL);
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
    foreach (const StringPair &rep, replacements)
        code.replace(rep.first, rep.second);
}

bool ShibokenGenerator::injectedCodeUsesCppSelf(const AbstractMetaFunction* func)
{
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
    foreach (const CodeSnip &snip, snips) {
        if (snip.code().contains(QLatin1String("%CPPSELF")))
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeUsesPySelf(const AbstractMetaFunction* func)
{
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode);
    foreach (const CodeSnip &snip, snips) {
        if (snip.code().contains(QLatin1String("%PYSELF")))
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeCallsCppFunction(const AbstractMetaFunction* func)
{
    QString funcCall = func->originalName() + QLatin1Char('(');
    QString wrappedCtorCall;
    if (func->isConstructor()) {
        funcCall.prepend(QLatin1String("new "));
        wrappedCtorCall = QStringLiteral("new %1(").arg(wrapperName(func->ownerClass()));
    }
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
    foreach (const CodeSnip &snip, snips) {
        if (snip.code().contains(QLatin1String("%FUNCTION_NAME(")) || snip.code().contains(funcCall)
            || (func->isConstructor()
                && ((func->ownerClass()->isPolymorphic() && snip.code().contains(wrappedCtorCall))
                    || snip.code().contains(QLatin1String("new %TYPE("))))
            )
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeCallsPythonOverride(const AbstractMetaFunction* func)
{
    static QRegExp overrideCallRegexCheck(QLatin1String("PyObject_Call\\s*\\(\\s*%PYTHON_METHOD_OVERRIDE\\s*,"));
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode);
    foreach (const CodeSnip &snip, snips) {
        if (overrideCallRegexCheck.indexIn(snip.code()) != -1)
            return true;
    }
    return false;
}

bool ShibokenGenerator::injectedCodeHasReturnValueAttribution(const AbstractMetaFunction* func, TypeSystem::Language language)
{
    static QRegExp retValAttributionRegexCheck_native(QLatin1String("%0\\s*=[^=]\\s*.+"));
    static QRegExp retValAttributionRegexCheck_target(QLatin1String("%PYARG_0\\s*=[^=]\\s*.+"));
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, language);
    foreach (const CodeSnip &snip, snips) {
        if (language == TypeSystem::TargetLangCode) {
            if (retValAttributionRegexCheck_target.indexIn(snip.code()) != -1)
                return true;
        } else {
            if (retValAttributionRegexCheck_native.indexIn(snip.code()) != -1)
                return true;
        }
    }
    return false;
}

bool ShibokenGenerator::injectedCodeUsesArgument(const AbstractMetaFunction* func, int argumentIndex)
{
    CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny);
    foreach (const CodeSnip &snip, snips) {
        QString code = snip.code();
        if (code.contains(QLatin1String("%ARGUMENT_NAMES")))
            return true;
        if (code.contains(QRegExp(QStringLiteral("%%1\\b").arg(argumentIndex + 1))))
            return true;
    }
    return false;
}

bool ShibokenGenerator::hasMultipleInheritanceInAncestry(const AbstractMetaClass* metaClass)
{
    if (!metaClass || metaClass->baseClassNames().isEmpty())
        return false;
    if (metaClass->baseClassNames().size() > 1)
        return true;
    return hasMultipleInheritanceInAncestry(metaClass->baseClass());
}

typedef QMap<QString, AbstractMetaFunctionList> FunctionGroupMap;
typedef FunctionGroupMap::const_iterator FunctionGroupMapIt;

bool ShibokenGenerator::classNeedsGetattroFunction(const AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return false;
    const FunctionGroupMap &functionGroup = getFunctionGroups(metaClass);
    for (FunctionGroupMapIt it = functionGroup.cbegin(), end = functionGroup.cend(); it != end; ++it) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, it.value()) {
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

AbstractMetaFunctionList ShibokenGenerator::getMethodsWithBothStaticAndNonStaticMethods(const AbstractMetaClass* metaClass)
{
    AbstractMetaFunctionList methods;
    if (metaClass) {
        const FunctionGroupMap &functionGroups = getFunctionGroups(metaClass);
        for (FunctionGroupMapIt it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
            AbstractMetaFunctionList overloads;
            foreach (AbstractMetaFunction* func, it.value()) {
                if (func->isAssignmentOperator() || func->isCastOperator() || func->isModifiedRemoved()
                    || func->isPrivate() || func->ownerClass() != func->implementingClass()
                    || func->isConstructor() || func->isOperatorOverload())
                    continue;
                overloads.append(func);
            }
            if (overloads.isEmpty())
                continue;
            if (OverloadData::hasStaticAndInstanceFunctions(overloads))
                methods.append(overloads.first());
        }
    }
    return methods;
}

AbstractMetaClassList ShibokenGenerator::getBaseClasses(const AbstractMetaClass* metaClass) const
{
    AbstractMetaClassList baseClasses;
    if (metaClass) {
        foreach (const QString &parent, metaClass->baseClassNames()) {
            AbstractMetaClass* clazz = classes().findClass(parent);
            if (clazz)
                baseClasses << clazz;
        }
    }
    return baseClasses;
}

const AbstractMetaClass* ShibokenGenerator::getMultipleInheritingClass(const AbstractMetaClass* metaClass)
{
    if (!metaClass || metaClass->baseClassNames().isEmpty())
        return 0;
    if (metaClass->baseClassNames().size() > 1)
        return metaClass;
    return getMultipleInheritingClass(metaClass->baseClass());
}

AbstractMetaClassList ShibokenGenerator::getAllAncestors(const AbstractMetaClass* metaClass) const
{
    AbstractMetaClassList result;
    if (metaClass) {
        AbstractMetaClassList baseClasses = getBaseClasses(metaClass);
        foreach (AbstractMetaClass* base, baseClasses) {
            result.append(base);
            result.append(getAllAncestors(base));
        }
    }
    return result;
}

QString ShibokenGenerator::getModuleHeaderFileName(const QString& moduleName) const
{
    QString result = moduleName.isEmpty() ? packageName() : moduleName;
    result.replace(QLatin1Char('.'), QLatin1Char('_'));
    return result.toLower() + QLatin1String("_python.h");
}

QString ShibokenGenerator::extendedIsConvertibleFunctionName(const TypeEntry* targetType) const
{
    QString p = targetType->targetLangPackage();
    p.replace(QLatin1Char('.'), QLatin1Char('_'));
    return QStringLiteral("ExtendedIsConvertible_%1_%2").arg(p, targetType->name());
}

QString ShibokenGenerator::extendedToCppFunctionName(const TypeEntry* targetType) const
{
    QString p = targetType->targetLangPackage();
    p.replace(QLatin1Char('.'), QLatin1Char('_'));
    return QStringLiteral("ExtendedToCpp_%1_%2").arg(p, targetType->name());
}

bool ShibokenGenerator::isCopyable(const AbstractMetaClass *metaClass)

{
    if (metaClass->isNamespace() || isObjectType(metaClass))
        return false;
    else if (metaClass->typeEntry()->copyable() == ComplexTypeEntry::Unknown)
        return metaClass->hasCloneOperator();
    else
        return (metaClass->typeEntry()->copyable() == ComplexTypeEntry::CopyableSet);

    return false;
}

AbstractMetaType* ShibokenGenerator::buildAbstractMetaTypeFromString(QString typeSignature)
{
    typeSignature = typeSignature.trimmed();
    if (typeSignature.startsWith(QLatin1String("::")))
        typeSignature.remove(0, 2);

    if (m_metaTypeFromStringCache.contains(typeSignature))
        return m_metaTypeFromStringCache.value(typeSignature);

    QString typeString = typeSignature;
    bool isConst = typeString.startsWith(QLatin1String("const "));
    if (isConst)
        typeString.remove(0, sizeof("const ") / sizeof(char) - 1);

    ReferenceType refType = NoReference;
    if (typeString.endsWith(QLatin1String("&&"))) {
        refType = RValueReference;
        typeString.chop(2);
        typeString = typeString.trimmed();
    } else if (typeString.endsWith(QLatin1Char('&'))) {
        refType = LValueReference;
        typeString.chop(1);
        typeString = typeString.trimmed();
    }

    int indirections = 0;
    while (typeString.endsWith(QLatin1Char('*'))) {
        ++indirections;
        typeString.chop(1);
        typeString = typeString.trimmed();
    }

    if (typeString.startsWith(QLatin1String("::")))
        typeString.remove(0, 2);

    QString adjustedTypeName = typeString;
    QStringList instantiatedTypes;
    int lpos = typeString.indexOf(QLatin1Char('<'));
    if (lpos > -1) {
        int rpos = typeString.lastIndexOf(QLatin1Char('>'));
        if ((lpos != -1) && (rpos != -1)) {
            QString type = typeString.mid(lpos + 1, rpos - lpos - 1);
            int depth = 0;
            int start = 0;
            for (int i = 0; i < type.count(); ++i) {
                if (type.at(i) == QLatin1Char('<')) {
                    ++depth;
                } else if (type.at(i) == QLatin1Char('>')) {
                    --depth;
                } else if (type.at(i) == QLatin1Char(',') && depth == 0) {
                    instantiatedTypes << type.mid(start, i - start).trimmed();
                    start = i + 1;
                }
            }
            instantiatedTypes << type.mid(start).trimmed();
            adjustedTypeName.truncate(lpos);
        }
    }

    TypeEntry* typeEntry = TypeDatabase::instance()->findType(adjustedTypeName);

    AbstractMetaType* metaType = 0;
    if (typeEntry) {
        metaType = new AbstractMetaType();
        metaType->setTypeEntry(typeEntry);
        metaType->setIndirections(indirections);
        metaType->setReferenceType(refType);
        metaType->setConstant(isConst);
        metaType->setTypeUsagePattern(AbstractMetaType::ContainerPattern);
        foreach (const QString& instantiation, instantiatedTypes) {
            AbstractMetaType* tmplArgType = buildAbstractMetaTypeFromString(instantiation);
            metaType->addInstantiation(tmplArgType);
        }
        metaType->decideUsagePattern();
        m_metaTypeFromStringCache.insert(typeSignature, metaType);
    }
    return metaType;
}

AbstractMetaType* ShibokenGenerator::buildAbstractMetaTypeFromTypeEntry(const TypeEntry* typeEntry)
{
    QString typeName = typeEntry->qualifiedCppName();
    if (typeName.startsWith(QLatin1String("::")))
        typeName.remove(0, 2);
    if (m_metaTypeFromStringCache.contains(typeName))
        return m_metaTypeFromStringCache.value(typeName);
    AbstractMetaType* metaType = new AbstractMetaType;
    metaType->setTypeEntry(typeEntry);
    metaType->setIndirections(0);
    metaType->setReferenceType(NoReference);
    metaType->setConstant(false);
    metaType->decideUsagePattern();
    m_metaTypeFromStringCache.insert(typeName, metaType);
    return metaType;
}
AbstractMetaType* ShibokenGenerator::buildAbstractMetaTypeFromAbstractMetaClass(const AbstractMetaClass* metaClass)
{
    return ShibokenGenerator::buildAbstractMetaTypeFromTypeEntry(metaClass->typeEntry());
}

/*
static void dumpFunction(AbstractMetaFunctionList lst)
{
    qDebug() << "DUMP FUNCTIONS: ";
    foreach (AbstractMetaFunction *func, lst)
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

static bool isGroupable(const AbstractMetaFunction* func)
{
    if (func->isSignal() || func->isDestructor() || (func->isModifiedRemoved() && !func->isAbstract()))
        return false;
    // weird operator overloads
    if (func->name() == QLatin1String("operator[]") || func->name() == QLatin1String("operator->"))  // FIXME: what about cast operators?
        return false;;
    return true;
}

QMap< QString, AbstractMetaFunctionList > ShibokenGenerator::getFunctionGroups(const AbstractMetaClass* scope)
{
    AbstractMetaFunctionList lst = scope ? scope->functions() : globalFunctions();

    QMap<QString, AbstractMetaFunctionList> results;
    foreach (AbstractMetaFunction* func, lst) {
        if (isGroupable(func))
            results[func->name()].append(func);
    }
    return results;
}

AbstractMetaFunctionList ShibokenGenerator::getFunctionOverloads(const AbstractMetaClass* scope, const QString& functionName)
{
    AbstractMetaFunctionList lst = scope ? scope->functions() : globalFunctions();

    AbstractMetaFunctionList results;
    foreach (AbstractMetaFunction* func, lst) {
        if (func->name() != functionName)
            continue;
        if (isGroupable(func))
            results << func;
    }
    return results;

}

QPair< int, int > ShibokenGenerator::getMinMaxArguments(const AbstractMetaFunction* metaFunction)
{
    AbstractMetaFunctionList overloads = getFunctionOverloads(metaFunction->ownerClass(), metaFunction->name());

    int minArgs = std::numeric_limits<int>::max();
    int maxArgs = 0;
    foreach (const AbstractMetaFunction* func, overloads) {
        int numArgs = 0;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (!func->argumentRemoved(arg->argumentIndex() + 1))
                numArgs++;
        }
        maxArgs = std::max(maxArgs, numArgs);
        minArgs = std::min(minArgs, numArgs);
    }
    return qMakePair(minArgs, maxArgs);
}

QMap<QString, QString> ShibokenGenerator::options() const
{
    QMap<QString, QString> opts(Generator::options());
    opts.insert(QLatin1String(AVOID_PROTECTED_HACK),
                QLatin1String("Avoid the use of the '#define protected public' hack."));
    opts.insert(QLatin1String(PARENT_CTOR_HEURISTIC),
                QLatin1String("Enable heuristics to detect parent relationship on constructors."));
    opts.insert(QLatin1String(RETURN_VALUE_HEURISTIC),
                QLatin1String("Enable heuristics to detect parent relationship on return values (USE WITH CAUTION!)"));
    opts.insert(QLatin1String(ENABLE_PYSIDE_EXTENSIONS),
                QLatin1String("Enable PySide extensions, such as support for signal/slots, use this if you are creating a binding for a Qt-based library."));
    opts.insert(QLatin1String(DISABLE_VERBOSE_ERROR_MESSAGES),
                QLatin1String("Disable verbose error messages. Turn the python code hard to debug but safe few kB on the generated bindings."));
    opts.insert(QLatin1String(USE_ISNULL_AS_NB_NONZERO),
                QLatin1String("If a class have an isNull()const method, it will be used to compute the value of boolean casts"));
    return opts;
}

static void getCode(QStringList& code, const CodeSnipList& codeSnips)
{
    foreach (const CodeSnip& snip, codeSnips)
        code.append(snip.code());
}

static void getCode(QStringList& code, const TypeEntry* type)
{
    getCode(code, type->codeSnips());

    CustomConversion* customConversion = type->customConversion();
    if (!customConversion)
        return;

    if (!customConversion->nativeToTargetConversion().isEmpty())
        code.append(customConversion->nativeToTargetConversion());

    const CustomConversion::TargetToNativeConversions& toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;

    foreach (CustomConversion::TargetToNativeConversion* toNative, toCppConversions)
        code.append(toNative->conversion());
}

bool ShibokenGenerator::doSetup(const QMap<QString, QString>& args)
{
    m_useCtorHeuristic = args.contains(QLatin1String(PARENT_CTOR_HEURISTIC));
    m_usePySideExtensions = args.contains(QLatin1String(ENABLE_PYSIDE_EXTENSIONS));
    m_userReturnValueHeuristic = args.contains(QLatin1String(RETURN_VALUE_HEURISTIC));
    m_verboseErrorMessagesDisabled = args.contains(QLatin1String(DISABLE_VERBOSE_ERROR_MESSAGES));
    m_useIsNullAsNbNonZero = args.contains(QLatin1String(USE_ISNULL_AS_NB_NONZERO));
    m_avoidProtectedHack = args.contains(QLatin1String(AVOID_PROTECTED_HACK));

    TypeDatabase* td = TypeDatabase::instance();
    QStringList snips;
    foreach (const PrimitiveTypeEntry* type, primitiveTypes())
        getCode(snips, type);
    foreach (const ContainerTypeEntry* type, containerTypes())
        getCode(snips, type);
    foreach (const AbstractMetaClass* metaClass, classes())
        getCode(snips, metaClass->typeEntry());
    getCode(snips, td->findType(packageName()));
    const FunctionGroupMap &functionGroups = getFunctionGroups();
    for (FunctionGroupMapIt it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        foreach (AbstractMetaFunction* func, it.value())
            getCode(snips, func->injectedCodeSnips());
    }

    foreach (const QString& code, snips) {
        collectContainerTypesFromConverterMacros(code, true);
        collectContainerTypesFromConverterMacros(code, false);
    }

    return true;
}

void ShibokenGenerator::collectContainerTypesFromConverterMacros(const QString& code, bool toPythonMacro)
{
    QString convMacro = toPythonMacro ? QLatin1String("%CONVERTTOPYTHON[") : QLatin1String("%CONVERTTOCPP[");
    int offset = toPythonMacro ? sizeof("%CONVERTTOPYTHON") : sizeof("%CONVERTTOCPP");
    int start = 0;
    while ((start = code.indexOf(convMacro, start)) != -1) {
        int end = code.indexOf(QLatin1Char(']'), start);
        start += offset;
        if (code.at(start) != QLatin1Char('%')) {
            QString typeString = code.mid(start, end - start);
            AbstractMetaType* type = buildAbstractMetaTypeFromString(typeString);
            addInstantiatedContainers(type, type->originalTypeDescription());
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

QString ShibokenGenerator::cppApiVariableName(const QString& moduleName) const
{
    QString result = moduleName.isEmpty() ? ShibokenGenerator::packageName() : moduleName;
    result.replace(QLatin1Char('.'), QLatin1Char('_'));
    result.prepend(QLatin1String("Sbk"));
    result.append(QLatin1String("Types"));
    return result;
}

QString ShibokenGenerator::convertersVariableName(const QString& moduleName) const
{
    QString result = cppApiVariableName(moduleName);
    result.chop(1);
    result.append(QLatin1String("Converters"));
    return result;
}

static QString processInstantiationsVariableName(const AbstractMetaType* type)
{
    QString res = QLatin1Char('_') + _fixedCppTypeName(type->typeEntry()->qualifiedCppName()).toUpper();
    foreach (const AbstractMetaType* instantiation, type->instantiations()) {
        res += instantiation->isContainer()
               ? processInstantiationsVariableName(instantiation)
               : QLatin1Char('_') + _fixedCppTypeName(instantiation->cppSignature()).toUpper();
    }
    return res;
}
QString ShibokenGenerator::getTypeIndexVariableName(const AbstractMetaClass* metaClass, bool alternativeTemplateName)
{
    if (alternativeTemplateName) {
        const AbstractMetaClass* templateBaseClass = metaClass->templateBaseClass();
        if (!templateBaseClass)
            return QString();
        QString base = _fixedCppTypeName(templateBaseClass->typeEntry()->qualifiedCppName()).toUpper();
        QString instantiations;
        foreach (const AbstractMetaType* instantiation, metaClass->templateBaseClassInstantiations())
            instantiations += processInstantiationsVariableName(instantiation);
        return QString::fromLatin1("SBK_%1%2_IDX").arg(base, instantiations);
    }
    return getTypeIndexVariableName(metaClass->typeEntry());
}
QString ShibokenGenerator::getTypeIndexVariableName(const TypeEntry* type)
{
    if (type->isCppPrimitive()) {
        const PrimitiveTypeEntry* trueType = (const PrimitiveTypeEntry*) type;
        if (trueType->basicReferencedTypeEntry())
            type = trueType->basicReferencedTypeEntry();
    }
    return QString::fromLatin1("SBK_%1_IDX").arg(_fixedCppTypeName(type->qualifiedCppName()).toUpper());
}
QString ShibokenGenerator::getTypeIndexVariableName(const AbstractMetaType* type)
{
    return QString::fromLatin1("SBK%1%2_IDX")
              .arg(type->typeEntry()->isContainer() ? QLatin1Char('_') + moduleName().toUpper() : QString(),
                   processInstantiationsVariableName(type));
}

bool ShibokenGenerator::verboseErrorMessagesDisabled() const
{
    return m_verboseErrorMessagesDisabled;
}

bool ShibokenGenerator::pythonFunctionWrapperUsesListOfArguments(const OverloadData& overloadData)
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

Generator::Options ShibokenGenerator::getConverterOptions(const AbstractMetaType* metaType)
{
    // exclude const on Objects
    Options flags;
    const TypeEntry* type = metaType->typeEntry();
    bool isCStr = isCString(metaType);
    if (metaType->indirections() && !isCStr) {
        flags = ExcludeConst;
    } else if (metaType->isContainer()
        || (type->isPrimitive() && !isCStr)
        // const refs become just the value, but pure refs must remain pure.
        || (type->isValue() && metaType->isConstant() && metaType->referenceType() == LValueReference)) {
        flags = ExcludeConst | ExcludeReference;
    }
    return flags;
}

QString  ShibokenGenerator::getDefaultValue(const AbstractMetaFunction* func, const AbstractMetaArgument* arg)
{
    if (!arg->defaultValueExpression().isEmpty())
        return arg->defaultValueExpression();

    //Check modifications
    foreach(FunctionModification  m, func->modifications()) {
        foreach(ArgumentModification am, m.argument_mods) {
            if (am.index == (arg->argumentIndex() + 1))
                return am.replacedDefaultExpression;
        }
    }
    return QString();
}

void ShibokenGenerator::writeMinimalConstructorExpression(QTextStream& s, const AbstractMetaType* type, const QString& defaultCtor)
{
    if (defaultCtor.isEmpty() && isCppPrimitive(type))
        return;
    QString ctor = defaultCtor.isEmpty() ? minimalConstructor(type) : defaultCtor;
    if (ctor.isEmpty())
        qFatal(qPrintable(QString::fromLatin1(MIN_CTOR_ERROR_MSG).arg(type->cppSignature())), NULL);
    s << " = " << ctor;
}

void ShibokenGenerator::writeMinimalConstructorExpression(QTextStream& s, const TypeEntry* type, const QString& defaultCtor)
{
    if (defaultCtor.isEmpty() && isCppPrimitive(type))
        return;
    QString ctor = defaultCtor.isEmpty() ? minimalConstructor(type) : defaultCtor;
    if (ctor.isEmpty())
        qFatal(qPrintable(QString::fromLatin1(MIN_CTOR_ERROR_MSG).arg(type->qualifiedCppName())), NULL);
    s << " = " << ctor;
}

bool ShibokenGenerator::isCppIntegralPrimitive(const TypeEntry* type)
{
    if (!type->isCppPrimitive())
        return false;
    const PrimitiveTypeEntry* trueType = (const PrimitiveTypeEntry*) type;
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    QString typeName = trueType->qualifiedCppName();
    return !typeName.contains(QLatin1String("double"))
        && !typeName.contains(QLatin1String("float"))
        && !typeName.contains(QLatin1String("wchar"));
}
bool ShibokenGenerator::isCppIntegralPrimitive(const AbstractMetaType* type)
{
    return isCppIntegralPrimitive(type->typeEntry());
}
