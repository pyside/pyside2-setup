/*
 * This file is part of the API Extractor project.
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

#include "generator.h"
#include "reporthandler.h"
#include "fileout.h"
#include "apiextractor.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QDebug>

Generator::Generator() : m_numGenerated(0), m_numGeneratedWritten(0)
{}

Generator::~Generator()
{
}

bool Generator::setup(const ApiExtractor& extractor, const QMap< QString, QString > args)
{
    m_globalEnums = extractor.globalEnums();
    m_globalFunctions = extractor.globalFunctions();
    m_classes = extractor.classes();
    m_primitiveTypes = extractor.primitiveTypes();
    m_containerTypes = extractor.containerTypes();

    // FIXME: Avoid this ugly hack to get the package name.. and... why the name "package"!?
    foreach (const AbstractMetaClass* cppClass, m_classes) {
        if (m_packageName.isEmpty()
            && cppClass->typeEntry()->generateCode()
            && !cppClass->package().isEmpty()) {
            m_packageName = cppClass->package();
            break;
        }
    }
    // does anyone use this?
    m_qmetatypeDeclaredTypenames = extractor.qtMetaTypeDeclaredTypeNames();
    return doSetup(args);
}

QMap< QString, QString > Generator::options() const
{
    return QMap<QString, QString>();
}

AbstractMetaClassList Generator::classes() const
{
    return m_classes;
}

AbstractMetaFunctionList Generator::globalFunctions() const
{
    return m_globalFunctions;
}

AbstractMetaEnumList Generator::globalEnums() const
{
    return m_globalEnums;
}

QList<const PrimitiveTypeEntry*> Generator::primitiveTypes() const
{
    return m_primitiveTypes;
}

QList<const ContainerTypeEntry*> Generator::containerTypes() const
{
    return m_containerTypes;
}

/// Returns the output directory
QString Generator::outputDirectory() const
{
    return m_outDir;
}

/// Set the output directory
void Generator::setOutputDirectory(const QString &outDir)
{
    m_outDir = outDir;
}

void Generator::generate()
{
    foreach (AbstractMetaClass *cls, m_classes) {
        if (!shouldGenerate(cls))
            continue;

        QString fileName = fileNameForClass(cls);
        if (fileName.isNull())
            continue;
        ReportHandler::debugSparse(QString("generating: %1").arg(fileName));

        FileOut fileOut(outputDirectory() + '/' + subDirectoryForClass(cls) + '/' + fileName);
        generateClass(fileOut.stream, cls);

        if (fileOut.done())
            ++m_numGeneratedWritten;
        ++m_numGenerated;
    }
    finishGeneration();
}

bool Generator::shouldGenerate(const AbstractMetaClass* metaClass) const
{
    return metaClass->typeEntry()->codeGeneration() & TypeEntry::GenerateTargetLang;
}

void Generator::verifyDirectoryFor(const QFile &file)
{
    QDir dir = QFileInfo(file).dir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath()))
            ReportHandler::warning(QString("unable to create directory '%1'")
                                   .arg(dir.absolutePath()));
    }
}

bool Generator::hasDefaultConstructor(const AbstractMetaType *type)
{
    QString full_name = type->typeEntry()->qualifiedTargetLangName();
    QString class_name = type->typeEntry()->targetLangName();

    foreach (const AbstractMetaClass *cls, m_classes) {
        if (cls->typeEntry()->qualifiedTargetLangName() == full_name) {
            AbstractMetaFunctionList functions = cls->functions();
            foreach (const AbstractMetaFunction *function, functions) {
                if (function->arguments().isEmpty() && function->name() == class_name)
                    return true;
            }
            return false;
        }
    }
    return false;
}

void Generator::replaceTemplateVariables(QString &code, const AbstractMetaFunction *func)
{
    const AbstractMetaClass *cpp_class = func->ownerClass();
    code.replace("%TYPE", cpp_class->name());

    foreach (AbstractMetaArgument *arg, func->arguments())
        code.replace("%" + QString::number(arg->argumentIndex() + 1), arg->argumentName());

    //template values
    code.replace("%RETURN_TYPE", translateType(func->type(), cpp_class));
    code.replace("%FUNCTION_NAME", func->originalName());

    if (code.contains("%ARGUMENT_NAMES")) {
        QString str;
        QTextStream aux_stream(&str);
        writeArgumentNames(aux_stream, func, Generator::SkipRemovedArguments);
        code.replace("%ARGUMENT_NAMES", str);
    }

    if (code.contains("%ARGUMENTS")) {
        QString str;
        QTextStream aux_stream(&str);
        writeFunctionArguments(aux_stream, func, Options(SkipDefaultValues) | SkipRemovedArguments);
        code.replace("%ARGUMENTS", str);
    }
}

AbstractMetaFunctionList Generator::queryFunctions(const AbstractMetaClass *cppClass, bool allFunctions)
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

AbstractMetaFunctionList Generator::filterFunctions(const AbstractMetaClass *cppClass)
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

AbstractMetaFunctionList Generator::queryGlobalOperators(const AbstractMetaClass *cppClass)
{
    AbstractMetaFunctionList result;

    foreach (AbstractMetaFunction *func, cppClass->functions()) {
        if (func->isInGlobalScope() && func->isOperatorOverload())
            result.append(func);
    }
    return result;
}

AbstractMetaFunctionList Generator::sortContructor(AbstractMetaFunctionList list)
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

FunctionModificationList Generator::functionModifications(const AbstractMetaFunction *metaFunction)
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

CodeSnipList Generator::getCodeSnips(const AbstractMetaFunction *func)
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

QString Generator::translateType(const AbstractMetaType *cType,
                                 const AbstractMetaClass *context,
                                 Options options) const
{
    QString s;

    if (context && cType &&
        context->typeEntry()->isGenericClass() &&
        cType->originalTemplateType()) {
        cType = cType->originalTemplateType();
    }

    if (!cType) {
        s = "void";
    } else if (cType->isArray()) {
        s = translateType(cType->arrayElementType(), context, options) + "[]";
    } else if (cType->isEnum() || cType->isFlags()) {
        if (options & Generator::EnumAsInts)
            s = "int";
        else
            s = cType->cppSignature();
    } else {
        s = cType->cppSignature();
        if (cType->isConstant() && (options & Generator::ExcludeConst))
            s.replace("const", "");
        if (cType->isReference() && (options & Generator::ExcludeReference))
            s.replace("&", "");
    }

    return s;
}
