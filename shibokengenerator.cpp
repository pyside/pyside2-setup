/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "shibokengenerator.h"
#include <reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>

#define NULL_VALUE "NULL"
#define COMMENT_LINE_WIDTH  77

static Indentor INDENT;
static void dumpFunction(AbstractMetaFunctionList lst);

QHash<QString, QString> ShibokenGenerator::m_pythonPrimitiveTypeName = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_pythonOperators = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_formatUnits = QHash<QString, QString>();
QHash<QString, QString> ShibokenGenerator::m_tpFuncs = QHash<QString, QString>();

ShibokenGenerator::ShibokenGenerator() : Generator()
{
    if (m_pythonPrimitiveTypeName.isEmpty())
        ShibokenGenerator::initPrimitiveTypesCorrespondences();

    if (m_tpFuncs.isEmpty())
        ShibokenGenerator::clearTpFuncs();
}

void ShibokenGenerator::clearTpFuncs()
{
    m_tpFuncs["__str__"] = QString("0");
    m_tpFuncs["__repr__"] = QString("0");
}

void ShibokenGenerator::initPrimitiveTypesCorrespondences()
{
    // Python primitive types names
    m_pythonPrimitiveTypeName.clear();

    // PyBool
    m_pythonPrimitiveTypeName["bool"] = "PyBool";

    // PyInt
    m_pythonPrimitiveTypeName["char"] = "PyInt";
    m_pythonPrimitiveTypeName["unsigned char"] = "PyInt";
    m_pythonPrimitiveTypeName["int"] = "PyInt";
    m_pythonPrimitiveTypeName["uint"] = "PyInt";
    m_pythonPrimitiveTypeName["unsigned int"] = "PyInt";
    m_pythonPrimitiveTypeName["short"] = "PyInt";
    m_pythonPrimitiveTypeName["ushort"] = "PyInt";
    m_pythonPrimitiveTypeName["unsigned short"] = "PyInt";
    m_pythonPrimitiveTypeName["long"] = "PyInt";

    // PyFloat
    m_pythonPrimitiveTypeName["double"] = "PyFloat";
    m_pythonPrimitiveTypeName["float"] = "PyFloat";

    // PyLong
    m_pythonPrimitiveTypeName["unsigned long"] = "PyLong";
    m_pythonPrimitiveTypeName["ulong"] = "PyLong";
    m_pythonPrimitiveTypeName["long long"] = "PyLong";
    m_pythonPrimitiveTypeName["__int64"] = "PyLong";
    m_pythonPrimitiveTypeName["unsigned long long"] = "PyLong";
    m_pythonPrimitiveTypeName["unsigned __int64"] = "PyLong";

    // Python operators
    m_pythonOperators.clear();

    // Arithmetic operators
    m_pythonOperators["operator+"] = "add";
    m_pythonOperators["operator-"] = "sub";
    m_pythonOperators["operator*"] = "mul";
    m_pythonOperators["operator/"] = "div";
    m_pythonOperators["operator%"] = "mod";

    // Inplace arithmetic operators
    m_pythonOperators["operator+="] = "iadd";
    m_pythonOperators["operator-="] = "isub";
    m_pythonOperators["operator*="] = "imul";
    m_pythonOperators["operator/="] = "idiv";
    m_pythonOperators["operator%="] = "imod";

    // Bitwise operators
    m_pythonOperators["operator&"] = "and";
    m_pythonOperators["operator^"] = "xor";
    m_pythonOperators["operator|"] = "or";
    m_pythonOperators["operator<<"] = "lshift";
    m_pythonOperators["operator>>"] = "rshift";
    m_pythonOperators["operator~"] = "invert";

    // Inplace bitwise operators
    m_pythonOperators["operator&="] = "iand";
    m_pythonOperators["operator^="] = "ixor";
    m_pythonOperators["operator|="] = "ior";
    m_pythonOperators["operator<<="] = "ilshift";
    m_pythonOperators["operator>>="] = "irshift";

    // Comparison operators
    m_pythonOperators["operator=="] = "eq";
    m_pythonOperators["operator!="] = "ne";
    m_pythonOperators["operator<"] = "lt";
    m_pythonOperators["operator>"] = "gt";
    m_pythonOperators["operator<="] = "le";
    m_pythonOperators["operator>="] = "ge";

    m_pythonOperators["operator[]"] = "getitem";

    // Initialize format units for C++->Python->C++ conversion
    m_formatUnits.clear();
    m_formatUnits.insert("char", "b");
    m_formatUnits.insert("unsigned char", "B");
    m_formatUnits.insert("int", "i");
    m_formatUnits.insert("unsigned int", "I");
    m_formatUnits.insert("short", "h");
    m_formatUnits.insert("unsigned short", "H");
    m_formatUnits.insert("long", "l");
    m_formatUnits.insert("unsigned long", "k");
    m_formatUnits.insert("long long", "L");
    m_formatUnits.insert("__int64", "L");
    m_formatUnits.insert("unsigned long long", "K");
    m_formatUnits.insert("unsigned __int64", "K");
    m_formatUnits.insert("double", "d");
    m_formatUnits.insert("float", "f");
}

FunctionModificationList ShibokenGenerator::functionModifications(const AbstractMetaFunction* func)
{
    FunctionModificationList mods;
    const AbstractMetaClass *cls = func->ownerClass();
    while (cls) {
        mods += func->modifications(cls);
        if (cls == cls->baseClass())
            break;
        cls = cls->baseClass();
    }
    return mods;
}

QString ShibokenGenerator::translateTypeForWrapperMethod(const AbstractMetaType* cType,
                                                         const AbstractMetaClass* context) const
{
    QString result;

    if (cType->isValue() || cType->isValuePointer() || cType->isObject()
        || (cType->isReference() && !cType->isContainer())) {
        result = cType->typeEntry()->qualifiedCppName();
        if (cType->isObject() || cType->isQObject())
            result.append('*');
        else if (cType->isValue() && cType->isReference())
            result.append('&');
    } else if (cType->isArray()) {
        result = translateTypeForWrapperMethod(cType->arrayElementType(), context) + "[]";
    } else {
        result = translateType(cType, context);
    }

    return result;
}

QString ShibokenGenerator::wrapperName(const AbstractMetaClass* metaClass)
{
    QString result = metaClass->name();
    if (metaClass->enclosingClass()) // is a inner class
        result.replace("::", "_");
    result +="Wrapper";
    return result;
}

QString ShibokenGenerator::cpythonFunctionName(const AbstractMetaFunction* func)
{
    QString result;

    if (func->ownerClass()) {
        result = cpythonBaseName(func->ownerClass()->typeEntry());
        result += '_';
        if (func->isConstructor() || func->isCopyConstructor())
            result += "New";
        else if (func->isOperatorOverload())
            result += ShibokenGenerator::pythonOperatorFunctionName(func);
        else
            result += func->name();
    } else {
        result = "Py" + moduleName() + "Module_" + func->name();
    }

    return result;
}

static QString cpythonEnumFlagsName(QString moduleName, QString qualifiedCppName)
{
    QString result = QString("Py%1_%2").arg(moduleName).arg(qualifiedCppName);
    result.replace("::", "_");
    return result;
}

QString ShibokenGenerator::cpythonEnumName(const EnumTypeEntry* enumEntry)
{
    return cpythonEnumFlagsName(moduleName(), enumEntry->qualifiedCppName());
}

QString ShibokenGenerator::cpythonFlagsName(const FlagsTypeEntry* flagsEntry)
{
    return cpythonEnumFlagsName(moduleName(), flagsEntry->originalName());
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
    if (type->isValue() || type->isObject())
        return QString("%1_cptr(%2)").arg(cpythonBaseName(type)).arg(argName);
    return QString();
}

QString ShibokenGenerator::getFunctionReturnType(const AbstractMetaFunction* func, Options options) const
{
    if (func->ownerClass() && (func->isConstructor() || func->isCopyConstructor()))
        return func->ownerClass()->qualifiedCppName() + '*';

    return translateTypeForWrapperMethod(func->type(), func->implementingClass());

    //TODO: check these lines
    //QString modifiedReturnType = QString(func->typeReplaced(0));
    //return modifiedReturnType.isNull() ?
    //translateType(func->type(), func->implementingClass()) : modifiedReturnType;
}

static QString baseConversionString(QString typeName)
{
    return QString("Shiboken::Converter<%1 >::").arg(typeName);
}

void ShibokenGenerator::writeBaseConversion(QTextStream& s, const TypeEntry* type)
{
    s << baseConversionString(type->name());
}

void ShibokenGenerator::writeBaseConversion(QTextStream& s, const AbstractMetaType* type,
                                            const AbstractMetaClass* context)
{
    QString typeName;
    if (type->isPrimitive()) {
        const PrimitiveTypeEntry* ptype = (const PrimitiveTypeEntry*) type->typeEntry();
        if (ptype->basicAliasedTypeEntry())
            ptype = ptype->basicAliasedTypeEntry();
        typeName = ptype->name();
    } else {
        typeName = translateTypeForWrapperMethod(type, context);
    }


    // If the type is an Object (and a pointer) remove its constness
    // (len("const ") == 6) since it is already inserted for everyone
    // in the generated converter declaration.
    if ((type->isQObject() || type->isObject()) && typeName.startsWith("const "))
        typeName.remove(0, 6);

    // Remove the constness, if any
    if (typeName.startsWith("const ") && type->name() != "char")
        typeName.remove(0, 6);

    if (typeName.endsWith("&") && !(type->isValue() && type->isReference()))
        typeName.chop(1);

    s << baseConversionString(typeName);
}

void ShibokenGenerator::writeToPythonConversion(QTextStream& s, const AbstractMetaType* type,
                                                const AbstractMetaClass* context, QString argumentName)
{
    if (!type)
        return;
    writeBaseConversion(s, type, context);
    s << "toPython";
    if (!argumentName.isEmpty())
        s << '(' << argumentName << ')';
}

void ShibokenGenerator::writeToCppConversion(QTextStream& s, const AbstractMetaType* type,
                                             const AbstractMetaClass* context, QString argumentName)
{
    writeBaseConversion(s, type, context);
    s << "toCpp(" << argumentName << ')';
}

QString ShibokenGenerator::getFormatUnitString(const AbstractMetaFunction* func) const
{
    QString result;
    foreach (const AbstractMetaArgument* arg, func->arguments()) {
        if (func->argumentRemoved(arg->argumentIndex()))
            continue;

        if (arg->type()->isQObject()
            || arg->type()->isObject()
            || arg->type()->isValue()
            || arg->type()->isReference()) {
            result += 'O';
        } else if (arg->type()->isPrimitive()) {
            const PrimitiveTypeEntry* ptype = (const PrimitiveTypeEntry*) arg->type()->typeEntry();
            if (ptype->basicAliasedTypeEntry())
                ptype = ptype->basicAliasedTypeEntry();
            if (m_formatUnits.contains(ptype->name()))
                result += m_formatUnits[ptype->name()];
            else
                result += 'O';
        } else if (arg->type()->isNativePointer() && arg->type()->name() == "char") {
            result += 'z';
        } else {
            result += 'Y';
        }
    }
    return result;
}

QString ShibokenGenerator::cpythonBaseName(const AbstractMetaType* type)
{
    if (type->name() == "char" && type->isNativePointer())
        return QString("PyString");
    return cpythonBaseName(type->typeEntry());
}

QString ShibokenGenerator::cpythonBaseName(const TypeEntry* type)
{
    QString baseName;
    if ((type->isObject() || type->isValue() || type->isNamespace())) { // && !type->isReference()) {
        baseName = QString("Py") + type->name();
    } else if (type->isPrimitive()) {
        const PrimitiveTypeEntry* ptype = (const PrimitiveTypeEntry*) type;
        if (ptype->basicAliasedTypeEntry())
            ptype = ptype->basicAliasedTypeEntry();
        if (ptype->targetLangApiName() == ptype->name())
            baseName = m_pythonPrimitiveTypeName[ptype->name()];
        else
            baseName = ptype->targetLangApiName();
    } else if (type->isEnum()) {
        baseName = cpythonEnumName((const EnumTypeEntry*) type);
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
                baseName = "PySequence";
                break;
            case ContainerTypeEntry::SetContainer:
                baseName = "PySet";
                break;
            case ContainerTypeEntry::MapContainer:
            case ContainerTypeEntry::MultiMapContainer:
            case ContainerTypeEntry::HashContainer:
            case ContainerTypeEntry::MultiHashContainer:
                baseName = "PyDict";
                break;
            default:
                Q_ASSERT(false);
        }
    } else if (type->isFlags()) {
        baseName = "PyMethod"; // FIXME: This is just a placeholder! Add support for qflags!
    } else {
        baseName = "PyObject";
    }
    return baseName.replace("::", "_");
}

QString ShibokenGenerator::cpythonTypeName(const AbstractMetaClass* metaClass)
{
    return cpythonTypeName(metaClass->typeEntry());
}

QString ShibokenGenerator::cpythonTypeName(const TypeEntry* type)
{
    return cpythonBaseName(type) + "_Type";
}

QString ShibokenGenerator::cpythonOperatorFunctionName(const AbstractMetaFunction* func)
{
    if (!func->isOperatorOverload())
        return QString();
    return QString("Py") + func->ownerClass()->name()
            + '_' + pythonOperatorFunctionName(func->originalName());
}

QString ShibokenGenerator::pythonPrimitiveTypeName(QString cppTypeName)
{
    return ShibokenGenerator::m_pythonPrimitiveTypeName.value(cppTypeName, QString());
}

QString ShibokenGenerator::pythonPrimitiveTypeName(const PrimitiveTypeEntry* type)
{
    if (type->basicAliasedTypeEntry())
        type = type->basicAliasedTypeEntry();
    return pythonPrimitiveTypeName(type->name());
}

QString ShibokenGenerator::pythonOperatorFunctionName(QString cppOpFuncName)
{
    QString value = m_pythonOperators.value(cppOpFuncName);
    if (value.isEmpty()) {
        ReportHandler::warning("Unknown operator: "+cppOpFuncName);
        value = "UNKNOWN_OPERATOR";
    }
    value.prepend("__").append("__");
    return value;
}

QString ShibokenGenerator::pythonOperatorFunctionName(const AbstractMetaFunction* func)
{
    QString op = pythonOperatorFunctionName(func->originalName());
    if (func->arguments().isEmpty()) {
        if (op == "__sub__")
            op = QString("__neg__");
        else if (op == "__add__")
            op = QString("__pos__");
    } else if (func->isStatic() && func->arguments().size() == 2) {
        // If a operator overload function has 2 arguments and
        // is static we assume that it is a reverse operator.
        op = op.insert(2, 'r');
    }
    return op;
}

QString ShibokenGenerator::pythonRichCompareOperatorId(QString cppOpFuncName)
{
    return QString("Py_%1").arg(m_pythonOperators.value(cppOpFuncName).toUpper());
}

QString ShibokenGenerator::pythonRichCompareOperatorId(const AbstractMetaFunction* func)
{
    return pythonRichCompareOperatorId(func->originalName());
}

bool ShibokenGenerator::isNumber(QString cpythonApiName)
{
    return cpythonApiName == "PyInt"
            || cpythonApiName == "PyFloat"
            || cpythonApiName == "PyLong";
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
    return pythonPrimitiveTypeName((const PrimitiveTypeEntry*) type) == "PyInt";
}

bool ShibokenGenerator::isPyInt(const AbstractMetaType* type)
{
    return isPyInt(type->typeEntry());
}

bool ShibokenGenerator::isReverseOperator(const AbstractMetaFunction* func)
{
    if (!func->isOperatorOverload())
        return false;

    const AbstractMetaClass* cppClass = func->ownerClass();
    AbstractMetaArgumentList args = func->arguments();
    // Here we expect static operator overloads with
    // 2 arguments to represent reverse operators.
    // e.g. static operator*(double,TYPE) => double * TYPE => TYPE.__rmul__(double).
    return args.size() == 2 && cppClass &&
            args[1]->type()->typeEntry() == cppClass->typeEntry();
}

static QString checkFunctionName(QString baseName, bool genericNumberType)
{
    if (genericNumberType && ShibokenGenerator::isNumber(baseName))
        baseName = "PyNumber";
    return baseName + "_Check";
}

QString ShibokenGenerator::cpythonCheckFunction(const AbstractMetaType* metaType, bool genericNumberType)
{
    return checkFunctionName(cpythonBaseName(metaType), genericNumberType);
}

QString ShibokenGenerator::cpythonCheckFunction(const TypeEntry* type, bool genericNumberType)
{
    return checkFunctionName(cpythonBaseName(type), genericNumberType);
}

QString ShibokenGenerator::cpythonIsConvertibleFunction(const AbstractMetaType* metaType)
{
    QString baseName;
    QTextStream s(&baseName);
    writeBaseConversion(s, metaType->typeEntry());
    s << "isConvertible";
    s.flush();
    return baseName;
}

QString ShibokenGenerator::argumentString(const AbstractMetaFunction *func,
                                          const AbstractMetaArgument *argument,
                                          Options options) const
{
    QString modified_type = func->typeReplaced(argument->argumentIndex() + 1);
    QString arg;

    if (modified_type.isEmpty())
        arg = translateType(argument->type(), func->implementingClass(), options);
    else
        arg = modified_type.replace('$', '.');

    if (!(options & Generator::SkipName)) {
        arg += " ";
        arg += argument->argumentName();
    }

    QList<ReferenceCount> referenceCounts;
    referenceCounts = func->referenceCounts(func->implementingClass(), argument->argumentIndex() + 1);
    if ((options & Generator::SkipDefaultValues) != Generator::SkipDefaultValues &&
        !argument->originalDefaultValueExpression().isEmpty())
    {
        QString default_value = argument->originalDefaultValueExpression();
        if (default_value == "NULL")
            default_value = NULL_VALUE;

        //WORKAROUND: fix this please
        if (default_value.startsWith("new "))
            default_value.remove(0, 4);

        arg += " = " + default_value;
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
                                             int argCount) const
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

QString ShibokenGenerator::signatureForDefaultVirtualMethod(const AbstractMetaFunction *func,
                                                            QString prepend,
                                                            QString append,
                                                            Options options,
                                                            int argCount) const
{
    QString defaultMethodSignature = functionSignature(func, prepend, append, options, argCount);
    QString staticSelf("(");
    if (func->isConstant())
        staticSelf += "const ";
    staticSelf += func->ownerClass()->qualifiedCppName()+"& self";
    if (!func->arguments().isEmpty())
        staticSelf += ", ";
    defaultMethodSignature.replace(defaultMethodSignature.lastIndexOf(") const"), 7, ")");
    defaultMethodSignature.replace(defaultMethodSignature.indexOf('('), 1, staticSelf);
    return defaultMethodSignature;
}

bool ShibokenGenerator::hasInjectedCodeOrSignatureModification(const AbstractMetaFunction* func)
{
    foreach (FunctionModification mod, functionModifications(func)) {
        if (mod.isCodeInjection() || mod.isRenameModifier())
            return true;
    }
    return false;
}

void ShibokenGenerator::writeArgumentNames(QTextStream &s,
                                           const AbstractMetaFunction *func,
                                           Options options) const
{
    AbstractMetaArgumentList arguments = func->arguments();
    int argCount = 0;
    for (int j = 0, max = arguments.size(); j < max; j++) {

        if ((options & Generator::SkipRemovedArguments) &&
            (func->argumentRemoved(arguments.at(j)->argumentIndex() +1)))
            continue;

        if (argCount > 0)
            s << ", ";

        QString argName;

        if ((options & Generator::BoxedPrimitive) &&
            !arguments.at(j)->type()->isReference() &&
            (arguments.at(j)->type()->isQObject() ||
             arguments.at(j)->type()->isObject())) {
            //s << "brian::wrapper_manager::instance()->retrieve( " << arguments.at(j)->argumentName() << " )";
            // TODO: replace boost thing
            s << "python::ptr( " << arguments.at(j)->argumentName() << " )";
        } else {
            s << arguments.at(j)->argumentName();
        }
        argCount++;
    }
}

AbstractMetaFunctionList ShibokenGenerator::queryGlobalOperators(const AbstractMetaClass *metaClass)
{
    AbstractMetaFunctionList result;

    foreach (AbstractMetaFunction *func, metaClass->functions()) {
        if (func->isInGlobalScope() && func->isOperatorOverload())
            result.append(func);
    }
    return result;
}

AbstractMetaFunctionList ShibokenGenerator::queryFunctions(const AbstractMetaClass *metaClass, bool allFunctions)
{
    AbstractMetaFunctionList result;

    if (allFunctions) {
        int defaultFlags = AbstractMetaClass::NormalFunctions | AbstractMetaClass::Visible;
        defaultFlags |= metaClass->isInterface() ? 0 : AbstractMetaClass::ClassImplements;

        // Constructors
        result = metaClass->queryFunctions(AbstractMetaClass::Constructors
                                           | defaultFlags);

        // put enum constructor first to avoid conflict with int contructor
        result = sortConstructor(result);

        // Final functions
        result += metaClass->queryFunctions(AbstractMetaClass::FinalInTargetLangFunctions
                                            | AbstractMetaClass::NonStaticFunctions
                                            | defaultFlags);

        //virtual
        result += metaClass->queryFunctions(AbstractMetaClass::VirtualInTargetLangFunctions
                                            | AbstractMetaClass::NonStaticFunctions
                                            | defaultFlags);

        // Static functions
        result += metaClass->queryFunctions(AbstractMetaClass::StaticFunctions | defaultFlags);

        // Empty, private functions, since they aren't caught by the other ones
        result += metaClass->queryFunctions(AbstractMetaClass::Empty
                                            | AbstractMetaClass::Invisible
                                            | defaultFlags);
        // Signals
        result += metaClass->queryFunctions(AbstractMetaClass::Signals | defaultFlags);
    } else {
        result = metaClass->functionsInTargetLang();
    }

    return result;
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

AbstractMetaFunctionList ShibokenGenerator::filterFunctions(const AbstractMetaClass* metaClass)
{
    AbstractMetaFunctionList lst = queryFunctions(metaClass, true);
    foreach (AbstractMetaFunction *func, lst) {
        //skip signals
        if (func->isSignal()
            || func->isDestructor()
            || (func->isModifiedRemoved() && !func->isAbstract()))
            lst.removeOne(func);
    }

    //virtual not implemented in current class
    AbstractMetaFunctionList virtualLst = metaClass->queryFunctions(AbstractMetaClass::VirtualFunctions);
    foreach (AbstractMetaFunction* func, virtualLst) {
        if ((func->implementingClass() != metaClass) && !lst.contains(func))
            lst.append(func);
    }

    //append global operators
    lst += queryGlobalOperators(metaClass);

    return lst;
    //return metaClass->functions();
}

CodeSnipList ShibokenGenerator::getCodeSnips(const AbstractMetaFunction *func)
{
    CodeSnipList result;
    const AbstractMetaClass* metaClass = func->implementingClass();
    while (metaClass) {
        foreach (FunctionModification mod, func->modifications(metaClass)) {
            if (mod.isCodeInjection())
                result << mod.snips;
        }

        if (metaClass == metaClass->baseClass())
            break;
        metaClass = metaClass->baseClass();
    }

    return result;
}

void ShibokenGenerator::writeCodeSnips(QTextStream& s,
                                       const CodeSnipList& codeSnips,
                                       CodeSnip::Position position,
                                       TypeSystem::Language language,
                                       const AbstractMetaFunction* func)
{
    foreach (CodeSnip snip, codeSnips) {
        if ((snip.position != position) || !(snip.language & language))
            continue;

        QString code;
        QTextStream tmpStream(&code);
        Indentation indent1(INDENT);
        Indentation indent2(INDENT);
        formatCode(tmpStream, snip.code(), INDENT);

        if (func) {
            // replace template variable for return variable name
            code.replace("%0", retvalVariableName());

            // replace template variable for self Python object
            code.replace("%PYSELF", "self");

            // replace template variable for pointer to C++ this object
            if (func->implementingClass()) {
                code.replace("%CPPSELF.", "cppSelf->");
                code.replace("%CPPSELF", "cppSelf");
            }

            // replace template variables for individual arguments
            int removed = 0;
            for (int i = 0; i < func->arguments().size(); i++) {
                if (func->argumentRemoved(i+1))
                    removed++;
                code.replace("%" + QString::number(i+1), QString("cpp_arg%1").arg(i - removed));
            }

            // replace template variables for not removed arguments
            int i = 0;
            QString argumentNames;
            foreach (const AbstractMetaArgument* arg, func->arguments()) {
                if (func->argumentRemoved(arg->argumentIndex()+1))
                    continue;
                if (i > 0)
                    argumentNames += ", ";
                argumentNames += QString("cpp_arg%1").arg(i++);
            }
            code.replace("%ARGUMENT_NAMES", argumentNames);

            replaceTemplateVariables(code, func);
        }

        s << code;
    }
}

QStringList ShibokenGenerator::getBaseClasses(const AbstractMetaClass* metaClass)
{
    QStringList baseClass;

    if (!metaClass->baseClassName().isEmpty() &&
        (metaClass->name() != metaClass->baseClassName()))
        baseClass.append(metaClass->baseClassName());

    foreach (AbstractMetaClass* interface, metaClass->interfaces()) {
        AbstractMetaClass* aux = interface->primaryInterfaceImplementor();
        if (!aux)
            continue;

        //skip templates
        if (!aux->templateArguments().isEmpty())
            continue;

        if (!aux->name().isEmpty() && (metaClass->name() != aux->name()))
            baseClass.append(aux->name());
    }

    return baseClass;
}

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


bool ShibokenGenerator::doSetup(const QMap<QString, QString>& args)
{
    return true;
}

