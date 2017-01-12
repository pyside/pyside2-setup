/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "headergenerator.h"
#include <abstractmetalang.h>
#include <typedatabase.h>
#include <reporthandler.h>
#include <fileout.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

QString HeaderGenerator::fileNamePrefix() const
{
    return QLatin1String("_wrapper.h");
}

QString HeaderGenerator::fileNameForContext(GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    if (!context.forSmartPointer()) {
        QString fileNameBase = metaClass->qualifiedCppName().toLower();
        fileNameBase.replace(QLatin1String("::"), QLatin1String("_"));
        return fileNameBase + fileNamePrefix();
    } else {
        const AbstractMetaType *smartPointerType = context.preciseType();
        QString fileNameBase = getFileNameBaseForSmartPointer(smartPointerType, metaClass);
        return fileNameBase + fileNamePrefix();
    }
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
    AbstractMetaType *metaType = field->type();
    QString fieldType = metaType->cppSignature();
    QString fieldName = field->enclosingClass()->qualifiedCppName() + QLatin1String("::") + field->name();

    // Force use of pointer to return internal variable memory
    bool useReference = (!metaType->isConstant() &&
                         !metaType->isEnum() &&
                         !metaType->isPrimitive() &&
                         metaType->indirections() == 0);


    // Get function
    s << INDENT << "inline " << fieldType
      << (useReference ? '*' : ' ')
      << ' ' << protectedFieldGetterName(field) << "()"
      << " { return "
      << (useReference ? '&' : ' ') << "this->" << fieldName << "; }" << endl;

    // Set function
    s << INDENT << "inline void " << protectedFieldSetterName(field) << '(' << fieldType << " value)"
      << " { " << fieldName << " = value; }" << endl;
}

void HeaderGenerator::generateClass(QTextStream &s, GeneratorContext &classContext)
{
    AbstractMetaClass *metaClass = classContext.metaClass();
    if (ReportHandler::isDebug(ReportHandler::SparseDebug))
        qCDebug(lcShiboken) << "Generating header for " << metaClass->fullName();
    m_inheritedOverloads.clear();
    Indentation indent(INDENT);

    // write license comment
    s << licenseComment();

    QString wrapperName;
    if (!classContext.forSmartPointer()) {
        wrapperName = HeaderGenerator::wrapperName(metaClass);
    } else {
        wrapperName = HeaderGenerator::wrapperName(classContext.preciseType());
    }
    QString headerGuard = getFilteredCppSignatureString(wrapperName).toUpper();

    // Header
    s << "#ifndef SBK_" << headerGuard << "_H" << endl;
    s << "#define SBK_" << headerGuard << "_H" << endl<< endl;

    if (!avoidProtectedHack())
        s << "#define protected public" << endl << endl;

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

        if (avoidProtectedHack() && metaClass->hasProtectedFields()) {
            foreach (AbstractMetaField* field, metaClass->fields()) {
                if (!field->isProtected())
                    continue;
                writeProtectedFieldAccessors(s, field);
            }
        }

        //destructor
        if (!avoidProtectedHack() || !metaClass->hasPrivateDestructor()) {
            s << INDENT;
            if (metaClass->hasVirtualDestructor() || hasVirtualFunction)
                s <<  "virtual ";
            s << "~" << wrapperName << "();" << endl;
        }

        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), TypeSystem::CodeSnipPositionDeclaration, TypeSystem::NativeCode);

        if ((!avoidProtectedHack() || !metaClass->hasPrivateDestructor())
            && usePySideExtensions() && metaClass->isQObject()) {
            s << "public:\n";
            s << INDENT << "virtual int qt_metacall(QMetaObject::Call call, int id, void** args);" << endl;
            s << INDENT << "virtual void* qt_metacast(const char* _clname);" << endl;
        }

        if (m_inheritedOverloads.size()) {
            s << INDENT << "// Inherited overloads, because the using keyword sux" << endl;
            writeInheritedOverloads(s);
        }

        if (usePySideExtensions())
            s << INDENT << "static void pysideInitQtMetaTypes();" << endl;

        s << "};" << endl << endl;
    }

    s << "#endif // SBK_" << headerGuard << "_H" << endl << endl;
}

void HeaderGenerator::writeFunction(QTextStream& s, const AbstractMetaFunction* func)
{

    // do not write copy ctors here.
    if (!func->isPrivate() && func->functionType() == AbstractMetaFunction::CopyConstructorFunction) {
        writeCopyCtor(s, func->ownerClass());
        return;
    }
    if (func->isUserAdded())
        return;

    if (avoidProtectedHack() && func->isProtected() && !func->isConstructor() && !func->isOperatorOverload()) {
        s << INDENT << "inline " << (func->isStatic() ? "static " : "");
        s << functionSignature(func, QString(), QLatin1String("_protected"), Generator::EnumAsInts|Generator::OriginalTypeDescription)
            << " { ";
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
                argName = QString::fromLatin1("%1(%2)").arg(arg->type()->cppSignature(), argName);
            args << argName;
        }
        s << args.join(QLatin1String(", ")) << ')';
        s << "; }" << endl;
    }

    // pure virtual functions need a default implementation
    if ((func->isPrivate() && !visibilityModifiedToPrivate(func))
        || (func->isModifiedRemoved() && !func->isAbstract()))
        return;

    if (avoidProtectedHack() && func->ownerClass()->hasPrivateDestructor()
        && (func->isAbstract() || func->isVirtual()))
        return;

    if (func->isConstructor() || func->isAbstract() || func->isVirtual()) {
        s << INDENT;
        Options virtualOption = Generator::OriginalTypeDescription;

        if (func->isVirtual() || func->isAbstract())
            s << "virtual ";
        else if (!func->hasSignatureModifications())
            virtualOption = Generator::NoOption;

        s << functionSignature(func, QString(), QString(), virtualOption) << ';' << endl;

        // Check if this method hide other methods in base classes
        foreach (const AbstractMetaFunction* f, func->ownerClass()->functions()) {
            if (f != func
                && !f->isConstructor()
                && !f->isPrivate()
                && !f->isVirtual()
                && !f->isAbstract()
                && !f->isStatic()
                && f->name() == func->name()) {
                m_inheritedOverloads << f;
            }
        }

        // TODO: when modified an abstract method ceases to be virtual but stays abstract
        //if (func->isModifiedRemoved() && func->isAbstract()) {
        //}
    }
}

static void _writeTypeIndexDefineLine(QTextStream& s, const QString& variableName, int typeIndex)
{
    s << "#define ";
    s.setFieldWidth(60);
    s << variableName;
    s.setFieldWidth(0);
    s << ' ' << typeIndex << endl;
}
void HeaderGenerator::writeTypeIndexDefineLine(QTextStream& s, const TypeEntry* typeEntry)
{
    if (!typeEntry || !typeEntry->generateCode())
        return;
    s.setFieldAlignment(QTextStream::AlignLeft);
    int typeIndex = getTypeIndex(typeEntry);
    _writeTypeIndexDefineLine(s, getTypeIndexVariableName(typeEntry), typeIndex);
    if (typeEntry->isComplex()) {
        const ComplexTypeEntry* cType = reinterpret_cast<const ComplexTypeEntry*>(typeEntry);
        if (cType->baseContainerType()) {
            const AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes(), cType);
            if (metaClass->templateBaseClass())
                _writeTypeIndexDefineLine(s, getTypeIndexVariableName(metaClass, true), typeIndex);
        }
    }
    if (typeEntry->isEnum()) {
        const EnumTypeEntry* ete = reinterpret_cast<const EnumTypeEntry*>(typeEntry);
        if (ete->flags())
            writeTypeIndexDefineLine(s, ete->flags());
    }
}

void HeaderGenerator::writeTypeIndexDefine(QTextStream& s, const AbstractMetaClass* metaClass)
{
    if (!metaClass->typeEntry()->generateCode())
        return;
    writeTypeIndexDefineLine(s, metaClass->typeEntry());
    foreach (const AbstractMetaEnum* metaEnum, metaClass->enums()) {
        if (metaEnum->isPrivate())
            continue;
        writeTypeIndexDefineLine(s, metaEnum->typeEntry());
    }
}

bool HeaderGenerator::finishGeneration()
{
    // Generate the main header for this module.
    // This header should be included by binding modules
    // extendind on top of this one.
    QSet<Include> includes;
    QString macros;
    QTextStream macrosStream(&macros);
    QString sbkTypeFunctions;
    QTextStream typeFunctions(&sbkTypeFunctions);
    QString protectedEnumSurrogates;
    QTextStream protEnumsSurrogates(&protectedEnumSurrogates);

    Indentation indent(INDENT);

    macrosStream << "// Type indices" << endl;
    AbstractMetaEnumList globalEnums = this->globalEnums();
    foreach (const AbstractMetaClass* metaClass, classes()) {
        writeTypeIndexDefine(macrosStream, metaClass);
        lookForEnumsInClassesNotToBeGenerated(globalEnums, metaClass);
    }

    foreach (const AbstractMetaEnum* metaEnum, globalEnums)
        writeTypeIndexDefineLine(macrosStream, metaEnum->typeEntry());

    // Write the smart pointer define indexes.
    int smartPointerCountIndex = getMaxTypeIndex();
    int smartPointerCount = 0;
    foreach (const AbstractMetaType *metaType, instantiatedSmartPointers()) {
        QString variableName = getTypeIndexVariableName(metaType);
        macrosStream << "#define ";
        macrosStream.setFieldWidth(60);
        macrosStream << variableName;
        macrosStream.setFieldWidth(0);
        macrosStream << ' ' << smartPointerCountIndex << " // " << metaType->cppSignature()
                     << endl;
        ++smartPointerCountIndex;
        ++smartPointerCount;
    }


    macrosStream << "#define ";
    macrosStream.setFieldWidth(60);
    macrosStream << QLatin1String("SBK_") + moduleName() + QLatin1String("_IDX_COUNT");
    macrosStream.setFieldWidth(0);
    macrosStream << ' ' << getMaxTypeIndex() + smartPointerCount << endl << endl;
    macrosStream << "// This variable stores all Python types exported by this module." << endl;
    macrosStream << "extern PyTypeObject** " << cppApiVariableName() << ';' << endl << endl;
    macrosStream << "// This variable stores all type converters exported by this module." << endl;
    macrosStream << "extern SbkConverter** " << convertersVariableName() << ';' << endl << endl;

    // TODO-CONVERTER ------------------------------------------------------------------------------
    // Using a counter would not do, a fix must be made to APIExtractor's getTypeIndex().
    macrosStream << "// Converter indices" << endl;
    QList<const PrimitiveTypeEntry*> primitives = primitiveTypes();
    int pCount = 0;
    foreach (const PrimitiveTypeEntry* ptype, primitives) {
        /* Note: do not generate indices for typedef'd primitive types
         * as they'll use the primitive type converters instead, so we
         * don't need to create any other.
         */
        if (!ptype->generateCode() || !ptype->customConversion())
            continue;

        _writeTypeIndexDefineLine(macrosStream, getTypeIndexVariableName(ptype), pCount++);
    }

    foreach (const AbstractMetaType* container, instantiatedContainers()) {
        //_writeTypeIndexDefineLine(macrosStream, getTypeIndexVariableName(container), pCount);
        // DEBUG
        QString variableName = getTypeIndexVariableName(container);
        macrosStream << "#define ";
        macrosStream.setFieldWidth(60);
        macrosStream << variableName;
        macrosStream.setFieldWidth(0);
        macrosStream << ' ' << pCount << " // " << container->cppSignature() << endl;
        // DEBUG
        pCount++;
    }

    // Because on win32 the compiler will not accept a zero length array.
    if (pCount == 0)
        pCount++;
    _writeTypeIndexDefineLine(macrosStream, QStringLiteral("SBK_%1_CONVERTERS_IDX_COUNT").arg(moduleName()), pCount);
    macrosStream << endl;
    // TODO-CONVERTER ------------------------------------------------------------------------------

    macrosStream << "// Macros for type check" << endl;
    foreach (const AbstractMetaEnum* cppEnum, globalEnums) {
        if (cppEnum->isAnonymous() || cppEnum->isPrivate())
            continue;
        includes << cppEnum->typeEntry()->include();
        writeProtectedEnumSurrogate(protEnumsSurrogates, cppEnum);
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
            writeProtectedEnumSurrogate(protEnumsSurrogates, cppEnum);
            writeSbkTypeFunction(typeFunctions, cppEnum);
        }

        if (!metaClass->isNamespace())
            writeSbkTypeFunction(typeFunctions, metaClass);
    }

    foreach (const AbstractMetaType *metaType, instantiatedSmartPointers()) {
        const TypeEntry *classType = metaType->typeEntry();
        includes << classType->include();
        writeSbkTypeFunction(typeFunctions, metaType);
    }

    QString moduleHeaderFileName(outputDirectory()
                                 + QDir::separator() + subDirectoryForPackage(packageName())
                                 + QDir::separator() + getModuleHeaderFileName());

    QString includeShield(QLatin1String("SBK_") + moduleName().toUpper() + QLatin1String("_PYTHON_H"));

    FileOut file(moduleHeaderFileName);
    QTextStream& s = file.stream;
    // write license comment
    s << licenseComment() << endl << endl;

    s << "#ifndef " << includeShield << endl;
    s << "#define " << includeShield << endl<< endl;
    if (!avoidProtectedHack()) {
        s << "//workaround to access protected functions" << endl;
        s << "#define protected public" << endl << endl;
    }

    s << "#include <sbkpython.h>" << endl;
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

    if (!protectedEnumSurrogates.isEmpty()) {
        s << "// Protected enum surrogates" << endl;
        s << protectedEnumSurrogates << endl;
    }

    s << "namespace Shiboken" << endl << '{' << endl << endl;

    s << "// PyType functions, to get the PyObjectType for a type T\n";
    s << sbkTypeFunctions << endl;

    s << "} // namespace Shiboken" << endl << endl;

    s << "#endif // " << includeShield << endl << endl;

    return file.done() != FileOut::Failure;
}

void HeaderGenerator::writeProtectedEnumSurrogate(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    if (avoidProtectedHack() && cppEnum->isProtected())
        s << "enum " << protectedEnumSurrogateName(cppEnum) << " {};" << endl;
}

void HeaderGenerator::writeSbkTypeFunction(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString enumName;
    if (avoidProtectedHack() && cppEnum->isProtected()) {
        enumName = protectedEnumSurrogateName(cppEnum);
    } else {
        enumName = cppEnum->name();
        if (cppEnum->enclosingClass())
            enumName = cppEnum->enclosingClass()->qualifiedCppName() + QLatin1String("::") + enumName;
    }

    s << "template<> inline PyTypeObject* SbkType< ::" << enumName << " >() ";
    s << "{ return " << cpythonTypeNameExt(cppEnum->typeEntry()) << "; }\n";

    FlagsTypeEntry* flag = cppEnum->typeEntry()->flags();
    if (flag) {
        s <<  "template<> inline PyTypeObject* SbkType< ::" << flag->name() << " >() "
          << "{ return " << cpythonTypeNameExt(flag) << "; }\n";
    }
}

void HeaderGenerator::writeSbkTypeFunction(QTextStream& s, const AbstractMetaClass* cppClass)
{
    s <<  "template<> inline PyTypeObject* SbkType< ::" << cppClass->qualifiedCppName() << " >() "
      <<  "{ return reinterpret_cast<PyTypeObject*>(" << cpythonTypeNameExt(cppClass->typeEntry()) << "); }\n";
}

void HeaderGenerator::writeSbkTypeFunction(QTextStream &s, const AbstractMetaType *metaType)
{
    s <<  "template<> inline PyTypeObject* SbkType< ::" << metaType->cppSignature() << " >() "
      <<  "{ return reinterpret_cast<PyTypeObject*>(" << cpythonTypeNameExt(metaType) << "); }\n";
}

void HeaderGenerator::writeInheritedOverloads(QTextStream& s)
{
    foreach (const AbstractMetaFunction* func, m_inheritedOverloads) {
        s << INDENT << "inline ";
        s << functionSignature(func, QString(), QString(), Generator::EnumAsInts|Generator::OriginalTypeDescription) << " { ";
        s << (func->type() ? "return " : "");
        s << func->ownerClass()->qualifiedCppName() << "::" << func->originalName() << '(';
        QStringList args;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            QString argName = arg->name();
            const TypeEntry* enumTypeEntry = 0;
            if (arg->type()->isFlags())
                enumTypeEntry = reinterpret_cast<const FlagsTypeEntry*>(arg->type()->typeEntry())->originator();
            else if (arg->type()->isEnum())
                enumTypeEntry = arg->type()->typeEntry();
            if (enumTypeEntry)
                argName = arg->type()->cppSignature() + QLatin1Char('(') + argName + QLatin1Char(')');
            args << argName;
        }
        s << args.join(QLatin1String(", ")) << ')';
        s << "; }" << endl;
    }
}
