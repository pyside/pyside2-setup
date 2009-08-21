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
        writeFunctionArguments(aux_stream, func, Generator::SkipDefaultValues | Generator::SkipRemovedArguments);
        code.replace("%ARGUMENTS", str);
    }
}

