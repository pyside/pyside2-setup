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

struct Generator::GeneratorPrivate {
    const ApiExtractor* apiextractor;
    QString outDir;
    // License comment
    QString licenseComment;
    QString packageName;
    int numGenerated;
    int numGeneratedWritten;
};

Generator::Generator() : m_d(new GeneratorPrivate)
{
    m_d->numGenerated = 0;
    m_d->numGeneratedWritten = 0;
}

Generator::~Generator()
{
    delete m_d;
}

bool Generator::setup(const ApiExtractor& extractor, const QMap< QString, QString > args)
{
    m_d->apiextractor = &extractor;
    // FIXME: Avoid this ugly hack to get the package name.. and... why the name "package"!?
    foreach (const AbstractMetaClass* cppClass, m_d->apiextractor->classes()) {
        if (m_d->packageName.isEmpty()
            && cppClass->typeEntry()->generateCode()
            && !cppClass->package().isEmpty()) {
            m_d->packageName = cppClass->package();
            break;
        }
    }
    return doSetup(args);
}

QMap< QString, QString > Generator::options() const
{
    return QMap<QString, QString>();
}

AbstractMetaClassList Generator::classes() const
{
    return m_d->apiextractor->classes();
}

AbstractMetaFunctionList Generator::globalFunctions() const
{
    return m_d->apiextractor->globalFunctions();
}

AbstractMetaEnumList Generator::globalEnums() const
{
    return m_d->apiextractor->globalEnums();
}

QList<const PrimitiveTypeEntry*> Generator::primitiveTypes() const
{
    return m_d->apiextractor->primitiveTypes();
}

QList<const ContainerTypeEntry*> Generator::containerTypes() const
{
    return m_d->apiextractor->containerTypes();
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const EnumTypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const TypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const FlagsTypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const AbstractMetaType* metaType) const
{
    return m_d->apiextractor->findAbstractMetaEnum(metaType);
}

QSet< QString > Generator::qtMetaTypeDeclaredTypeNames() const
{
    return m_d->apiextractor->qtMetaTypeDeclaredTypeNames();
}

QString Generator::licenseComment() const
{
    return m_d->licenseComment;
}

void Generator::setLicenseComment(const QString& licenseComment)
{
    m_d->licenseComment = licenseComment;
}

QString Generator::packageName() const
{
    return m_d->packageName;
}

QString Generator::moduleName() const
{
    QString& pkgName = m_d->packageName;
    return QString(pkgName).remove(0, pkgName.lastIndexOf('.') + 1);
}

QString Generator::outputDirectory() const
{
    return m_d->outDir;
}

void Generator::setOutputDirectory(const QString &outDir)
{
    m_d->outDir = outDir;
}

int Generator::numGenerated() const
{
    return m_d->numGenerated;
}

int Generator::numGeneratedAndWritten() const
{
    return m_d->numGeneratedWritten;
}

void Generator::generate()
{
    foreach (AbstractMetaClass *cls, m_d->apiextractor->classes()) {
        if (!shouldGenerate(cls))
            continue;

        QString fileName = fileNameForClass(cls);
        if (fileName.isNull())
            continue;
        ReportHandler::debugSparse(QString("generating: %1").arg(fileName));

        FileOut fileOut(outputDirectory() + '/' + subDirectoryForClass(cls) + '/' + fileName);
        generateClass(fileOut.stream, cls);

        if (fileOut.done())
            ++m_d->numGeneratedWritten;
        ++m_d->numGenerated;
    }
    finishGeneration();
}

bool Generator::shouldGenerate(const AbstractMetaClass* metaClass) const
{
    return metaClass->typeEntry()->codeGeneration() & TypeEntry::GenerateTargetLang;
}

void verifyDirectoryFor(const QFile &file)
{
    QDir dir = QFileInfo(file).dir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath()))
            ReportHandler::warning(QString("unable to create directory '%1'")
                                   .arg(dir.absolutePath()));
    }
}

void Generator::replaceTemplateVariables(QString &code, const AbstractMetaFunction *func)
{
    const AbstractMetaClass *cpp_class = func->ownerClass();
    if (cpp_class)
        code.replace("%TYPE", cpp_class->name());

    foreach (AbstractMetaArgument *arg, func->arguments())
        code.replace("%" + QString::number(arg->argumentIndex() + 1), arg->name());

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

QTextStream& formatCode(QTextStream &s, const QString& code, Indentor &indentor)
{
    // detect number of spaces before the first character
    QStringList lst(code.split("\n"));
    QRegExp nonSpaceRegex("[^\\s]");
    int spacesToRemove = 0;
    foreach(QString line, lst) {
        if (!line.trimmed().isEmpty()) {
            spacesToRemove = line.indexOf(nonSpaceRegex);
            if (spacesToRemove == -1)
                spacesToRemove = 0;
            break;
        }
    }

    foreach(QString line, lst) {
        while (line.end()->isSpace())
            line.chop(1);
        int limit = 0;
        for(int i = 0; i < spacesToRemove; ++i) {
            if (!line[i].isSpace())
                break;
            limit++;
        }
        s << indentor << line.remove(0, limit) << endl;
    }
    return s;
}

AbstractMetaFunctionList Generator::implicitConversions(const TypeEntry* type) const
{
    if (type->isValue()) {
        const AbstractMetaClass* metaClass = classes().findClass(type);
        if (metaClass)
            return metaClass->implicitConversions();
    }
    return AbstractMetaFunctionList();
}

AbstractMetaFunctionList Generator::implicitConversions(const AbstractMetaType* metaType) const
{
    return implicitConversions(metaType->typeEntry());
}

QString Generator::translateType(const AbstractMetaType *cType,
                                 const AbstractMetaClass *context,
                                 Options options) const
{
    QString s;
    static int constLen = strlen("const");

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
    } else if (cType->typeEntry()->isPrimitive()) {
        s = cType->cppSignature();
        if ((options & Generator::ExcludeConst) && cType->isConstant())
            s.remove(0, 5);
        if ((options & Generator::ExcludeReference) && cType->isReference())
            s.chop(1);
    } else {
        if (options & Generator::OriginalName) {
            s = cType->originalTypeDescription().trimmed();
            if ((options & Generator::ExcludeReference) && s.endsWith("&"))
                s = s.left(s.size()-1);

            // remove only the last const (avoid remove template const)
            if (options & Generator::ExcludeConst) {
                int index = s.lastIndexOf("const");

                if (index >= (s.size() - (constLen + 1))) // (VarType const)  or (VarType const[*|&])
                    s = s.remove(index, constLen);
            }
        } else {
            AbstractMetaType *copyType = cType->copy();

            if (options & Generator::ExcludeConst)
                copyType->setConstant(false);

            if (options & Generator::ExcludeReference)
                copyType->setReference(false);

            s = copyType->cppSignature();
            delete copyType;
        }
    }

    return s;
}


QString Generator::subDirectoryForClass(const AbstractMetaClass* clazz) const
{
    return subDirectoryForPackage(clazz->package());
}

QString Generator::subDirectoryForPackage(QString packageName) const
{
    if (packageName.isEmpty())
        packageName = m_d->packageName;
    return QString(packageName).replace(".", QDir::separator());
}

