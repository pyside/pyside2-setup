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
#include "hppgenerator.h"
#include <apiextractor/reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

static Indentor INDENT;

QString HppGenerator::fileNameForClass(const AbstractMetaClass *cppClass) const
{
    return getWrapperName(cppClass) + QLatin1String(".hpp");
}

void HppGenerator::writeCopyCtor(QTextStream &s, const AbstractMetaClass *cppClass)
{
    s << INDENT <<  getWrapperName(cppClass) << "(PyObject *py_self, const " << cppClass->qualifiedCppName() << "& self)"
      << " :  " << cppClass->qualifiedCppName() << "(self), wrapper(py_self)" << endl
      << INDENT << "{" << endl
      << INDENT << "}" << endl;
}

void HppGenerator::generateClass(QTextStream &s, const AbstractMetaClass *cppClass)
{
    ReportHandler::debugSparse("Generating header for " + cppClass->fullName());
    Indentation indent(INDENT);

    // write license comment
    s << licenseComment() << endl;

    QString wrapperName = HppGenerator::getWrapperName(cppClass);
    // Header
    s << "#ifndef __" << wrapperName.toUpper() << "__" << endl;
    s << "#define __" << wrapperName.toUpper() << "__" << endl << endl;

    s << "#include <pyside.hpp>" << endl;
    //Includes
    if (cppClass->typeEntry()->include().isValid())
        s << cppClass->typeEntry()->include().toString() << endl << endl;

    s << "using namespace PySide;" << endl << endl;

    if (!cppClass->isPolymorphic() || cppClass->hasPrivateDestructor() || cppClass->isNamespace())
        s << "namespace " << wrapperName << " {" << endl << endl;

    bool needWriteBackReference = false;
    if (cppClass->isNamespace()) {
        s << INDENT << "struct Namespace {};" << endl;
    } else {
        QString className;
        bool create_wrapper = canCreateWrapperFor(cppClass);
        bool is_wrapper = false;
        // detect the held type
        QString held_type = cppClass->typeEntry()->heldTypeValue();
        if (held_type.isEmpty() && create_wrapper)
            held_type = "qptr";

        writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                       CodeSnip::Declaration, TypeSystem::NativeCode);

        if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor()) {
            /*
            if (!held_type.isEmpty()) {
                s << "// held type forward decalration" << endl;
                s << "template<typename T> class " << held_type << ';' << endl;
            }
            */

            // Class
            s << "class PYSIDE_LOCAL " << wrapperName;
            if (create_wrapper) {
                s << " : public " << cppClass->qualifiedCppName() << ", public PySide::wrapper";
            }
            s << endl;
            s << "{" << endl;
        }

        writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                       CodeSnip::Declaration, TypeSystem::ShellDeclaration);

        if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor()) {
            s << endl << "private:" << endl;
            className = wrapperName;
            is_wrapper = true;
        } else {
            className = cppClass->qualifiedCppName();
        }

        // print the huge boost::python::class_ typedef
        s << INDENT << "typedef boost::python::class_< " << cppClass->qualifiedCppName();

        writeBaseClass(s, cppClass);

        if (!held_type.isEmpty())
            s << ", PySide::" << held_type << " < " << className << ", qptr_base::no_check_cache | qptr_base::"
              << ( is_wrapper ? "wrapper_pointer" : "no_wrapper_pointer") << "> ";

        if (!isCopyable(cppClass))
            s << ", boost::noncopyable";

        s << " > class_type;" << endl;

        if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor()) {
            s << "public:" << endl;

            if (isCopyable(cppClass))
                writeCopyCtor(s, cppClass);

            foreach (AbstractMetaFunction *func, filterFunctions(cppClass))
                writeFunction(s, func);

            if (create_wrapper) {
                //destructor
                s << INDENT << "~" << wrapperName << "();" << endl;

                if (cppClass->isQObject() && (cppClass->name() != "QObject"))
                    s << INDENT << "using QObject::parent;" << endl;
            }
        }

        writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                       CodeSnip::End, TypeSystem::ShellDeclaration);
    }

    QString staticKeyword = cppClass->isNamespace() ? QLatin1String("") : QLatin1String("static ");
    s << INDENT;
    if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor()) {
        s << "//static member used to export class" << endl;
        s << INDENT << staticKeyword;
    }
    s << "void define_python_class() throw();" << endl << endl;

    writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                   CodeSnip::PrototypeInitialization, TypeSystem::NativeCode);


    s << "};" << endl << endl;

    s << "#endif // __" << wrapperName.toUpper() << "__" << endl << endl;
}

void HppGenerator::writeFunction(QTextStream &s, const AbstractMetaFunction* func)
{
    // pure virtual functions need a default implementation
    if ((func->isPrivate() && !func->isConstructor()) || (func->isModifiedRemoved() && !func->isAbstract()))
        return;

    // do not write copy ctors here.
    if (func->isCopyConstructor())
        return;

    if (func->isConstructor() || func->isAbstract() || func->isVirtual()) {
        if (func->isVirtual() && !func->isAbstract() && !func->isConstructor()
            && !func->ownerClass()->hasPrivateDestructor()
            && func->implementingClass() == func->ownerClass()) {
            s << INDENT << "static " << signatureForDefaultVirtualMethod(func, "", "_default", Generator::SkipName) << ';' << endl;
        }

        if (func->isConstructor()) {
            s << INDENT << getWrapperName(func->ownerClass()) << "(PyObject *py_self" << (func->arguments().size() ? "," : "");
            writeFunctionArguments(s, func, Generator::OriginalTypeDescription |  Generator::SkipName);
            s << ")";
        } else {
            s << INDENT << functionSignature(func, "", "", Generator::OriginalTypeDescription |  Generator::SkipName);
        }

        if (func->isModifiedRemoved() && func->isAbstract())
            writeDefaultImplementation(s, func);
        else
            s << ';' << endl;
    }
}

void HppGenerator::writeDefaultImplementation(QTextStream& s, const AbstractMetaFunction* func)
{
    QString returnValue;
    if (func->type()) {
        if (func->type()->isObject() || func->type()->isQObject() || func->type()->name() == "void")
            returnValue = "0";
        else
            returnValue = functionReturnType(func) + "()";
    }
    s << " { return " << returnValue << "; }" << endl;
}

void HppGenerator::writeBaseClass(QTextStream& s, const AbstractMetaClass* cppClass)
{
    if (!cppClass->isNamespace() && !cppClass->isInterface()) {
        QStringList baseClass = getBaseClasses(cppClass);

        if (baseClass.isEmpty()) {
            const ComplexTypeEntry *type = cppClass->typeEntry();
            if (cppClass->name() != type->defaultSuperclass()) {
                QString sc = type->defaultSuperclass();
                if (!sc.isEmpty())
                    s << ", python::bases< " << sc << "> ";
            }
        } else {
            s << ", boost::python::bases< " << baseClass.join(", ") << " > ";
        }
    }
}

