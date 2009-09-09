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
    return metaClass->qualifiedCppName().toLower() + QLatin1String("_wrapper.h");
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

    // Header
    s << "#ifndef " << wrapperName.toUpper() << "_H" << endl;
    s << "#define " << wrapperName.toUpper() << "_H" << endl<< endl;

    if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        s << "// The mother of all C++ binding hacks!" << endl;
        s << "#define protected public" << endl << endl;
    }

    s << "#include <shiboken.h>" << endl << endl;

    //Includes
    if (metaClass->typeEntry()->include().isValid())
        s << metaClass->typeEntry()->include().toString() << endl << endl;

    writeCodeSnips(s, metaClass->typeEntry()->codeSnips(),
                   CodeSnip::Declaration, TypeSystem::NativeCode);

    if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        /*
         * BOTOWTI (Beast of The Old World to be Investigated)
        // detect the held type
        QString held_type = metaClass->typeEntry()->heldTypeValue();
        if (held_type.isEmpty() && createWrapper) {
            held_type = "qptr";
        }

        if (!held_type.isEmpty()) {
            s << "// held type forward decalration" << endl;
            s << "template<typename T> class " << held_type << ';' << endl;
        }
        */

        // Class
        s << "class SHIBOKEN_LOCAL " << wrapperName;
        s << " : public " << metaClass->qualifiedCppName();

        s << endl << '{' << endl << "public:" << endl;

        if (metaClass->hasCloneOperator())
            writeCopyCtor(s, metaClass);

        foreach (AbstractMetaFunction *func, filterFunctions(metaClass))
            writeFunction(s, func);

        //destructor
        s << INDENT << "~" << wrapperName << "();" << endl;

        if (metaClass->isQObject() && (metaClass->name() != "QObject"))
            s << INDENT << "using QObject::parent;" << endl;

        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(),
                       CodeSnip::PrototypeInitialization, TypeSystem::NativeCode);

        s << "};" << endl << endl;
    }

    s << "#endif // " << wrapperName.toUpper() << "_H" << endl << endl;
}

void HeaderGenerator::writeFunction(QTextStream& s, const AbstractMetaFunction* func) const
{
    // pure virtual functions need a default implementation
    if (func->isPrivate() || (func->isModifiedRemoved() && !func->isAbstract()))
        return;

    // do not write copy ctors here.
    if (func->isCopyConstructor())
        return;

    if (func->isConstructor() || func->isAbstract() || func->isVirtual()) {
        s << INDENT;
        if (func->isVirtual() || func->isAbstract())
            s << "virtual ";
        s << functionSignature(func) << ';' << endl;

        // TODO: when modified an abstract method ceases to be virtual but stays abstract
        //if (func->isModifiedRemoved() && func->isAbstract()) {
        //}

        // TODO: APIExtractor: strange that something that is abstract couldn't be considered virtual too.
        if (func->isVirtual() && !func->isAbstract() && !func->isConstructor() &&
            !func->ownerClass()->hasPrivateDestructor() &&
            func->implementingClass() == func->ownerClass()) {
            writeVirtualDispatcher(s, func);
        }
    }
}

void HeaderGenerator::writeVirtualDispatcher(QTextStream& s, const AbstractMetaFunction* func) const
{
    QString returnKeyword = func->type() ? QLatin1String("return ") : QString();
    s << INDENT << "static " << signatureForDefaultVirtualMethod(func, "", "_dispatcher") << " {" << endl;
    {
        Indentation indentation(INDENT);
        s << INDENT << returnKeyword;
        if (func->isModifiedRemoved() && func->isAbstract()) {
            if (func->type()
                && (func->type()->isObject()
                || func->type()->isQObject()
                || func->type()->name() == "void"))
                s << "0";
            else
                s << functionReturnType(func) << "()";
        } else {
            s << "self." << func->implementingClass()->qualifiedCppName() << "::";
            writeFunctionCall(s, func);
        }
        s << ';' << endl;
    }
    s << INDENT << '}' << endl;
}

void HeaderGenerator::writeTypeCheckMacro(QTextStream& s, const TypeEntry* type)
{
    QString pyTypeName = cpythonTypeName(type);
    QString checkFunction = cpythonCheckFunction(type);
    s << "PyAPI_DATA(PyTypeObject) " << pyTypeName << ';' << endl;
    s << "#define " << checkFunction << "(op) PyObject_TypeCheck(op, &";
    s << pyTypeName << ')' << endl;
    s << "#define " << checkFunction << "Exact(op) ((op)->ob_type == &";
    s << pyTypeName << ')' << endl;
}

void HeaderGenerator::writeTypeConverterDecl(QTextStream& s, const TypeEntry* type)
{
    QString cppName = type->name();
    if (type->isObject())
        cppName.append('*');

    s << "template<>" << endl;
    s << "struct Converter< " << cppName << " >" << endl << '{' << endl;

    s << INDENT << "static PyObject* toPython(ValueHolder< " << cppName << " > cppobj);" << endl;
    s << INDENT << "static " << cppName << " toCpp(PyObject* pyobj);" << endl;
    s << "};" << endl;
}

void HeaderGenerator::writeTypeConverterImpl(QTextStream& s, const TypeEntry* type)
{
    QString pyTypeName = cpythonTypeName(type);
    QString cppName = type->name();
    if (type->isObject())
        cppName.append('*');

    s << "inline PyObject* Converter< " << cppName << " >::toPython(ValueHolder< " << cppName << " > cppobj)" << endl;
    s << '{' << endl;
    s << INDENT << "PyObject* pyobj;" << endl;

    if (!type->isEnum()) {
        s << INDENT << "ValueHolder<void*> holder((void*) ";
        if (type->isValue())
            s << "new " << cppName << "(cppobj.value)";
        else
            s << "cppobj.value";
        s << ");" << endl;
    }

    s << INDENT << "pyobj = ";

    if (type->isEnum()) {
        s << "Shiboken::PyEnumObject_New(&" << pyTypeName << ',' << endl;
        s << INDENT << INDENT << "\"ReturnedValue\", (long) cppobj.value);" << endl;
    } else {
        QString newWrapper = QString("Shiboken::PyBaseWrapper_New(&")
                                + pyTypeName + ", &" + pyTypeName
                                + ", holder.value);";
        if (type->isValue()) {
            s << newWrapper << endl;
        } else {
            s << "Shiboken::Converter<void*>::toPython(holder);" << endl;
            s << INDENT << "if (!pyobj)" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "pyobj = " << newWrapper << endl;
            }
        }
    }

    s << INDENT << "return pyobj;" << endl;
    s << '}' << endl << endl;

    s << "inline " << cppName << " Converter< " << cppName << " >::toCpp(PyObject* pyobj)" << endl;
    s << '{' << endl;

    if (type->isValue()) {
        AbstractMetaFunctionList implicitConverters;
        if (type->isValue()) {
            const AbstractMetaClass* metaClass = classes().findClass(type->qualifiedCppName());
            if (metaClass)
                implicitConverters = metaClass->implicitConversions();
        }
        bool firstImplicitIf = true;
        foreach (const AbstractMetaFunction* ctor, implicitConverters) {
            if (ctor->isModifiedRemoved())
                continue;

            const AbstractMetaType* argType = ctor->arguments().first()->type();
            s << INDENT;
            if (firstImplicitIf)
                firstImplicitIf = false;
            else
                s << "else ";
            s << "if (" << cpythonCheckFunction(argType) << "(pyobj))" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "return " << cppName;
                s << "(Converter< " << argType->cppSignature() << " >::toCpp(pyobj));" << endl;
            }
        }
    }

    s << INDENT << "return ";
    if (type->isEnum()) {
        s << '(' << type->qualifiedCppName() << ") ((Shiboken::PyEnumObject*)pyobj)->ob_ival";
    } else {
        if (type->isValue())
            s << '*';
        s << "((" << cppName;
        if (type->isValue())
            s << '*';
        s << ") ((Shiboken::PyBaseWrapper*)pyobj)->cptr)";
    }
    s << ';' << endl;
    s << '}' << endl << endl;
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
    QString convertersImpl;
    QTextStream convDecl(&convertersDecl);
    QTextStream convImpl(&convertersImpl);

    Indentation indent(INDENT);

    s_pts << endl << "// Global enums" << endl;
    foreach (const AbstractMetaEnum* cppEnum, globalEnums()) {
        QString incFile = cppEnum->includeFile().split(QDir::separator()).takeLast();
        if (!incFile.isEmpty())
            enumIncludes << cppEnum->includeFile();
        writeTypeCheckMacro(s_pts, cppEnum->typeEntry());
        s_pts << endl;
        writeTypeConverterDecl(convDecl, cppEnum->typeEntry());
        writeTypeConverterImpl(convImpl, cppEnum->typeEntry());
        convDecl << endl;
    }

    foreach (AbstractMetaClass* metaClass, classes()) {
        const TypeEntry* classType = metaClass->typeEntry();
        if (!shouldGenerate(metaClass) || metaClass->enclosingClass() ||
            !(classType->isObject() || classType->isValue() || classType->isNamespace()))
            continue;

        if (m_packageName.isEmpty())
            m_packageName = metaClass->package();

        //Includes
        if (metaClass->typeEntry()->include().isValid())
            s_cin << metaClass->typeEntry()->include().toString() << endl;

        foreach (AbstractMetaClass* innerClass, metaClass->innerClasses()) {
            if (shouldGenerate(innerClass))
                s_cin << innerClass->typeEntry()->include().toString() << endl;
        }

        foreach (const AbstractMetaEnum* cppEnum, metaClass->enums()) {
            writeTypeCheckMacro(s_pts, cppEnum->typeEntry());
            s_pts << endl;
            writeTypeConverterDecl(convDecl, cppEnum->typeEntry());
            writeTypeConverterImpl(convImpl, cppEnum->typeEntry());
            convDecl << endl;
        }

        if (!metaClass->isNamespace()) {
            s_pts << "PyAPI_FUNC(PyObject*) " << cpythonBaseName(metaClass->typeEntry());
            s_pts << "_New(PyTypeObject* type, PyObject* args, PyObject* kwds);" << endl;
            writeTypeCheckMacro(s_pts, classType);
            s_pts << "#define Py" << metaClass->name() << "_cptr(pyobj) ((";
            s_pts << metaClass->name() << "*)PyBaseWrapper_cptr(pyobj))" << endl << endl;
            writeTypeConverterDecl(convDecl, classType);
            writeTypeConverterImpl(convImpl, classType);
            convDecl << endl;
        }
    }

    QString moduleHeaderFileName(outputDirectory() + QDir::separator()
                                 + subDirectoryForPackage(m_packageName));
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

        s << "// User defined converters --------------------------------------------" << endl;

        foreach (const PrimitiveTypeEntry* ptype, primitiveTypes()) {
            if (!ptype->codeSnips().isEmpty()) {
                foreach (CodeSnip snip, ptype->codeSnips())
                    s << snip.code();
            }
        }

        foreach (const ContainerTypeEntry* ctype, containerTypes()) {
            if (!ctype->codeSnips().isEmpty()) {
                foreach (CodeSnip snip, ctype->codeSnips())
                    s << snip.code();
            }
        }

        s << "// Generated converters -----------------------------------------------" << endl << endl;

        s << convertersDecl << endl;
        s << convertersImpl << endl;

        s << "} // namespace Shiboken" << endl << endl;

        s << "#endif // " << includeShield << endl << endl;
    }
}
