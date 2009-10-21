/*
 * This file is part of the Boost Python Generator project.
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

#include "boostpythongenerator.h"
#include <reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

#define NULL_VALUE "NULL"
#define COMMENT_LINE_WIDTH  77

static Indentor INDENT;
static void dump_function(AbstractMetaFunctionList lst);

QString BoostPythonGenerator::getWrapperName(const AbstractMetaClass* metaClass)
{
    QString result = metaClass->typeEntry()->qualifiedCppName().toLower();
    result.replace("::", "_");
    result += "_wrapper";
    return result;
}

QString BoostPythonGenerator::argumentString(const AbstractMetaFunction *cppFunction,
                                             const AbstractMetaArgument *cppArgument,
                                             Options options) const
{
    QString modifiedType = cppFunction->typeReplaced(cppArgument->argumentIndex() + 1);
    QString arg;

    if ((options & OriginalTypeDescription) || modifiedType.isEmpty())
        arg = translateType(cppArgument->type(), cppFunction->implementingClass(), options);
    else
        arg = modifiedType.replace('$', '.');

    if (!(options & Generator::SkipName)) {
        arg += " ";
        arg += cppArgument->argumentName();
    }

    QList<ReferenceCount> referenceCounts;
    referenceCounts = cppFunction->referenceCounts(cppFunction->implementingClass(), cppArgument->argumentIndex() + 1);
    if ((options & Generator::SkipDefaultValues) != Generator::SkipDefaultValues &&
        !cppArgument->defaultValueExpression().isEmpty()) {
        QString defaultValue = cppArgument->defaultValueExpression();
        if (defaultValue == "NULL")
            defaultValue = NULL_VALUE;

        //WORKAROUND: fix this please
        if (defaultValue.startsWith("new "))
            defaultValue.remove(0, 4);

        arg += " = " + defaultValue;
    }

    return arg;
}

void BoostPythonGenerator::writeArgument(QTextStream &s,
                                         const AbstractMetaFunction *func,
                                         const AbstractMetaArgument *cppArgument,
                                         Options options) const
{
    s << argumentString(func, cppArgument, options);
}

void BoostPythonGenerator::writeFunctionArguments(QTextStream &s,
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
        if ((options & Generator::SkipRemovedArguments) && func->argumentRemoved(i + 1))
            continue;

        if ((options & Generator::WriteSelf) || argUsed)
            s << ", ";

        writeArgument(s, func, arguments[i], options);
        argUsed++;
    }
}

QString BoostPythonGenerator::functionReturnType(const AbstractMetaFunction* func, Options options)
{
    QString modifiedReturnType = QString(func->typeReplaced(0));
    if (!modifiedReturnType.isNull() && (!(options & OriginalTypeDescription)))
        return modifiedReturnType;
    else
        return translateType(func->type(), func->implementingClass(), options);
}

QString BoostPythonGenerator::functionSignature(const AbstractMetaFunction *func,
                                                QString prepend,
                                                QString append,
                                                Options options,
                                                int argCount)
{
    AbstractMetaArgumentList arguments = func->arguments();
    int argument_count = argCount < 0 ? arguments.size() : argCount;


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
        name = getWrapperName(func->ownerClass());

    s << prepend << name << append << "(";
    writeFunctionArguments(s, func, options);
    s << ")";

    if (func->isConstant() && (!(options & Generator::ExcludeMethodConst)))
        s << " const";

    return result;
}

QString BoostPythonGenerator::signatureForDefaultVirtualMethod(const AbstractMetaFunction *cppFunction,
                                                               QString prepend,
                                                               QString append,
                                                               Options options,
                                                               int arg_count)
{
    QString defaultMethodSignature = functionSignature(cppFunction, prepend, append, options, arg_count);
    QString staticSelf("(");
    if (cppFunction->isConstant())
        staticSelf += "const ";

    staticSelf += cppFunction->ownerClass()->qualifiedCppName() + "& ";
    if (!(options & SkipName))
        staticSelf += " self";

    if (cppFunction->arguments().size() > 0)
        staticSelf += ", ";

    defaultMethodSignature.replace(defaultMethodSignature.lastIndexOf(") const"), 7, ")");
    defaultMethodSignature.replace(defaultMethodSignature.indexOf('('), 1, staticSelf);
    return defaultMethodSignature;
}

void BoostPythonGenerator::writeArgumentNames(QTextStream &s,
                                              const AbstractMetaFunction *func,
                                              Options options) const
{
    AbstractMetaArgumentList arguments = func->arguments();
    int argCount = 0;
    for (int j = 0, max = arguments.size(); j < max; j++) {

        if ((options & Generator::SkipRemovedArguments) &&
            (func->argumentRemoved(arguments.at(j)->argumentIndex() + 1))) {
            continue;
        }

        if (argCount > 0)
            s << ", ";

        QString argName = arguments.at(j)->argumentName();
        if (((options & Generator::VirtualCall) == 0) &&
                (!func->conversionRule(TypeSystem::NativeCode, arguments.at(j)->argumentIndex() + 1).isEmpty() ||
                 !func->conversionRule(TypeSystem::TargetLangCode, arguments.at(j)->argumentIndex() + 1).isEmpty())
           )
            argName += "_out";

        if ((options & Generator::BoxedPrimitive) &&
            !arguments.at(j)->type()->isReference() &&
            (arguments.at(j)->type()->isQObject() ||
             arguments.at(j)->type()->isObject())) {

            s << "PySide::ptr( " << argName << ")";
        } else {
            s << argName;
        }
        argCount++;
    }
}

void BoostPythonGenerator::writeFunctionCall(QTextStream &s,
                                             const AbstractMetaFunction* func,
                                             Options options)

{
    if (!(options & Generator::SkipName))
        s << (func->isConstructor() ? func->ownerClass()->qualifiedCppName() : func->originalName());

    s << '(';
    writeArgumentNames(s, func, options);
    s << ')';
}

void BoostPythonGenerator::writeCodeSnips(QTextStream &s,
                                          const CodeSnipList &codeSnips,
                                          CodeSnip::Position position,
                                          TypeSystem::Language language,
                                          const AbstractMetaFunction *func)
{
    Indentation indentation(INDENT);
    foreach (CodeSnip snip, codeSnips) {
        if ((snip.position != position) ||
            !(snip.language & language)) {
            continue;
        }

        QString code;
        QTextStream tmpStream(&code);
        formatCode(tmpStream, snip.code(), INDENT);

        if (func)
            replaceTemplateVariables(code, func);

        s << code << endl;
    }
}

bool BoostPythonGenerator::canCreateWrapperFor(const AbstractMetaClass* cppClass)
{
    return !cppClass->hasPrivateDestructor() && !cppClass->isNamespace();
}



QStringList BoostPythonGenerator::getBaseClasses(const AbstractMetaClass *cppClass)
{
    QStringList baseClass;

    if (!cppClass->baseClassName().isEmpty() &&
        (cppClass->name() != cppClass->baseClassName())) {
        baseClass.append(cppClass->baseClassName());
    }

    foreach (AbstractMetaClass *interface, cppClass->interfaces()) {
        AbstractMetaClass *aux = interface->primaryInterfaceImplementor();
        if (!aux)
            continue;

        //skip templates
        if (aux->templateArguments().size() > 0)
            continue;

        if (!aux->name().isEmpty() && (cppClass->qualifiedCppName() != aux->qualifiedCppName()))
            baseClass.append(aux->qualifiedCppName());
    }

    return baseClass;
}


bool BoostPythonGenerator::isCopyable(const AbstractMetaClass *cppClass)
{
    if (cppClass->isNamespace())
        return false;
    else if (cppClass->typeEntry()->copyable() == ComplexTypeEntry::Unknown)
        return cppClass->hasCloneOperator();
    else
        return (cppClass->typeEntry()->copyable() == ComplexTypeEntry::CopyableSet);

    return false;
}

static void dump_function(AbstractMetaFunctionList lst)
{
    qDebug() << "DUMP FUNCTIONS: ";
    foreach (AbstractMetaFunction *func, lst) {
        qDebug() << "*" << func->ownerClass()->name()
             << func->signature()
             << "Private: " << func->isPrivate()
             << "Empty: " << func->isEmptyFunction()
             << "Static:" << func->isStatic()
             << "Signal:" << func->isSignal()
             << "ClassImplements: " << (func->ownerClass() != func->implementingClass())
             << "is operator:" << func->isOperatorOverload()
             << "is global:" << func->isInGlobalScope();
    }
}


bool BoostPythonGenerator::doSetup(const QMap<QString, QString>&)
{
    return true;
}
