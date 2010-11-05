/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <typedatabase.h>
#include <reporthandler.h>
#include <fileout.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

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

void HeaderGenerator::writeProtectedFieldAccessors(QTextStream& s, const AbstractMetaField* field) const
{
    QString fieldType = field->type()->cppSignature();
    QString fieldName = field->enclosingClass()->qualifiedCppName() + "::" + field->name();

    s << INDENT << "inline " << fieldType << ' ' << protectedFieldGetterName(field) << "()";
    s << " { return " << fieldName << "; }" << endl;
    s << INDENT << "inline void " << protectedFieldSetterName(field) << '(' << fieldType << " value)";
    s << " { " << fieldName << " = value; }" << endl;
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
    s << metaClass->typeEntry()->include() << endl;

    if (shouldGenerateCppWrapper(metaClass)) {

        if (usePySideExtensions() && metaClass->isQObject())
            s << "namespace PySide { class DynamicQMetaObject; }\n\n";

        // Class
        s << "class " << wrapperName;
        s << " : public " << metaClass->qualifiedCppName();

        s << endl << '{' << endl << "public:" << endl;

        bool hasVirtualFunction = false;
        foreach (AbstractMetaFunction *func, filterFunctions(metaClass)) {
            if (func->isVirtual())
                hasVirtualFunction = true;
            writeFunction(s, func);
        }

#ifdef AVOID_PROTECTED_HACK
        if (metaClass->hasProtectedFields()) {
            foreach (AbstractMetaField* field, metaClass->fields()) {
                if (!field->isProtected())
                    continue;
                writeProtectedFieldAccessors(s, field);
            }
        }
#endif

        //destructor
#ifdef AVOID_PROTECTED_HACK
        if (!metaClass->hasPrivateDestructor())
#endif
            s << INDENT << (metaClass->hasVirtualDestructor() || hasVirtualFunction ? "virtual " : "") << "~" << wrapperName << "();" << endl;

        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Declaration, TypeSystem::NativeCode);

#ifdef AVOID_PROTECTED_HACK
        if (!metaClass->hasPrivateDestructor()) {
#endif

        if (usePySideExtensions() && metaClass->isQObject()) {
            s << "public:\n";
            s << INDENT << "virtual int qt_metacall(QMetaObject::Call call, int id, void** args);\n";
            s << "private:\n";
            s << INDENT << "mutable PySide::DynamicQMetaObject* m_metaObject;\n";
        }

#ifdef AVOID_PROTECTED_HACK
        }
#endif

        s << "};" << endl << endl;
    }

    s << "#endif // SBK_" << headerGuard << "_H" << endl << endl;
}

void HeaderGenerator::writeFunction(QTextStream& s, const AbstractMetaFunction* func) const
{

    // do not write copy ctors here.
    if (!func->isPrivate() && func->isCopyConstructor()) {
        writeCopyCtor(s, func->ownerClass());
        return;
    }
    if (func->isUserAdded())
        return;

#ifdef AVOID_PROTECTED_HACK
    if (func->isProtected() && !func->isConstructor() && !func->isOperatorOverload()) {
        s << INDENT << "inline " << (func->isStatic() ? "static " : "");
        s << functionSignature(func, "", "_protected", Generator::EnumAsInts|Generator::OriginalTypeDescription) << " { ";
        s << (func->type() ? "return " : "");
        if (!func->isAbstract())
            s << func->ownerClass()->qualifiedCppName() << "::";
        s << func->originalName() << '(';
        QStringList args;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            QString argName = arg->name();
            const TypeEntry* enumTypeEntry = 0;
            if (arg->type()->isFlags())
                enumTypeEntry = reinterpret_cast<const FlagsTypeEntry*>(arg->type()->typeEntry())->originator();
            else if (arg->type()->isEnum())
                enumTypeEntry = arg->type()->typeEntry();
            if (enumTypeEntry)
                argName = QString("%1(%2)").arg(arg->type()->cppSignature()).arg(argName);
            args << argName;
        }
        s << args.join(", ") << ')';
        s << "; }" << endl;
    }
#endif

    // pure virtual functions need a default implementation
    if ((func->isPrivate() && !visibilityModifiedToPrivate(func))
        || (func->isModifiedRemoved() && !func->isAbstract()))
        return;

#ifdef AVOID_PROTECTED_HACK
    if (func->ownerClass()->hasPrivateDestructor() && (func->isAbstract() || func->isVirtual()))
        return;
#endif

    if (func->isConstructor() || func->isAbstract() || func->isVirtual()) {
        s << INDENT;
        Options virtualOption = Generator::OriginalTypeDescription;

        if (func->isVirtual() || func->isAbstract())
            s << "virtual ";
        else if (!func->hasSignatureModifications())
            virtualOption = Generator::NoOption;

        s << functionSignature(func, "", "", virtualOption) << ';' << endl;

        // TODO: when modified an abstract method ceases to be virtual but stays abstract
        //if (func->isModifiedRemoved() && func->isAbstract()) {
        //}
    }
}

void HeaderGenerator::writeTypeConverterDecl(QTextStream& s, const TypeEntry* type)
{
    s << "template<>" << endl;

    const AbstractMetaClass* metaClass = classes().findClass(type->name());
    bool isAbstractOrObjectType = (metaClass &&  metaClass->isAbstract()) || type->isObject();

    AbstractMetaFunctionList implicitConvs;
    foreach (AbstractMetaFunction* func, implicitConversions(type)) {
        if (!func->isUserAdded())
            implicitConvs << func;
    }
    bool isValueTypeWithImplConversions = type->isValue() && !implicitConvs.isEmpty();
    bool hasCustomConversion = type->hasNativeConversionRule();
    QString typeT = type->name() + (isAbstractOrObjectType ? "*" : "");
    QString typeName = type->name();

#ifdef AVOID_PROTECTED_HACK
    const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(type);
    if (metaEnum && metaEnum->isProtected()) {
        typeT = protectedEnumSurrogateName(metaEnum);
        typeName = typeT;
    }
#endif

    s << "struct Converter<" << typeT << " >";
    if (!hasCustomConversion) {
        if (type->isEnum())
            s << " : EnumConverter";
        else if (type->isFlags())
            s << " : QFlagsConverter";
        else if (isAbstractOrObjectType)
            s << " : ObjectTypeConverter";
        else
            s << " : ValueTypeConverter";
        s << '<' << typeName << " >";
    }
    s << endl << '{' << endl;
    if (isValueTypeWithImplConversions || hasCustomConversion) {
        s << INDENT << "static " << type->name() << " toCpp(PyObject* pyobj);" << endl;
        s << INDENT << "static bool isConvertible(PyObject* pyobj);" << endl;
        if (hasCustomConversion) {
            s << INDENT << "static bool checkType(PyObject* pyobj);" << endl;
            s << INDENT << "static inline PyObject* toPython(void* cppObj) { return toPython(*reinterpret_cast<"
              << type->name() << (isAbstractOrObjectType ? "" : "*") << " >(cppObj)); }" << endl;
            s << INDENT << "static PyObject* toPython(const " << type->name() << "& cppObj);" << endl;
        }
    }
    s << "};" << endl;

    // write value-type like converter to object-types
    if (isAbstractOrObjectType) {
        s << endl << "template<>" << endl;
        s << "struct Converter<" << type->name() << "& > : ObjectTypeReferenceConverter<" << type->name() << " >" << endl << '{' << endl;
        s << "};" << endl;
    }
}

void HeaderGenerator::writeTypeIndexDefineLine(QTextStream& s, const TypeEntry* typeEntry, int& idx)
{
    if (!typeEntry || !typeEntry->generateCode())
        return;
    s.setFieldAlignment(QTextStream::AlignLeft);
    s << "#define ";
    s.setFieldWidth(60);
    s << getTypeIndexVariableName(typeEntry);
    s.setFieldWidth(0);
    s << ' ' << (idx++) << endl;
    if (typeEntry->isEnum()) {
        const EnumTypeEntry* ete = reinterpret_cast<const EnumTypeEntry*>(typeEntry);
        if (ete->flags())
            writeTypeIndexDefineLine(s, ete->flags(), idx);
    }
}

void HeaderGenerator::writeTypeIndexDefine(QTextStream& s, const AbstractMetaClass* metaClass, int& idx)
{
    if (!metaClass->typeEntry()->generateCode())
        return;
    writeTypeIndexDefineLine(s, metaClass->typeEntry(), idx);
    foreach (const AbstractMetaEnum* metaEnum, metaClass->enums()) {
        if (metaEnum->isPrivate())
            continue;
        writeTypeIndexDefineLine(s, metaEnum->typeEntry(), idx);
    }
}

void HeaderGenerator::finishGeneration()
{
    if (classes().isEmpty())
        return;

    // Generate the main header for this module.
    // This header should be included by binding modules
    // extendind on top of this one.
    QSet<Include> includes;
    QString macros;
    QTextStream macrosStream(&macros);
    QString convertersDecl;
    QTextStream convDecl(&convertersDecl);
    QString sbkTypeFunctions;
    QTextStream typeFunctions(&sbkTypeFunctions);
    QString converterImpl;
    QTextStream convImpl(&converterImpl);

    Indentation indent(INDENT);

    macrosStream << "// Type indices" << endl;
    int idx = 0;
    foreach (const AbstractMetaClass* metaClass, classes())
        writeTypeIndexDefine(macrosStream, metaClass, idx);
    foreach (const AbstractMetaEnum* metaEnum, globalEnums())
        writeTypeIndexDefineLine(macrosStream, metaEnum->typeEntry(), idx);
    macrosStream << "#define ";
    macrosStream.setFieldWidth(60);
    macrosStream << "SBK_"+moduleName()+"_IDX_COUNT";
    macrosStream.setFieldWidth(0);
    macrosStream << ' ' << idx << endl << endl;
    macrosStream << "// This variable stores all python types exported by this module" << endl;
    macrosStream << "extern PyTypeObject** " << cppApiVariableName() << ';' << endl << endl;

    macrosStream << "// Macros for type check" << endl;
    foreach (const AbstractMetaEnum* cppEnum, globalEnums()) {
        if (cppEnum->isAnonymous() || cppEnum->isPrivate())
            continue;
        includes << cppEnum->typeEntry()->include();
        writeTypeConverterDecl(convDecl, cppEnum->typeEntry());
        convDecl << endl;
        writeSbkTypeFunction(typeFunctions, cppEnum);
    }

    foreach (AbstractMetaClass* metaClass, classes()) {
        if (!shouldGenerate(metaClass))
            continue;

        //Includes
        const TypeEntry* classType = metaClass->typeEntry();
        includes << classType->include();

        foreach (const AbstractMetaEnum* cppEnum, metaClass->enums()) {
            if (cppEnum->isAnonymous() || cppEnum->isPrivate())
                continue;
            EnumTypeEntry* enumType = cppEnum->typeEntry();
            includes << enumType->include();
            writeTypeConverterDecl(convDecl, enumType);
            FlagsTypeEntry* flagsEntry = enumType->flags();
            if (flagsEntry)
                writeTypeConverterDecl(convDecl, flagsEntry);
            convDecl << endl;
            writeSbkTypeFunction(typeFunctions, cppEnum);
        }

        if (!metaClass->isNamespace()) {
            writeSbkTypeFunction(typeFunctions, metaClass);
            writeSbkCopyCppObjectFunction(convDecl, metaClass);
            writeTypeConverterDecl(convDecl, classType);
            writeTypeConverterImpl(convImpl, classType);
            convDecl << endl;
        }
    }

    QString moduleHeaderFileName(outputDirectory()
                                 + QDir::separator() + subDirectoryForPackage(packageName())
                                 + QDir::separator() + getModuleHeaderFileName());

    QString includeShield("SBK_" + moduleName().toUpper() + "_PYTHON_H");

    FileOut file(moduleHeaderFileName);
    QTextStream& s = file.stream;
    // write license comment
    s << licenseComment() << endl << endl;

    s << "#ifndef " << includeShield << endl;
    s << "#define " << includeShield << endl<< endl;
    #ifndef AVOID_PROTECTED_HACK
    s << "//workaround to access protected functions" << endl;
    s << "#define protected public" << endl << endl;
    #endif

    s << "#include <Python.h>" << endl;
    s << "#include <conversions.h>" << endl;
    s << "#include <sbkenum.h>" << endl;
    s << "#include <basewrapper.h>" << endl;
    s << "#include <bindingmanager.h>" << endl;
    s << "#include <memory>" << endl << endl;
    if (usePySideExtensions())
        s << "#include <pysidesignal.h>" << endl;

    QStringList requiredTargetImports = TypeDatabase::instance()->requiredTargetImports();
    if (!requiredTargetImports.isEmpty()) {
        s << "// Module Includes" << endl;
        foreach (const QString& requiredModule, requiredTargetImports)
            s << "#include <" << getModuleHeaderFileName(requiredModule) << ">" << endl;
        s << endl;
    }

    s << "// Binded library includes" << endl;
    foreach (const Include& include, includes)
        s << include;

    if (!primitiveTypes().isEmpty()) {
        s << "// Conversion Includes - Primitive Types" << endl;
        foreach (const PrimitiveTypeEntry* ptype, primitiveTypes())
            s << ptype->include();
        s << endl;
    }

    if (!containerTypes().isEmpty()) {
        s << "// Conversion Includes - Container Types" << endl;
        foreach (const ContainerTypeEntry* ctype, containerTypes())
            s << ctype->include();
        s << endl;
    }

    s << macros << endl;

    s << "namespace Shiboken" << endl << '{' << endl << endl;

    s << "// PyType functions, to get the PyObjectType for a type T\n";
    s << sbkTypeFunctions << endl;

    if (usePySideExtensions()) {
        foreach (AbstractMetaClass* metaClass, classes()) {
            if (!metaClass->isQObject() || !metaClass->typeEntry()->generateCode())
                continue;

            s << "template<>" << endl;
            s << "inline PyObject* createWrapper<" << metaClass->qualifiedCppName() << " >(const ";
            s << metaClass->qualifiedCppName() << "* cppobj, bool hasOwnership, bool isExactType)" << endl;
            s << '{' << endl;
            s << INDENT << "const char* typeName = 0;" << endl;
            s << INDENT << metaClass->qualifiedCppName() << "* value = const_cast<" << metaClass->qualifiedCppName() << "* >(cppobj);" << endl;
            s << INDENT << "if (!isExactType)" << endl;
            s << INDENT << INDENT << "typeName = typeid(*value).name();" << endl;
            s << INDENT << "PyObject* pyObj = Shiboken::SbkBaseWrapper_New(reinterpret_cast<SbkBaseWrapperType*>(SbkType<" << metaClass->qualifiedCppName() << " >()),"
            << "value, hasOwnership, isExactType, typeName);" << endl;
            s << INDENT << "PySide::Signal::updateSourceObject(pyObj);" << endl;
            s << INDENT << "return pyObj;" << endl;
            s << '}' << endl;

        }
    }

    s << "// Generated converters declarations ----------------------------------" << endl << endl;
    s << convertersDecl;
    s << "} // namespace Shiboken" << endl << endl;

    s << "// User defined converters --------------------------------------------" << endl;
    foreach (TypeEntry* typeEntry, TypeDatabase::instance()->entries()) {
        if (typeEntry->hasNativeConversionRule()) {
            s << "// Conversion rule for: " << typeEntry->name() << endl;
            s << typeEntry->conversionRule();
        }
    }
    s << "// Generated converters implemantations -------------------------------" << endl << endl;
    s << converterImpl << endl;

    s << "#endif // " << includeShield << endl << endl;
}

void HeaderGenerator::writeSbkTypeFunction(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString enumName = cppEnum->name();
    if (cppEnum->enclosingClass())
        enumName = cppEnum->enclosingClass()->qualifiedCppName() + "::" + enumName;
#ifdef AVOID_PROTECTED_HACK
    if (cppEnum->isProtected()) {
        enumName = protectedEnumSurrogateName(cppEnum);
        s << "enum " << enumName << " {};" << endl;
    }
#endif

    s << "template<> inline PyTypeObject* SbkType<" << enumName << " >() ";
    s << "{ return " << cpythonTypeNameExt(cppEnum->typeEntry()) << "; }\n";

    FlagsTypeEntry* flag = cppEnum->typeEntry()->flags();
    if (flag) {
        s <<  "template<> inline PyTypeObject* SbkType<" << flag->name() << " >() "
          << "{ return " << cpythonTypeNameExt(flag) << "; }\n";
    }
}

void HeaderGenerator::writeSbkTypeFunction(QTextStream& s, const AbstractMetaClass* cppClass)
{
    s <<  "template<> inline PyTypeObject* SbkType<" << cppClass->qualifiedCppName() << " >() "
      <<  "{ return reinterpret_cast<PyTypeObject*>(" << cpythonTypeNameExt(cppClass->typeEntry()) << "); }\n";
}

void HeaderGenerator::writeSbkCopyCppObjectFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    if (!metaClass->typeEntry()->isValue() || !shouldGenerateCppWrapper(metaClass))
        return;
    QString className = metaClass->qualifiedCppName();
    s << "template <>" << endl;
    s << "struct SbkTypeInfo<" << className << " >" << endl;
    s << '{' << endl;
    s << INDENT << "static const bool isCppWrapper = true;" << endl;
    s << "};" << endl;
}

void HeaderGenerator::writeTypeConverterImpl(QTextStream& s, const TypeEntry* type)
{
    if (type->hasNativeConversionRule())
        return;

    QString pyTypeName = cpythonTypeName(type);

    AbstractMetaFunctionList implicitConvs;
    foreach (AbstractMetaFunction* func, implicitConversions(type)) {
        if (!func->isUserAdded())
            implicitConvs << func;
    }

    bool hasImplicitConversions = !implicitConvs.isEmpty();

    // A specialized Converter<T>::toCpp method is only need for
    // classes with implicit conversions.
    if (!hasImplicitConversions)
        return;

    // Write Converter<T>::isConvertible
    s << "inline bool Shiboken::Converter<" << type->name() << " >::isConvertible(PyObject* pyobj)" << endl;
    s << '{' << endl;

    if (type->isValue()) {
        s << INDENT << "if (ValueTypeConverter<" << type->name() << " >::isConvertible(pyobj))" << endl;
        Indentation indent(INDENT);
        s << INDENT << "return true;" << endl;
    }


    s << INDENT << "SbkBaseWrapperType* shiboType = reinterpret_cast<SbkBaseWrapperType*>(SbkType<";
    s << type->name() << " >());" << endl;
    s << INDENT << "return ";
    bool isFirst = true;
    foreach (const AbstractMetaFunction* ctor, implicitConvs) {
        Indentation indent(INDENT);
        if (isFirst)
            isFirst = false;
        else
            s << endl << INDENT << " || ";
        if (ctor->isConversionOperator())
            s << cpythonCheckFunction(ctor->ownerClass()->typeEntry());
        else
            s << cpythonCheckFunction(ctor->arguments().first()->type());
        s << "(pyobj)";
    }
    s << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << " || (shiboType->ext_isconvertible && shiboType->ext_isconvertible(pyobj));" << endl;
    }
    s << '}' << endl << endl;

    // Write Converter<T>::toCpp function
    s << "inline " << type->name() << " Shiboken::Converter<" << type->name() << " >::toCpp(PyObject* pyobj)" << endl;
    s << '{' << endl;

    s << INDENT << "if (!Shiboken_TypeCheck(pyobj, " << type->name() << ")) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "SbkBaseWrapperType* shiboType = reinterpret_cast<SbkBaseWrapperType*>(SbkType<";
        s << type->name() << " >());" << endl;
    }
    bool firstImplicitIf = true;
    foreach (const AbstractMetaFunction* ctor, implicitConvs) {
        if (ctor->isModifiedRemoved())
            continue;

        Indentation indent(INDENT);
        s << INDENT;
        if (firstImplicitIf)
            firstImplicitIf = false;
        else
            s << "else ";

        QString typeCheck;
        QString toCppConv;
        QTextStream tcc(&toCppConv);
        if (ctor->isConversionOperator()) {
            const AbstractMetaClass* metaClass = ctor->ownerClass();
            typeCheck = cpythonCheckFunction(metaClass->typeEntry());
            writeToCppConversion(tcc, metaClass, "pyobj");
        } else {
            const AbstractMetaType* argType = ctor->arguments().first()->type();
            typeCheck = cpythonCheckFunction(argType);
            writeToCppConversion(tcc, argType, 0, "pyobj");
        }

        s << "if (" << typeCheck << "(pyobj))" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return " << type->name() << '(' << toCppConv << ");" << endl;
        }
    }

    {
        Indentation indent(INDENT);
        s << INDENT << "else if (shiboType->ext_isconvertible && shiboType->ext_tocpp && shiboType->ext_isconvertible(pyobj)) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << type->name() << "* cptr = reinterpret_cast<" << type->name() << "*>(shiboType->ext_tocpp(pyobj));" << endl;
            s << INDENT << "std::auto_ptr<" << type->name() << " > cptr_auto_ptr(cptr);" << endl;
            s << INDENT << "return *cptr;" << endl;
        }
        s << INDENT << '}' << endl;
    }

    s << INDENT << '}' << endl;

    s << INDENT << "return *" << cpythonWrapperCPtr(type, "pyobj") << ';' << endl;
    s << '}' << endl << endl;
}

