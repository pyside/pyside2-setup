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

static QString formattedCodeHelper(QTextStream &s, Indentor &indentor, QStringList &lines)
{
    bool multilineComment = false;
    bool lastEmpty = true;
    QString lastLine;
    while (!lines.isEmpty()) {
        const QString line = lines.takeFirst().trimmed();
        if (line.isEmpty()) {
            if (!lastEmpty)
                s << endl;
            lastEmpty = true;
            continue;
        } else
            lastEmpty = false;

        if (line.startsWith("/*"))
            multilineComment = true;

        if (multilineComment) {
            s << indentor;
            if (line.startsWith("*"))
                s << " ";
            s << line << endl;
            if (line.endsWith("*/"))
                multilineComment = false;
        } else if (line.startsWith("}"))
            return line;
        else if (line.endsWith("")) {
            s << indentor << line << endl;
            return 0;
        } else if (line.endsWith("{")) {
            s << indentor << line << endl;
            QString tmp;
            {
                Indentation indent(indentor);
                tmp = formattedCodeHelper(s, indentor, lines);
            }
            if (!tmp.isNull())
                s << indentor << tmp << endl;

            lastLine = tmp;
            continue;
        } else {
            s << indentor;
            if (!lastLine.isEmpty() &&
                !lastLine.endsWith(";") &&
                !line.startsWith("@") &&
                !line.startsWith("//") &&
                !lastLine.startsWith("//") &&
                !lastLine.endsWith("}") &&
                !line.startsWith("{"))
                s << "    ";
            s << line << endl;
        }
        lastLine = line;
    }
    return 0;
}

QTextStream& formatCode(QTextStream &s, const QString& code, Indentor &indentor)
{
    QStringList lst(code.split("\n"));
    while (!lst.isEmpty()) {
        QString tmp = formattedCodeHelper(s, indentor, lst);
        if (!tmp.isNull())
            s << indentor << tmp << endl;

    }
    s.flush();
    return s;
}

FunctionModificationList BoostPythonGenerator::functionModifications(const AbstractMetaFunction *metaFunction)
{
    FunctionModificationList mods;
    const AbstractMetaClass *cls = metaFunction->implementingClass();
    while (cls) {
        mods += metaFunction->modifications(cls);

        if (cls == cls->baseClass())
            break;
        cls = cls->baseClass();
    }
    return mods;
}

QString BoostPythonGenerator::translateType(const AbstractMetaType *cType,
                                            const AbstractMetaClass *context,
                                            int option) const
{
    QString s;

    if (context && cType &&
        context->typeEntry()->isGenericClass() &&
        cType->originalTemplateType()) {
            qDebug() << "set original templateType" << cType->name();
            cType = cType->originalTemplateType();
    }

    if (!cType) {
        s = "void";
    } else if (cType->isArray()) {
        s = translateType(cType->arrayElementType(), context) + "[]";
    } else if (cType->isEnum() || cType->isFlags()) {
        if (option & Generator::EnumAsInts)
            s = "int";
        else
            s = cType->cppSignature();
#if 0
    } else if (c_type->isContainer()) {
        qDebug() << "is container" << c_type->cppSignature();
        s = c_type->name();
        if (!(option & SkipTemplateParameters)) {
            s += " < ";
            QList<AbstractMetaType *> args = c_type->instantiations();
            for (int i = 0; i < args.size(); ++i) {
                if (i)
                    s += ", ";
                qDebug() << "container type: " << args.at(i)->cppSignature() << " / " << args.at(i)->instantiations().count();
                s += translateType(args.at(i), context, option);
            }
            s += " > ";
        }
#endif
    } else {
        s = cType->cppSignature();
        if (cType->isConstant() && (option & Generator::ExcludeConst))
            s.replace("const", "");
        if (cType->isReference() && (option & Generator::ExcludeReference))
            s.replace("&", "");
    }

    return s;
}

QString BoostPythonGenerator::getWrapperName(const AbstractMetaClass* clazz)
{
    QString result = clazz->name().toLower();
    result.replace("::", "_");
    result += "_wrapper";
    return result;
}

QString BoostPythonGenerator::argumentString(const AbstractMetaFunction *cppFunction,
                                             const AbstractMetaArgument *cppArgument,
                                             uint options) const
{
    QString modifiedType = cppFunction->typeReplaced(cppArgument->argumentIndex() + 1);
    QString arg;

    if (modifiedType.isEmpty())
        arg = translateType(cppArgument->type(), cppFunction->implementingClass(), (Generator::Option) options);
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
                                         uint options) const
{
    s << argumentString(func, cppArgument, options);
}

void BoostPythonGenerator::writeFunctionArguments(QTextStream &s,
                                                  const AbstractMetaFunction *func,
                                                  uint options) const
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

QString BoostPythonGenerator::functionReturnType(const AbstractMetaFunction* func, int option)
{
    QString modifiedReturnType = QString(func->typeReplaced(0));
    if (!modifiedReturnType.isNull() && (!(option & OriginalTypeDescription)))
        return modifiedReturnType;
    else
        return translateType(func->type(), func->implementingClass(), option);
}

QString BoostPythonGenerator::functionSignature(const AbstractMetaFunction *func,
                                                QString prepend,
                                                QString append,
                                                int option,
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
        option = Option(option | Generator::SkipReturnType);
    } else {
        s << functionReturnType(func, option) << ' ';
    }

    // name
    QString name(func->originalName());
    if (func->isConstructor())
        name = getWrapperName(func->ownerClass());

    s << prepend << name << append << "(";
    writeFunctionArguments(s, func, option);
    s << ")";

    if (func->isConstant() && (!(option & Generator::ExcludeMethodConst)))
        s << " const";

    return result;
}

QString BoostPythonGenerator::signatureForDefaultVirtualMethod(const AbstractMetaFunction *cppFunction,
                                                               QString prepend,
                                                               QString append,
                                                               int option,
                                                               int arg_count)
{
    QString defaultMethodSignature = functionSignature(cppFunction, prepend, append, option, arg_count);
    QString staticSelf("(");
    if (cppFunction->isConstant())
        staticSelf += "const ";

    staticSelf += cppFunction->ownerClass()->qualifiedCppName() + "& ";
    if (!(option & SkipName))
        staticSelf += " self";

    if (cppFunction->arguments().size() > 0)
        staticSelf += ", ";

    defaultMethodSignature.replace(defaultMethodSignature.lastIndexOf(") const"), 7, ")");
    defaultMethodSignature.replace(defaultMethodSignature.indexOf('('), 1, staticSelf);
    return defaultMethodSignature;
}

void BoostPythonGenerator::writeArgumentNames(QTextStream &s,
                                              const AbstractMetaFunction *func,
                                              uint options) const
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

        if ((options & Generator::BoxedPrimitive) &&
            !arguments.at(j)->type()->isReference() &&
            (arguments.at(j)->type()->isQObject() ||
             arguments.at(j)->type()->isObject())) {
            s << "PySide::ptr( " << arguments.at(j)->argumentName() << ")";
        } else {
            s << arguments.at(j)->argumentName();
        }
        argCount++;
    }
}

AbstractMetaFunctionList BoostPythonGenerator::queryGlobalOperators(const AbstractMetaClass *cppClass)
{
    AbstractMetaFunctionList result;

    foreach (AbstractMetaFunction *func, cppClass->functions()) {
        if (func->isInGlobalScope() && func->isOperatorOverload())
            result.append(func);
    }
    return result;
}

AbstractMetaFunctionList BoostPythonGenerator::sortContructor(AbstractMetaFunctionList list)
{
    AbstractMetaFunctionList result;

    foreach (AbstractMetaFunction *func, list) {
        bool inserted = false;
        foreach (AbstractMetaArgument *arg, func->arguments()) {
            if (arg->type()->isFlags() || arg->type()->isEnum()) {
                result.push_back(func);
                inserted = true;
                break;
            }
        }
        if (!inserted)
            result.push_front(func);
    }

    return result;
}

AbstractMetaFunctionList BoostPythonGenerator::queryFunctions(const AbstractMetaClass *cppClass, bool allFunctions)
{
    AbstractMetaFunctionList result;

    if (allFunctions) {
        int default_flags = AbstractMetaClass::NormalFunctions |  AbstractMetaClass::Visible;
        default_flags |= cppClass->isInterface() ? 0 :  AbstractMetaClass::ClassImplements;

        // Constructors
        result = cppClass->queryFunctions(AbstractMetaClass::Constructors |
                                           default_flags);

        // put enum constructor first to avoid conflict with int contructor
        result = sortContructor(result);

        // Final functions
        result += cppClass->queryFunctions(AbstractMetaClass::FinalInTargetLangFunctions |
                                            AbstractMetaClass::NonStaticFunctions |
                                            default_flags);

        //virtual
        result += cppClass->queryFunctions(AbstractMetaClass::VirtualInTargetLangFunctions |
                                            AbstractMetaClass::NonStaticFunctions |
                                            default_flags);

        // Static functions
        result += cppClass->queryFunctions(AbstractMetaClass::StaticFunctions | default_flags);

        // Empty, private functions, since they aren't caught by the other ones
        result += cppClass->queryFunctions(AbstractMetaClass::Empty |
                                            AbstractMetaClass::Invisible | default_flags);
        // Signals
        result += cppClass->queryFunctions(AbstractMetaClass::Signals | default_flags);
    } else {
        result = cppClass->functionsInTargetLang();
    }

    return result;
}

void BoostPythonGenerator::writeFunctionCall(QTextStream &s,
                                             const AbstractMetaFunction* func,
                                             uint options)

{
    if (!(options & Generator::SkipName))
        s << (func->isConstructor() ? func->ownerClass()->qualifiedCppName() : func->originalName());

    s << '(';
    writeArgumentNames(s, func, options);
    s << ')';
}

AbstractMetaFunctionList BoostPythonGenerator::filterFunctions(const AbstractMetaClass *cppClass)
{
    AbstractMetaFunctionList lst = queryFunctions(cppClass, true);
    foreach (AbstractMetaFunction *func, lst) {
        //skip signals
        if (func->isSignal() ||
            func->isDestructor() ||
            (func->isModifiedRemoved() && !func->isAbstract())) {
            lst.removeOne(func);
        }
    }

    //virtual not implemented in current class
    AbstractMetaFunctionList virtual_lst = cppClass->queryFunctions(AbstractMetaClass::VirtualFunctions);
    foreach (AbstractMetaFunction *func, virtual_lst) {
        if ((func->implementingClass() != cppClass) &&
            !lst.contains(func)) {
            lst.append(func);
        }
    }

    //append global operators
    foreach (AbstractMetaFunction *func , queryGlobalOperators(cppClass)) {
        if (!lst.contains(func))
            lst.append(func);
    }

    return lst;
    //return cpp_class->functions();
}

CodeSnipList BoostPythonGenerator::getCodeSnips(const AbstractMetaFunction *func)
{
    CodeSnipList result;
    const AbstractMetaClass *cppClass = func->implementingClass();
    while (cppClass) {
        foreach (FunctionModification mod, func->modifications(cppClass)) {
            if (mod.isCodeInjection())
                result << mod.snips;
        }

        if (cppClass == cppClass->baseClass())
            break;
        cppClass = cppClass->baseClass();
    }

    return result;
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
    if (cppClass->typeEntry()->copyable() == ComplexTypeEntry::Unknown)
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
