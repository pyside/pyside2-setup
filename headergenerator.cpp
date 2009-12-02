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

#include "headergenerator.h"
#include <apiextractor/reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

static Indentor INDENT;

QString HeaderGenerator::fileNameForClass(const AbstractMetaClass* metaClass) const
{
    return metaClass->qualifiedCppName().toLower().replace("::", "_") + QLatin1String("_wrapper.h");
}

void HeaderGenerator::writeCopyCtor(QTextStream& s, const AbstractMetaClass* metaClass) const
{
    s << INDENT <<  wrapperName(metaClass) << "(const " << metaClass->qualifiedCppName() << "& self)";
    s << " : " << metaClass->qualifiedCppName() << "(self)" << endl;
    s << INDENT << "{" << endl;
    s << INDENT << "}" << endl << endl;
}

void HeaderGenerator::generateClass(QTextStream& s, const AbstractMetaClass* metaClass)
{
    ReportHandler::debugSparse("Generating header for " + metaClass->fullName());
    Indentation indent(INDENT);

    // write license comment
    s << licenseComment();

    QString wrapperName = HeaderGenerator::wrapperName(metaClass);
    QString headerGuard = wrapperName.replace("::", "_").toUpper();

    // Header
    s << "#ifndef SBK_" << headerGuard << "_H" << endl;
    s << "#define SBK_" << headerGuard << "_H" << endl<< endl;

#ifndef AVOID_PROTECTED_HACK
    s << "#define protected public" << endl << endl;
#endif

    s << "#include <shiboken.h>" << endl << endl;

    //Includes
    if (metaClass->typeEntry()->include().isValid())
        s << metaClass->typeEntry()->include().toString() << endl << endl;

    if (shouldGenerateCppWrapper(metaClass)) {
        // Class
        s << "class " << wrapperName;
        s << " : public " << metaClass->qualifiedCppName();

        s << endl << '{' << endl << "public:" << endl;

        if (metaClass->hasCloneOperator())
            writeCopyCtor(s, metaClass);

        foreach (AbstractMetaFunction *func, filterFunctions(metaClass))
            writeFunction(s, func);

        //destructor
        s << INDENT << (metaClass->hasVirtualDestructor() ? "virtual " : "") << "~" << wrapperName << "();" << endl;

        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Declaration, TypeSystem::NativeCode);

        s << "};" << endl << endl;
    }

    s << "#endif // SBK_" << headerGuard << "_H" << endl << endl;
}

void HeaderGenerator::writeFunction(QTextStream& s, const AbstractMetaFunction* func) const
{
    // do not write copy ctors here.
    if (func->isCopyConstructor())
        return;

#ifdef AVOID_PROTECTED_HACK
    if (func->isProtected() && !func->isConstructor()) {
        s << INDENT << "inline " << (func->isStatic() ? "static " : "");
        s << functionSignature(func, "", "_protected") << " { ";
        s << (func->type() ? "return " : "") << func->ownerClass()->qualifiedCppName() << "::";
        writeFunctionCall(s, func);
        s << "; }" << endl;
    }
#endif

    // pure virtual functions need a default implementation
    if (func->isPrivate() || (func->isModifiedRemoved() && !func->isAbstract()))
        return;

    if (func->isConstructor() || func->isAbstract() || func->isVirtual()) {
        s << INDENT;
        if (func->isVirtual() || func->isAbstract())
            s << "virtual ";
        s << functionSignature(func) << ';' << endl;

        // TODO: when modified an abstract method ceases to be virtual but stays abstract
        //if (func->isModifiedRemoved() && func->isAbstract()) {
        //}
    }
}

void HeaderGenerator::writeTypeCheckMacro(QTextStream& s, const TypeEntry* type)
{
    QString pyTypeName = cpythonTypeName(type);
    QString checkFunction = cpythonCheckFunction(type);
    s << "PyAPI_DATA(";
    if (type->isObject() || type->isValue())
        s << "Shiboken::ShiboTypeObject";
    else
        s << "PyTypeObject";
    s << ") " << pyTypeName << ';' << endl;
    s << "#define " << checkFunction << "(op) PyObject_TypeCheck(op, (PyTypeObject*)&";
    s << pyTypeName << ')' << endl;
    s << "#define " << checkFunction << "Exact(op) ((op)->ob_type == (PyTypeObject*)&";
    s << pyTypeName << ')' << endl;
}

void HeaderGenerator::writeTypeConverterDecl(QTextStream& s, const TypeEntry* type)
{
    s << "template<>" << endl;

    const AbstractMetaClass* metaClass = classes().findClass(type->name());
    bool isAbstractOrObjectType = (metaClass &&  metaClass->isAbstract()) || type->isObject();

    s << "struct Converter<" << type->name() << (isAbstractOrObjectType ? "*" : "") << " > : ";
    if (type->isEnum() || type->isFlags())
        s << "Converter_CppEnum";
    else
        s << "ConverterBase";
    s << '<' << type->name() << (isAbstractOrObjectType ? "*" : "") << " >" << endl;
    s << '{' << endl;

    bool hasImplicitConversions = !implicitConversions(type).isEmpty();

    if (hasImplicitConversions)
        s << INDENT << "static bool isConvertible(PyObject* pyobj);" << endl;

    s << INDENT << "static PyObject* createWrapper(";
    QString convArg = type->name();
    if (!type->isEnum() && !type->isFlags()) {
        convArg.prepend("const ");
        convArg.append('*');
    }
    s << convArg << " cppobj);" << endl;

    if (type->isValue() && hasImplicitConversions) {
        s << INDENT << "static " << type->name() << "* copyCppObject(const ";
        s << type->name() << "& cppobj);" << endl;
    }

    if (type->isValue()) {
        s << INDENT << "static PyObject* toPython(const " << type->name() << "& cppobj);" << endl;
        if (hasImplicitConversions)
            s << INDENT << "static " << type->name() << " toCpp(PyObject* pyobj);" << endl;
    }
    s << "};" << endl;
}

void HeaderGenerator::finishGeneration()
{
    // Generate the main header for this module.
    // This header should be included by binding modules
    // extendind on top of this one.
    QString classIncludes;
    QTextStream s_cin(&classIncludes);
    QSet<QString> enumIncludes;
    QString pythonTypeStuff;
    QTextStream s_pts(&pythonTypeStuff);
    QString convertersDecl;
    QTextStream convDecl(&convertersDecl);

    Indentation indent(INDENT);

    s_pts << endl << "// Global enums" << endl;
    foreach (const AbstractMetaEnum* cppEnum, globalEnums()) {
        QString incFile = cppEnum->includeFile().split(QDir::separator()).takeLast();
        if (!incFile.isEmpty())
            enumIncludes << cppEnum->includeFile();
        writeTypeCheckMacro(s_pts, cppEnum->typeEntry());
        FlagsTypeEntry* flags = cppEnum->typeEntry()->flags();
        if (flags)
            writeTypeCheckMacro(s_pts, flags);
        s_pts << endl;
        writeTypeConverterDecl(convDecl, cppEnum->typeEntry());
        convDecl << endl;
    }

    foreach (AbstractMetaClass* metaClass, classes()) {
        const TypeEntry* classType = metaClass->typeEntry();
        if (!shouldGenerate(metaClass) || metaClass->enclosingClass() ||
            !(classType->isObject() || classType->isValue() || classType->isNamespace()))
            continue;

        //Includes
        if (metaClass->typeEntry()->include().isValid())
            s_cin << metaClass->typeEntry()->include().toString() << endl;

        foreach (const AbstractMetaEnum* cppEnum, metaClass->enums()) {
            writeTypeCheckMacro(s_pts, cppEnum->typeEntry());
            writeTypeConverterDecl(convDecl, cppEnum->typeEntry());
            FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
            if (flagsEntry) {
                writeTypeCheckMacro(s_pts, flagsEntry);
                writeTypeConverterDecl(convDecl, flagsEntry);
            }
            s_pts << endl;
            convDecl << endl;
        }

        if (!metaClass->isNamespace()) {
            foreach (AbstractMetaClass* innerClass, metaClass->innerClasses()) {
                if (shouldGenerate(innerClass)) {
                    s_cin << innerClass->typeEntry()->include().toString() << endl;
                    s_pts << "PyAPI_FUNC(PyObject*) " << cpythonBaseName(innerClass->typeEntry());
                    s_pts << "_New(PyTypeObject* type, PyObject* args, PyObject* kwds);" << endl;
                    writeTypeCheckMacro(s_pts, innerClass->typeEntry());
                    writeTypeConverterDecl(convDecl, innerClass->typeEntry());
                    convDecl << endl;
                }
            }
            s_pts << "PyAPI_FUNC(PyObject*) " << cpythonBaseName(metaClass->typeEntry());
            s_pts << "_New(PyTypeObject* type, PyObject* args, PyObject* kwds);" << endl;
            writeTypeCheckMacro(s_pts, classType);
            s_pts << "#define Py" << metaClass->name() << "_cptr(pyobj) ((";
            s_pts << metaClass->name() << "*)PyBaseWrapper_cptr(pyobj))" << endl << endl;
            writeTypeConverterDecl(convDecl, classType);
            convDecl << endl;
        }
    }

    QString moduleHeaderFileName(outputDirectory() + QDir::separator()
                                 + subDirectoryForPackage(packageName()));
    moduleHeaderFileName += QDir::separator() + moduleName().toLower() + "_python.h";

    QString includeShield = moduleName().toUpper() + "_PYTHON_H";

    QFile file(moduleHeaderFileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);

        // write license comment
        s << licenseComment() << endl << endl;

        s << "#ifndef " << includeShield << endl;
        s << "#define " << includeShield << endl<< endl;

        s << "#include <Python.h>" << endl;
        s << "#include <conversions.h>" << endl;
        s << "#include <pyenum.h>" << endl;
        s << "#include <basewrapper.h>" << endl;
        s << "#include <bindingmanager.h>" << endl << endl;

        s << "#include <memory>" << endl << endl;
        writeExportMacros(s);

        s << "// Class Includes" << endl;
        s << classIncludes << endl;

        if (!enumIncludes.isEmpty()) {
            s << "// Enum Includes" << endl;
            foreach (const QString& include, enumIncludes)
              s << "#include <" << include << ">" << endl;
            s << endl;
        }

        if (!primitiveTypes().isEmpty()) {
            s << "// Conversion Includes - Primitive Types" << endl;
            foreach (const PrimitiveTypeEntry* ptype, primitiveTypes()) {
                if (ptype->include().isValid())
                    s << ptype->include().toString() << endl;
            }
            s << endl;
        }

        if (!containerTypes().isEmpty()) {
            s << "// Conversion Includes - Container Types" << endl;
            foreach (const ContainerTypeEntry* ctype, containerTypes()) {
                if (ctype->include().isValid())
                    s << ctype->include().toString() << endl;
            }
            s << endl;
        }

        s << "extern \"C\"" << endl << '{' << endl << endl;
        s << pythonTypeStuff << endl;
        s << "} // extern \"C\"" << endl << endl;

        s << "namespace Shiboken" << endl << '{' << endl << endl;

        s << "// Generated converters declarations ----------------------------------" << endl << endl;
        s << convertersDecl << endl;

        s << "// User defined converters --------------------------------------------" << endl;
        foreach (TypeEntry* typeEntry, TypeDatabase::instance()->entries()) {
            if (typeEntry->hasConversionRule()) {
                s << "// Conversion rule for: " << typeEntry->name() << endl;
                s << typeEntry->conversionRule();
            }
        }

        s << "} // namespace Shiboken" << endl << endl;

        s << "#endif // " << includeShield << endl << endl;
    }
}


void HeaderGenerator::writeExportMacros(QTextStream& s)
{
    QString macro = getApiExportMacro();
    s << "\
#if defined _WIN32 || defined __CYGWIN__\n\
    #define " << macro << " __declspec(dllexport)\n\
#else\n\
#if __GNUC__ >= 4\n\
    #define " << macro << " __attribute__ ((visibility(\"default\")))\n\
#else\n\
    #define " << macro << "\n\
#endif\n\
#endif\n\
\n";
}

