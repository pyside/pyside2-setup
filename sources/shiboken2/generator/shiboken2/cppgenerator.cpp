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

#include <memory>

#include "cppgenerator.h"
#include "overloaddata.h"
#include <abstractmetalang.h>
#include <reporthandler.h>
#include <typedatabase.h>

#include <QtCore/QDir>
#include <QtCore/QMetaObject>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QMetaType>

QHash<QString, QString> CppGenerator::m_nbFuncs = QHash<QString, QString>();
QHash<QString, QString> CppGenerator::m_sqFuncs = QHash<QString, QString>();
QHash<QString, QString> CppGenerator::m_mpFuncs = QHash<QString, QString>();
QString CppGenerator::m_currentErrorCode(QLatin1String("0"));

// utility functions
inline AbstractMetaType* getTypeWithoutContainer(AbstractMetaType* arg)
{
    if (arg && arg->typeEntry()->isContainer()) {
        AbstractMetaTypeList lst = arg->instantiations();
        // only support containers with 1 type
        if (lst.size() == 1)
            return lst[0];
    }
    return arg;
}

CppGenerator::CppGenerator()
{
    // Number protocol structure members names
    m_nbFuncs.insert(QLatin1String("__add__"), QLatin1String("nb_add"));
    m_nbFuncs.insert(QLatin1String("__sub__"), QLatin1String("nb_subtract"));
    m_nbFuncs.insert(QLatin1String("__mul__"), QLatin1String("nb_multiply"));
    m_nbFuncs.insert(QLatin1String("__div__"), QLatin1String("nb_divide"));
    m_nbFuncs.insert(QLatin1String("__mod__"), QLatin1String("nb_remainder"));
    m_nbFuncs.insert(QLatin1String("__neg__"), QLatin1String("nb_negative"));
    m_nbFuncs.insert(QLatin1String("__pos__"), QLatin1String("nb_positive"));
    m_nbFuncs.insert(QLatin1String("__invert__"), QLatin1String("nb_invert"));
    m_nbFuncs.insert(QLatin1String("__lshift__"), QLatin1String("nb_lshift"));
    m_nbFuncs.insert(QLatin1String("__rshift__"), QLatin1String("nb_rshift"));
    m_nbFuncs.insert(QLatin1String("__and__"), QLatin1String("nb_and"));
    m_nbFuncs.insert(QLatin1String("__xor__"), QLatin1String("nb_xor"));
    m_nbFuncs.insert(QLatin1String("__or__"), QLatin1String("nb_or"));
    m_nbFuncs.insert(QLatin1String("__iadd__"), QLatin1String("nb_inplace_add"));
    m_nbFuncs.insert(QLatin1String("__isub__"), QLatin1String("nb_inplace_subtract"));
    m_nbFuncs.insert(QLatin1String("__imul__"), QLatin1String("nb_inplace_multiply"));
    m_nbFuncs.insert(QLatin1String("__idiv__"), QLatin1String("nb_inplace_divide"));
    m_nbFuncs.insert(QLatin1String("__imod__"), QLatin1String("nb_inplace_remainder"));
    m_nbFuncs.insert(QLatin1String("__ilshift__"), QLatin1String("nb_inplace_lshift"));
    m_nbFuncs.insert(QLatin1String("__irshift__"), QLatin1String("nb_inplace_rshift"));
    m_nbFuncs.insert(QLatin1String("__iand__"), QLatin1String("nb_inplace_and"));
    m_nbFuncs.insert(QLatin1String("__ixor__"), QLatin1String("nb_inplace_xor"));
    m_nbFuncs.insert(QLatin1String("__ior__"), QLatin1String("nb_inplace_or"));
    m_nbFuncs.insert(QLatin1String("bool"), QLatin1String("nb_nonzero"));

    // sequence protocol functions
    typedef QPair<QString, QString> StrPair;
    m_sequenceProtocol.insert(QLatin1String("__len__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR), QLatin1String("Py_ssize_t")));
    m_sequenceProtocol.insert(QLatin1String("__getitem__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i"),
                                      QLatin1String("PyObject*")));
    m_sequenceProtocol.insert(QLatin1String("__setitem__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i, PyObject* _value"),
                                      QLatin1String("int")));
    m_sequenceProtocol.insert(QLatin1String("__getslice__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i1, Py_ssize_t _i2"),
                                      QLatin1String("PyObject*")));
    m_sequenceProtocol.insert(QLatin1String("__setslice__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i1, Py_ssize_t _i2, PyObject* _value"),
                                      QLatin1String("int")));
    m_sequenceProtocol.insert(QLatin1String("__contains__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", PyObject* _value"),
                                      QLatin1String("int")));
    m_sequenceProtocol.insert(QLatin1String("__concat__"),
                              StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", PyObject* _other"),
                                      QLatin1String("PyObject*")));

    // Sequence protocol structure members names
    m_sqFuncs.insert(QLatin1String("__concat__"), QLatin1String("sq_concat"));
    m_sqFuncs.insert(QLatin1String("__contains__"), QLatin1String("sq_contains"));
    m_sqFuncs.insert(QLatin1String("__getitem__"), QLatin1String("sq_item"));
    m_sqFuncs.insert(QLatin1String("__getslice__"), QLatin1String("sq_slice"));
    m_sqFuncs.insert(QLatin1String("__len__"), QLatin1String("sq_length"));
    m_sqFuncs.insert(QLatin1String("__setitem__"), QLatin1String("sq_ass_item"));
    m_sqFuncs.insert(QLatin1String("__setslice__"), QLatin1String("sq_ass_slice"));

    // mapping protocol function
    m_mappingProtocol.insert(QLatin1String("__mlen__"),
                             StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR),
                                     QLatin1String("Py_ssize_t")));
    m_mappingProtocol.insert(QLatin1String("__mgetitem__"),
                             StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", PyObject* _key"),
                                     QLatin1String("PyObject*")));
    m_mappingProtocol.insert(QLatin1String("__msetitem__"),
                             StrPair(QLatin1String("PyObject* " PYTHON_SELF_VAR ", PyObject* _key, PyObject* _value"),
                                     QLatin1String("int")));

    // Sequence protocol structure members names
    m_mpFuncs.insert(QLatin1String("__mlen__"), QLatin1String("mp_length"));
    m_mpFuncs.insert(QLatin1String("__mgetitem__"), QLatin1String("mp_subscript"));
    m_mpFuncs.insert(QLatin1String("__msetitem__"), QLatin1String("mp_ass_subscript"));
}

QString CppGenerator::fileNamePrefix() const
{
    return QLatin1String("_wrapper.cpp");
}

QString CppGenerator::fileNameForContext(GeneratorContext &context) const
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

QList<AbstractMetaFunctionList> CppGenerator::filterGroupedOperatorFunctions(const AbstractMetaClass* metaClass,
                                                                             uint queryIn)
{
    // ( func_name, num_args ) => func_list
    QMap<QPair<QString, int >, AbstractMetaFunctionList> results;
    const AbstractMetaClass::OperatorQueryOptions query(queryIn);
    foreach (AbstractMetaFunction* func, metaClass->operatorOverloads(query)) {
        if (func->isModifiedRemoved()
            || func->usesRValueReferences()
            || func->name() == QLatin1String("operator[]")
            || func->name() == QLatin1String("operator->")
            || func->name() == QLatin1String("operator!")) {
            continue;
        }
        int args;
        if (func->isComparisonOperator()) {
            args = -1;
        } else {
            args = func->arguments().size();
        }
        QPair<QString, int > op(func->name(), args);
        results[op].append(func);
    }
    return results.values();
}

bool CppGenerator::hasBoolCast(const AbstractMetaClass* metaClass) const
{
    if (!useIsNullAsNbNonZero())
        return false;
    // TODO: This could be configurable someday
    const AbstractMetaFunction* func = metaClass->findFunction(QLatin1String("isNull"));
    if (!func || !func->type() || !func->type()->typeEntry()->isPrimitive() || !func->isPublic())
        return false;
    const PrimitiveTypeEntry* pte = static_cast<const PrimitiveTypeEntry*>(func->type()->typeEntry());
    while (pte->referencedTypeEntry())
        pte = pte->referencedTypeEntry();
    return func && func->isConstant() && pte->name() == QLatin1String("bool") && func->arguments().isEmpty();
}

typedef QMap<QString, AbstractMetaFunctionList> FunctionGroupMap;
typedef FunctionGroupMap::const_iterator FunctionGroupMapIt;

// Prevent ELF symbol qt_version_tag from being generated into the source
static const char includeQDebug[] =
"#ifndef QT_NO_VERSION_TAGGING\n"
"#  define QT_NO_VERSION_TAGGING\n"
"#endif\n"
"#include <QDebug>\n";

/*!
    Function used to write the class generated binding code on the buffer
    \param s the output buffer
    \param metaClass the pointer to metaclass information
*/
void CppGenerator::generateClass(QTextStream &s, GeneratorContext &classContext)
{
    AbstractMetaClass *metaClass = classContext.metaClass();
    if (ReportHandler::isDebug(ReportHandler::SparseDebug))
        qCDebug(lcShiboken) << "Generating wrapper implementation for " << metaClass->fullName();

    // write license comment
    s << licenseComment() << endl;

    if (!avoidProtectedHack() && !metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        s << "//workaround to access protected functions" << endl;
        s << "#define protected public" << endl << endl;
    }

    // headers
    s << "// default includes" << endl;
    s << "#include <shiboken.h>" << endl;
    if (usePySideExtensions()) {
        s << includeQDebug;
        s << "#include <pysidesignal.h>" << endl;
        s << "#include <pysideproperty.h>" << endl;
        s << "#include <pyside.h>" << endl;
        s << "#include <destroylistener.h>" << endl;
        s << "#include <qapp_macro.h>" << endl;
    }

    s << "#include <typeresolver.h>" << endl;
    s << "#include <typeinfo>" << endl;
    if (usePySideExtensions() && metaClass->isQObject()) {
        s << "#include <signalmanager.h>" << endl;
        s << "#include <pysidemetafunction.h>" << endl;
    }

    // The multiple inheritance initialization function
    // needs the 'set' class from C++ STL.
    if (hasMultipleInheritanceInAncestry(metaClass))
        s << "#include <set>" << endl;

    s << endl << "// module include" << endl << "#include \"" << getModuleHeaderFileName() << '"' << endl;

    QString headerfile = fileNameForContext(classContext);
    headerfile.replace(QLatin1String(".cpp"), QLatin1String(".h"));
    s << endl << "// main header" << endl << "#include \"" << headerfile << '"' << endl;

    s << endl << "// inner classes" << endl;
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses()) {
        GeneratorContext innerClassContext(innerClass);
        if (shouldGenerate(innerClass)) {
            QString headerfile = fileNameForContext(innerClassContext);
            headerfile.replace(QLatin1String(".cpp"), QLatin1String(".h"));
            s << "#include \"" << headerfile << '"' << endl;
        }
    }

    AbstractMetaEnumList classEnums = metaClass->enums();
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses())
        lookForEnumsInClassesNotToBeGenerated(classEnums, innerClass);

    //Extra includes
    s << endl << "// Extra includes" << endl;
    QList<Include> includes = metaClass->typeEntry()->extraIncludes();
    foreach (AbstractMetaEnum* cppEnum, classEnums)
        includes.append(cppEnum->typeEntry()->extraIncludes());
    qSort(includes.begin(), includes.end());
    foreach (const Include &inc, includes)
        s << inc.toString() << endl;
    s << endl;

    if (metaClass->typeEntry()->typeFlags() & ComplexTypeEntry::Deprecated)
        s << "#Deprecated" << endl;

    // Use class base namespace
    {
        const AbstractMetaClass *context = metaClass->enclosingClass();
        while (context) {
            if (context->isNamespace() && !context->enclosingClass()) {
                s << "using namespace " << context->qualifiedCppName() << ";" << endl;
                break;
            }
            context = context->enclosingClass();
        }
    }

    s << endl;

    // Create string literal for smart pointer getter method.
    if (classContext.forSmartPointer()) {
        const SmartPointerTypeEntry *typeEntry =
                static_cast<const SmartPointerTypeEntry *>(classContext.preciseType()
                                                           ->typeEntry());
        QString rawGetter = typeEntry->getter();
        s << "static const char * " SMART_POINTER_GETTER " = \"" << rawGetter << "\";";
    }

    // class inject-code native/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode, metaClass);
        s << endl;
    }

    // python conversion rules
    if (metaClass->typeEntry()->hasTargetConversionRule()) {
        s << "// Python Conversion" << endl;
        s << metaClass->typeEntry()->conversionRule() << endl;
    }

    if (shouldGenerateCppWrapper(metaClass)) {
        s << "// Native ---------------------------------------------------------" << endl;
        s << endl;

        if (avoidProtectedHack() && usePySideExtensions()) {
            s << "void " << wrapperName(metaClass) << "::pysideInitQtMetaTypes()\n{\n";
            Indentation indent(INDENT);
            writeInitQtMetaTypeFunctionBody(s, classContext);
            s << "}\n\n";
        }

        foreach (const AbstractMetaFunction* func, filterFunctions(metaClass)) {
            if ((func->isPrivate() && !visibilityModifiedToPrivate(func))
                || (func->isModifiedRemoved() && !func->isAbstract()))
                continue;
            if (func->functionType() == AbstractMetaFunction::ConstructorFunction && !func->isUserAdded())
                writeConstructorNative(s, func);
            else if ((!avoidProtectedHack() || !metaClass->hasPrivateDestructor())
                     && (func->isVirtual() || func->isAbstract()))
                writeVirtualMethodNative(s, func);
        }

        if (!avoidProtectedHack() || !metaClass->hasPrivateDestructor()) {
            if (usePySideExtensions() && metaClass->isQObject())
                writeMetaObjectMethod(s, metaClass);
            writeDestructorNative(s, metaClass);
        }
    }

    Indentation indentation(INDENT);

    QString methodsDefinitions;
    QTextStream md(&methodsDefinitions);
    QString singleMethodDefinitions;
    QTextStream smd(&singleMethodDefinitions);
    QString signaturesString;
    QTextStream signatureStream(&signaturesString);

    s << endl << "// Target ---------------------------------------------------------" << endl << endl;
    s << "extern \"C\" {" << endl;
    const FunctionGroupMap &functionGroups = getFunctionGroups(metaClass);
    for (FunctionGroupMapIt it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        AbstractMetaFunctionList overloads;
        QSet<QString> seenSignatures;
        foreach (AbstractMetaFunction* func, it.value()) {
            if (!func->isAssignmentOperator()
                && !func->usesRValueReferences()
                && !func->isCastOperator()
                && !func->isModifiedRemoved()
                && (!func->isPrivate() || func->functionType() == AbstractMetaFunction::EmptyFunction)
                && func->ownerClass() == func->implementingClass()
                && (func->name() != QLatin1String("qt_metacall"))) {
                // PYSIDE-331: Inheritance works correctly when there are disjoint functions.
                // But when a function is both in a class and inherited in a subclass,
                // then we need to search through all subclasses and collect the new signatures.
                overloads << getFunctionAndInheritedOverloads(func, &seenSignatures);
            }
        }

        if (overloads.isEmpty())
            continue;

        const AbstractMetaFunction* rfunc = overloads.first();
        if (m_sequenceProtocol.contains(rfunc->name()) || m_mappingProtocol.contains(rfunc->name()))
            continue;

        if (rfunc->isConstructor()) {
            // @TODO: Implement constructor support for smart pointers, so that they can be
            // instantiated in python code.
            if (classContext.forSmartPointer())
                continue;
            writeConstructorWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
        // call operators
        else if (rfunc->name() == QLatin1String("operator()")) {
            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
        else if (!rfunc->isOperatorOverload()) {

            if (classContext.forSmartPointer()) {
                const SmartPointerTypeEntry *smartPointerTypeEntry =
                        static_cast<const SmartPointerTypeEntry *>(
                            classContext.preciseType()->typeEntry());

                if (smartPointerTypeEntry->getter() == rfunc->name()) {
                    // Replace the return type of the raw pointer getter method with the actual
                    // return type.
                    QString innerTypeName =
                            classContext.preciseType()->getSmartPointerInnerType()->name();
                    QString pointerToInnerTypeName = innerTypeName + QLatin1Char('*');
                    // @TODO: This possibly leaks, but there are a bunch of other places where this
                    // is done, so this will be fixed in bulk with all the other cases, because the
                    // ownership of the pointers is not clear at the moment.
                    AbstractMetaType *pointerToInnerType =
                            buildAbstractMetaTypeFromString(pointerToInnerTypeName);

                    AbstractMetaFunction *mutableRfunc = overloads.first();
                    mutableRfunc->replaceType(pointerToInnerType);
                } else if (smartPointerTypeEntry->refCountMethodName().isEmpty()
                           || smartPointerTypeEntry->refCountMethodName() != rfunc->name()) {
                    // Skip all public methods of the smart pointer except for the raw getter and
                    // the ref count method.
                    continue;
                }
            }

            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
            if (OverloadData::hasStaticAndInstanceFunctions(overloads)) {
                QString methDefName = cpythonMethodDefinitionName(rfunc);
                smd << "static PyMethodDef " << methDefName << " = {" << endl;
                smd << INDENT;
                writeMethodDefinitionEntry(smd, overloads);
                smd << endl << "};" << endl << endl;
            }
            writeMethodDefinition(md, overloads);
        }
    }

    QString className = cpythonTypeName(metaClass);
    className.remove(QRegExp(QLatin1String("_Type$")));

    if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer()) {
        writeCopyFunction(s, classContext);
        signatureStream << INDENT << metaClass->fullName() << ".__copy__()" << endl;
    }

    // Write single method definitions
    s << singleMethodDefinitions;

    // Write methods definition
    s << "static PyMethodDef " << className << "_methods[] = {" << endl;
    s << methodsDefinitions << endl;
    if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer())
        s << INDENT << "{\"__copy__\", (PyCFunction)" << className << "___copy__" << ", METH_NOARGS}," << endl;
    s << INDENT << "{0} // Sentinel" << endl;
    s << "};" << endl << endl;

    // Write tp_getattro function
    if ((usePySideExtensions() && metaClass->qualifiedCppName() == QLatin1String("QObject"))) {
        writeGetattroFunction(s, classContext);
        s << endl;
        writeSetattroFunction(s, classContext);
        s << endl;
    } else {
        if (classNeedsGetattroFunction(metaClass)) {
            writeGetattroFunction(s, classContext);
            s << endl;
        }
        if (classNeedsSetattroFunction(metaClass)) {
            writeSetattroFunction(s, classContext);
            s << endl;
        }
    }

    if (hasBoolCast(metaClass)) {
        ErrorCode errorCode(-1);
        s << "static int " << cpythonBaseName(metaClass) << "___nb_bool(PyObject* " PYTHON_SELF_VAR ")" << endl;
        s << '{' << endl;
        writeCppSelfDefinition(s, classContext);
        s << INDENT << "int result;" << endl;
        s << INDENT << BEGIN_ALLOW_THREADS << endl;
        s << INDENT << "result = !" CPP_SELF_VAR "->isNull();" << endl;
        s << INDENT << END_ALLOW_THREADS << endl;
        s << INDENT << "return result;" << endl;
        s << '}' << endl << endl;
    }

    if (supportsNumberProtocol(metaClass) && !metaClass->typeEntry()->isSmartPointer()) {
        QList<AbstractMetaFunctionList> opOverloads = filterGroupedOperatorFunctions(
                metaClass,
                AbstractMetaClass::ArithmeticOp
                | AbstractMetaClass::LogicalOp
                | AbstractMetaClass::BitwiseOp);

        foreach (const AbstractMetaFunctionList &allOverloads, opOverloads) {
            AbstractMetaFunctionList overloads;
            foreach (AbstractMetaFunction* func, allOverloads) {
                if (!func->isModifiedRemoved()
                    && !func->isPrivate()
                    && (func->ownerClass() == func->implementingClass() || func->isAbstract()))
                    overloads.append(func);
            }

            if (overloads.isEmpty())
                continue;

            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
    }

    if (supportsSequenceProtocol(metaClass)) {
        writeSequenceMethods(s, metaClass, classContext);
    }

    if (supportsMappingProtocol(metaClass)) {
        writeMappingMethods(s, metaClass, classContext);
    }

    if (metaClass->hasComparisonOperatorOverload()) {
        s << "// Rich comparison" << endl;
        writeRichCompareFunction(s, classContext);
    }

    if (shouldGenerateGetSetList(metaClass) && !classContext.forSmartPointer()) {
        foreach (const AbstractMetaField* metaField, metaClass->fields()) {
            if (metaField->isStatic())
                continue;
            writeGetterFunction(s, metaField, classContext);
            if (!metaField->type()->isConstant())
                writeSetterFunction(s, metaField, classContext);
            s << endl;
        }

        s << "// Getters and Setters for " << metaClass->name() << endl;
        s << "static PyGetSetDef " << cpythonGettersSettersDefinitionName(metaClass) << "[] = {" << endl;
        foreach (const AbstractMetaField* metaField, metaClass->fields()) {
            if (metaField->isStatic())
                continue;

            bool hasSetter = !metaField->type()->isConstant();
            s << INDENT << "{const_cast<char*>(\"" << metaField->name() << "\"), ";
            s << cpythonGetterFunctionName(metaField);
            s << ", " << (hasSetter ? cpythonSetterFunctionName(metaField) : QLatin1String("0"));
            s << "}," << endl;
        }
        s << INDENT << "{0}  // Sentinel" << endl;
        s << "};" << endl << endl;
    }

    s << "} // extern \"C\"" << endl << endl;

    if (!metaClass->typeEntry()->hashFunction().isEmpty())
        writeHashFunction(s, classContext);

    // Write tp_traverse and tp_clear functions.
    writeTpTraverseFunction(s, metaClass);
    writeTpClearFunction(s, metaClass);

    writeClassDefinition(s, metaClass, classContext);
    s << endl;

    if (metaClass->isPolymorphic() && metaClass->baseClass())
        writeTypeDiscoveryFunction(s, metaClass);


    foreach (AbstractMetaEnum* cppEnum, classEnums) {
        if (cppEnum->isAnonymous() || cppEnum->isPrivate())
            continue;

        bool hasFlags = cppEnum->typeEntry()->flags();
        if (hasFlags) {
            writeFlagsMethods(s, cppEnum);
            writeFlagsNumberMethodsDefinition(s, cppEnum);
            s << endl;
        }
    }
    s << endl;

    writeConverterFunctions(s, metaClass, classContext);
    writeClassRegister(s, metaClass, classContext, signatureStream);

    // class inject-code native/end
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode, metaClass);
        s << endl;
    }
}

void CppGenerator::writeConstructorNative(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    s << functionSignature(func, wrapperName(func->ownerClass()) + QLatin1String("::"), QString(),
                           OriginalTypeDescription | SkipDefaultValues);
    s << " : ";
    writeFunctionCall(s, func);
    s << " {" << endl;
    const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
    writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode, func, lastArg);
    s << INDENT << "// ... middle" << endl;
    writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode, func, lastArg);
    s << '}' << endl << endl;
}

void CppGenerator::writeDestructorNative(QTextStream &s, const AbstractMetaClass *metaClass)
{
    Indentation indentation(INDENT);
    s << wrapperName(metaClass) << "::~" << wrapperName(metaClass) << "()" << endl << '{' << endl;
    // kill pyobject
    s << INDENT << "SbkObject* wrapper = Shiboken::BindingManager::instance().retrieveWrapper(this);" << endl;
    s << INDENT << "Shiboken::Object::destroy(wrapper, this);" << endl;
    s << '}' << endl;
}

static bool allArgumentsRemoved(const AbstractMetaFunction* func)
{
    if (func->arguments().isEmpty())
        return false;
    foreach (const AbstractMetaArgument* arg, func->arguments()) {
        if (!func->argumentRemoved(arg->argumentIndex() + 1))
            return false;
    }
    return true;
}

QString CppGenerator::getVirtualFunctionReturnTypeName(const AbstractMetaFunction* func)
{
    if (!func->type())
        return QLatin1String("\"\"");

    if (!func->typeReplaced(0).isEmpty())
        return QLatin1Char('"') + func->typeReplaced(0) + QLatin1Char('"');

    // SbkType would return null when the type is a container.
    if (func->type()->typeEntry()->isContainer()) {
        return QLatin1Char('"')
               + reinterpret_cast<const ContainerTypeEntry*>(func->type()->typeEntry())->typeName()
               + QLatin1Char('"');
    }

    if (avoidProtectedHack()) {
        const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(func->type());
        if (metaEnum && metaEnum->isProtected())
            return QLatin1Char('"') + protectedEnumSurrogateName(metaEnum) + QLatin1Char('"');
    }

    if (func->type()->isPrimitive())
        return QLatin1Char('"') + func->type()->name() + QLatin1Char('"');

    return QString::fromLatin1("Shiboken::SbkType< %1 >()->tp_name").arg(func->type()->typeEntry()->qualifiedCppName());
}

void CppGenerator::writeVirtualMethodNative(QTextStream&s, const AbstractMetaFunction* func)
{
    //skip metaObject function, this will be written manually ahead
    if (usePySideExtensions() && func->ownerClass() && func->ownerClass()->isQObject() &&
        ((func->name() == QLatin1String("metaObject")) || (func->name() == QLatin1String("qt_metacall"))))
        return;

    const TypeEntry* retType = func->type() ? func->type()->typeEntry() : 0;
    const QString funcName = func->isOperatorOverload() ? pythonOperatorFunctionName(func) : func->name();

    QString prefix = wrapperName(func->ownerClass()) + QLatin1String("::");
    s << functionSignature(func, prefix, QString(), Generator::SkipDefaultValues|Generator::OriginalTypeDescription)
      << endl << '{' << endl;

    Indentation indentation(INDENT);

    QString defaultReturnExpr;
    if (retType) {
        foreach (const FunctionModification &mod, func->modifications()) {
            foreach (const ArgumentModification &argMod, mod.argument_mods) {
                if (argMod.index == 0 && !argMod.replacedDefaultExpression.isEmpty()) {
                    QRegExp regex(QLatin1String("%(\\d+)"));
                    defaultReturnExpr = argMod.replacedDefaultExpression;
                    int offset = 0;
                    while ((offset = regex.indexIn(defaultReturnExpr, offset)) != -1) {
                        int argId = regex.cap(1).toInt() - 1;
                        if (argId < 0 || argId > func->arguments().count()) {
                            qCWarning(lcShiboken) << "The expression used in return value contains an invalid index.";
                            break;
                        }
                        defaultReturnExpr.replace(regex.cap(0), func->arguments()[argId]->name());
                    }
                }
            }
        }
        if (defaultReturnExpr.isEmpty())
            defaultReturnExpr = minimalConstructor(func->type());
        if (defaultReturnExpr.isEmpty()) {
            QString errorMsg = QLatin1String(__FUNCTION__) + QLatin1String(": ");
            if (const AbstractMetaClass *c = func->implementingClass())
                errorMsg += c->qualifiedCppName() + QLatin1String("::");
            errorMsg += func->signature();
            errorMsg = ShibokenGenerator::msgCouldNotFindMinimalConstructor(errorMsg, func->type()->cppSignature());
            qCWarning(lcShiboken).noquote().nospace() << errorMsg;
            s << endl << INDENT << "#error " << errorMsg << endl;
        }
    }

    if (func->isAbstract() && func->isModifiedRemoved()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QString::fromLatin1("Pure virtual method '%1::%2' must be implement but was "\
                                   "completely removed on type system.")
                                   .arg(func->ownerClass()->name(), func->minimalSignature());
        s << INDENT << "return " << defaultReturnExpr << ';' << endl;
        s << '}' << endl << endl;
        return;
    }

    //Write declaration/native injected code
    if (func->hasInjectedCode()) {
        CodeSnipList snips = func->injectedCodeSnips();
        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionDeclaration, TypeSystem::NativeCode, func, lastArg);
        s << endl;
    }

    s << INDENT << "Shiboken::GilState gil;" << endl;

    // Get out of virtual method call if someone already threw an error.
    s << INDENT << "if (PyErr_Occurred())" << endl;
    {
        Indentation indentation(INDENT);
        s << INDENT << "return " << defaultReturnExpr << ';' << endl;
    }

    s << INDENT << "Shiboken::AutoDecRef " PYTHON_OVERRIDE_VAR "(Shiboken::BindingManager::instance().getOverride(this, \"";
    s << funcName << "\"));" << endl;

    s << INDENT << "if (" PYTHON_OVERRIDE_VAR ".isNull()) {" << endl;
    {
        Indentation indentation(INDENT);
        CodeSnipList snips;
        if (func->hasInjectedCode()) {
            snips = func->injectedCodeSnips();
            const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
            writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::ShellCode, func, lastArg);
            s << endl;
        }

        if (func->isAbstract()) {
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
            s << func->ownerClass()->name() << '.' << funcName;
            s << "()' not implemented.\");" << endl;
            s << INDENT << "return " << (retType ? defaultReturnExpr : QString());
        } else {
            s << INDENT << "gil.release();" << endl;
            s << INDENT;
            if (retType)
                s << "return ";
            s << "this->::" << func->implementingClass()->qualifiedCppName() << "::";
            writeFunctionCall(s, func, Generator::VirtualCall);
            if (!retType)
                s << ";\n" << INDENT << "return";
        }
    }
    s << ';' << endl;
    s << INDENT << '}' << endl << endl;

    writeConversionRule(s, func, TypeSystem::TargetLangCode);

    s << INDENT << "Shiboken::AutoDecRef " PYTHON_ARGS "(";

    if (func->arguments().isEmpty() || allArgumentsRemoved(func)) {
        s << "PyTuple_New(0));" << endl;
    } else {
        QStringList argConversions;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (func->argumentRemoved(arg->argumentIndex() + 1))
                continue;

            QString argConv;
            QTextStream ac(&argConv);
            const PrimitiveTypeEntry* argType = (const PrimitiveTypeEntry*) arg->type()->typeEntry();
            bool convert = argType->isObject()
                            || arg->type()->isQObject()
                            || argType->isValue()
                            || arg->type()->isValuePointer()
                            || arg->type()->isNativePointer()
                            || argType->isFlags()
                            || argType->isEnum()
                            || argType->isContainer()
                            || arg->type()->referenceType() == LValueReference;

            if (!convert && argType->isPrimitive()) {
                if (argType->basicReferencedTypeEntry())
                    argType = argType->basicReferencedTypeEntry();
                convert = !m_formatUnits.contains(argType->name());
            }

            Indentation indentation(INDENT);
            ac << INDENT;
            if (!func->conversionRule(TypeSystem::TargetLangCode, arg->argumentIndex() + 1).isEmpty()) {
                // Has conversion rule.
                ac << arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
            } else {
                QString argName = arg->name();
                if (convert)
                    writeToPythonConversion(ac, arg->type(), func->ownerClass(), argName);
                else
                    ac << argName;
            }

            argConversions << argConv;
        }

        s << "Py_BuildValue(\"(" << getFormatUnitString(func, false) << ")\"," << endl;
        s << argConversions.join(QLatin1String(",\n")) << endl;
        s << INDENT << "));" << endl;
    }

    bool invalidateReturn = false;
    QSet<int> invalidateArgs;
    foreach (const FunctionModification &funcMod, func->modifications()) {
        foreach (const ArgumentModification &argMod, funcMod.argument_mods) {
            if (argMod.resetAfterUse && !invalidateArgs.contains(argMod.index)) {
                invalidateArgs.insert(argMod.index);
                s << INDENT << "bool invalidateArg" << argMod.index;
                s << " = PyTuple_GET_ITEM(" PYTHON_ARGS ", " << argMod.index - 1 << ")->ob_refcnt == 1;" << endl;
            } else if (argMod.index == 0 && argMod.ownerships[TypeSystem::TargetLangCode] == TypeSystem::CppOwnership) {
                invalidateReturn = true;
            }
        }
    }
    s << endl;

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = func->injectedCodeSnips();

        if (injectedCodeUsesPySelf(func))
            s << INDENT << "PyObject* pySelf = BindingManager::instance().retrieveWrapper(this);" << endl;

        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode, func, lastArg);
        s << endl;
    }

    if (!injectedCodeCallsPythonOverride(func)) {
        s << INDENT;
        s << "Shiboken::AutoDecRef " PYTHON_RETURN_VAR "(PyObject_Call(" PYTHON_OVERRIDE_VAR ", " PYTHON_ARGS ", NULL));" << endl;

        s << INDENT << "// An error happened in python code!" << endl;
        s << INDENT << "if (" PYTHON_RETURN_VAR ".isNull()) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_Print();" << endl;
            s << INDENT << "return " << defaultReturnExpr << ';' << endl;
        }
        s << INDENT << '}' << endl;

        if (retType) {
            if (invalidateReturn)
                s << INDENT << "bool invalidateArg0 = " PYTHON_RETURN_VAR "->ob_refcnt == 1;" << endl;

            if (func->typeReplaced(0) != QLatin1String("PyObject")) {

                s << INDENT << "// Check return type" << endl;
                s << INDENT;
                if (func->typeReplaced(0).isEmpty()) {
                    s << "PythonToCppFunc " PYTHON_TO_CPP_VAR " = " << cpythonIsConvertibleFunction(func->type());
                    s << PYTHON_RETURN_VAR ");" << endl;
                    s << INDENT << "if (!" PYTHON_TO_CPP_VAR ") {" << endl;
                    {
                        Indentation indent(INDENT);
                        s << INDENT << "Shiboken::warning(PyExc_RuntimeWarning, 2, "\
                                       "\"Invalid return value in function %s, expected %s, got %s.\", \"";
                        s << func->ownerClass()->name() << '.' << funcName << "\", " << getVirtualFunctionReturnTypeName(func);
                        s << ", " PYTHON_RETURN_VAR "->ob_type->tp_name);" << endl;
                        s << INDENT << "return " << defaultReturnExpr << ';' << endl;
                    }
                    s << INDENT << '}' << endl;

                } else {

                    s << INDENT << "// Check return type" << endl;
                    s << INDENT << "bool typeIsValid = ";
                    writeTypeCheck(s, func->type(), QLatin1String(PYTHON_RETURN_VAR),
                                   isNumber(func->type()->typeEntry()), func->typeReplaced(0));
                    s << ';' << endl;
                    s << INDENT << "if (!typeIsValid";
                    s << (isPointerToWrapperType(func->type()) ? " && " PYTHON_RETURN_VAR " != Py_None" : "");
                    s << ") {" << endl;
                    {
                        Indentation indent(INDENT);
                        s << INDENT << "Shiboken::warning(PyExc_RuntimeWarning, 2, "\
                                       "\"Invalid return value in function %s, expected %s, got %s.\", \"";
                        s << func->ownerClass()->name() << '.' << funcName << "\", " << getVirtualFunctionReturnTypeName(func);
                        s << ", " PYTHON_RETURN_VAR "->ob_type->tp_name);" << endl;
                        s << INDENT << "return " << defaultReturnExpr << ';' << endl;
                    }
                    s << INDENT << '}' << endl;

                }
            }

            if (!func->conversionRule(TypeSystem::NativeCode, 0).isEmpty()) {
                // Has conversion rule.
                writeConversionRule(s, func, TypeSystem::NativeCode, QLatin1String(CPP_RETURN_VAR));
            } else if (!injectedCodeHasReturnValueAttribution(func, TypeSystem::NativeCode)) {
                writePythonToCppTypeConversion(s, func->type(), QLatin1String(PYTHON_RETURN_VAR),
                                               QLatin1String(CPP_RETURN_VAR), func->implementingClass());
            }
        }
    }

    if (invalidateReturn) {
        s << INDENT << "if (invalidateArg0)" << endl;
        Indentation indentation(INDENT);
        s << INDENT << "Shiboken::Object::releaseOwnership(" << PYTHON_RETURN_VAR  ".object());" << endl;
    }
    foreach (int argIndex, invalidateArgs) {
        s << INDENT << "if (invalidateArg" << argIndex << ')' << endl;
        Indentation indentation(INDENT);
        s << INDENT << "Shiboken::Object::invalidate(PyTuple_GET_ITEM(" PYTHON_ARGS ", ";
        s << (argIndex - 1) << "));" << endl;
    }


    foreach (const FunctionModification &funcMod, func->modifications()) {
        foreach (const ArgumentModification &argMod, funcMod.argument_mods) {
            if (argMod.ownerships.contains(TypeSystem::NativeCode)
                && argMod.index == 0 && argMod.ownerships[TypeSystem::NativeCode] == TypeSystem::CppOwnership) {
                s << INDENT << "if (Shiboken::Object::checkType(" PYTHON_RETURN_VAR "))" << endl;
                Indentation indent(INDENT);
                s << INDENT << "Shiboken::Object::releaseOwnership(" PYTHON_RETURN_VAR ");" << endl;
            }
        }
    }

    if (func->hasInjectedCode()) {
        s << endl;
        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode, func, lastArg);
    }

    if (retType) {
        s << INDENT << "return ";
        if (avoidProtectedHack() && retType->isEnum()) {
            const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(retType);
            bool isProtectedEnum = metaEnum && metaEnum->isProtected();
            if (isProtectedEnum) {
                QString typeCast;
                if (metaEnum->enclosingClass())
                    typeCast += QLatin1String("::") + metaEnum->enclosingClass()->qualifiedCppName();
                typeCast += QLatin1String("::") + metaEnum->name();
                s << '(' << typeCast << ')';
            }
        }
        if (func->type()->referenceType() == LValueReference && !isPointer(func->type()))
            s << '*';
        s << CPP_RETURN_VAR ";" << endl;
    }

    s << '}' << endl << endl;
}

void CppGenerator::writeMetaObjectMethod(QTextStream& s, const AbstractMetaClass* metaClass)
{
    Indentation indentation(INDENT);
    QString wrapperClassName = wrapperName(metaClass);
    s << "const QMetaObject* " << wrapperClassName << "::metaObject() const" << endl;
    s << '{' << endl;
    s << INDENT << "if (QObject::d_ptr->metaObject)" << endl
      << INDENT << INDENT << "return QObject::d_ptr->dynamicMetaObject();" << endl;
    s << INDENT << "SbkObject* pySelf = Shiboken::BindingManager::instance().retrieveWrapper(this);" << endl;
    s << INDENT << "if (pySelf == NULL)" << endl;
    s << INDENT << INDENT << "return " << metaClass->qualifiedCppName() << "::metaObject();" << endl;
    s << INDENT << "return PySide::SignalManager::retriveMetaObject(reinterpret_cast<PyObject*>(pySelf));" << endl;
    s << '}' << endl << endl;

    // qt_metacall function
    s << "int " << wrapperClassName << "::qt_metacall(QMetaObject::Call call, int id, void** args)" << endl;
    s << "{" << endl;

    AbstractMetaFunction *func = NULL;
    AbstractMetaFunctionList list = metaClass->queryFunctionsByName(QLatin1String("qt_metacall"));
    if (list.size() == 1)
        func = list[0];

    CodeSnipList snips;
    if (func) {
        snips = func->injectedCodeSnips();
        if (func->isUserAdded()) {
            CodeSnipList snips = func->injectedCodeSnips();
            writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode, func);
        }
    }

    s << INDENT << "int result = " << metaClass->qualifiedCppName() << "::qt_metacall(call, id, args);" << endl;
    s << INDENT << "return result < 0 ? result : PySide::SignalManager::qt_metacall(this, call, id, args);" << endl;
    s << "}" << endl << endl;

    // qt_metacast function
    writeMetaCast(s, metaClass);
}

void CppGenerator::writeMetaCast(QTextStream& s, const AbstractMetaClass* metaClass)
{
    Indentation indentation(INDENT);
    QString wrapperClassName = wrapperName(metaClass);
    s << "void* " << wrapperClassName << "::qt_metacast(const char* _clname)" << endl;
    s << '{' << endl;
    s << INDENT << "if (!_clname) return 0;" << endl;
    s << INDENT << "SbkObject* pySelf = Shiboken::BindingManager::instance().retrieveWrapper(this);" << endl;
    s << INDENT << "if (pySelf && PySide::inherits(Py_TYPE(pySelf), _clname))" << endl;
    s << INDENT << INDENT << "return static_cast<void*>(const_cast< " << wrapperClassName << "* >(this));" << endl;
    s << INDENT << "return " << metaClass->qualifiedCppName() << "::qt_metacast(_clname);" << endl;
    s << "}" << endl << endl;
}

void CppGenerator::writeEnumConverterFunctions(QTextStream& s, const AbstractMetaEnum* metaEnum)
{
    if (metaEnum->isPrivate() || metaEnum->isAnonymous())
        return;
    writeEnumConverterFunctions(s, metaEnum->typeEntry());
}

void CppGenerator::writeEnumConverterFunctions(QTextStream& s, const TypeEntry* enumType)
{
    if (!enumType)
        return;
    QString enumFlagName = enumType->isFlags() ? QLatin1String("flag") : QLatin1String("enum");
    QString typeName = fixedCppTypeName(enumType);
    QString enumPythonType = cpythonTypeNameExt(enumType);
    QString cppTypeName = getFullTypeName(enumType).trimmed();
    if (avoidProtectedHack()) {
        const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(enumType);
        if (metaEnum && metaEnum->isProtected())
            cppTypeName = protectedEnumSurrogateName(metaEnum);
    }
    QString code;
    QTextStream c(&code);
    c << INDENT << "*((" << cppTypeName << "*)cppOut) = ";
    if (enumType->isFlags())
        c << cppTypeName << "(QFlag((int)PySide::QFlags::getValue(reinterpret_cast<PySideQFlagsObject*>(pyIn))))";
    else
        c << "(" << cppTypeName << ") Shiboken::Enum::getValue(pyIn)";
    c << ';' << endl;
    writePythonToCppFunction(s, code, typeName, typeName);

    QString pyTypeCheck = QStringLiteral("PyObject_TypeCheck(pyIn, %1)").arg(enumPythonType);
    writeIsPythonConvertibleToCppFunction(s, typeName, typeName, pyTypeCheck);

    code.clear();

    c << INDENT << "int castCppIn = *((" << cppTypeName << "*)cppIn);" << endl;
    c << INDENT;
    c << "return ";
    if (enumType->isFlags())
        c << "reinterpret_cast<PyObject*>(PySide::QFlags::newObject(castCppIn, " << enumPythonType << "))";

    else
        c << "Shiboken::Enum::newItem(" << enumPythonType << ", castCppIn)";
    c << ';' << endl;
    writeCppToPythonFunction(s, code, typeName, typeName);
    s << endl;

    if (enumType->isFlags())
        return;

    const FlagsTypeEntry* flags = reinterpret_cast<const EnumTypeEntry*>(enumType)->flags();
    if (!flags)
        return;

    // QFlags part.

    writeEnumConverterFunctions(s, flags);

    code.clear();
    cppTypeName = getFullTypeName(flags).trimmed();
    c << INDENT << "*((" << cppTypeName << "*)cppOut) = " << cppTypeName;
    c << "(QFlag((int)Shiboken::Enum::getValue(pyIn)));" << endl;

    QString flagsTypeName = fixedCppTypeName(flags);
    writePythonToCppFunction(s, code, typeName, flagsTypeName);
    writeIsPythonConvertibleToCppFunction(s, typeName, flagsTypeName, pyTypeCheck);

    code.clear();
    c << INDENT << "Shiboken::AutoDecRef pyLong(PyNumber_Long(pyIn));" << endl;
    c << INDENT << "*((" << cppTypeName << "*)cppOut) = " << cppTypeName;
    c << "(QFlag((int)PyLong_AsLong(pyLong.object())));" << endl;
    writePythonToCppFunction(s, code, QLatin1String("number"), flagsTypeName);
    writeIsPythonConvertibleToCppFunction(s, QLatin1String("number"), flagsTypeName,
                                          QLatin1String("PyNumber_Check(pyIn)"));
}

void CppGenerator::writeConverterFunctions(QTextStream &s, const AbstractMetaClass *metaClass,
                                           GeneratorContext &classContext)
{
    s << "// Type conversion functions." << endl << endl;

    AbstractMetaEnumList classEnums = metaClass->enums();
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses())
        lookForEnumsInClassesNotToBeGenerated(classEnums, innerClass);
    if (!classEnums.isEmpty())
        s << "// Python to C++ enum conversion." << endl;
    foreach (const AbstractMetaEnum* metaEnum, classEnums)
        writeEnumConverterFunctions(s, metaEnum);

    if (metaClass->isNamespace())
        return;

    QString typeName;
    if (!classContext.forSmartPointer())
        typeName = getFullTypeName(metaClass);
    else
        typeName = getFullTypeName(classContext.preciseType());

    QString cpythonType = cpythonTypeName(metaClass);

    // Returns the C++ pointer of the Python wrapper.
    s << "// Python to C++ pointer conversion - returns the C++ object of the Python wrapper (keeps object identity)." << endl;

    QString sourceTypeName = metaClass->name();
    QString targetTypeName = metaClass->name() + QLatin1String("_PTR");
    QString code;
    QTextStream c(&code);
    c << INDENT << "Shiboken::Conversions::pythonToCppPointer(&" << cpythonType << ", pyIn, cppOut);";
    writePythonToCppFunction(s, code, sourceTypeName, targetTypeName);

    // "Is convertible" function for the Python object to C++ pointer conversion.
    QString pyTypeCheck = QStringLiteral("PyObject_TypeCheck(pyIn, (PyTypeObject*)&%1)").arg(cpythonType);
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, pyTypeCheck, QString(), true);
    s << endl;

    // C++ pointer to a Python wrapper, keeping identity.
    s << "// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity)." << endl;
    code.clear();
    if (usePySideExtensions() && metaClass->isQObject())
    {
        c << INDENT << "return PySide::getWrapperForQObject((" << typeName << "*)cppIn, &" << cpythonType << ");" << endl;
    } else {
        c << INDENT << "PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);" << endl;
        c << INDENT << "if (pyOut) {" << endl;
        {
            Indentation indent(INDENT);
            c << INDENT << "Py_INCREF(pyOut);" << endl;
            c << INDENT << "return pyOut;" << endl;
        }
        c << INDENT << '}' << endl;
        c << INDENT << "const char* typeName = typeid(*((" << typeName << "*)cppIn)).name();" << endl;
        c << INDENT << "return Shiboken::Object::newObject(&" << cpythonType;
        c << ", const_cast<void*>(cppIn), false, false, typeName);";
    }
    std::swap(targetTypeName, sourceTypeName);
    writeCppToPythonFunction(s, code, sourceTypeName, targetTypeName);

    // The conversions for an Object Type end here.
    if (!metaClass->typeEntry()->isValue() && !metaClass->typeEntry()->isSmartPointer()) {
        s << endl;
        return;
    }

    // Always copies C++ value (not pointer, and not reference) to a new Python wrapper.
    s << endl << "// C++ to Python copy conversion." << endl;
    if (!classContext.forSmartPointer())
        targetTypeName = metaClass->name();
    else
        targetTypeName = classContext.preciseType()->name();

    sourceTypeName = targetTypeName + QLatin1String("_COPY");

    code.clear();

    QString computedWrapperName;
    if (!classContext.forSmartPointer())
        computedWrapperName = wrapperName(metaClass);
    else
        computedWrapperName = wrapperName(classContext.preciseType());

    c << INDENT << "return Shiboken::Object::newObject(&" << cpythonType << ", new ::" << computedWrapperName;
    c << "(*((" << typeName << "*)cppIn)), true, true);";
    writeCppToPythonFunction(s, code, sourceTypeName, targetTypeName);
    s << endl;

    // Python to C++ copy conversion.
    s << "// Python to C++ copy conversion." << endl;
    if (!classContext.forSmartPointer())
        sourceTypeName = metaClass->name();
    else
        sourceTypeName = classContext.preciseType()->name();

    targetTypeName = QStringLiteral("%1_COPY").arg(sourceTypeName);
    code.clear();

    QString pyInVariable = QLatin1String("pyIn");
    QString wrappedCPtrExpression;
    if (!classContext.forSmartPointer())
        wrappedCPtrExpression = cpythonWrapperCPtr(metaClass->typeEntry(), pyInVariable);
    else
        wrappedCPtrExpression = cpythonWrapperCPtr(classContext.preciseType(), pyInVariable);

    c << INDENT << "*((" << typeName << "*)cppOut) = *"
        << wrappedCPtrExpression << ';';
    writePythonToCppFunction(s, code, sourceTypeName, targetTypeName);

    // "Is convertible" function for the Python object to C++ value copy conversion.
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, pyTypeCheck);
    s << endl;

    // User provided implicit conversions.
    CustomConversion* customConversion = metaClass->typeEntry()->customConversion();

    // Implicit conversions.
    AbstractMetaFunctionList implicitConvs;
    if (!customConversion || !customConversion->replaceOriginalTargetToNativeConversions()) {
        foreach (AbstractMetaFunction* func, implicitConversions(metaClass->typeEntry())) {
            if (!func->isUserAdded())
                implicitConvs << func;
        }
    }

    if (!implicitConvs.isEmpty())
        s << "// Implicit conversions." << endl;

    AbstractMetaType* targetType = buildAbstractMetaTypeFromAbstractMetaClass(metaClass);
    foreach (const AbstractMetaFunction* conv, implicitConvs) {
        if (conv->isModifiedRemoved())
            continue;

        QString typeCheck;
        QString toCppConv;
        QString toCppPreConv;
        if (conv->isConversionOperator()) {
            const AbstractMetaClass* sourceClass = conv->ownerClass();
            typeCheck = QStringLiteral("PyObject_TypeCheck(pyIn, %1)").arg(cpythonTypeNameExt(sourceClass->typeEntry()));
            toCppConv = QLatin1Char('*') + cpythonWrapperCPtr(sourceClass->typeEntry(), QLatin1String("pyIn"));
        } else {
            // Constructor that does implicit conversion.
            if (!conv->typeReplaced(1).isEmpty())
                continue;
            const AbstractMetaType* sourceType = conv->arguments().first()->type();
            typeCheck = cpythonCheckFunction(sourceType);
            bool isUserPrimitiveWithoutTargetLangName = isUserPrimitive(sourceType)
                                                        && sourceType->typeEntry()->targetLangApiName() == sourceType->typeEntry()->name();
            if (!isWrapperType(sourceType)
                && !isUserPrimitiveWithoutTargetLangName
                && !sourceType->typeEntry()->isEnum()
                && !sourceType->typeEntry()->isFlags()
                && !sourceType->typeEntry()->isContainer()) {
                typeCheck += QLatin1Char('(');
            }
            if (isWrapperType(sourceType)) {
                typeCheck += QLatin1String("pyIn)");
                toCppConv = (sourceType->referenceType() == LValueReference || !isPointerToWrapperType(sourceType))
                    ? QLatin1String("*") : QString();
                toCppConv += cpythonWrapperCPtr(sourceType->typeEntry(), QLatin1String("pyIn"));
            } else if (typeCheck.contains(QLatin1String("%in"))) {
                typeCheck.replace(QLatin1String("%in"), QLatin1String("pyIn"));
                typeCheck.append(QLatin1Char(')'));
            } else {
                typeCheck += QLatin1String("pyIn)");
            }

            if (isUserPrimitive(sourceType)
                || isCppPrimitive(sourceType)
                || sourceType->typeEntry()->isContainer()
                || sourceType->typeEntry()->isEnum()
                || sourceType->typeEntry()->isFlags()) {
                QTextStream pc(&toCppPreConv);
                pc << INDENT << getFullTypeNameWithoutModifiers(sourceType) << " cppIn";
                writeMinimalConstructorExpression(pc, sourceType);
                pc << ';' << endl;
                writeToCppConversion(pc, sourceType, 0, QLatin1String("pyIn"), QLatin1String("cppIn"));
                pc << ';';
                toCppConv.append(QLatin1String("cppIn"));
            } else if (!isWrapperType(sourceType)) {
                QTextStream tcc(&toCppConv);
                writeToCppConversion(tcc, sourceType, metaClass, QLatin1String("pyIn"), QLatin1String("/*BOZO-1061*/"));
            }


        }
        const AbstractMetaType* sourceType = conv->isConversionOperator()
                                             ? buildAbstractMetaTypeFromAbstractMetaClass(conv->ownerClass())
                                             : conv->arguments().first()->type();
        writePythonToCppConversionFunctions(s, sourceType, targetType, typeCheck, toCppConv, toCppPreConv);
    }

    writeCustomConverterFunctions(s, customConversion);
}

void CppGenerator::writeCustomConverterFunctions(QTextStream& s, const CustomConversion* customConversion)
{
    if (!customConversion)
        return;
    const CustomConversion::TargetToNativeConversions& toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;
    s << "// Python to C++ conversions for type '" << customConversion->ownerType()->qualifiedCppName() << "'." << endl;
    foreach (CustomConversion::TargetToNativeConversion* toNative, toCppConversions)
        writePythonToCppConversionFunctions(s, toNative, customConversion->ownerType());
    s << endl;
}

void CppGenerator::writeConverterRegister(QTextStream &s, const AbstractMetaClass *metaClass,
                                          GeneratorContext &classContext)
{
    if (metaClass->isNamespace())
        return;
    s << INDENT << "// Register Converter" << endl;
    s << INDENT << "SbkConverter* converter = Shiboken::Conversions::createConverter(&";
    s << cpythonTypeName(metaClass) << ',' << endl;
    {
        Indentation indent(INDENT);
        QString sourceTypeName = metaClass->name();
        QString targetTypeName = sourceTypeName + QLatin1String("_PTR");
        s << INDENT << pythonToCppFunctionName(sourceTypeName, targetTypeName) << ',' << endl;
        s << INDENT << convertibleToCppFunctionName(sourceTypeName, targetTypeName) << ',' << endl;
        std::swap(targetTypeName, sourceTypeName);
        s << INDENT << cppToPythonFunctionName(sourceTypeName, targetTypeName);
        if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer()) {
            s << ',' << endl;
            sourceTypeName = metaClass->name() + QLatin1String("_COPY");
            s << INDENT << cppToPythonFunctionName(sourceTypeName, targetTypeName);
        }
    }
    s << ");" << endl;

    s << endl;

    QStringList cppSignature;
    if (!classContext.forSmartPointer()) {
        cppSignature = metaClass->qualifiedCppName().split(QLatin1String("::"),
                                                                       QString::SkipEmptyParts);
    } else {
        cppSignature = classContext.preciseType()->cppSignature().split(QLatin1String("::"),
                                                                        QString::SkipEmptyParts);
    }
    while (!cppSignature.isEmpty()) {
        QString signature = cppSignature.join(QLatin1String("::"));
        s << INDENT << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "\");" << endl;
        s << INDENT << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "*\");" << endl;
        s << INDENT << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "&\");" << endl;
        cppSignature.removeFirst();
    }

    s << INDENT << "Shiboken::Conversions::registerConverterName(converter, typeid(::";
    QString qualifiedCppNameInvocation;
    if (!classContext.forSmartPointer())
        qualifiedCppNameInvocation = metaClass->qualifiedCppName();
    else
        qualifiedCppNameInvocation = classContext.preciseType()->cppSignature();

    s << qualifiedCppNameInvocation << ").name());" << endl;

    if (shouldGenerateCppWrapper(metaClass)) {
        s << INDENT << "Shiboken::Conversions::registerConverterName(converter, typeid(::";
        s << wrapperName(metaClass) << ").name());" << endl;
    }

    s << endl;

    if (!metaClass->typeEntry()->isValue() && !metaClass->typeEntry()->isSmartPointer())
        return;

    // Python to C++ copy (value, not pointer neither reference) conversion.
    s << INDENT << "// Add Python to C++ copy (value, not pointer neither reference) conversion to type converter." << endl;
    QString sourceTypeName = metaClass->name();
    QString targetTypeName = sourceTypeName + QLatin1String("_COPY");
    QString toCpp = pythonToCppFunctionName(sourceTypeName, targetTypeName);
    QString isConv = convertibleToCppFunctionName(sourceTypeName, targetTypeName);
    writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);

    // User provided implicit conversions.
    CustomConversion* customConversion = metaClass->typeEntry()->customConversion();

    // Add implicit conversions.
    AbstractMetaFunctionList implicitConvs;
    if (!customConversion || !customConversion->replaceOriginalTargetToNativeConversions()) {
        foreach (AbstractMetaFunction* func, implicitConversions(metaClass->typeEntry())) {
            if (!func->isUserAdded())
                implicitConvs << func;
        }
    }

    if (!implicitConvs.isEmpty())
        s << INDENT << "// Add implicit conversions to type converter." << endl;

    AbstractMetaType* targetType = buildAbstractMetaTypeFromAbstractMetaClass(metaClass);
    foreach (const AbstractMetaFunction* conv, implicitConvs) {
        if (conv->isModifiedRemoved())
            continue;
        const AbstractMetaType* sourceType;
        if (conv->isConversionOperator()) {
            sourceType = buildAbstractMetaTypeFromAbstractMetaClass(conv->ownerClass());
        } else {
            // Constructor that does implicit conversion.
            if (!conv->typeReplaced(1).isEmpty())
                continue;
            sourceType = conv->arguments().first()->type();
        }
        QString toCpp = pythonToCppFunctionName(sourceType, targetType);
        QString isConv = convertibleToCppFunctionName(sourceType, targetType);
        writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
    }

    writeCustomConverterRegister(s, customConversion, QLatin1String("converter"));
}

void CppGenerator::writeCustomConverterRegister(QTextStream& s, const CustomConversion* customConversion, const QString& converterVar)
{
    if (!customConversion)
        return;
    const CustomConversion::TargetToNativeConversions& toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;
    s << INDENT << "// Add user defined implicit conversions to type converter." << endl;
    foreach (CustomConversion::TargetToNativeConversion* toNative, toCppConversions) {
        QString toCpp = pythonToCppFunctionName(toNative, customConversion->ownerType());
        QString isConv = convertibleToCppFunctionName(toNative, customConversion->ownerType());
        writeAddPythonToCppConversion(s, converterVar, toCpp, isConv);
    }
}

void CppGenerator::writeContainerConverterRegister(QTextStream& s, const AbstractMetaType* container, const QString& converterVar)
{
    s << INDENT << "// Add user defined container conversion to type converter." << endl;
    QString typeName = fixedCppTypeName(container);
    QString toCpp = pythonToCppFunctionName(typeName, typeName);
    QString isConv = convertibleToCppFunctionName(typeName, typeName);
    writeAddPythonToCppConversion(s, converterVar, toCpp, isConv);
}

void CppGenerator::writeContainerConverterFunctions(QTextStream& s, const AbstractMetaType* containerType)
{
    writeCppToPythonFunction(s, containerType);
    writePythonToCppConversionFunctions(s, containerType);
}

void CppGenerator::writeMethodWrapperPreamble(QTextStream &s, OverloadData &overloadData,
                                              GeneratorContext &context)
{
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    const AbstractMetaClass* ownerClass = rfunc->ownerClass();
    int minArgs = overloadData.minArgs();
    int maxArgs = overloadData.maxArgs();
    bool initPythonArguments;
    bool usesNamedArguments;

    // If method is a constructor...
    if (rfunc->isConstructor()) {
        // Check if the right constructor was called.
        if (!ownerClass->hasPrivateDestructor()) {
            s << INDENT;
            s << "if (Shiboken::Object::isUserType(" PYTHON_SELF_VAR ") && !Shiboken::ObjectType::canCallConstructor(" PYTHON_SELF_VAR "->ob_type, Shiboken::SbkType< ::";
            QString qualifiedCppName;
            if (!context.forSmartPointer())
                qualifiedCppName = ownerClass->qualifiedCppName();
            else
                qualifiedCppName = context.preciseType()->cppSignature();

            s << qualifiedCppName << " >()))" << endl;
            Indentation indent(INDENT);
            s << INDENT << "return " << m_currentErrorCode << ';' << endl << endl;
        }
        // Declare pointer for the underlying C++ object.
        s << INDENT << "::";
        if (!context.forSmartPointer()) {
            s << (shouldGenerateCppWrapper(ownerClass) ? wrapperName(ownerClass)
                                                       : ownerClass->qualifiedCppName());
        } else {
            s << context.preciseType()->cppSignature();
        }
        s << "* cptr = 0;" << endl;

        initPythonArguments = maxArgs > 0;
        usesNamedArguments = !ownerClass->isQObject() && overloadData.hasArgumentWithDefaultValue();

    } else {
        if (rfunc->implementingClass() &&
            (!rfunc->implementingClass()->isNamespace() && overloadData.hasInstanceFunction())) {
            writeCppSelfDefinition(s, rfunc, context, overloadData.hasStaticFunction());
        }
        if (!rfunc->isInplaceOperator() && overloadData.hasNonVoidReturnType())
            s << INDENT << "PyObject* " PYTHON_RETURN_VAR " = 0;" << endl;

        initPythonArguments = minArgs != maxArgs || maxArgs > 1;
        usesNamedArguments = rfunc->isCallOperator() || overloadData.hasArgumentWithDefaultValue();
    }

    if (maxArgs > 0) {
        s << INDENT << "int overloadId = -1;" << endl;
        s << INDENT << "PythonToCppFunc " PYTHON_TO_CPP_VAR;
        if (pythonFunctionWrapperUsesListOfArguments(overloadData))
            s << "[] = { 0" << QString::fromLatin1(", 0").repeated(maxArgs-1) << " }";
        s << ';' << endl;
        writeUnusedVariableCast(s, QLatin1String(PYTHON_TO_CPP_VAR));
    }

    if (usesNamedArguments && !rfunc->isCallOperator())
        s << INDENT << "int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);" << endl;

    if (initPythonArguments) {
        s << INDENT << "int numArgs = ";
        if (minArgs == 0 && maxArgs == 1 && !rfunc->isConstructor() && !pythonFunctionWrapperUsesListOfArguments(overloadData))
            s << "(" PYTHON_ARG " == 0 ? 0 : 1);" << endl;
        else
            writeArgumentsInitializer(s, overloadData);
    }
}

void CppGenerator::writeConstructorWrapper(QTextStream &s, const AbstractMetaFunctionList overloads,
                                           GeneratorContext &classContext)
{
    ErrorCode errorCode(-1);
    OverloadData overloadData(overloads, this);

    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    const AbstractMetaClass* metaClass = rfunc->ownerClass();

    s << "static int" << endl;
    s << cpythonFunctionName(rfunc) << "(PyObject* " PYTHON_SELF_VAR ", PyObject* args, PyObject* kwds)" << endl;
    s << '{' << endl;

    QSet<QString> argNamesSet;
    if (usePySideExtensions() && metaClass->isQObject()) {
        // Write argNames variable with all known argument names.
        foreach (const AbstractMetaFunction* func, overloadData.overloads()) {
            foreach (const AbstractMetaArgument* arg, func->arguments()) {
                if (arg->defaultValueExpression().isEmpty() || func->argumentRemoved(arg->argumentIndex() + 1))
                    continue;
                argNamesSet << arg->name();
            }
        }
        QStringList argNamesList = argNamesSet.toList();
        qSort(argNamesList.begin(), argNamesList.end());
        if (argNamesList.isEmpty()) {
            s << INDENT << "const char** argNames = 0;" << endl;
        } else {
            s << INDENT << "const char* argNames[] = {\""
                << argNamesList.join(QLatin1String("\", \"")) << "\"};" << endl;
        }
        s << INDENT << "const QMetaObject* metaObject;" << endl;
    }

    s << INDENT << "SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR ");" << endl;

    if (metaClass->isAbstract() || metaClass->baseClassNames().size() > 1) {
        s << INDENT << "SbkObjectType* type = reinterpret_cast<SbkObjectType*>(" PYTHON_SELF_VAR "->ob_type);" << endl;
        s << INDENT << "SbkObjectType* myType = reinterpret_cast<SbkObjectType*>(" << cpythonTypeNameExt(metaClass->typeEntry()) << ");" << endl;
    }

    if (metaClass->isAbstract()) {
        s << INDENT << "if (type == myType) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError," << endl;
            {
                Indentation indentation(INDENT);
                s << INDENT << "\"'" << metaClass->qualifiedCppName();
            }
            s << "' represents a C++ abstract class and cannot be instantiated\");" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
        s << INDENT << '}' << endl << endl;
    }

    if (metaClass->baseClassNames().size() > 1) {
        if (!metaClass->isAbstract()) {
            s << INDENT << "if (type != myType) {" << endl;
        }
        {
            Indentation indentation(INDENT);
            s << INDENT << "Shiboken::ObjectType::copyMultimpleheritance(type, myType);" << endl;
        }
        if (!metaClass->isAbstract())
            s << INDENT << '}' << endl << endl;
    }

    writeMethodWrapperPreamble(s, overloadData, classContext);

    s << endl;

    if (overloadData.maxArgs() > 0)
        writeOverloadedFunctionDecisor(s, overloadData);

    writeFunctionCalls(s, overloadData, classContext);
    s << endl;

    s << INDENT << "if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::" << metaClass->qualifiedCppName() << " >(), cptr)) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "delete cptr;" << endl;
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << INDENT << '}' << endl;
    if (overloadData.maxArgs() > 0) {
        s << INDENT << "if (!cptr) goto " << cpythonFunctionName(rfunc) << "_TypeError;" << endl;
        s << endl;
    }

    s << INDENT << "Shiboken::Object::setValidCpp(sbkSelf, true);" << endl;
    // If the created C++ object has a C++ wrapper the ownership is assigned to Python
    // (first "1") and the flag indicating that the Python wrapper holds an C++ wrapper
    // is marked as true (the second "1"). Otherwise the default values apply:
    // Python owns it and C++ wrapper is false.
    if (shouldGenerateCppWrapper(overloads.first()->ownerClass()))
        s << INDENT << "Shiboken::Object::setHasCppWrapper(sbkSelf, true);" << endl;
    // Need to check if a wrapper for same pointer is already registered
    // Caused by bug PYSIDE-217, where deleted objects' wrappers are not released
    s << INDENT << "if (Shiboken::BindingManager::instance().hasWrapper(cptr)) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "Shiboken::BindingManager::instance().releaseWrapper(Shiboken::BindingManager::instance().retrieveWrapper(cptr));" << endl;
    }
    s << INDENT << "}" << endl;
    s << INDENT << "Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);" << endl;

    // Create metaObject and register signal/slot
    if (metaClass->isQObject() && usePySideExtensions()) {
        s << endl << INDENT << "// QObject setup" << endl;
        s << INDENT << "PySide::Signal::updateSourceObject(" PYTHON_SELF_VAR ");" << endl;
        s << INDENT << "metaObject = cptr->metaObject(); // <- init python qt properties" << endl;
        s << INDENT << "if (kwds && !PySide::fillQtProperties(" PYTHON_SELF_VAR ", metaObject, kwds, argNames, " << argNamesSet.count() << "))" << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
    }

    // Constructor code injections, position=end
    bool hasCodeInjectionsAtEnd = false;
    foreach(AbstractMetaFunction* func, overloads) {
        foreach (const CodeSnip &cs, func->injectedCodeSnips()) {
            if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                hasCodeInjectionsAtEnd = true;
                break;
            }
        }
    }
    if (hasCodeInjectionsAtEnd) {
        // FIXME: C++ arguments are not available in code injection on constructor when position = end.
        s << INDENT << "switch(overloadId) {" << endl;
        foreach(AbstractMetaFunction* func, overloads) {
            Indentation indent(INDENT);
            foreach (const CodeSnip &cs, func->injectedCodeSnips()) {
                if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                    s << INDENT << "case " << metaClass->functions().indexOf(func) << ':' << endl;
                    s << INDENT << '{' << endl;
                    {
                        Indentation indent(INDENT);
                        writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode, func);
                    }
                    s << INDENT << '}' << endl;
                    break;
                }
            }
        }
        s << '}' << endl;
    }

    s << endl;
    s << endl << INDENT << "return 1;" << endl;
    if (overloadData.maxArgs() > 0)
        writeErrorSection(s, overloadData);
    s << '}' << endl << endl;
}

void CppGenerator::writeMethodWrapper(QTextStream &s, const AbstractMetaFunctionList overloads,
                                      GeneratorContext &classContext)
{
    OverloadData overloadData(overloads, this);
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();

    int maxArgs = overloadData.maxArgs();

    s << "static PyObject* ";
    s << cpythonFunctionName(rfunc) << "(PyObject* " PYTHON_SELF_VAR;
    if (maxArgs > 0) {
        s << ", PyObject* " << (pythonFunctionWrapperUsesListOfArguments(overloadData) ? "args" : PYTHON_ARG);
        if (overloadData.hasArgumentWithDefaultValue() || rfunc->isCallOperator())
            s << ", PyObject* kwds";
    }
    s << ')' << endl << '{' << endl;

    writeMethodWrapperPreamble(s, overloadData, classContext);

    s << endl;

    /*
     * Make sure reverse <</>> operators defined in other classes (specially from other modules)
     * are called. A proper and generic solution would require an reengineering in the operator
     * system like the extended converters.
     *
     * Solves #119 - QDataStream <</>> operators not working for QPixmap
     * http://bugs.openbossa.org/show_bug.cgi?id=119
     */
    bool hasReturnValue = overloadData.hasNonVoidReturnType();
    bool callExtendedReverseOperator = hasReturnValue
                                       && !rfunc->isInplaceOperator()
                                       && !rfunc->isCallOperator()
                                       && rfunc->isOperatorOverload();
    if (callExtendedReverseOperator) {
        QString revOpName = ShibokenGenerator::pythonOperatorFunctionName(rfunc).insert(2, QLatin1Char('r'));
        if (rfunc->isBinaryOperator()) {
            s << INDENT << "if (!isReverse" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "&& Shiboken::Object::checkType(" PYTHON_ARG ")" << endl;
                s << INDENT << "&& !PyObject_TypeCheck(" PYTHON_ARG ", " PYTHON_SELF_VAR "->ob_type)" << endl;
                s << INDENT << "&& PyObject_HasAttrString(" PYTHON_ARG ", const_cast<char*>(\"" << revOpName << "\"))) {" << endl;

                // This PyObject_CallMethod call will emit lots of warnings like
                // "deprecated conversion from string constant to char *" during compilation
                // due to the method name argument being declared as "char*" instead of "const char*"
                // issue 6952 http://bugs.python.org/issue6952
                s << INDENT << "PyObject* revOpMethod = PyObject_GetAttrString(" PYTHON_ARG ", const_cast<char*>(\"" << revOpName << "\"));" << endl;
                s << INDENT << "if (revOpMethod && PyCallable_Check(revOpMethod)) {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << PYTHON_RETURN_VAR " = PyObject_CallFunction(revOpMethod, const_cast<char*>(\"O\"), " PYTHON_SELF_VAR ");" << endl;
                    s << INDENT << "if (PyErr_Occurred() && (PyErr_ExceptionMatches(PyExc_NotImplementedError)";
                    s << " || PyErr_ExceptionMatches(PyExc_AttributeError))) {" << endl;
                    {
                        Indentation indent(INDENT);
                        s << INDENT << "PyErr_Clear();" << endl;
                        s << INDENT << "Py_XDECREF(" PYTHON_RETURN_VAR ");" << endl;
                        s << INDENT << PYTHON_RETURN_VAR " = 0;" << endl;
                    }
                    s << INDENT << '}' << endl;
                }
                s << INDENT << "}" << endl;
                s << INDENT << "Py_XDECREF(revOpMethod);" << endl << endl;
            }
            s << INDENT << "}" << endl;
        }
        s << INDENT << "// Do not enter here if other object has implemented a reverse operator." << endl;
        s << INDENT << "if (!" PYTHON_RETURN_VAR ") {" << endl << endl;
    }

    if (maxArgs > 0)
        writeOverloadedFunctionDecisor(s, overloadData);

    writeFunctionCalls(s, overloadData, classContext);

    if (callExtendedReverseOperator)
        s << endl << INDENT << "} // End of \"if (!" PYTHON_RETURN_VAR ")\"" << endl;

    s << endl;

    writeFunctionReturnErrorCheckSection(s, hasReturnValue && !rfunc->isInplaceOperator());

    if (hasReturnValue) {
        if (rfunc->isInplaceOperator()) {
            s << INDENT << "Py_INCREF(" PYTHON_SELF_VAR ");\n";
            s << INDENT << "return " PYTHON_SELF_VAR ";\n";
        } else {
            s << INDENT << "return " PYTHON_RETURN_VAR ";\n";
        }
    } else {
        s << INDENT << "Py_RETURN_NONE;" << endl;
    }

    if (maxArgs > 0)
        writeErrorSection(s, overloadData);

    s << '}' << endl << endl;
}

void CppGenerator::writeArgumentsInitializer(QTextStream& s, OverloadData& overloadData)
{
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    s << "PyTuple_GET_SIZE(args);" << endl;

    int minArgs = overloadData.minArgs();
    int maxArgs = overloadData.maxArgs();

    s << INDENT << "PyObject* ";
    s << PYTHON_ARGS "[] = {"
        << QString(maxArgs, QLatin1Char('0')).split(QLatin1String(""), QString::SkipEmptyParts).join(QLatin1String(", "))
        << "};" << endl;
    s << endl;

    if (overloadData.hasVarargs()) {
        maxArgs--;
        if (minArgs > maxArgs)
            minArgs = maxArgs;

        s << INDENT << "PyObject* nonvarargs = PyTuple_GetSlice(args, 0, " << maxArgs << ");" << endl;
        s << INDENT << "Shiboken::AutoDecRef auto_nonvarargs(nonvarargs);" << endl;
        s << INDENT << PYTHON_ARGS "[" << maxArgs << "] = PyTuple_GetSlice(args, " << maxArgs << ", numArgs);" << endl;
        s << INDENT << "Shiboken::AutoDecRef auto_varargs(" PYTHON_ARGS "[" << maxArgs << "]);" << endl;
        s << endl;
    }

    bool usesNamedArguments = overloadData.hasArgumentWithDefaultValue();

    s << INDENT << "// invalid argument lengths" << endl;
    bool ownerClassIsQObject = rfunc->ownerClass() && rfunc->ownerClass()->isQObject() && rfunc->isConstructor();
    if (usesNamedArguments) {
        if (!ownerClassIsQObject) {
            s << INDENT << "if (numArgs" << (overloadData.hasArgumentWithDefaultValue() ? " + numNamedArgs" : "") << " > " << maxArgs << ") {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyErr_SetString(PyExc_TypeError, \"" << fullPythonFunctionName(rfunc) << "(): too many arguments\");" << endl;
                s << INDENT << "return " << m_currentErrorCode << ';' << endl;
            }
            s << INDENT << '}';
        }
        if (minArgs > 0) {
            if (ownerClassIsQObject)
                s << INDENT;
            else
                s << " else ";
            s << "if (numArgs < " << minArgs << ") {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyErr_SetString(PyExc_TypeError, \"" << fullPythonFunctionName(rfunc) << "(): not enough arguments\");" << endl;
                s << INDENT << "return " << m_currentErrorCode << ';' << endl;
            }
            s << INDENT << '}';
        }
    }
    QList<int> invalidArgsLength = overloadData.invalidArgumentLengths();
    if (!invalidArgsLength.isEmpty()) {
        QStringList invArgsLen;
        foreach (int i, invalidArgsLength)
            invArgsLen << QStringLiteral("numArgs == %1").arg(i);
        if (usesNamedArguments && (!ownerClassIsQObject || minArgs > 0))
            s << " else ";
        else
            s << INDENT;
        s << "if (" << invArgsLen.join(QLatin1String(" || ")) << ")" << endl;
        Indentation indent(INDENT);
        s << INDENT << "goto " << cpythonFunctionName(rfunc) << "_TypeError;";
    }
    s << endl << endl;

    QString funcName;
    if (rfunc->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
    else
        funcName = rfunc->name();

    QString argsVar = overloadData.hasVarargs() ?  QLatin1String("nonvarargs") : QLatin1String("args");
    s << INDENT << "if (!";
    if (usesNamedArguments)
        s << "PyArg_ParseTuple(" << argsVar << ", \"|" << QByteArray(maxArgs, 'O') << ':' << funcName << '"';
    else
        s << "PyArg_UnpackTuple(" << argsVar << ", \"" << funcName << "\", " << minArgs << ", " << maxArgs;
    QStringList palist;
    for (int i = 0; i < maxArgs; i++)
        palist << QString::fromLatin1("&(" PYTHON_ARGS "[%1])").arg(i);
    s << ", " << palist.join(QLatin1String(", ")) << "))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << endl;
}

void CppGenerator::writeCppSelfDefinition(QTextStream &s,
                                          GeneratorContext &context,
                                          bool hasStaticOverload,
                                          bool cppSelfAsReference)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    bool useWrapperClass = avoidProtectedHack() && metaClass->hasProtectedMembers();
    QString className;
    if (!context.forSmartPointer()) {
        className = useWrapperClass
            ? wrapperName(metaClass)
            : (QLatin1String("::") + metaClass->qualifiedCppName());
    } else {
        className = context.preciseType()->cppSignature();
    }

    QString cppSelfAttribution;
    QString pythonSelfVar = QLatin1String(PYTHON_SELF_VAR);
    QString cpythonWrapperCPtrResult;
    if (!context.forSmartPointer())
        cpythonWrapperCPtrResult = cpythonWrapperCPtr(metaClass, pythonSelfVar);
    else
        cpythonWrapperCPtrResult = cpythonWrapperCPtr(context.preciseType(), pythonSelfVar);

    if (cppSelfAsReference) {
        QString cast = useWrapperClass ? QString::fromLatin1("(%1*)").arg(className) : QString();
        cppSelfAttribution = QString::fromLatin1("%1& %2 = *(%3%4)")
                                .arg(className, QLatin1String(CPP_SELF_VAR), cast,
                                     cpythonWrapperCPtrResult);
    } else {
        s << INDENT << className << "* " CPP_SELF_VAR " = 0;" << endl;
        writeUnusedVariableCast(s, QLatin1String(CPP_SELF_VAR));
        cppSelfAttribution = QString::fromLatin1("%1 = %2%3")
                                .arg(QLatin1String(CPP_SELF_VAR),
                                     (useWrapperClass ? QString::fromLatin1("(%1*)").arg(className) : QString()),
                                     cpythonWrapperCPtrResult);
    }

    // Checks if the underlying C++ object is valid.
    if (hasStaticOverload && !cppSelfAsReference) {
        s << INDENT << "if (" PYTHON_SELF_VAR ") {" << endl;
        {
            Indentation indent(INDENT);
            writeInvalidPyObjectCheck(s, QLatin1String(PYTHON_SELF_VAR));
            s << INDENT << cppSelfAttribution << ';' << endl;
        }
        s << INDENT << '}' << endl;
        return;
    }

    writeInvalidPyObjectCheck(s, QLatin1String(PYTHON_SELF_VAR));
    s << INDENT << cppSelfAttribution << ';' << endl;
}

void CppGenerator::writeCppSelfDefinition(QTextStream &s,
                                          const AbstractMetaFunction *func,
                                          GeneratorContext &context,
                                          bool hasStaticOverload)
{
    if (!func->ownerClass() || func->isConstructor())
        return;

    if (func->isOperatorOverload() && func->isBinaryOperator()) {
        QString checkFunc = cpythonCheckFunction(func->ownerClass()->typeEntry());
        s << INDENT << "bool isReverse = " << checkFunc << PYTHON_ARG ")" << endl;
        {
            Indentation indent1(INDENT);
            Indentation indent2(INDENT);
            Indentation indent3(INDENT);
            Indentation indent4(INDENT);
            s << INDENT << "&& !" << checkFunc << PYTHON_SELF_VAR ");" << endl;
        }
        s << INDENT << "if (isReverse)" << endl;
        Indentation indent(INDENT);
        s << INDENT << "std::swap(" PYTHON_SELF_VAR ", " PYTHON_ARG ");" << endl;
    }

    writeCppSelfDefinition(s, context, hasStaticOverload);
}

void CppGenerator::writeErrorSection(QTextStream& s, OverloadData& overloadData)
{
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    s << endl << INDENT << cpythonFunctionName(rfunc) << "_TypeError:" << endl;
    Indentation indentation(INDENT);
    QString funcName = fullPythonFunctionName(rfunc);

    QString argsVar = pythonFunctionWrapperUsesListOfArguments(overloadData)
        ? QLatin1String("args") : QLatin1String(PYTHON_ARG);
    if (verboseErrorMessagesDisabled()) {
        s << INDENT << "Shiboken::setErrorAboutWrongArguments(" << argsVar << ", \"" << funcName << "\", 0);" << endl;
    } else {
        QStringList overloadSignatures;
        foreach (const AbstractMetaFunction* f, overloadData.overloads()) {
            QStringList args;
            foreach(AbstractMetaArgument* arg, f->arguments()) {
                QString strArg;
                AbstractMetaType* argType = arg->type();
                if (isCString(argType)) {
                    strArg = QLatin1String("\" SBK_STR_NAME \"");
                } else if (argType->isPrimitive()) {
                    const PrimitiveTypeEntry* ptp = reinterpret_cast<const PrimitiveTypeEntry*>(argType->typeEntry());
                    while (ptp->referencedTypeEntry())
                        ptp = ptp->referencedTypeEntry();
                    strArg = ptp->name();
                    if (strArg == QLatin1String("QString")) {
                        strArg = QLatin1String("unicode");
                    } else if (strArg == QLatin1String("QChar")) {
                        strArg = QLatin1String("1-unicode");
                    } else {
                        strArg = ptp->name();
                        strArg.remove(QRegExp(QLatin1String("^signed\\s+")));
                        if (strArg == QLatin1String("double"))
                            strArg = QLatin1String("float");
                    }
                } else if (argType->typeEntry()->isContainer()) {
                    strArg = argType->fullName();
                    if (strArg == QLatin1String("QList") || strArg == QLatin1String("QVector")
                        || strArg == QLatin1String("QLinkedList") || strArg == QLatin1String("QStack")
                        || strArg == QLatin1String("QQueue")) {
                        strArg = QLatin1String("list");
                    } else if (strArg == QLatin1String("QMap") || strArg == QLatin1String("QHash")
                               || strArg == QLatin1String("QMultiMap") || strArg == QLatin1String("QMultiHash")) {
                        strArg = QLatin1String("dict");
                    } else if (strArg == QLatin1String("QPair")) {
                        strArg = QLatin1String("2-tuple");
                    }
                } else {
                    strArg = argType->fullName();
                    if (strArg == QLatin1String("PyUnicode"))
                        strArg = QLatin1String("unicode");
                    else if (strArg == QLatin1String("PyString"))
                        strArg = QLatin1String("str");
                    else if (strArg == QLatin1String("PyBytes"))
                        strArg = QLatin1String("\" SBK_STR_NAME \"");
                    else if (strArg == QLatin1String("PySequence"))
                        strArg = QLatin1String("list");
                    else if (strArg == QLatin1String("PyTuple"))
                        strArg = QLatin1String("tuple");
                    else if (strArg == QLatin1String("PyDict"))
                        strArg = QLatin1String("dict");
                    else if (strArg == QLatin1String("PyObject"))
                        strArg = QLatin1String("object");
                    else if (strArg == QLatin1String("PyCallable"))
                        strArg = QLatin1String("callable");
                    else if (strArg == QLatin1String("uchar"))
                        strArg = QLatin1String("buffer"); // This depends on an inject code to be true, but if it's not true
                                           // the function wont work at all, so it must be true.
                }
                if (!arg->defaultValueExpression().isEmpty()) {
                    strArg += QLatin1String(" = ");
                    if ((isCString(argType) || isPointerToWrapperType(argType))
                        && arg->defaultValueExpression() == QLatin1String("0")) {
                        strArg += QLatin1String("None");
                    } else {
                        QString e = arg->defaultValueExpression();
                        e.replace(QLatin1String("::"), QLatin1String("."));
                        e.replace(QLatin1String("\""), QLatin1String("\\\""));
                        strArg += e;
                    }
                }
                args << strArg;
            }
            overloadSignatures << QLatin1Char('"') + args.join(QLatin1String(", ")) + QLatin1Char('"');
        }
        s << INDENT << "const char* overloads[] = {" << overloadSignatures.join(QLatin1String(", "))
              << ", 0};" << endl;
        s << INDENT << "Shiboken::setErrorAboutWrongArguments(" << argsVar << ", \"" << funcName << "\", overloads);" << endl;
    }
    s << INDENT << "return " << m_currentErrorCode << ';' << endl;
}

void CppGenerator::writeFunctionReturnErrorCheckSection(QTextStream& s, bool hasReturnValue)
{
    s << INDENT << "if (PyErr_Occurred()" << (hasReturnValue ? " || !" PYTHON_RETURN_VAR : "") << ") {" << endl;
    {
        Indentation indent(INDENT);
        if (hasReturnValue)
            s << INDENT << "Py_XDECREF(" PYTHON_RETURN_VAR ");" << endl;
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << INDENT << '}' << endl;
}

void CppGenerator::writeInvalidPyObjectCheck(QTextStream& s, const QString& pyObj)
{
    s << INDENT << "if (!Shiboken::Object::isValid(" << pyObj << "))" << endl;
    Indentation indent(INDENT);
    s << INDENT << "return " << m_currentErrorCode << ';' << endl;
}

static QString pythonToCppConverterForArgumentName(const QString& argumentName)
{
    static QRegExp pyArgsRegex(QLatin1String(PYTHON_ARGS"(\\[\\d+[-]?\\d*\\])"));
    pyArgsRegex.indexIn(argumentName);
    return QLatin1String(PYTHON_TO_CPP_VAR) + pyArgsRegex.cap(1);
}

void CppGenerator::writeTypeCheck(QTextStream& s, const AbstractMetaType* argType, QString argumentName, bool isNumber, QString customType, bool rejectNull)
{
    QString customCheck;
    if (!customType.isEmpty()) {
        AbstractMetaType* metaType;
        customCheck = guessCPythonCheckFunction(customType, &metaType);
        if (metaType)
            argType = metaType;
    }

    // TODO-CONVERTER: merge this with the code below.
    QString typeCheck;
    if (customCheck.isEmpty())
        typeCheck = cpythonIsConvertibleFunction(argType, argType->isEnum() ? false : isNumber);
    else
        typeCheck = customCheck;
    typeCheck.append(QString::fromLatin1("(%1)").arg(argumentName));

    // TODO-CONVERTER -----------------------------------------------------------------------
    if (customCheck.isEmpty() && !argType->typeEntry()->isCustom()) {
        typeCheck = QString::fromLatin1("(%1 = %2))").arg(pythonToCppConverterForArgumentName(argumentName), typeCheck);
        if (!isNumber && argType->typeEntry()->isCppPrimitive())
            typeCheck.prepend(QString::fromLatin1("%1(%2) && ").arg(cpythonCheckFunction(argType), argumentName));
    }
    // TODO-CONVERTER -----------------------------------------------------------------------

    if (rejectNull)
        typeCheck = QString::fromLatin1("(%1 != Py_None && %2)").arg(argumentName, typeCheck);

    s << typeCheck;
}

static void checkTypeViability(const AbstractMetaFunction* func, const AbstractMetaType* type, int argIdx)
{
    if (!type
        || !type->typeEntry()->isPrimitive()
        || type->indirections() == 0
        || ShibokenGenerator::isCString(type)
        || func->argumentRemoved(argIdx)
        || !func->typeReplaced(argIdx).isEmpty()
        || !func->conversionRule(TypeSystem::All, argIdx).isEmpty()
        || func->hasInjectedCode())
        return;
    QString prefix;
    if (func->ownerClass())
        prefix = func->ownerClass()->qualifiedCppName() + QLatin1String("::");
    qCWarning(lcShiboken).noquote().nospace()
        << QString::fromLatin1("There's no user provided way (conversion rule, argument removal, custom code, etc) "
                               "to handle the primitive %1 type '%2' in function '%3%4'.")
                               .arg(argIdx == 0 ? QStringLiteral("return") : QStringLiteral("argument"),
                               type->cppSignature(), prefix, func->signature());
}

static void checkTypeViability(const AbstractMetaFunction* func)
{
    if (func->isUserAdded())
        return;
    const AbstractMetaType* type = func->type();
    checkTypeViability(func, type, 0);
    for (int i = 0; i < func->arguments().count(); ++i)
        checkTypeViability(func, func->arguments().at(i)->type(), i + 1);
}

void CppGenerator::writeTypeCheck(QTextStream& s, const OverloadData* overloadData, QString argumentName)
{
    QSet<const TypeEntry*> numericTypes;

    foreach (OverloadData* od, overloadData->previousOverloadData()->nextOverloadData()) {
        foreach (const AbstractMetaFunction* func, od->overloads()) {
            checkTypeViability(func);
            const AbstractMetaType* argType = od->argument(func)->type();
            if (!argType->isPrimitive())
                continue;
            if (ShibokenGenerator::isNumber(argType->typeEntry()))
                numericTypes << argType->typeEntry();
        }
    }

    // This condition trusts that the OverloadData object will arrange for
    // PyInt type to come after the more precise numeric types (e.g. float and bool)
    const AbstractMetaType* argType = overloadData->argType();
    bool numberType = numericTypes.count() == 1 || ShibokenGenerator::isPyInt(argType);
    QString customType = (overloadData->hasArgumentTypeReplace() ? overloadData->argumentTypeReplaced() : QString());
    bool rejectNull = shouldRejectNullPointerArgument(overloadData->referenceFunction(), overloadData->argPos());
    writeTypeCheck(s, argType, argumentName, numberType, customType, rejectNull);
}

void CppGenerator::writeArgumentConversion(QTextStream& s,
                                           const AbstractMetaType* argType,
                                           const QString& argName, const QString& pyArgName,
                                           const AbstractMetaClass* context,
                                           const QString& defaultValue,
                                           bool castArgumentAsUnused)
{
    if (argType->typeEntry()->isCustom() || argType->typeEntry()->isVarargs())
        return;
    if (isWrapperType(argType))
        writeInvalidPyObjectCheck(s, pyArgName);
    writePythonToCppTypeConversion(s, argType, pyArgName, argName, context, defaultValue);
    if (castArgumentAsUnused)
        writeUnusedVariableCast(s, argName);
}

const AbstractMetaType* CppGenerator::getArgumentType(const AbstractMetaFunction* func, int argPos)
{
    if (argPos < 0 || argPos > func->arguments().size()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("Argument index for function '%1' out of range.").arg(func->signature());
        return 0;
    }

    const AbstractMetaType* argType = 0;
    QString typeReplaced = func->typeReplaced(argPos);
    if (typeReplaced.isEmpty())
        argType = (argPos == 0) ? func->type() : func->arguments().at(argPos-1)->type();
    else
        argType = buildAbstractMetaTypeFromString(typeReplaced);
    if (!argType && !m_knownPythonTypes.contains(typeReplaced)) {
        qCWarning(lcShiboken).noquote().nospace()
            << QString::fromLatin1("Unknown type '%1' used as argument type replacement "\
                                   "in function '%2', the generated code may be broken.")
                                   .arg(typeReplaced, func->signature());
    }
    return argType;
}

void CppGenerator::writePythonToCppTypeConversion(QTextStream& s,
                                                  const AbstractMetaType* type,
                                                  const QString& pyIn,
                                                  const QString& cppOut,
                                                  const AbstractMetaClass* /* context */,
                                                  const QString& defaultValue)
{
    const TypeEntry* typeEntry = type->typeEntry();
    if (typeEntry->isCustom() || typeEntry->isVarargs())
        return;

    QString cppOutAux = cppOut + QLatin1String("_local");

    bool treatAsPointer = isValueTypeWithCopyConstructorOnly(type);
    bool isPointerOrObjectType = (isObjectType(type) || isPointer(type)) && !isUserPrimitive(type) && !isCppPrimitive(type);
    bool isNotContainerEnumOrFlags = !typeEntry->isContainer() && !typeEntry->isEnum() && !typeEntry->isFlags();
    bool mayHaveImplicitConversion = type->referenceType() == LValueReference
                                     && !isUserPrimitive(type)
                                     && !isCppPrimitive(type)
                                     && isNotContainerEnumOrFlags
                                     && !(treatAsPointer || isPointerOrObjectType);
    QString typeName = getFullTypeNameWithoutModifiers(type);

    bool isProtectedEnum = false;

    if (mayHaveImplicitConversion) {
        s << INDENT << typeName << ' ' << cppOutAux;
        writeMinimalConstructorExpression(s, type, defaultValue);
        s << ';' << endl;
    } else if (avoidProtectedHack() && type->typeEntry()->isEnum()) {
        const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(type);
        if (metaEnum && metaEnum->isProtected()) {
            typeName = QLatin1String("long");
            isProtectedEnum = true;
        }
    }

    s << INDENT << typeName;
    if (treatAsPointer || isPointerOrObjectType) {
        s << "* " << cppOut;
        if (!defaultValue.isEmpty())
            s << " = " << defaultValue;
    } else if (type->referenceType() == LValueReference && !typeEntry->isPrimitive() && isNotContainerEnumOrFlags) {
        s << "* " << cppOut << " = &" << cppOutAux;
    } else {
        s << ' ' << cppOut;
        if (isProtectedEnum && avoidProtectedHack()) {
            s << " = ";
            if (defaultValue.isEmpty())
                s << "0";
            else
                s << "(long)" << defaultValue;
        } else if (isUserPrimitive(type) || typeEntry->isEnum() || typeEntry->isFlags()) {
            writeMinimalConstructorExpression(s, typeEntry, defaultValue);
        } else if (!type->isContainer() && !type->isSmartPointer()) {
            writeMinimalConstructorExpression(s, type, defaultValue);
        }
    }
    s << ';' << endl;

    QString pythonToCppFunc = pythonToCppConverterForArgumentName(pyIn);

    s << INDENT;
    if (!defaultValue.isEmpty())
        s << "if (" << pythonToCppFunc << ") ";

    QString pythonToCppCall = QString::fromLatin1("%1(%2, &%3)").arg(pythonToCppFunc, pyIn, cppOut);
    if (!mayHaveImplicitConversion) {
        s << pythonToCppCall << ';' << endl;
        return;
    }

    if (!defaultValue.isEmpty())
        s << '{' << endl << INDENT;

    s << "if (Shiboken::Conversions::isImplicitConversion(reinterpret_cast<const SbkObjectType *>("
        << cpythonTypeNameExt(type) << "), " << pythonToCppFunc << "))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << pythonToCppFunc << '(' << pyIn << ", &" << cppOutAux << ");" << endl;
    }
    s << INDENT << "else" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << pythonToCppCall << ';' << endl;
    }

    if (!defaultValue.isEmpty())
        s << INDENT << '}';
    s << endl;
}

static void addConversionRuleCodeSnippet(CodeSnipList& snippetList, QString& rule,
                                         TypeSystem::Language /* conversionLanguage */,
                                         TypeSystem::Language snippetLanguage,
                                         QString outputName = QString(),
                                         QString inputName = QString())
{
    if (rule.isEmpty())
        return;
    if (snippetLanguage == TypeSystem::TargetLangCode) {
        rule.replace(QLatin1String("%in"), inputName);
        rule.replace(QLatin1String("%out"), outputName + QLatin1String("_out"));
    } else {
        rule.replace(QLatin1String("%out"), outputName);
    }
    CodeSnip snip(0, snippetLanguage);
    snip.position = (snippetLanguage == TypeSystem::NativeCode) ? TypeSystem::CodeSnipPositionAny : TypeSystem::CodeSnipPositionBeginning;
    snip.addCode(rule);
    snippetList << snip;
}

void CppGenerator::writeConversionRule(QTextStream& s, const AbstractMetaFunction* func, TypeSystem::Language language)
{
    CodeSnipList snippets;
    foreach (AbstractMetaArgument* arg, func->arguments()) {
        QString rule = func->conversionRule(language, arg->argumentIndex() + 1);
        addConversionRuleCodeSnippet(snippets, rule, language, TypeSystem::TargetLangCode,
                                     arg->name(), arg->name());
    }
    writeCodeSnips(s, snippets, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode, func);
}

void CppGenerator::writeConversionRule(QTextStream& s, const AbstractMetaFunction* func, TypeSystem::Language language, const QString& outputVar)
{
    CodeSnipList snippets;
    QString rule = func->conversionRule(language, 0);
    addConversionRuleCodeSnippet(snippets, rule, language, language, outputVar);
    writeCodeSnips(s, snippets, TypeSystem::CodeSnipPositionAny, language, func);
}

void CppGenerator::writeNoneReturn(QTextStream& s, const AbstractMetaFunction* func, bool thereIsReturnValue)
{
    if (thereIsReturnValue && (!func->type() || func->argumentRemoved(0)) && !injectedCodeHasReturnValueAttribution(func)) {
        s << INDENT << PYTHON_RETURN_VAR " = Py_None;" << endl;
        s << INDENT << "Py_INCREF(Py_None);" << endl;
    }
}

void CppGenerator::writeOverloadedFunctionDecisor(QTextStream& s, const OverloadData& overloadData)
{
    s << INDENT << "// Overloaded function decisor" << endl;
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    QList<const AbstractMetaFunction*> functionOverloads = overloadData.overloadsWithoutRepetition();
    for (int i = 0; i < functionOverloads.count(); i++)
        s << INDENT << "// " << i << ": " << functionOverloads.at(i)->minimalSignature() << endl;
    writeOverloadedFunctionDecisorEngine(s, &overloadData);
    s << endl;

    // Ensure that the direct overload that called this reverse
    // is called.
    if (rfunc->isOperatorOverload() && !rfunc->isCallOperator()) {
        s << INDENT << "if (isReverse && overloadId == -1) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"reverse operator not implemented.\");" << endl;
            s << INDENT << "return 0;" << endl;
        }
        s << INDENT << "}" << endl << endl;
    }

    s << INDENT << "// Function signature not found." << endl;
    s << INDENT << "if (overloadId == -1) goto " << cpythonFunctionName(overloadData.referenceFunction()) << "_TypeError;" << endl;
    s << endl;
}

void CppGenerator::writeOverloadedFunctionDecisorEngine(QTextStream& s, const OverloadData* parentOverloadData)
{
    bool hasDefaultCall = parentOverloadData->nextArgumentHasDefaultValue();
    const AbstractMetaFunction* referenceFunction = parentOverloadData->referenceFunction();

    // If the next argument has not an argument with a default value, it is still possible
    // that one of the overloads for the current overload data has its final occurrence here.
    // If found, the final occurrence of a method is attributed to the referenceFunction
    // variable to be used further on this method on the conditional that identifies default
    // method calls.
    if (!hasDefaultCall) {
        foreach (const AbstractMetaFunction* func, parentOverloadData->overloads()) {
            if (parentOverloadData->isFinalOccurrence(func)) {
                referenceFunction = func;
                hasDefaultCall = true;
                break;
            }
        }
    }

    int maxArgs = parentOverloadData->maxArgs();
    // Python constructors always receive multiple arguments.
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(*parentOverloadData);

    // Functions without arguments are identified right away.
    if (maxArgs == 0) {
        s << INDENT << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(referenceFunction);
        s << "; // " << referenceFunction->minimalSignature() << endl;
        return;

    // To decide if a method call is possible at this point the current overload
    // data object cannot be the head, since it is just an entry point, or a root,
    // for the tree of arguments and it does not represent a valid method call.
    } else if (!parentOverloadData->isHeadOverloadData()) {
        bool isLastArgument = parentOverloadData->nextOverloadData().isEmpty();
        bool signatureFound = parentOverloadData->overloads().size() == 1;

        // The current overload data describes the last argument of a signature,
        // so the method can be identified right now.
        if (isLastArgument || (signatureFound && !hasDefaultCall)) {
            const AbstractMetaFunction* func = parentOverloadData->referenceFunction();
            s << INDENT << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(func);
            s << "; // " << func->minimalSignature() << endl;
            return;
        }
    }

    bool isFirst = true;

    // If the next argument has a default value the decisor can perform a method call;
    // it just need to check if the number of arguments received from Python are equal
    // to the number of parameters preceding the argument with the default value.
    if (hasDefaultCall) {
        isFirst = false;
        int numArgs = parentOverloadData->argPos() + 1;
        s << INDENT << "if (numArgs == " << numArgs << ") {" << endl;
        {
            Indentation indent(INDENT);
            const AbstractMetaFunction* func = referenceFunction;
            foreach (OverloadData* overloadData, parentOverloadData->nextOverloadData()) {
                const AbstractMetaFunction* defValFunc = overloadData->getFunctionWithDefaultValue();
                if (defValFunc) {
                    func = defValFunc;
                    break;
                }
            }
            s << INDENT << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(func);
            s << "; // " << func->minimalSignature() << endl;
        }
        s << INDENT << '}';
    }

    foreach (OverloadData* overloadData, parentOverloadData->nextOverloadData()) {
        bool signatureFound = overloadData->overloads().size() == 1
                                && !overloadData->getFunctionWithDefaultValue()
                                && !overloadData->findNextArgWithDefault();

        const AbstractMetaFunction* refFunc = overloadData->referenceFunction();

        QStringList typeChecks;
        QString pyArgName = (usePyArgs && maxArgs > 1)
            ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(overloadData->argPos())
            : QLatin1String(PYTHON_ARG);
        OverloadData* od = overloadData;
        int startArg = od->argPos();
        int sequenceArgCount = 0;
        while (od && !od->argType()->isVarargs()) {
            bool typeReplacedByPyObject = od->argumentTypeReplaced() == QLatin1String("PyObject");
            if (!typeReplacedByPyObject) {
                if (usePyArgs)
                    pyArgName = QString::fromLatin1(PYTHON_ARGS "[%1]").arg(od->argPos());
                QString typeCheck;
                QTextStream tck(&typeCheck);
                const AbstractMetaFunction* func = od->referenceFunction();

                if (func->isConstructor() && func->arguments().count() == 1) {
                    const AbstractMetaClass* ownerClass = func->ownerClass();
                    const ComplexTypeEntry* baseContainerType = ownerClass->typeEntry()->baseContainerType();
                    if (baseContainerType && baseContainerType == func->arguments().first()->type()->typeEntry() && isCopyable(ownerClass)) {
                        tck << '!' << cpythonCheckFunction(ownerClass->typeEntry()) << pyArgName << ')' << endl;
                        Indentation indent(INDENT);
                        tck << INDENT << "&& ";
                    }
                }
                writeTypeCheck(tck, od, pyArgName);
                typeChecks << typeCheck;
            }

            sequenceArgCount++;

            if (od->nextOverloadData().isEmpty()
                || od->nextArgumentHasDefaultValue()
                || od->nextOverloadData().size() != 1
                || od->overloads().size() != od->nextOverloadData().first()->overloads().size()) {
                overloadData = od;
                od = 0;
            } else {
                od = od->nextOverloadData().first();
            }
        }

        if (usePyArgs && signatureFound) {
            AbstractMetaArgumentList args = refFunc->arguments();
            int lastArgIsVarargs = (int) (args.size() > 1 && args.last()->type()->isVarargs());
            int numArgs = args.size() - OverloadData::numberOfRemovedArguments(refFunc) - lastArgIsVarargs;
            typeChecks.prepend(QString::fromLatin1("numArgs %1 %2").arg(lastArgIsVarargs ? QLatin1String(">=") : QLatin1String("==")).arg(numArgs));
        } else if (sequenceArgCount > 1) {
            typeChecks.prepend(QString::fromLatin1("numArgs >= %1").arg(startArg + sequenceArgCount));
        } else if (refFunc->isOperatorOverload() && !refFunc->isCallOperator()) {
            typeChecks.prepend(QString::fromLatin1("%1isReverse").arg(refFunc->isReverseOperator() ? QString() : QLatin1String("!")));
        }

        if (isFirst) {
            isFirst = false;
            s << INDENT;
        } else {
            s << " else ";
        }
        s << "if (";
        if (typeChecks.isEmpty()) {
            s << "true";
        } else {
            Indentation indent(INDENT);
            QString separator;
            QTextStream sep(&separator);
            sep << endl << INDENT << "&& ";
            s << typeChecks.join(separator);
        }
        s << ") {" << endl;
        {
            Indentation indent(INDENT);
            writeOverloadedFunctionDecisorEngine(s, overloadData);
        }
        s << INDENT << "}";
    }
    s << endl;
}

void CppGenerator::writeFunctionCalls(QTextStream &s, const OverloadData &overloadData,
                                      GeneratorContext &context)
{
    QList<const AbstractMetaFunction*> overloads = overloadData.overloadsWithoutRepetition();
    s << INDENT << "// Call function/method" << endl;
    s << INDENT << (overloads.count() > 1 ? "switch (overloadId) " : "") << '{' << endl;
    {
        Indentation indent(INDENT);
        if (overloads.count() == 1) {
            writeSingleFunctionCall(s, overloadData, overloads.first(), context);
        } else {
            for (int i = 0; i < overloads.count(); i++) {
                const AbstractMetaFunction* func = overloads.at(i);
                s << INDENT << "case " << i << ": // " << func->signature() << endl;
                s << INDENT << '{' << endl;
                {
                    Indentation indent(INDENT);
                    writeSingleFunctionCall(s, overloadData, func, context);
                    s << INDENT << "break;" << endl;
                }
                s << INDENT << '}' << endl;
            }
        }
    }
    s << INDENT << '}' << endl;
}

void CppGenerator::writeSingleFunctionCall(QTextStream &s,
                                           const OverloadData &overloadData,
                                           const AbstractMetaFunction *func,
                                           GeneratorContext &context)
{
    if (func->isDeprecated()) {
        s << INDENT << "Shiboken::warning(PyExc_DeprecationWarning, 1, \"Function: '"
                    << func->signature().replace(QLatin1String("::"), QLatin1String("."))
                    << "' is marked as deprecated, please check the documentation for more information.\");" << endl;
    }

    if (func->functionType() == AbstractMetaFunction::EmptyFunction) {
        s << INDENT << "PyErr_Format(PyExc_TypeError, \"%s is a private method.\", \""
          << func->signature().replace(QLatin1String("::"), QLatin1String("."))
          << "\");" << endl;
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        return;
    }

    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(overloadData);

    // Handle named arguments.
    writeNamedArgumentResolution(s, func, usePyArgs);

    bool injectCodeCallsFunc = injectedCodeCallsCppFunction(func);
    bool mayHaveUnunsedArguments = !func->isUserAdded() && func->hasInjectedCode() && injectCodeCallsFunc;
    int removedArgs = 0;
    for (int argIdx = 0; argIdx < func->arguments().count(); ++argIdx) {
        bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, argIdx + 1).isEmpty();
        const AbstractMetaArgument* arg = func->arguments().at(argIdx);
        if (func->argumentRemoved(argIdx + 1)) {
            if (!arg->defaultValueExpression().isEmpty()) {
                QString cppArgRemoved = QString::fromLatin1(CPP_ARG_REMOVED "%1").arg(argIdx);
                s << INDENT << getFullTypeName(arg->type()) << ' ' << cppArgRemoved;
                s << " = " << guessScopeForDefaultValue(func, arg) << ';' << endl;
                writeUnusedVariableCast(s, cppArgRemoved);
            } else if (!injectCodeCallsFunc && !func->isUserAdded() && !hasConversionRule) {
                // When an argument is removed from a method signature and no other means of calling
                // the method are provided (as with code injection) the generator must abort.
                qFatal(qPrintable(QString::fromLatin1("No way to call '%1::%2' with the modifications described in the type system.")
                                                      .arg(func->ownerClass()->name(), func->signature())), NULL);
            }
            removedArgs++;
            continue;
        }
        if (hasConversionRule)
            continue;
        const AbstractMetaType* argType = getArgumentType(func, argIdx + 1);
        if (!argType || (mayHaveUnunsedArguments && !injectedCodeUsesArgument(func, argIdx)))
            continue;
        int argPos = argIdx - removedArgs;
        QString argName = QString::fromLatin1(CPP_ARG"%1").arg(argPos);
        QString pyArgName = usePyArgs ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(argPos) : QLatin1String(PYTHON_ARG);
        QString defaultValue = guessScopeForDefaultValue(func, arg);
        writeArgumentConversion(s, argType, argName, pyArgName, func->implementingClass(), defaultValue, func->isUserAdded());
    }

    s << endl;

    int numRemovedArgs = OverloadData::numberOfRemovedArguments(func);

    s << INDENT << "if (!PyErr_Occurred()) {" << endl;
    {
        Indentation indentation(INDENT);
        writeMethodCall(s, func, context, func->arguments().size() - numRemovedArgs);
        if (!func->isConstructor())
            writeNoneReturn(s, func, overloadData.hasNonVoidReturnType());
    }
    s << INDENT << '}' << endl;
}

QString CppGenerator::cppToPythonFunctionName(const QString& sourceTypeName, QString targetTypeName)
{
    if (targetTypeName.isEmpty())
        targetTypeName = sourceTypeName;
    return QString::fromLatin1("%1_CppToPython_%2").arg(sourceTypeName, targetTypeName);
}

QString CppGenerator::pythonToCppFunctionName(const QString& sourceTypeName, const QString& targetTypeName)
{
    return QString::fromLatin1("%1_PythonToCpp_%2").arg(sourceTypeName, targetTypeName);
}
QString CppGenerator::pythonToCppFunctionName(const AbstractMetaType* sourceType, const AbstractMetaType* targetType)
{
    return pythonToCppFunctionName(fixedCppTypeName(sourceType), fixedCppTypeName(targetType));
}
QString CppGenerator::pythonToCppFunctionName(const CustomConversion::TargetToNativeConversion* toNative,
                                              const TypeEntry* targetType)
{
    return pythonToCppFunctionName(fixedCppTypeName(toNative), fixedCppTypeName(targetType));
}

QString CppGenerator::convertibleToCppFunctionName(const QString& sourceTypeName, const QString& targetTypeName)
{
    return QString::fromLatin1("is_%1_PythonToCpp_%2_Convertible").arg(sourceTypeName, targetTypeName);
}
QString CppGenerator::convertibleToCppFunctionName(const AbstractMetaType* sourceType, const AbstractMetaType* targetType)
{
    return convertibleToCppFunctionName(fixedCppTypeName(sourceType), fixedCppTypeName(targetType));
}
QString CppGenerator::convertibleToCppFunctionName(const CustomConversion::TargetToNativeConversion* toNative,
                                                   const TypeEntry* targetType)
{
    return convertibleToCppFunctionName(fixedCppTypeName(toNative), fixedCppTypeName(targetType));
}

void CppGenerator::writeCppToPythonFunction(QTextStream& s, const QString& code, const QString& sourceTypeName, QString targetTypeName)
{
    QString prettyCode;
    QTextStream c(&prettyCode);
    formatCode(c, code, INDENT);
    processCodeSnip(prettyCode);

    s << "static PyObject* " << cppToPythonFunctionName(sourceTypeName, targetTypeName);
    s << "(const void* cppIn) {" << endl;
    s << prettyCode;
    s << '}' << endl;
}

static void replaceCppToPythonVariables(QString& code, const QString& typeName)
{
    code.prepend(QString::fromLatin1("%1& cppInRef = *((%1*)cppIn);\n").arg(typeName));
    code.replace(QLatin1String("%INTYPE"), typeName);
    code.replace(QLatin1String("%OUTTYPE"), QLatin1String("PyObject*"));
    code.replace(QLatin1String("%in"), QLatin1String("cppInRef"));
    code.replace(QLatin1String("%out"), QLatin1String("pyOut"));
}
void CppGenerator::writeCppToPythonFunction(QTextStream& s, const CustomConversion* customConversion)
{
    QString code = customConversion->nativeToTargetConversion();
    replaceCppToPythonVariables(code, getFullTypeName(customConversion->ownerType()));
    writeCppToPythonFunction(s, code, fixedCppTypeName(customConversion->ownerType()));
}
void CppGenerator::writeCppToPythonFunction(QTextStream& s, const AbstractMetaType* containerType)
{
    const CustomConversion* customConversion = containerType->typeEntry()->customConversion();
    if (!customConversion) {
        qFatal(qPrintable(QString::fromLatin1("Can't write the C++ to Python conversion function for container type '%1' - "\
                                  "no conversion rule was defined for it in the type system.")
                             .arg(containerType->typeEntry()->qualifiedCppName())), NULL);
    }
    if (!containerType->typeEntry()->isContainer()) {
        writeCppToPythonFunction(s, customConversion);
        return;
    }
    QString code = customConversion->nativeToTargetConversion();
    for (int i = 0; i < containerType->instantiations().count(); ++i) {
        AbstractMetaType* type = containerType->instantiations().at(i);
        QString typeName = getFullTypeName(type);
        if (type->isConstant())
            typeName = QLatin1String("const ") + typeName;
        code.replace(QString::fromLatin1("%INTYPE_%1").arg(i), typeName);
    }
    replaceCppToPythonVariables(code, getFullTypeNameWithoutModifiers(containerType));
    processCodeSnip(code);
    writeCppToPythonFunction(s, code, fixedCppTypeName(containerType));
}

void CppGenerator::writePythonToCppFunction(QTextStream& s, const QString& code, const QString& sourceTypeName, const QString& targetTypeName)
{
    QString prettyCode;
    QTextStream c(&prettyCode);
    formatCode(c, code, INDENT);
    processCodeSnip(prettyCode);
    s << "static void " << pythonToCppFunctionName(sourceTypeName, targetTypeName);
    s << "(PyObject* pyIn, void* cppOut) {" << endl;
    s << prettyCode;
    s << '}' << endl;
}

void CppGenerator::writeIsPythonConvertibleToCppFunction(QTextStream& s,
                                                         const QString& sourceTypeName,
                                                         const QString& targetTypeName,
                                                         const QString& condition,
                                                         QString pythonToCppFuncName,
                                                         bool acceptNoneAsCppNull)
{
    if (pythonToCppFuncName.isEmpty())
        pythonToCppFuncName = pythonToCppFunctionName(sourceTypeName, targetTypeName);

    s << "static PythonToCppFunc " << convertibleToCppFunctionName(sourceTypeName, targetTypeName);
    s << "(PyObject* pyIn) {" << endl;
    if (acceptNoneAsCppNull) {
        s << INDENT << "if (pyIn == Py_None)" << endl;
        Indentation indent(INDENT);
        s << INDENT << "return Shiboken::Conversions::nonePythonToCppNullPtr;" << endl;
    }
    s << INDENT << "if (" << condition << ')' << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return " << pythonToCppFuncName << ';' << endl;
    }
    s << INDENT << "return 0;" << endl;
    s << '}' << endl;
}

void CppGenerator::writePythonToCppConversionFunctions(QTextStream& s,
                                                       const AbstractMetaType* sourceType,
                                                       const AbstractMetaType* targetType,
                                                       QString typeCheck,
                                                       QString conversion,
                                                       QString preConversion)
{
    QString sourcePyType = cpythonTypeNameExt(sourceType);

    // Python to C++ conversion function.
    QString code;
    QTextStream c(&code);
    if (conversion.isEmpty())
        conversion = QLatin1Char('*') + cpythonWrapperCPtr(sourceType->typeEntry(), QLatin1String("pyIn"));
    if (!preConversion.isEmpty())
        c << INDENT << preConversion << endl;
    c << INDENT << QString::fromLatin1("*((%1*)cppOut) = %1(%2);")
                            .arg(getFullTypeName(targetType->typeEntry()), conversion);
    QString sourceTypeName = fixedCppTypeName(sourceType);
    QString targetTypeName = fixedCppTypeName(targetType);
    writePythonToCppFunction(s, code, sourceTypeName, targetTypeName);

    // Python to C++ convertible check function.
    if (typeCheck.isEmpty())
        typeCheck = QString::fromLatin1("PyObject_TypeCheck(pyIn, %1)").arg(sourcePyType);
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, typeCheck);
    s << endl;
}

void CppGenerator::writePythonToCppConversionFunctions(QTextStream& s,
                                                      const CustomConversion::TargetToNativeConversion* toNative,
                                                      const TypeEntry* targetType)
{
    // Python to C++ conversion function.
    QString code = toNative->conversion();
    QString inType;
    if (toNative->sourceType())
        inType = cpythonTypeNameExt(toNative->sourceType());
    else
        inType = QString::fromLatin1("(&%1_Type)").arg(toNative->sourceTypeName());
    code.replace(QLatin1String("%INTYPE"), inType);
    code.replace(QLatin1String("%OUTTYPE"), targetType->qualifiedCppName());
    code.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    code.replace(QLatin1String("%out"), QString::fromLatin1("*((%1*)cppOut)").arg(getFullTypeName(targetType)));

    QString sourceTypeName = fixedCppTypeName(toNative);
    QString targetTypeName = fixedCppTypeName(targetType);
    writePythonToCppFunction(s, code, sourceTypeName, targetTypeName);

    // Python to C++ convertible check function.
    QString typeCheck = toNative->sourceTypeCheck();
    if (typeCheck.isEmpty()) {
        QString pyTypeName = toNative->sourceTypeName();
        if (pyTypeName == QLatin1String("Py_None") || pyTypeName == QLatin1String("PyNone"))
            typeCheck = QLatin1String("%in == Py_None");
        else if (pyTypeName == QLatin1String("SbkEnumType"))
            typeCheck = QLatin1String("Shiboken::isShibokenEnum(%in)");
        else if (pyTypeName == QLatin1String("SbkObject"))
            typeCheck = QLatin1String("Shiboken::Object::checkType(%in)");
        else if (pyTypeName == QLatin1String("PyTypeObject"))
            typeCheck = QLatin1String("PyType_Check(%in)");
        else if (pyTypeName == QLatin1String("PyObject"))
            typeCheck = QLatin1String("PyObject_TypeCheck(%in, &PyBaseObject_Type)");
        else if (pyTypeName.startsWith(QLatin1String("Py")))
            typeCheck = pyTypeName + QLatin1String("_Check(%in)");
    }
    if (typeCheck.isEmpty()) {
        if (!toNative->sourceType() || toNative->sourceType()->isPrimitive()) {
            qFatal(qPrintable(QString::fromLatin1("User added implicit conversion for C++ type '%1' must provide either an input "\
                                      "type check function or a non primitive type entry.")
                                 .arg(targetType->qualifiedCppName())), NULL);

        }
        typeCheck = QString::fromLatin1("PyObject_TypeCheck(%in, %1)").arg(cpythonTypeNameExt(toNative->sourceType()));
    }
    typeCheck.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    processCodeSnip(typeCheck);
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, typeCheck);
}

void CppGenerator::writePythonToCppConversionFunctions(QTextStream& s, const AbstractMetaType* containerType)
{
    const CustomConversion* customConversion = containerType->typeEntry()->customConversion();
    if (!customConversion) {
        //qFatal
        return;
    }
    const CustomConversion::TargetToNativeConversions& toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty()) {
        //qFatal
        return;
    }
    // Python to C++ conversion function.
    QString cppTypeName = getFullTypeNameWithoutModifiers(containerType);
    QString code;
    QTextStream c(&code);
    c << INDENT << QString::fromLatin1("%1& cppOutRef = *((%1*)cppOut);").arg(cppTypeName) << endl;
    code.append(toCppConversions.first()->conversion());
    for (int i = 0; i < containerType->instantiations().count(); ++i) {
        const AbstractMetaType* type = containerType->instantiations().at(i);
        QString typeName = getFullTypeName(type);
        if (type->isValue() && isValueTypeWithCopyConstructorOnly(type)) {
            static QRegExp regex(QLatin1String(CONVERTTOCPP_REGEX));
            int pos = 0;
            while ((pos = regex.indexIn(code, pos)) != -1) {
                pos += regex.matchedLength();
                QStringList list = regex.capturedTexts();
                QString varName = list.at(1);
                QString leftCode = code.left(pos);
                QString rightCode = code.mid(pos);
                rightCode.replace(varName, QLatin1Char('*') + varName);
                code = leftCode + rightCode;
            }
            typeName.append(QLatin1Char('*'));
        }
        code.replace(QString::fromLatin1("%OUTTYPE_%1").arg(i), typeName);
    }
    code.replace(QLatin1String("%OUTTYPE"), cppTypeName);
    code.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    code.replace(QLatin1String("%out"), QLatin1String("cppOutRef"));
    QString typeName = fixedCppTypeName(containerType);
    writePythonToCppFunction(s, code, typeName, typeName);

    // Python to C++ convertible check function.
    QString typeCheck = cpythonCheckFunction(containerType);
    if (typeCheck.isEmpty())
        typeCheck = QLatin1String("false");
    else
        typeCheck = QString::fromLatin1("%1pyIn)").arg(typeCheck);
    writeIsPythonConvertibleToCppFunction(s, typeName, typeName, typeCheck);
    s << endl;
}

void CppGenerator::writeAddPythonToCppConversion(QTextStream& s, const QString& converterVar, const QString& pythonToCppFunc, const QString& isConvertibleFunc)
{
    s << INDENT << "Shiboken::Conversions::addPythonToCppValueConversion(" << converterVar << ',' << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << pythonToCppFunc << ',' << endl;
        s << INDENT << isConvertibleFunc;
    }
    s << ");" << endl;
}

void CppGenerator::writeNamedArgumentResolution(QTextStream& s, const AbstractMetaFunction* func, bool usePyArgs)
{
    AbstractMetaArgumentList args = OverloadData::getArgumentsWithDefaultValues(func);
    if (args.isEmpty())
        return;

    QString pyErrString(QLatin1String("PyErr_SetString(PyExc_TypeError, \"") + fullPythonFunctionName(func)
                        + QLatin1String("(): got multiple values for keyword argument '%1'.\");"));

    s << INDENT << "if (kwds) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyObject* ";
        foreach (const AbstractMetaArgument* arg, args) {
            int pyArgIndex = arg->argumentIndex() - OverloadData::numberOfRemovedArguments(func, arg->argumentIndex());
            QString pyArgName = usePyArgs
                ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(pyArgIndex)
                : QLatin1String(PYTHON_ARG);
            s << "value = PyDict_GetItemString(kwds, \"" << arg->name() << "\");" << endl;
            s << INDENT << "if (value && " << pyArgName << ") {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << pyErrString.arg(arg->name()) << endl;
                s << INDENT << "return " << m_currentErrorCode << ';' << endl;
            }
            s << INDENT << "} else if (value) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << pyArgName << " = value;" << endl;
                s << INDENT << "if (!";
                writeTypeCheck(s, arg->type(), pyArgName, isNumber(arg->type()->typeEntry()), func->typeReplaced(arg->argumentIndex() + 1));
                s << ')' << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "goto " << cpythonFunctionName(func) << "_TypeError;" << endl;
                }
            }
            s << INDENT << '}' << endl;
            if (arg != args.last())
                s << INDENT;
        }
    }
    s << INDENT << '}' << endl;
}

QString CppGenerator::argumentNameFromIndex(const AbstractMetaFunction* func, int argIndex, const AbstractMetaClass** wrappedClass)
{
    *wrappedClass = 0;
    QString pyArgName;
    if (argIndex == -1) {
        pyArgName = QLatin1String(PYTHON_SELF_VAR);
        *wrappedClass = func->implementingClass();
    } else if (argIndex == 0) {
        AbstractMetaType *funcType = func->type();
        AbstractMetaType *returnType = getTypeWithoutContainer(funcType);
        if (returnType) {
            pyArgName = QLatin1String(PYTHON_RETURN_VAR);
            *wrappedClass = AbstractMetaClass::findClass(classes(), returnType->typeEntry()->name());
        } else {
            QString message = QLatin1String("Invalid Argument index (0, return value) on function modification: ")
                + (funcType ? funcType->name() : QLatin1String("void")) + QLatin1Char(' ');
            if (const AbstractMetaClass *declaringClass = func->declaringClass())
                message += declaringClass->name() + QLatin1String("::");
            message += func->name() + QLatin1String("()");
            qCWarning(lcShiboken).noquote().nospace() << message;
        }
    } else {
        int realIndex = argIndex - 1 - OverloadData::numberOfRemovedArguments(func, argIndex - 1);
        AbstractMetaType* argType = getTypeWithoutContainer(func->arguments().at(realIndex)->type());

        if (argType) {
            *wrappedClass = AbstractMetaClass::findClass(classes(), argType->typeEntry()->name());
            if (argIndex == 1
                && !func->isConstructor()
                && OverloadData::isSingleArgument(getFunctionGroups(func->implementingClass())[func->name()]))
                pyArgName = QLatin1String(PYTHON_ARG);
            else
                pyArgName = QString::fromLatin1(PYTHON_ARGS "[%1]").arg(argIndex - 1);
        }
    }
    return pyArgName;
}

void CppGenerator::writeMethodCall(QTextStream &s, const AbstractMetaFunction *func,
                                   GeneratorContext &context, int maxArgs)
{
    s << INDENT << "// " << func->minimalSignature() << (func->isReverseOperator() ? " [reverse operator]": "") << endl;
    if (func->isConstructor()) {
        foreach (const CodeSnip &cs, func->injectedCodeSnips()) {
            if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                s << INDENT << "overloadId = " << func->ownerClass()->functions().indexOf(const_cast<AbstractMetaFunction* const>(func)) << ';' << endl;
                break;
            }
        }
    }

    if (func->isAbstract()) {
        s << INDENT << "if (Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR "))) {\n";
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
            s << func->ownerClass()->name() << '.' << func->name() << "()' not implemented.\");" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
        s << INDENT << "}\n";
    }

    // Used to provide contextual information to custom code writer function.
    const AbstractMetaArgument* lastArg = 0;

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = func->injectedCodeSnips();

        // Find the last argument available in the method call to provide
        // the injected code writer with information to avoid invalid replacements
        // on the %# variable.
        if (maxArgs > 0 && maxArgs < func->arguments().size() - OverloadData::numberOfRemovedArguments(func)) {
            int removedArgs = 0;
            for (int i = 0; i < maxArgs + removedArgs; i++) {
                lastArg = func->arguments().at(i);
                if (func->argumentRemoved(i + 1))
                    removedArgs++;
            }
        } else if (maxArgs != 0 && !func->arguments().isEmpty()) {
            lastArg = func->arguments().last();
        }

        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode, func, lastArg);
        s << endl;
    }

    writeConversionRule(s, func, TypeSystem::NativeCode);

    if (!func->isUserAdded()) {
        QStringList userArgs;
        if (func->functionType() != AbstractMetaFunction::CopyConstructorFunction) {
            int removedArgs = 0;
            for (int i = 0; i < maxArgs + removedArgs; i++) {
                const AbstractMetaArgument* arg = func->arguments().at(i);
                bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, arg->argumentIndex() + 1).isEmpty();
                if (func->argumentRemoved(i + 1)) {
                    // If some argument with default value is removed from a
                    // method signature, the said value must be explicitly
                    // added to the method call.
                    removedArgs++;

                    // If have conversion rules I will use this for removed args
                    if (hasConversionRule)
                        userArgs << arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
                    else if (!arg->defaultValueExpression().isEmpty())
                        userArgs << QString::fromLatin1(CPP_ARG_REMOVED "%1").arg(i);
                } else {
                    int idx = arg->argumentIndex() - removedArgs;
                    bool deRef = isValueTypeWithCopyConstructorOnly(arg->type())
                                 || isObjectTypeUsedAsValueType(arg->type())
                                 || (arg->type()->referenceType() == LValueReference && isWrapperType(arg->type()) && !isPointer(arg->type()));
                    QString argName = hasConversionRule
                                      ? arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX)
                                      : QString::fromLatin1("%1" CPP_ARG "%2").arg(deRef ? QLatin1String("*") : QString()).arg(idx);
                    userArgs << argName;
                }
            }

            // If any argument's default value was modified the method must be called
            // with this new value whenever the user doesn't pass an explicit value to it.
            // Also, any unmodified default value coming after the last user specified
            // argument and before the modified argument must be explicitly stated.
            QStringList otherArgs;
            bool otherArgsModified = false;
            bool argsClear = true;
            for (int i = func->arguments().size() - 1; i >= maxArgs + removedArgs; i--) {
                const AbstractMetaArgument* arg = func->arguments().at(i);
                bool defValModified = arg->defaultValueExpression() != arg->originalDefaultValueExpression();
                bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, arg->argumentIndex() + 1).isEmpty();
                if (argsClear && !defValModified && !hasConversionRule)
                    continue;
                else
                    argsClear = false;
                otherArgsModified |= defValModified || hasConversionRule || func->argumentRemoved(i + 1);
                if (hasConversionRule)
                    otherArgs.prepend(arg->name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX));
                else
                    otherArgs.prepend(QString::fromLatin1(CPP_ARG_REMOVED "%1").arg(i));
            }
            if (otherArgsModified)
                userArgs << otherArgs;
        }

        bool isCtor = false;
        QString methodCall;
        QTextStream mc(&methodCall);
        QString useVAddr;
        QTextStream uva(&useVAddr);
        if (func->isOperatorOverload() && !func->isCallOperator()) {
            QString firstArg = QLatin1String("(*" CPP_SELF_VAR ")");
            if (func->isPointerOperator())
                firstArg.remove(1, 1); // remove the de-reference operator

            QString secondArg = QLatin1String(CPP_ARG0);
            if (!func->isUnaryOperator() && shouldDereferenceArgumentPointer(func->arguments().first())) {
                secondArg.prepend(QLatin1String("(*"));
                secondArg.append(QLatin1Char(')'));
            }

            if (func->isUnaryOperator())
                std::swap(firstArg, secondArg);

            QString op = func->originalName();
            op = op.right(op.size() - (sizeof("operator")/sizeof(char)-1));

            if (func->isBinaryOperator()) {
                if (func->isReverseOperator())
                    std::swap(firstArg, secondArg);

                if (((op == QLatin1String("++")) || (op == QLatin1String("--"))) && !func->isReverseOperator())  {
                    s << endl << INDENT << "for(int i=0; i < " << secondArg << "; i++, " << firstArg << op << ");" << endl;
                    mc << firstArg;
                } else {
                    mc << firstArg << ' ' << op << ' ' << secondArg;
                }
            } else {
                mc << op << ' ' << secondArg;
            }
        } else if (!injectedCodeCallsCppFunction(func)) {
            if (func->isConstructor()) {
                isCtor = true;
                QString className = wrapperName(func->ownerClass());

                if (func->functionType() == AbstractMetaFunction::CopyConstructorFunction && maxArgs == 1) {
                    mc << "new ::" << className << "(*" << CPP_ARG0 << ')';
                } else {
                    QString ctorCall = className + QLatin1Char('(') + userArgs.join(QLatin1String(", ")) + QLatin1Char(')');
                    if (usePySideExtensions() && func->ownerClass()->isQObject()) {
                        s << INDENT << "void* addr = PySide::nextQObjectMemoryAddr();" << endl;
                        uva << "if (addr) {" << endl;
                        {
                            Indentation indent(INDENT);

                            uva << INDENT << "cptr = " << "new (addr) ::"
                                << ctorCall << ';' << endl
                                << INDENT
                                << "PySide::setNextQObjectMemoryAddr(0);"
                                << endl;
                        }
                        uva << INDENT << "} else {" << endl;
                        {
                            Indentation indent(INDENT);

                            uva << INDENT << "cptr = " << "new ::"
                                << ctorCall << ';' << endl;
                        }
                        uva << INDENT << "}" << endl;
                    } else {
                        mc << "new ::" << ctorCall;
                    }
                }
            } else {
                QString methodCallClassName;
                if (context.forSmartPointer())
                    methodCallClassName = context.preciseType()->cppSignature();
                else if (func->ownerClass())
                    methodCallClassName = func->ownerClass()->qualifiedCppName();

                if (func->ownerClass()) {
                    if (!avoidProtectedHack() || !func->isProtected()) {
                        if (func->isStatic()) {
                            mc << "::" << methodCallClassName << "::";
                        } else {
                            const QString selfVarCast = func->ownerClass() == func->implementingClass()
                                ? QLatin1String(CPP_SELF_VAR)
                                : QLatin1String("reinterpret_cast<") + methodCallClassName + QLatin1String(" *>(" CPP_SELF_VAR ")");
                            if (func->isConstant()) {
                                if (avoidProtectedHack()) {
                                    mc << "const_cast<const ::";
                                    if (func->ownerClass()->hasProtectedMembers()) {
                                        // PYSIDE-500: Need a special wrapper cast when inherited
                                        const QString selfWrapCast = func->ownerClass() == func->implementingClass()
                                            ? QLatin1String(CPP_SELF_VAR)
                                            : QLatin1String("reinterpret_cast<") + wrapperName(func->ownerClass()) + QLatin1String(" *>(" CPP_SELF_VAR ")");
                                        mc << wrapperName(func->ownerClass());
                                        mc << "*>(" << selfWrapCast << ")->";
                                    }
                                    else {
                                        mc << methodCallClassName;
                                        mc << "*>(" << selfVarCast << ")->";
                                    }
                                } else {
                                    mc << "const_cast<const ::" << methodCallClassName;
                                    mc <<  "*>(" << selfVarCast << ")->";
                                }
                            } else {
                                mc << selfVarCast << "->";
                            }
                        }

                        if (!func->isAbstract() && func->isVirtual())
                            mc << "::%CLASS_NAME::";

                        mc << func->originalName();
                    } else {
                        if (!func->isStatic())
                            mc << "((::" << wrapperName(func->ownerClass()) << "*) " << CPP_SELF_VAR << ")->";

                        if (!func->isAbstract())
                            mc << (func->isProtected() ? wrapperName(func->ownerClass()) :
                                                         QLatin1String("::")
                                                         + methodCallClassName) << "::";
                        mc << func->originalName() << "_protected";
                    }
                } else {
                    mc << func->originalName();
                }
                mc << '(' << userArgs.join(QLatin1String(", ")) << ')';
                if (!func->isAbstract() && func->isVirtual()) {
                    mc.flush();
                    if (!avoidProtectedHack() || !func->isProtected()) {
                        QString virtualCall(methodCall);
                        QString normalCall(methodCall);
                        virtualCall = virtualCall.replace(QLatin1String("%CLASS_NAME"),
                                                          methodCallClassName);
                        normalCall.remove(QLatin1String("::%CLASS_NAME::"));
                        methodCall.clear();
                        mc << "Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR ")) ? ";
                        mc << virtualCall << " : " <<  normalCall;
                    }
                }
            }
        }

        if (!injectedCodeCallsCppFunction(func)) {
            s << INDENT << BEGIN_ALLOW_THREADS << endl << INDENT;
            if (isCtor) {
                s << (useVAddr.isEmpty() ?
                      QString::fromLatin1("cptr = %1;").arg(methodCall) : useVAddr) << endl;
            } else if (func->type() && !func->isInplaceOperator()) {
                bool writeReturnType = true;
                if (avoidProtectedHack()) {
                    const AbstractMetaEnum* metaEnum = findAbstractMetaEnum(func->type());
                    if (metaEnum) {
                        QString enumName;
                        if (metaEnum->isProtected())
                            enumName = protectedEnumSurrogateName(metaEnum);
                        else
                            enumName = func->type()->cppSignature();
                        methodCall.prepend(enumName + QLatin1Char('('));
                        methodCall.append(QLatin1Char(')'));
                        s << enumName;
                        writeReturnType = false;
                    }
                }
                if (writeReturnType) {
                    s << func->type()->cppSignature();
                    if (isObjectTypeUsedAsValueType(func->type())) {
                        s << '*';
                        methodCall.prepend(QString::fromLatin1("new %1(").arg(func->type()->typeEntry()->qualifiedCppName()));
                        methodCall.append(QLatin1Char(')'));
                    }
                }
                s << " " CPP_RETURN_VAR " = ";
                s << methodCall << ';' << endl;
            } else {
                s << methodCall << ';' << endl;
            }
            s << INDENT << END_ALLOW_THREADS << endl;

            if (!func->conversionRule(TypeSystem::TargetLangCode, 0).isEmpty()) {
                writeConversionRule(s, func, TypeSystem::TargetLangCode, QLatin1String(PYTHON_RETURN_VAR));
            } else if (!isCtor && !func->isInplaceOperator() && func->type()
                && !injectedCodeHasReturnValueAttribution(func, TypeSystem::TargetLangCode)) {
                s << INDENT << PYTHON_RETURN_VAR " = ";
                if (isObjectTypeUsedAsValueType(func->type())) {
                    s << "Shiboken::Object::newObject(reinterpret_cast<SbkObjectType *>(" << cpythonTypeNameExt(func->type()->typeEntry())
                        << "), " << CPP_RETURN_VAR << ", true, true)";
                } else {
                    writeToPythonConversion(s, func->type(), func->ownerClass(), QLatin1String(CPP_RETURN_VAR));
                }
                s << ';' << endl;
            }
        }
    }

    if (func->hasInjectedCode() && !func->isConstructor()) {
        s << endl;
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode, func, lastArg);
    }

    bool hasReturnPolicy = false;

    // Ownership transference between C++ and Python.
    QList<ArgumentModification> ownership_mods;
    // Python object reference management.
    QList<ArgumentModification> refcount_mods;
    foreach (const FunctionModification &func_mod, func->modifications()) {
        foreach (const ArgumentModification &arg_mod, func_mod.argument_mods) {
            if (!arg_mod.ownerships.isEmpty() && arg_mod.ownerships.contains(TypeSystem::TargetLangCode))
                ownership_mods.append(arg_mod);
            else if (!arg_mod.referenceCounts.isEmpty())
                refcount_mods.append(arg_mod);
        }
    }

    // If there's already a setParent(return, me), don't use the return heuristic!
    if (func->argumentOwner(func->ownerClass(), -1).index == 0)
        hasReturnPolicy = true;

    if (!ownership_mods.isEmpty()) {
        s << endl << INDENT << "// Ownership transferences." << endl;
        foreach (const ArgumentModification &arg_mod, ownership_mods) {
            const AbstractMetaClass* wrappedClass = 0;
            QString pyArgName = argumentNameFromIndex(func, arg_mod.index, &wrappedClass);
            if (!wrappedClass) {
                s << "#error Invalid ownership modification for argument " << arg_mod.index << '(' << pyArgName << ')' << endl << endl;
                break;
            }

            if (arg_mod.index == 0 || arg_mod.owner.index == 0)
                hasReturnPolicy = true;

            // The default ownership does nothing. This is useful to avoid automatic heuristically
            // based generation of code defining parenting.
            if (arg_mod.ownerships[TypeSystem::TargetLangCode] == TypeSystem::DefaultOwnership)
                continue;

            s << INDENT << "Shiboken::Object::";
            if (arg_mod.ownerships[TypeSystem::TargetLangCode] == TypeSystem::TargetLangOwnership) {
                s << "getOwnership(" << pyArgName << ");";
            } else if (wrappedClass->hasVirtualDestructor()) {
                if (arg_mod.index == 0)
                    s << "releaseOwnership(" PYTHON_RETURN_VAR ");";
                else
                    s << "releaseOwnership(" << pyArgName << ");";
            } else {
                s << "invalidate(" << pyArgName << ");";
            }
            s << endl;
        }

    } else if (!refcount_mods.isEmpty()) {
        foreach (const ArgumentModification &arg_mod, refcount_mods) {
            ReferenceCount refCount = arg_mod.referenceCounts.first();
            if (refCount.action != ReferenceCount::Set
                && refCount.action != ReferenceCount::Remove
                && refCount.action != ReferenceCount::Add) {
                qCWarning(lcShiboken) << "\"set\", \"add\" and \"remove\" are the only values supported by Shiboken for action attribute of reference-count tag.";
                continue;
            }
            const AbstractMetaClass* wrappedClass = 0;

            QString pyArgName;
            if (refCount.action == ReferenceCount::Remove) {
                pyArgName = QLatin1String("Py_None");
            } else {
                pyArgName = argumentNameFromIndex(func, arg_mod.index, &wrappedClass);
                if (pyArgName.isEmpty()) {
                    s << "#error Invalid reference count modification for argument " << arg_mod.index << endl << endl;
                    break;
                }
            }

            if (refCount.action == ReferenceCount::Add || refCount.action == ReferenceCount::Set)
                s << INDENT << "Shiboken::Object::keepReference(";
            else
                s << INDENT << "Shiboken::Object::removeReference(";

            s << "reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR "), \"";
            QString varName = arg_mod.referenceCounts.first().varName;
            if (varName.isEmpty())
                varName = func->minimalSignature() + QString().number(arg_mod.index);

            s << varName << "\", " << pyArgName
              << (refCount.action == ReferenceCount::Add ? ", true" : "")
              << ");" << endl;

            if (arg_mod.index == 0)
                hasReturnPolicy = true;
        }
    }
    writeParentChildManagement(s, func, !hasReturnPolicy);
}

QStringList CppGenerator::getAncestorMultipleInheritance(const AbstractMetaClass* metaClass)
{
    QStringList result;
    AbstractMetaClassList baseClases = getBaseClasses(metaClass);
    if (!baseClases.isEmpty()) {
        foreach (const AbstractMetaClass* baseClass, baseClases) {
            result.append(QString::fromLatin1("((size_t) static_cast<const %1*>(class_ptr)) - base")
                                              .arg(baseClass->qualifiedCppName()));
            result.append(QString::fromLatin1("((size_t) static_cast<const %1*>((%2*)((void*)class_ptr))) - base")
                                              .arg(baseClass->qualifiedCppName(), metaClass->qualifiedCppName()));
        }
        foreach (const AbstractMetaClass* baseClass, baseClases)
            result.append(getAncestorMultipleInheritance(baseClass));
    }
    return result;
}

void CppGenerator::writeMultipleInheritanceInitializerFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString className = metaClass->qualifiedCppName();
    QStringList ancestors = getAncestorMultipleInheritance(metaClass);
    s << "static int mi_offsets[] = { ";
    for (int i = 0; i < ancestors.size(); i++)
        s << "-1, ";
    s << "-1 };" << endl;
    s << "int*" << endl;
    s << multipleInheritanceInitializerFunctionName(metaClass) << "(const void* cptr)" << endl;
    s << '{' << endl;
    s << INDENT << "if (mi_offsets[0] == -1) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "std::set<int> offsets;" << endl;
        s << INDENT << "std::set<int>::iterator it;" << endl;
        s << INDENT << "const " << className << "* class_ptr = reinterpret_cast<const " << className << "*>(cptr);" << endl;
        s << INDENT << "size_t base = (size_t) class_ptr;" << endl;

        foreach (const QString &ancestor, ancestors)
            s << INDENT << "offsets.insert(" << ancestor << ");" << endl;

        s << endl;
        s << INDENT << "offsets.erase(0);" << endl;
        s << endl;

        s << INDENT << "int i = 0;" << endl;
        s << INDENT << "for (it = offsets.begin(); it != offsets.end(); it++) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "mi_offsets[i] = *it;" << endl;
            s << INDENT << "i++;" << endl;
        }
        s << INDENT << '}' << endl;
    }
    s << INDENT << '}' << endl;
    s << INDENT << "return mi_offsets;" << endl;
    s << '}' << endl;
}

void CppGenerator::writeSpecialCastFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString className = metaClass->qualifiedCppName();
    s << "static void* " << cpythonSpecialCastFunctionName(metaClass) << "(void* obj, SbkObjectType* desiredType)\n";
    s << "{\n";
    s << INDENT << className << "* me = reinterpret_cast< ::" << className << "*>(obj);\n";
    bool firstClass = true;
    foreach (const AbstractMetaClass* baseClass, getAllAncestors(metaClass)) {
        s << INDENT << (!firstClass ? "else " : "") << "if (desiredType == reinterpret_cast<SbkObjectType*>(" << cpythonTypeNameExt(baseClass->typeEntry()) << "))\n";
        Indentation indent(INDENT);
        s << INDENT << "return static_cast< ::" << baseClass->qualifiedCppName() << "*>(me);\n";
        firstClass = false;
    }
    s << INDENT << "return me;\n";
    s << "}\n\n";
}

void CppGenerator::writePrimitiveConverterInitialization(QTextStream& s, const CustomConversion* customConversion)
{
    const TypeEntry* type = customConversion->ownerType();
    QString converter = converterObject(type);
    s << INDENT << "// Register converter for type '" << type->qualifiedTargetLangName() << "'." << endl;
    s << INDENT << converter << " = Shiboken::Conversions::createConverter(";
    if (type->targetLangApiName() == type->name())
        s << '0';
    else if (type->targetLangApiName() == QLatin1String("PyObject"))
        s << "&PyBaseObject_Type";
    else
        s << '&' << type->targetLangApiName() << "_Type";
    QString typeName = fixedCppTypeName(type);
    s << ", " << cppToPythonFunctionName(typeName, typeName) << ");" << endl;
    s << INDENT << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << type->qualifiedCppName() << "\");" << endl;
    writeCustomConverterRegister(s, customConversion, converter);
}

void CppGenerator::writeEnumConverterInitialization(QTextStream& s, const AbstractMetaEnum* metaEnum)
{
    if (metaEnum->isPrivate() || metaEnum->isAnonymous())
        return;
    writeEnumConverterInitialization(s, metaEnum->typeEntry());
}

void CppGenerator::writeEnumConverterInitialization(QTextStream& s, const TypeEntry* enumType)
{
    if (!enumType)
        return;
    QString enumFlagName = enumType->isFlags() ? QLatin1String("flag") : QLatin1String("enum");
    QString enumPythonType = cpythonTypeNameExt(enumType);

    const FlagsTypeEntry* flags = 0;
    if (enumType->isFlags())
        flags = reinterpret_cast<const FlagsTypeEntry*>(enumType);

    s << INDENT << "// Register converter for " << enumFlagName << " '" << enumType->qualifiedCppName() << "'." << endl;
    s << INDENT << '{' << endl;
    {
        Indentation indent(INDENT);
        QString typeName = fixedCppTypeName(enumType);
        s << INDENT << "SbkConverter* converter = Shiboken::Conversions::createConverter(" << enumPythonType << ',' << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << cppToPythonFunctionName(typeName, typeName) << ");" << endl;
        }

        if (flags) {
            QString enumTypeName = fixedCppTypeName(flags->originator());
            QString toCpp = pythonToCppFunctionName(enumTypeName, typeName);
            QString isConv = convertibleToCppFunctionName(enumTypeName, typeName);
            writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
        }

        QString toCpp = pythonToCppFunctionName(typeName, typeName);
        QString isConv = convertibleToCppFunctionName(typeName, typeName);
        writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);

        if (flags) {
            QString toCpp = pythonToCppFunctionName(QLatin1String("number"), typeName);
            QString isConv = convertibleToCppFunctionName(QLatin1String("number"), typeName);
            writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
        }

        s << INDENT << "Shiboken::Enum::setTypeConverter(" << enumPythonType << ", converter);" << endl;
        s << INDENT << "Shiboken::Enum::setTypeConverter(" << enumPythonType << ", converter);" << endl;
        QStringList cppSignature = enumType->qualifiedCppName().split(QLatin1String("::"), QString::SkipEmptyParts);
        while (!cppSignature.isEmpty()) {
            QString signature = cppSignature.join(QLatin1String("::"));
            s << INDENT << "Shiboken::Conversions::registerConverterName(converter, \"";
            if (flags)
                s << "QFlags<";
            s << signature << "\");" << endl;
            cppSignature.removeFirst();
        }
    }
    s << INDENT << '}' << endl;

    if (!flags)
        writeEnumConverterInitialization(s, reinterpret_cast<const EnumTypeEntry*>(enumType)->flags());
}

void CppGenerator::writeContainerConverterInitialization(QTextStream& s, const AbstractMetaType* type)
{
    QByteArray cppSignature = QMetaObject::normalizedSignature(type->cppSignature().toUtf8());
    s << INDENT << "// Register converter for type '" << cppSignature << "'." << endl;
    QString converter = converterObject(type);
    s << INDENT << converter << " = Shiboken::Conversions::createConverter(";
    if (type->typeEntry()->targetLangApiName() == QLatin1String("PyObject")) {
        s << "&PyBaseObject_Type";
    } else {
        QString baseName = cpythonBaseName(type->typeEntry());
        if (baseName == QLatin1String("PySequence"))
            baseName = QLatin1String("PyList");
        s << '&' << baseName << "_Type";
    }
    QString typeName = fixedCppTypeName(type);
    s << ", " << cppToPythonFunctionName(typeName, typeName) << ");" << endl;
    QString toCpp = pythonToCppFunctionName(typeName, typeName);
    QString isConv = convertibleToCppFunctionName(typeName, typeName);
    s << INDENT << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << cppSignature << "\");" << endl;
    if (usePySideExtensions() && cppSignature.startsWith("const ") && cppSignature.endsWith("&")) {
        cppSignature.chop(1);
        cppSignature.remove(0, sizeof("const ") / sizeof(char) - 1);
        s << INDENT << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << cppSignature << "\");" << endl;
    }
    writeAddPythonToCppConversion(s, converterObject(type), toCpp, isConv);
}

void CppGenerator::writeExtendedConverterInitialization(QTextStream& s, const TypeEntry* externalType, const QList<const AbstractMetaClass*>& conversions)
{
    s << INDENT << "// Extended implicit conversions for " << externalType->qualifiedTargetLangName() << '.' << endl;
    foreach (const AbstractMetaClass* sourceClass, conversions) {
        const QString converterVar = QLatin1String("reinterpret_cast<SbkObjectType *>(")
            + cppApiVariableName(externalType->targetLangPackage()) + QLatin1Char('[')
            + getTypeIndexVariableName(externalType) + QLatin1String("])");
        QString sourceTypeName = fixedCppTypeName(sourceClass->typeEntry());
        QString targetTypeName = fixedCppTypeName(externalType);
        QString toCpp = pythonToCppFunctionName(sourceTypeName, targetTypeName);
        QString isConv = convertibleToCppFunctionName(sourceTypeName, targetTypeName);
        writeAddPythonToCppConversion(s, converterVar, toCpp, isConv);
    }
}

QString CppGenerator::multipleInheritanceInitializerFunctionName(const AbstractMetaClass* metaClass)
{
    if (!hasMultipleInheritanceInAncestry(metaClass))
        return QString();
    return cpythonBaseName(metaClass->typeEntry()) + QLatin1String("_mi_init");
}

typedef QHash<QString, QPair<QString, QString> >::const_iterator ProtocolIt;

bool CppGenerator::supportsMappingProtocol(const AbstractMetaClass* metaClass)
{
    for (ProtocolIt it = m_mappingProtocol.cbegin(), end = m_mappingProtocol.cend(); it != end; ++it) {
        if (metaClass->hasFunction(it.key()))
            return true;
    }

    return false;
}

bool CppGenerator::supportsNumberProtocol(const AbstractMetaClass* metaClass)
{
    return metaClass->hasArithmeticOperatorOverload()
            || metaClass->hasLogicalOperatorOverload()
            || metaClass->hasBitwiseOperatorOverload()
            || hasBoolCast(metaClass);
}

bool CppGenerator::supportsSequenceProtocol(const AbstractMetaClass* metaClass)
{
    for (ProtocolIt it = m_sequenceProtocol.cbegin(), end = m_sequenceProtocol.cend(); it != end; ++it) {
        if (metaClass->hasFunction(it.key()))
            return true;
    }

    const ComplexTypeEntry* baseType = metaClass->typeEntry()->baseContainerType();
    if (baseType && baseType->isContainer())
        return true;

    return false;
}

bool CppGenerator::shouldGenerateGetSetList(const AbstractMetaClass* metaClass)
{
    foreach (AbstractMetaField* f, metaClass->fields()) {
        if (!f->isStatic())
            return true;
    }
    return false;
}

void CppGenerator::writeClassDefinition(QTextStream &s,
                                        const AbstractMetaClass *metaClass,
                                        GeneratorContext &classContext)
{
    QString tp_flags;
    QString tp_init;
    QString tp_new;
    QString tp_dealloc;
    QString tp_hash(QLatin1Char('0'));
    QString tp_call = tp_hash;
    QString cppClassName = metaClass->qualifiedCppName();
    QString className = cpythonTypeName(metaClass);
    className.remove(QRegExp(QLatin1String("_Type$")));
    QString baseClassName(QLatin1Char('0'));
    AbstractMetaFunctionList ctors;
    foreach (AbstractMetaFunction* f, metaClass->queryFunctions(AbstractMetaClass::Constructors)) {
        if (!f->isPrivate() && !f->isModifiedRemoved() && !classContext.forSmartPointer())
            ctors.append(f);
    }

    if (!metaClass->baseClass())
        baseClassName = QLatin1String("reinterpret_cast<PyTypeObject*>(&SbkObject_Type)");

    bool onlyPrivCtor = !metaClass->hasNonPrivateConstructor();

    const AbstractMetaClass *qCoreApp = AbstractMetaClass::findClass(classes(), QLatin1String("QCoreApplication"));
    const bool isQApp = qCoreApp != Q_NULLPTR && metaClass->inheritsFrom(qCoreApp);

    if (metaClass->isNamespace() || metaClass->hasPrivateDestructor()) {
        tp_flags = QLatin1String("Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES");
        tp_dealloc = metaClass->hasPrivateDestructor() ?
                     QLatin1String("SbkDeallocWrapperWithPrivateDtor") : QLatin1String("0");
        tp_init = QLatin1String("0");
    } else {
        if (onlyPrivCtor)
            tp_flags = QLatin1String("Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES");
        else
            tp_flags = QLatin1String("Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES");

        QString deallocClassName;
        if (shouldGenerateCppWrapper(metaClass))
            deallocClassName = wrapperName(metaClass);
        else
            deallocClassName = cppClassName;
        if (isQApp)
            tp_dealloc = QLatin1String("&SbkDeallocQAppWrapper");
        else
            tp_dealloc = QLatin1String("&SbkDeallocWrapper");
        // avoid constFirst to stay Qt 5.5 compatible
        tp_init = (onlyPrivCtor || ctors.isEmpty()) ? QLatin1String("0") : cpythonFunctionName(ctors.first());
    }

    QString tp_getattro(QLatin1Char('0'));
    QString tp_setattro = tp_getattro;
    if (usePySideExtensions() && (metaClass->qualifiedCppName() == QLatin1String("QObject"))) {
        tp_getattro = cpythonGetattroFunctionName(metaClass);
        tp_setattro = cpythonSetattroFunctionName(metaClass);
    } else {
        if (classNeedsGetattroFunction(metaClass))
            tp_getattro = cpythonGetattroFunctionName(metaClass);
        if (classNeedsSetattroFunction(metaClass))
            tp_setattro = cpythonSetattroFunctionName(metaClass);
    }

    if (metaClass->hasPrivateDestructor() || onlyPrivCtor) {
        tp_new = QLatin1String("0");
        tp_flags.append(QLatin1String("|Py_TPFLAGS_HAVE_GC"));
    }
    else if (isQApp) {
        tp_new = QLatin1String("SbkQAppTpNew"); // PYSIDE-571: need singleton app
    }
    else {
        tp_new = QLatin1String("SbkObjectTpNew");
        tp_flags.append(QLatin1String("|Py_TPFLAGS_HAVE_GC"));
    }

    QString tp_richcompare = QString(QLatin1Char('0'));
    if (metaClass->hasComparisonOperatorOverload())
        tp_richcompare = cpythonBaseName(metaClass) + QLatin1String("_richcompare");

    QString tp_getset = QString(QLatin1Char('0'));
    if (shouldGenerateGetSetList(metaClass) && !classContext.forSmartPointer())
        tp_getset = cpythonGettersSettersDefinitionName(metaClass);

    // search for special functions
    ShibokenGenerator::clearTpFuncs();
    foreach (AbstractMetaFunction* func, metaClass->functions()) {
        if (m_tpFuncs.contains(func->name()))
            m_tpFuncs[func->name()] = cpythonFunctionName(func);
    }
    if (m_tpFuncs[QLatin1String("__repr__")] == QLatin1String("0")
        && !metaClass->isQObject()
        && metaClass->hasToStringCapability()) {
        m_tpFuncs[QLatin1String("__repr__")] = writeReprFunction(s, classContext);
    }

    // class or some ancestor has multiple inheritance
    const AbstractMetaClass* miClass = getMultipleInheritingClass(metaClass);
    if (miClass) {
        if (metaClass == miClass)
            writeMultipleInheritanceInitializerFunction(s, metaClass);
        writeSpecialCastFunction(s, metaClass);
        s << endl;
    }

    if (!metaClass->typeEntry()->hashFunction().isEmpty())
        tp_hash = QLatin1Char('&') + cpythonBaseName(metaClass) + QLatin1String("_HashFunc");

    const AbstractMetaFunction* callOp = metaClass->findFunction(QLatin1String("operator()"));
    if (callOp && !callOp->isModifiedRemoved())
        tp_call = QLatin1Char('&') + cpythonFunctionName(callOp);

    s << "// Class Definition -----------------------------------------------" << endl;
    s << "extern \"C\" {" << endl;

    if (supportsNumberProtocol(metaClass)) {
        s << "static PyNumberMethods " << className + QLatin1String("_TypeAsNumber") << ";" << endl;
        s << endl;
    }

    if (supportsSequenceProtocol(metaClass)) {
        s << "static PySequenceMethods " << className + QLatin1String("_TypeAsSequence") << ";" << endl;
        s << endl;
    }

    if (supportsMappingProtocol(metaClass)) {
        s << "static PyMappingMethods " << className + QLatin1String("_TypeAsMapping") << ";" << endl;
        s << endl;
    }

    s << "static SbkObjectType " << className + QLatin1String("_Type") << " = { { {" << endl;
    s << INDENT << "PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)" << endl;
    QString computedClassTargetFullName;
    if (!classContext.forSmartPointer())
        computedClassTargetFullName = getClassTargetFullName(metaClass);
    else
        computedClassTargetFullName = getClassTargetFullName(classContext.preciseType());

    s << INDENT << "/*tp_name*/             \"" << computedClassTargetFullName << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(SbkObject)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          " << tp_dealloc << ',' << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             " << m_tpFuncs[QLatin1String("__repr__")] << "," << endl;
    s << INDENT << "/*tp_as_number*/        0," << endl;
    s << INDENT << "/*tp_as_sequence*/      0," << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             " << tp_hash << ',' << endl;
    s << INDENT << "/*tp_call*/             " << tp_call << ',' << endl;
    s << INDENT << "/*tp_str*/              " << m_tpFuncs[QLatin1String("__str__")] << ',' << endl;
    s << INDENT << "/*tp_getattro*/         " << tp_getattro << ',' << endl;
    s << INDENT << "/*tp_setattro*/         " << tp_setattro << ',' << endl;
    s << INDENT << "/*tp_as_buffer*/        0," << endl;
    s << INDENT << "/*tp_flags*/            " << tp_flags << ',' << endl;
    s << INDENT << "/*tp_doc*/              0," << endl;
    s << INDENT << "/*tp_traverse*/         " << className << "_traverse," << endl;
    s << INDENT << "/*tp_clear*/            " << className << "_clear," << endl;
    s << INDENT << "/*tp_richcompare*/      " << tp_richcompare << ',' << endl;
    s << INDENT << "/*tp_weaklistoffset*/   0," << endl;
    s << INDENT << "/*tp_iter*/             " << m_tpFuncs[QLatin1String("__iter__")] << ',' << endl;
    s << INDENT << "/*tp_iternext*/         " << m_tpFuncs[QLatin1String("__next__")] << ',' << endl;
    s << INDENT << "/*tp_methods*/          " << className << "_methods," << endl;
    s << INDENT << "/*tp_members*/          0," << endl;
    s << INDENT << "/*tp_getset*/           " << tp_getset << ',' << endl;
    s << INDENT << "/*tp_base*/             " << baseClassName << ',' << endl;
    s << INDENT << "/*tp_dict*/             0," << endl;
    s << INDENT << "/*tp_descr_get*/        0," << endl;
    s << INDENT << "/*tp_descr_set*/        0," << endl;
    s << INDENT << "/*tp_dictoffset*/       0," << endl;
    s << INDENT << "/*tp_init*/             " << tp_init << ',' << endl;
    s << INDENT << "/*tp_alloc*/            0," << endl;
    s << INDENT << "/*tp_new*/              " << tp_new << ',' << endl;
    s << INDENT << "/*tp_free*/             0," << endl;
    s << INDENT << "/*tp_is_gc*/            0," << endl;
    s << INDENT << "/*tp_bases*/            0," << endl;
    s << INDENT << "/*tp_mro*/              0," << endl;
    s << INDENT << "/*tp_cache*/            0," << endl;
    s << INDENT << "/*tp_subclasses*/       0," << endl;
    s << INDENT << "/*tp_weaklist*/         0" << endl;
    s << "}, }," << endl;
    s << INDENT << "/*priv_data*/           0" << endl;
    s << "};" << endl;
    QString suffix;
    if (isObjectType(metaClass))
        suffix = QLatin1String("*");
    s << "} //extern"  << endl;
}

void CppGenerator::writeMappingMethods(QTextStream &s,
                                       const AbstractMetaClass *metaClass,
                                       GeneratorContext &context)
{

    QMap<QString, QString> funcs;

    QHash< QString, QPair< QString, QString > >::const_iterator it = m_mappingProtocol.begin();
    for (; it != m_mappingProtocol.end(); ++it) {
        const AbstractMetaFunction* func = metaClass->findFunction(it.key());
        if (!func)
            continue;
        QString funcName = cpythonFunctionName(func);
        QString funcArgs = it.value().first;
        QString funcRetVal = it.value().second;

        CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
        s << funcRetVal << ' ' << funcName << '(' << funcArgs << ')' << endl << '{' << endl;
        writeInvalidPyObjectCheck(s, QLatin1String(PYTHON_SELF_VAR));

        writeCppSelfDefinition(s, func, context);

        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode, func, lastArg);
        s << '}' << endl << endl;
    }
}

void CppGenerator::writeSequenceMethods(QTextStream &s,
                                        const AbstractMetaClass *metaClass,
                                        GeneratorContext &context)
{

    QMap<QString, QString> funcs;
    bool injectedCode = false;

    QHash< QString, QPair< QString, QString > >::const_iterator it = m_sequenceProtocol.begin();
    for (; it != m_sequenceProtocol.end(); ++it) {
        const AbstractMetaFunction* func = metaClass->findFunction(it.key());
        if (!func)
            continue;
        injectedCode = true;
        QString funcName = cpythonFunctionName(func);
        QString funcArgs = it.value().first;
        QString funcRetVal = it.value().second;

        CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
        s << funcRetVal << ' ' << funcName << '(' << funcArgs << ')' << endl << '{' << endl;
        writeInvalidPyObjectCheck(s, QLatin1String(PYTHON_SELF_VAR));

        writeCppSelfDefinition(s, func, context);

        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips,TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode, func, lastArg);
        s << '}' << endl << endl;
    }

    if (!injectedCode)
        writeStdListWrapperMethods(s, context);
}

void CppGenerator::writeTypeAsSequenceDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    bool hasFunctions = false;
    QMap<QString, QString> funcs;
    for (ProtocolIt it = m_sequenceProtocol.cbegin(), end = m_sequenceProtocol.cend(); it != end; ++it) {
        const QString &funcName = it.key();
        const AbstractMetaFunction* func = metaClass->findFunction(funcName);
        funcs[funcName] = func ? cpythonFunctionName(func).prepend(QLatin1Char('&')) : QString();
        if (!hasFunctions && func)
            hasFunctions = true;
    }

    QString baseName = cpythonBaseName(metaClass);

    //use default implementation
    if (!hasFunctions) {
        funcs[QLatin1String("__len__")] = baseName + QLatin1String("__len__");
        funcs[QLatin1String("__getitem__")] = baseName + QLatin1String("__getitem__");
        funcs[QLatin1String("__setitem__")] = baseName + QLatin1String("__setitem__");
    }

    s << INDENT << "memset(&" << baseName << "_TypeAsSequence, 0, sizeof(PySequenceMethods));" << endl;
    for (QHash<QString, QString>::const_iterator it = m_sqFuncs.cbegin(), end = m_sqFuncs.cend(); it != end; ++it) {
        const QString& sqName = it.key();
        if (funcs[sqName].isEmpty())
            continue;
        if (it.value() == QLatin1String("sq_slice"))
            s << "#ifndef IS_PY3K" << endl;
        s << INDENT << baseName << "_TypeAsSequence." << it.value() << " = " << funcs[sqName] << ';' << endl;
        if (it.value() == QLatin1String("sq_slice"))
            s << "#endif" << endl;
    }
}

void CppGenerator::writeTypeAsMappingDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    bool hasFunctions = false;
    QMap<QString, QString> funcs;
    for (ProtocolIt it = m_mappingProtocol.cbegin(), end = m_mappingProtocol.cend(); it != end; ++it) {
        const QString &funcName = it.key();
        const AbstractMetaFunction* func = metaClass->findFunction(funcName);
        funcs[funcName] = func ? cpythonFunctionName(func).prepend(QLatin1Char('&')) : QLatin1String("0");
        if (!hasFunctions && func)
            hasFunctions = true;
    }

    //use default implementation
    if (!hasFunctions) {
        funcs.insert(QLatin1String("__mlen__"), QString());
        funcs.insert(QLatin1String("__mgetitem__"), QString());
        funcs.insert(QLatin1String("__msetitem__"), QString());
    }

    QString baseName = cpythonBaseName(metaClass);
    s << INDENT << "memset(&" << baseName << "_TypeAsMapping, 0, sizeof(PyMappingMethods));" << endl;
    for (QHash<QString, QString>::const_iterator it =  m_mpFuncs.cbegin(), end = m_mpFuncs.end(); it != end; ++it) {
        const QString &mpName = it.key();
        if (funcs[mpName].isEmpty())
            continue;
        s << INDENT << baseName << "_TypeAsMapping." << it.value() << " = " << funcs[mpName] << ';' << endl;
    }
}

void CppGenerator::writeTypeAsNumberDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QMap<QString, QString> nb;

    nb.insert(QLatin1String("__add__"), QString());
    nb.insert(QLatin1String("__sub__"), QString());
    nb.insert(QLatin1String("__mul__"), QString());
    nb.insert(QLatin1String("__div__"), QString());
    nb.insert(QLatin1String("__mod__"), QString());
    nb.insert(QLatin1String("__neg__"), QString());
    nb.insert(QLatin1String("__pos__"), QString());
    nb.insert(QLatin1String("__invert__"), QString());
    nb.insert(QLatin1String("__lshift__"), QString());
    nb.insert(QLatin1String("__rshift__"), QString());
    nb.insert(QLatin1String("__and__"), QString());
    nb.insert(QLatin1String("__xor__"), QString());
    nb.insert(QLatin1String("__or__"), QString());
    nb.insert(QLatin1String("__iadd__"), QString());
    nb.insert(QLatin1String("__isub__"), QString());
    nb.insert(QLatin1String("__imul__"), QString());
    nb.insert(QLatin1String("__idiv__"), QString());
    nb.insert(QLatin1String("__imod__"), QString());
    nb.insert(QLatin1String("__ilshift__"), QString());
    nb.insert(QLatin1String("__irshift__"), QString());
    nb.insert(QLatin1String("__iand__"), QString());
    nb.insert(QLatin1String("__ixor__"), QString());
    nb.insert(QLatin1String("__ior__"), QString());

    QList<AbstractMetaFunctionList> opOverloads =
            filterGroupedOperatorFunctions(metaClass,
                                           AbstractMetaClass::ArithmeticOp
                                           | AbstractMetaClass::LogicalOp
                                           | AbstractMetaClass::BitwiseOp);

    foreach (const AbstractMetaFunctionList &opOverload, opOverloads) {
        const AbstractMetaFunction* rfunc = opOverload[0];
        QString opName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
        nb[opName] = cpythonFunctionName(rfunc);
    }

    QString baseName = cpythonBaseName(metaClass);

    nb[QLatin1String("bool")] = hasBoolCast(metaClass) ? baseName + QLatin1String("___nb_bool") : QString();

    s << INDENT << "memset(&" << baseName << "_TypeAsNumber, 0, sizeof(PyNumberMethods));" << endl;
    for (QHash<QString, QString>::const_iterator it = m_nbFuncs.cbegin(), end = m_nbFuncs.cend(); it != end; ++it) {
        const QString &nbName = it.key();
        if (nb[nbName].isEmpty())
            continue;

        // bool is special because the field name differs on Python 2 and 3 (nb_nonzero vs nb_bool)
        // so a shiboken macro is used.
        if (nbName == QLatin1String("bool")) {
            s << INDENT << "SBK_NB_BOOL(" << baseName << "_TypeAsNumber) = " << nb[nbName] << ';' << endl;
        } else {
            bool excludeFromPy3K = nbName == QLatin1String("__div__") || nbName == QLatin1String("__idiv__");
            if (excludeFromPy3K) {
                s << "#ifdef IS_PY3K" << endl;
                s << INDENT << "SBK_UNUSED(&" << nb[nbName] << ");" << endl;
                s << "#else" << endl;
            }
            s << INDENT << baseName << "_TypeAsNumber." << it.value() << " = " << nb[nbName] << ';' << endl;
            if (excludeFromPy3K)
                s << "#endif" << endl;
        }
    }
    if (!nb[QLatin1String("__div__")].isEmpty())
        s << INDENT << baseName << "_TypeAsNumber.nb_true_divide = " << nb[QLatin1String("__div__")] << ';' << endl;
}

void CppGenerator::writeTpTraverseFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString baseName = cpythonBaseName(metaClass);
    s << "static int ";
    s << baseName << "_traverse(PyObject* " PYTHON_SELF_VAR ", visitproc visit, void* arg)" << endl;
    s << '{' << endl;
    s << INDENT << "return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(" PYTHON_SELF_VAR ", visit, arg);" << endl;
    s << '}' << endl;
}

void CppGenerator::writeTpClearFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString baseName = cpythonBaseName(metaClass);
    s << "static int ";
    s << baseName << "_clear(PyObject* " PYTHON_SELF_VAR ")" << endl;
    s << '{' << endl;
    s << INDENT << "return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(" PYTHON_SELF_VAR ");" << endl;
    s << '}' << endl;
}

void CppGenerator::writeCopyFunction(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    QString className = cpythonTypeName(metaClass);
    className.remove(QRegExp(QLatin1String("_Type$")));
    s << "static PyObject* " << className << "___copy__(PyObject* " PYTHON_SELF_VAR ")" << endl;
    s << "{" << endl;
    writeCppSelfDefinition(s, context, false, true);
    QString conversionCode;
    if (!context.forSmartPointer())
        conversionCode = cpythonToPythonConversionFunction(metaClass);
    else
        conversionCode = cpythonToPythonConversionFunction(context.preciseType());

    s << INDENT << "PyObject* " << PYTHON_RETURN_VAR << " = " << conversionCode;
    s << CPP_SELF_VAR ");" << endl;
    writeFunctionReturnErrorCheckSection(s);
    s << INDENT << "return " PYTHON_RETURN_VAR ";" << endl;
    s << "}" << endl;
    s << endl;
}

void CppGenerator::writeGetterFunction(QTextStream &s,
                                       const AbstractMetaField *metaField,
                                       GeneratorContext &context)
{
    ErrorCode errorCode(0);
    s << "static PyObject* " << cpythonGetterFunctionName(metaField) << "(PyObject* " PYTHON_SELF_VAR ", void*)" << endl;
    s << '{' << endl;

    writeCppSelfDefinition(s, context);

    AbstractMetaType* fieldType = metaField->type();
    // Force use of pointer to return internal variable memory
    bool newWrapperSameObject = !fieldType->isConstant() && isWrapperType(fieldType) && !isPointer(fieldType);

    QString cppField;
    if (avoidProtectedHack() && metaField->isProtected()) {
        cppField = QString::fromLatin1("((%1*)%2)->%3()")
                .arg(wrapperName(metaField->enclosingClass()), QLatin1String(CPP_SELF_VAR),
                     protectedFieldGetterName(metaField));
    } else {
        cppField = QLatin1String(CPP_SELF_VAR) + QLatin1String("->") + metaField->name();
        if (newWrapperSameObject) {
            cppField.prepend(QLatin1String("&("));
            cppField.append(QLatin1Char(')'));
        }
    }
    if (isCppIntegralPrimitive(fieldType) || fieldType->isEnum()) {
        s << INDENT << getFullTypeNameWithoutModifiers(fieldType) << " cppOut_local = " << cppField << ';' << endl;
        cppField = QLatin1String("cppOut_local");
    } else if (avoidProtectedHack() && metaField->isProtected()) {
        s << INDENT << getFullTypeNameWithoutModifiers(fieldType);
        if (fieldType->isContainer() || fieldType->isFlags() || fieldType->isSmartPointer()) {
            s << '&';
            cppField.prepend(QLatin1Char('*'));
        } else if ((!fieldType->isConstant() && !fieldType->isEnum() && !fieldType->isPrimitive()) || fieldType->indirections() == 1) {
            s << '*';
        }
        s << " fieldValue = " << cppField << ';' << endl;
        cppField = QLatin1String("fieldValue");
    }

    s << INDENT << "PyObject* pyOut = 0;\n";
    if (newWrapperSameObject) {
        // Special case colocated field with same address (first field in a struct)
        s << INDENT << "if (reinterpret_cast<void *>("
                    << cppField
                    << ") == reinterpret_cast<void *>("
                    << CPP_SELF_VAR << ")) {\n";
        {
            Indentation indent(INDENT);
            s << INDENT << "pyOut = reinterpret_cast<PyObject *>(Shiboken::Object::findColocatedChild("
                        << "reinterpret_cast<SbkObject *>(self), reinterpret_cast<const SbkObjectType *>("
                        << cpythonTypeNameExt(fieldType)
                        << ")));\n";
            s << INDENT << "if (pyOut) {Py_IncRef(pyOut); return pyOut;}\n";
        }
        s << INDENT << "}\n";
        // Check if field wrapper has already been created.
        s << INDENT << "else if (Shiboken::BindingManager::instance().hasWrapper(" << cppField << ")) {" << "\n";
        {
            Indentation indent(INDENT);
            s << INDENT << "pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(" << cppField << ");" << "\n";
            s << INDENT << "Py_IncRef(pyOut);" << "\n";
            s << INDENT << "return pyOut;" << "\n";
        }
        s << INDENT << "}\n";
        // Create and register new wrapper
        s << INDENT << "pyOut = ";
        s << "Shiboken::Object::newObject(reinterpret_cast<SbkObjectType *>(" << cpythonTypeNameExt(fieldType)
            << "), " << cppField << ", false, true);" << endl;
        s << INDENT << "Shiboken::Object::setParent(" PYTHON_SELF_VAR ", pyOut)";
    } else {
        s << INDENT << "pyOut = ";
        writeToPythonConversion(s, fieldType, metaField->enclosingClass(), cppField);
    }
    s << ';' << endl;

    s << INDENT << "return pyOut;" << endl;
    s << '}' << endl;
}

void CppGenerator::writeSetterFunction(QTextStream &s,
                                       const AbstractMetaField *metaField,
                                       GeneratorContext &context)
{
    ErrorCode errorCode(0);
    s << "static int " << cpythonSetterFunctionName(metaField) << "(PyObject* " PYTHON_SELF_VAR ", PyObject* pyIn, void*)" << endl;
    s << '{' << endl;

    writeCppSelfDefinition(s, context);

    s << INDENT << "if (pyIn == 0) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"'";
        s << metaField->name() << "' may not be deleted\");" << endl;
        s << INDENT << "return -1;" << endl;
    }
    s << INDENT << '}' << endl;

    AbstractMetaType* fieldType = metaField->type();

    s << INDENT << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << ';' << endl;
    s << INDENT << "if (!";
    writeTypeCheck(s, fieldType, QLatin1String("pyIn"), isNumber(fieldType->typeEntry()));
    s << ") {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"wrong type attributed to '";
        s << metaField->name() << "', '" << fieldType->name() << "' or convertible type expected\");" << endl;
        s << INDENT << "return -1;" << endl;
    }
    s << INDENT << '}' << endl << endl;

    QString cppField = QString::fromLatin1("%1->%2").arg(QLatin1String(CPP_SELF_VAR), metaField->name());
    s << INDENT;
    if (avoidProtectedHack() && metaField->isProtected()) {
        s << getFullTypeNameWithoutModifiers(fieldType);
        s << (fieldType->indirections() == 1 ? "*" : "") << " cppOut;" << endl;
        s << INDENT << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut);" << endl;
        s << INDENT << QString::fromLatin1("((%1*)%2)->%3(cppOut)")
                .arg(wrapperName(metaField->enclosingClass()),
                     QLatin1String(CPP_SELF_VAR), protectedFieldSetterName(metaField));
    } else if (isCppIntegralPrimitive(fieldType) || fieldType->typeEntry()->isEnum() || fieldType->typeEntry()->isFlags()) {
        s << getFullTypeNameWithoutModifiers(fieldType) << " cppOut_local = " << cppField << ';' << endl;
        s << INDENT << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut_local);" << endl;
        s << INDENT << cppField << " = cppOut_local";
    } else {
        s << getFullTypeNameWithoutModifiers(fieldType);
        s << QString::fromLatin1("*").repeated(fieldType->indirections()) << "& cppOut_ptr = ";
        s << cppField << ';' << endl;
        s << INDENT << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut_ptr)";
    }
    s << ';' << endl << endl;

    if (isPointerToWrapperType(fieldType)) {
        s << INDENT << "Shiboken::Object::keepReference(reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR "), \"";
        s << metaField->name() << "\", pyIn);" << endl;
    }

    s << INDENT << "return 0;" << endl;
    s << '}' << endl;
}

void CppGenerator::writeRichCompareFunction(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    QString baseName = cpythonBaseName(metaClass);
    s << "static PyObject* ";
    s << baseName << "_richcompare(PyObject* " PYTHON_SELF_VAR ", PyObject* " PYTHON_ARG ", int op)" << endl;
    s << '{' << endl;
    writeCppSelfDefinition(s, context, false, true);
    writeUnusedVariableCast(s, QLatin1String(CPP_SELF_VAR));
    s << INDENT << "PyObject* " PYTHON_RETURN_VAR " = 0;" << endl;
    s << INDENT << "PythonToCppFunc " PYTHON_TO_CPP_VAR << ';' << endl;
    writeUnusedVariableCast(s, QLatin1String(PYTHON_TO_CPP_VAR));
    s << endl;

    s << INDENT << "switch (op) {" << endl;
    {
        Indentation indent(INDENT);
        foreach (const AbstractMetaFunctionList &overloads, filterGroupedOperatorFunctions(metaClass, AbstractMetaClass::ComparisonOp)) {
            const AbstractMetaFunction* rfunc = overloads[0];

            QString operatorId = ShibokenGenerator::pythonRichCompareOperatorId(rfunc);
            s << INDENT << "case " << operatorId << ':' << endl;

            Indentation indent(INDENT);

            QString op = rfunc->originalName();
            op = op.right(op.size() - QLatin1String("operator").size());

            int alternativeNumericTypes = 0;
            foreach (const AbstractMetaFunction* func, overloads) {
                if (!func->isStatic() &&
                    ShibokenGenerator::isNumber(func->arguments()[0]->type()->typeEntry()))
                    alternativeNumericTypes++;
            }

            bool first = true;
            OverloadData overloadData(overloads, this);
            foreach (OverloadData* od, overloadData.nextOverloadData()) {
                const AbstractMetaFunction* func = od->referenceFunction();
                if (func->isStatic())
                    continue;
                const AbstractMetaType* argType = getArgumentType(func, 1);
                if (!argType)
                    continue;
                if (!first) {
                    s << " else ";
                } else {
                    first = false;
                    s << INDENT;
                }
                s << "if (";
                writeTypeCheck(s, argType, QLatin1String(PYTHON_ARG), alternativeNumericTypes == 1 || isPyInt(argType));
                s << ") {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "// " << func->signature() << endl;
                    writeArgumentConversion(s, argType, QLatin1String(CPP_ARG0),
                                            QLatin1String(PYTHON_ARG), metaClass,
                                            QString(), func->isUserAdded());

                    // If the function is user added, use the inject code
                    if (func->isUserAdded()) {
                        CodeSnipList snips = func->injectedCodeSnips();
                        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode, func, func->arguments().last());
                    } else {
                        QString expression = QString::fromLatin1("%1%2 %3 (%4" CPP_ARG0 ")")
                                                .arg(func->isPointerOperator() ? QLatin1String("&") : QString(),
                                                     QLatin1String(CPP_SELF_VAR), op,
                                                     shouldDereferenceAbstractMetaTypePointer(argType) ? QLatin1String("*") : QString());
                        s << INDENT;
                        if (func->type())
                            s << func->type()->cppSignature() << " " CPP_RETURN_VAR " = ";
                        s << expression << ';' << endl;
                        s << INDENT << PYTHON_RETURN_VAR " = ";
                        if (func->type())
                            writeToPythonConversion(s, func->type(), metaClass, QLatin1String(CPP_RETURN_VAR));
                        else
                            s << "Py_None;" << endl << INDENT << "Py_INCREF(Py_None)";
                        s << ';' << endl;
                    }
                }
                s << INDENT << '}';
            }

            s << " else {" << endl;
            if (operatorId == QLatin1String("Py_EQ") || operatorId == QLatin1String("Py_NE")) {
                Indentation indent(INDENT);
                s << INDENT << PYTHON_RETURN_VAR " = "
                  << (operatorId == QLatin1String("Py_EQ") ? "Py_False" : "Py_True") << ';' << endl;
                s << INDENT << "Py_INCREF(" PYTHON_RETURN_VAR ");" << endl;
            } else {
                Indentation indent(INDENT);
                s << INDENT << "goto " << baseName << "_RichComparison_TypeError;" << endl;
            }
            s << INDENT << '}' << endl << endl;

            s << INDENT << "break;" << endl;
        }
        s << INDENT << "default:" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "goto " << baseName << "_RichComparison_TypeError;" << endl;
        }
    }
    s << INDENT << '}' << endl << endl;

    s << INDENT << "if (" PYTHON_RETURN_VAR " && !PyErr_Occurred())" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return " PYTHON_RETURN_VAR ";" << endl;
    }
    s << INDENT << baseName << "_RichComparison_TypeError:" << endl;
    s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"operator not implemented.\");" << endl;
    s << INDENT << "return " << m_currentErrorCode << ';' << endl << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeMethodDefinitionEntry(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    Q_ASSERT(!overloads.isEmpty());
    OverloadData overloadData(overloads, this);
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(overloadData);
    const AbstractMetaFunction* func = overloadData.referenceFunction();
    int min = overloadData.minArgs();
    int max = overloadData.maxArgs();

    s << '"' << func->name() << "\", (PyCFunction)" << cpythonFunctionName(func) << ", ";
    if ((min == max) && (max < 2) && !usePyArgs) {
        if (max == 0)
            s << "METH_NOARGS";
        else
            s << "METH_O";
    } else {
        s << "METH_VARARGS";
        if (overloadData.hasArgumentWithDefaultValue())
            s << "|METH_KEYWORDS";
    }
    if (func->ownerClass() && overloadData.hasStaticFunction())
        s << "|METH_STATIC";
}

void CppGenerator::writeMethodDefinition(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    Q_ASSERT(!overloads.isEmpty());
    const AbstractMetaFunction* func = overloads.first();
    if (m_tpFuncs.contains(func->name()))
        return;

    s << INDENT;
    if (OverloadData::hasStaticAndInstanceFunctions(overloads)) {
        s << cpythonMethodDefinitionName(func);
    } else {
        s << '{';
        writeMethodDefinitionEntry(s, overloads);
        s << '}';
    }
    s << ',' << endl;
}

static QString resolveRetOrArgType(const AbstractMetaType *someType)
{
    QString strRetArg;
    if (CppGenerator::isCString(someType)) {
        strRetArg = QLatin1String("str");
    } else if (someType->isPrimitive()) {
        const PrimitiveTypeEntry* ptp = static_cast<const PrimitiveTypeEntry*>(someType->typeEntry());
        while (ptp->referencedTypeEntry())
            ptp = ptp->referencedTypeEntry();
        strRetArg = ptp->name();
    } else {
        strRetArg = someType->fullName();
    }
    strRetArg.replace(QLatin1String("::"), QLatin1String("."));
    return strRetArg;
}

void CppGenerator::writeSignatureInfo(QTextStream &s, const AbstractMetaFunctionList &overloads)
{
    OverloadData overloadData(overloads, this);
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    QString funcName = fullPythonFunctionName(rfunc);

    int idx = overloads.length() - 1;
    bool multiple = idx > 0;

// after merging, the #if may be removed!
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    for (const AbstractMetaFunction *f : overloads) {
        QStringList args;
        const AbstractMetaArgumentList &arguments = f->arguments();
        for (AbstractMetaArgument *arg : arguments)  {
#else
    foreach (const AbstractMetaFunction *f, overloads) {
        QStringList args;
        const AbstractMetaArgumentList &arguments = f->arguments();
        foreach (const AbstractMetaArgument *arg, arguments) {
#endif
            QString strArg = resolveRetOrArgType(arg->type());
            if (!arg->defaultValueExpression().isEmpty()) {
                strArg += QLatin1Char('=');
                QString e = arg->defaultValueExpression();
                e.replace(QLatin1String("::"), QLatin1String("."));
                // the tests insert stuff like Str("<unknown>"):
                e.replace(QLatin1Char('"'), QLatin1String("\\\""));
                strArg += e;
            }
            args << arg->name() + QLatin1Char(':') + strArg;
        }
        // mark the multiple signatures as such, to make it easier to generate different code
        if (multiple)
            s << idx-- << ':';
        // now calculate the return type.
        s << funcName << '(' << args.join(QLatin1Char(',')) << ')';
        AbstractMetaType *returnType = getTypeWithoutContainer(f->type());
        if (returnType) {
            s << "->" << resolveRetOrArgType(returnType);
        }
        s << endl;
    }
}

void CppGenerator::writeEnumsInitialization(QTextStream& s, AbstractMetaEnumList& enums)
{
    if (enums.isEmpty())
        return;
    s << INDENT << "// Initialization of enums." << endl << endl;
    foreach (const AbstractMetaEnum* cppEnum, enums) {
        if (cppEnum->isPrivate())
            continue;
        writeEnumInitialization(s, cppEnum);
    }
}

void CppGenerator::writeEnumInitialization(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    const AbstractMetaClass* enclosingClass = getProperEnclosingClassForEnum(cppEnum);
    const AbstractMetaClass* upper = enclosingClass ? enclosingClass->enclosingClass() : 0;
    bool hasUpperEnclosingClass = upper && upper->typeEntry()->codeGeneration() != TypeEntry::GenerateForSubclass;
    QString enclosingObjectVariable;
    if (enclosingClass)
        enclosingObjectVariable = QLatin1Char('&') + cpythonTypeName(enclosingClass);
    else if (hasUpperEnclosingClass)
        enclosingObjectVariable = QLatin1String("enclosingClass");
    else
        enclosingObjectVariable = QLatin1String("module");

    s << INDENT << "// Initialization of ";
    s << (cppEnum->isAnonymous() ? "anonymous enum identified by enum value" : "enum");
    s << " '" << cppEnum->name() << "'." << endl;

    if (!cppEnum->isAnonymous()) {
        FlagsTypeEntry* flags = cppEnum->typeEntry()->flags();
        if (flags) {
            s << INDENT << cpythonTypeNameExt(flags) << " = PySide::QFlags::create(\"" << flags->flagsName() << "\", &"
              << cpythonEnumName(cppEnum) << "_as_number);" << endl;
        }

        s << INDENT << cpythonTypeNameExt(cppEnum->typeEntry()) << " = Shiboken::Enum::";
        s << ((enclosingClass || hasUpperEnclosingClass) ? "createScopedEnum" : "createGlobalEnum");
        s << '(' << enclosingObjectVariable << ',' << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << '"' << cppEnum->name() << "\"," << endl;
            s << INDENT << '"' << getClassTargetFullName(cppEnum) << "\"," << endl;
            s << INDENT << '"' << (cppEnum->enclosingClass() ? (cppEnum->enclosingClass()->qualifiedCppName() + QLatin1String("::")) : QString());
            s << cppEnum->name() << '"';
            if (flags)
                s << ',' << endl << INDENT << cpythonTypeNameExt(flags);
            s << ");" << endl;
        }
        s << INDENT << "if (!" << cpythonTypeNameExt(cppEnum->typeEntry()) << ')' << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return " << m_currentErrorCode << ';' << endl << endl;
        }
    }

    foreach (const AbstractMetaEnumValue* enumValue, cppEnum->values()) {
        if (cppEnum->typeEntry()->isEnumValueRejected(enumValue->name()))
            continue;

        QString enumValueText;
        if (!avoidProtectedHack() || !cppEnum->isProtected()) {
            enumValueText = QLatin1String("(long) ");
            if (cppEnum->enclosingClass())
                enumValueText += cppEnum->enclosingClass()->qualifiedCppName() + QLatin1String("::");
            enumValueText += enumValue->name();
        } else {
            enumValueText += QString::number(enumValue->value());
        }

        if (cppEnum->isAnonymous()) {
            if (enclosingClass || hasUpperEnclosingClass) {
                s << INDENT << '{' << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "PyObject* anonEnumItem = PyInt_FromLong(" << enumValueText << ");" << endl;
                    s << INDENT << "if (PyDict_SetItemString(reinterpret_cast<SbkObjectType *>(" << enclosingObjectVariable
                        << ")->super.ht_type.tp_dict, \"" << enumValue->name() << "\", anonEnumItem) < 0)" << endl;
                    {
                        Indentation indent(INDENT);
                        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
                    }
                    s << INDENT << "Py_DECREF(anonEnumItem);" << endl;
                }
                s << INDENT << '}' << endl;
            } else {
                s << INDENT << "if (PyModule_AddIntConstant(module, \"" << enumValue->name() << "\", ";
                s << enumValueText << ") < 0)" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "return " << m_currentErrorCode << ';' << endl;
                }
            }
        } else {
            s << INDENT << "if (!Shiboken::Enum::";
            s << ((enclosingClass || hasUpperEnclosingClass) ? "createScopedEnumItem" : "createGlobalEnumItem");
            s << '(' << cpythonTypeNameExt(cppEnum->typeEntry()) << ',' << endl;
            Indentation indent(INDENT);
            s << INDENT << enclosingObjectVariable << ", \"" << enumValue->name() << "\", ";
            s << enumValueText << "))" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
    }

    writeEnumConverterInitialization(s, cppEnum);

    s << INDENT << "// End of '" << cppEnum->name() << "' enum";
    if (cppEnum->typeEntry()->flags())
        s << "/flags";
    s << '.' << endl << endl;
}

void CppGenerator::writeSignalInitialization(QTextStream& s, const AbstractMetaClass* metaClass)
{
    // Try to check something and print some warnings
    foreach (const AbstractMetaFunction* cppSignal, metaClass->cppSignalFunctions()) {
        if (cppSignal->declaringClass() != metaClass)
            continue;
        foreach (AbstractMetaArgument* arg, cppSignal->arguments()) {
            AbstractMetaType* metaType = arg->type();
            const QByteArray origType =
                QMetaObject::normalizedType(qPrintable(metaType->originalTypeDescription()));
            const QByteArray cppSig =
                QMetaObject::normalizedType(qPrintable(metaType->cppSignature()));
            if ((origType != cppSig) && (!metaType->isFlags())) {
                qCWarning(lcShiboken).noquote().nospace()
                    << "Typedef used on signal " << metaClass->qualifiedCppName() << "::"
                    << cppSignal->signature();
                }
        }
    }

    s << INDENT << "PySide::Signal::registerSignals(&" << cpythonTypeName(metaClass) << ", &::"
                << metaClass->qualifiedCppName() << "::staticMetaObject);" << endl;
}

void CppGenerator::writeFlagsToLong(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    if (!flagsEntry)
        return;
    s << "static PyObject* " << cpythonEnumName(cppEnum) << "_long(PyObject* " PYTHON_SELF_VAR ")" << endl;
    s << "{" << endl;
    s << INDENT << "int val;" << endl;
    AbstractMetaType* flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << INDENT << cpythonToCppConversionFunction(flagsType) << PYTHON_SELF_VAR << ", &val);" << endl;
    s << INDENT << "return Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<int>(), &val);" << endl;
    s << "}" << endl;
}

void CppGenerator::writeFlagsNonZero(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    if (!flagsEntry)
        return;
    s << "static int " << cpythonEnumName(cppEnum) << "__nonzero(PyObject* " PYTHON_SELF_VAR ")" << endl;
    s << "{" << endl;

    s << INDENT << "int val;" << endl;
    AbstractMetaType* flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << INDENT << cpythonToCppConversionFunction(flagsType) << PYTHON_SELF_VAR << ", &val);" << endl;
    s << INDENT << "return val != 0;" << endl;
    s << "}" << endl;
}

void CppGenerator::writeFlagsMethods(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("and"), QLatin1String("&"));
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("or"), QLatin1String("|"));
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("xor"), QLatin1String("^"));

    writeFlagsUnaryOperator(s, cppEnum, QLatin1String("invert"), QLatin1String("~"));
    writeFlagsToLong(s, cppEnum);
    writeFlagsNonZero(s, cppEnum);

    s << endl;
}

void CppGenerator::writeFlagsNumberMethodsDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);

    s << "static PyNumberMethods " << cpythonName << "_as_number = {" << endl;
    s << INDENT << "/*nb_add*/                  0," << endl;
    s << INDENT << "/*nb_subtract*/             0," << endl;
    s << INDENT << "/*nb_multiply*/             0," << endl;
    s << INDENT << "#ifndef IS_PY3K" << endl;
    s << INDENT << "/* nb_divide */             0," << endl;
    s << INDENT << "#endif" << endl;
    s << INDENT << "/*nb_remainder*/            0," << endl;
    s << INDENT << "/*nb_divmod*/               0," << endl;
    s << INDENT << "/*nb_power*/                0," << endl;
    s << INDENT << "/*nb_negative*/             0," << endl;
    s << INDENT << "/*nb_positive*/             0," << endl;
    s << INDENT << "/*nb_absolute*/             0," << endl;
    s << INDENT << "/*nb_nonzero*/              " << cpythonName << "__nonzero," << endl;
    s << INDENT << "/*nb_invert*/               (unaryfunc)" << cpythonName << "___invert__," << endl;
    s << INDENT << "/*nb_lshift*/               0," << endl;
    s << INDENT << "/*nb_rshift*/               0," << endl;
    s << INDENT << "/*nb_and*/                  (binaryfunc)" << cpythonName  << "___and__," << endl;
    s << INDENT << "/*nb_xor*/                  (binaryfunc)" << cpythonName  << "___xor__," << endl;
    s << INDENT << "/*nb_or*/                   (binaryfunc)" << cpythonName  << "___or__," << endl;
    s << INDENT << "#ifndef IS_PY3K" << endl;
    s << INDENT << "/* nb_coerce */             0," << endl;
    s << INDENT << "#endif" << endl;
    s << INDENT << "/*nb_int*/                  " << cpythonName << "_long," << endl;
    s << INDENT << "#ifdef IS_PY3K" << endl;
    s << INDENT << "/*nb_reserved*/             0," << endl;
    s << INDENT << "/*nb_float*/                0," << endl;
    s << INDENT << "#else" << endl;
    s << INDENT << "/*nb_long*/                 " << cpythonName << "_long," << endl;
    s << INDENT << "/*nb_float*/                0," << endl;
    s << INDENT << "/*nb_oct*/                  0," << endl;
    s << INDENT << "/*nb_hex*/                  0," << endl;
    s << INDENT << "#endif" << endl;
    s << INDENT << "/*nb_inplace_add*/          0," << endl;
    s << INDENT << "/*nb_inplace_subtract*/     0," << endl;
    s << INDENT << "/*nb_inplace_multiply*/     0," << endl;
    s << INDENT << "#ifndef IS_PY3K" << endl;
    s << INDENT << "/*nb_inplace_divide*/       0," << endl;
    s << INDENT << "#endif" << endl;
    s << INDENT << "/*nb_inplace_remainder*/    0," << endl;
    s << INDENT << "/*nb_inplace_power*/        0," << endl;
    s << INDENT << "/*nb_inplace_lshift*/       0," << endl;
    s << INDENT << "/*nb_inplace_rshift*/       0," << endl;
    s << INDENT << "/*nb_inplace_and*/          0," << endl;
    s << INDENT << "/*nb_inplace_xor*/          0," << endl;
    s << INDENT << "/*nb_inplace_or*/           0," << endl;
    s << INDENT << "/*nb_floor_divide*/         0," << endl;
    s << INDENT << "/*nb_true_divide*/          0," << endl;
    s << INDENT << "/*nb_inplace_floor_divide*/ 0," << endl;
    s << INDENT << "/*nb_inplace_true_divide*/  0," << endl;
    s << INDENT << "/*nb_index*/                0" << endl;
    s << "};" << endl << endl;
}

void CppGenerator::writeFlagsBinaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                            QString pyOpName, QString cppOpName)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject* " << cpythonEnumName(cppEnum) << "___" << pyOpName << "__(PyObject* " PYTHON_SELF_VAR ", PyObject* " PYTHON_ARG ")" << endl;
    s << '{' << endl;

    AbstractMetaType* flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << INDENT << "::" << flagsEntry->originalName() << " cppResult, " CPP_SELF_VAR ", cppArg;" << endl;
    s << "#ifdef IS_PY3K" << endl;
    s << INDENT << CPP_SELF_VAR " = (::" << flagsEntry->originalName() << ")(int)PyLong_AsLong(" PYTHON_SELF_VAR ");" << endl;
    s << INDENT << "cppArg = (" << flagsEntry->originalName() << ")(int)PyLong_AsLong(" PYTHON_ARG ");" << endl;
    s << "#else" << endl;
    s << INDENT << CPP_SELF_VAR " = (::" << flagsEntry->originalName() << ")(int)PyInt_AsLong(" PYTHON_SELF_VAR ");" << endl;
    s << INDENT << "cppArg = (" << flagsEntry->originalName() << ")(int)PyInt_AsLong(" PYTHON_ARG ");" << endl;
    s << "#endif" << endl << endl;
    s << INDENT << "cppResult = " CPP_SELF_VAR " " << cppOpName << " cppArg;" << endl;
    s << INDENT << "return ";
    writeToPythonConversion(s, flagsType, 0, QLatin1String("cppResult"));
    s << ';' << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeFlagsUnaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                           QString pyOpName, QString cppOpName, bool boolResult)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject* " << cpythonEnumName(cppEnum) << "___" << pyOpName << "__(PyObject* " PYTHON_SELF_VAR ", PyObject* " PYTHON_ARG ")" << endl;
    s << '{' << endl;

    AbstractMetaType* flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << INDENT << "::" << flagsEntry->originalName() << " " CPP_SELF_VAR ";" << endl;
    s << INDENT << cpythonToCppConversionFunction(flagsType) << PYTHON_SELF_VAR << ", &" CPP_SELF_VAR ");" << endl;
    s << INDENT;
    if (boolResult)
        s << "bool";
    else
        s << "::" << flagsEntry->originalName();
    s << " cppResult = " << cppOpName << CPP_SELF_VAR ";" << endl;
    s << INDENT << "return ";
    if (boolResult)
        s << "PyBool_FromLong(cppResult)";
    else
        writeToPythonConversion(s, flagsType, 0, QLatin1String("cppResult"));
    s << ';' << endl;
    s << '}' << endl << endl;
}

QString CppGenerator::getInitFunctionName(GeneratorContext &context) const
{
    QString initFunctionName;
    if (!context.forSmartPointer()) {
        initFunctionName = context.metaClass()->qualifiedCppName();
        initFunctionName.replace(QLatin1String("::"), QLatin1String("_"));
    } else {
        initFunctionName = getFilteredCppSignatureString(context.preciseType()->cppSignature());
    }
    return initFunctionName;
}

void CppGenerator::writeClassRegister(QTextStream &s,
                                      const AbstractMetaClass *metaClass,
                                      GeneratorContext &classContext,
                                      QTextStream &signatureStream)
{
    const ComplexTypeEntry* classTypeEntry = metaClass->typeEntry();

    const AbstractMetaClass* enc = metaClass->enclosingClass();
    bool hasEnclosingClass = enc && enc->typeEntry()->codeGeneration() != TypeEntry::GenerateForSubclass;
    QString enclosingObjectVariable = hasEnclosingClass ? QLatin1String("enclosingClass") : QLatin1String("module");

    QString pyTypeName = cpythonTypeName(metaClass);
    QString initFunctionName = getInitFunctionName(classContext);

    // PYSIDE-510: Create a signatures string for the introspection feature.
    s << "// The signatures string for the functions." << endl;
    s << "// Multiple signatures have their index \"n:\" in front." << endl;
    s << "const char " << initFunctionName << "_SignaturesString[] = \"\"" << endl;
    QString line;
    while (signatureStream.readLineInto(&line)) {
        s << INDENT << '"' << line << "\\n\"" << endl;
    }
    s << ';' << endl << endl;
    s << "void init_" << initFunctionName;
    s << "(PyObject* " << enclosingObjectVariable << ")" << endl;
    s << '{' << endl;

    if (supportsNumberProtocol(metaClass)) {
        s << INDENT << "// type has number operators" << endl;
        writeTypeAsNumberDefinition(s, metaClass);
        s << INDENT << pyTypeName << ".super.ht_type.tp_as_number = &" << pyTypeName << "AsNumber;" << endl;
        s << endl;
    }

    if (supportsSequenceProtocol(metaClass)) {
        s << INDENT << "// type supports sequence protocol" << endl;
        writeTypeAsSequenceDefinition(s, metaClass);
        s << INDENT << pyTypeName << ".super.ht_type.tp_as_sequence = &" << pyTypeName << "AsSequence;" << endl;
        s << endl;
    }

    if (supportsMappingProtocol(metaClass)) {
        s << INDENT << "// type supports mapping protocol" << endl;
        writeTypeAsMappingDefinition(s, metaClass);
        s << INDENT << pyTypeName << ".super.ht_type.tp_as_mapping = &" << pyTypeName << "AsMapping;" << endl;
        s << endl;
    }

    if (!classContext.forSmartPointer())
        s << INDENT << cpythonTypeNameExt(classTypeEntry) << endl;
    else
        s << INDENT << cpythonTypeNameExt(classContext.preciseType()) << endl;

    s << INDENT << "    = reinterpret_cast<PyTypeObject*>(&" << pyTypeName << ");" << endl;
    s << endl;

    // Multiple inheritance
    QString pyTypeBasesVariable = pyTypeName + QLatin1String("_bases");
    const AbstractMetaClassList baseClasses = getBaseClasses(metaClass);
    if (metaClass->baseClassNames().size() > 1) {
        s << INDENT << "PyObject* " << pyTypeBasesVariable << " = PyTuple_Pack(" << baseClasses.size() << ',' << endl;
        QStringList bases;
        foreach (const AbstractMetaClass* base, baseClasses)
            bases << QLatin1String("(PyObject*)") + cpythonTypeNameExt(base->typeEntry());
        Indentation indent(INDENT);
        QString separator;
        QTextStream sep(&separator);
        sep << "," << endl << INDENT;
        s << INDENT << bases.join(separator) << ");" << endl << endl;
    }

    // Create type and insert it in the module or enclosing class.
    s << INDENT << "if (!Shiboken::ObjectType::introduceWrapperType(" << enclosingObjectVariable;
    QString typeName;
    if (!classContext.forSmartPointer())
        typeName = metaClass->name();
    else
        typeName = classContext.preciseType()->cppSignature();

    s << ", \"" << typeName << "\", \"";

    // Original name
    if (!classContext.forSmartPointer())
        s << metaClass->qualifiedCppName() << (isObjectType(classTypeEntry) ?  "*" : "");
    else
        s << classContext.preciseType()->cppSignature();

    s << "\"," << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "&" << pyTypeName << "," << endl;
        s << INDENT << initFunctionName << "_SignaturesString";

        // Set destructor function
        if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
            QString dtorClassName = metaClass->qualifiedCppName();
            if ((avoidProtectedHack() && metaClass->hasProtectedDestructor()) || classTypeEntry->isValue())
                dtorClassName = wrapperName(metaClass);
            if (classContext.forSmartPointer())
                dtorClassName = wrapperName(classContext.preciseType());

            s << ", &Shiboken::callCppDestructor< ::" << dtorClassName << " >";
        } else if (metaClass->baseClass() || hasEnclosingClass) {
            s << ", 0";
        }

        // Base type
        if (metaClass->baseClass()) {
            s << ", reinterpret_cast<SbkObjectType *>(" << cpythonTypeNameExt(metaClass->baseClass()->typeEntry()) << ')';
            // The other base types
            if (metaClass->baseClassNames().size() > 1)
                s << ", " << pyTypeBasesVariable;
            else if (hasEnclosingClass)
                s << ", 0";
        } else if (hasEnclosingClass) {
            s << ", 0, 0";
        }
        if (hasEnclosingClass)
            s << ", true";
        s << ")) {" << endl;
        s << INDENT << "return;" << endl;
    }
    s << INDENT << '}' << endl << endl;

    // Register conversions for the type.
    writeConverterRegister(s, metaClass, classContext);
    s << endl;

    // class inject-code target/beginning
    if (!classTypeEntry->codeSnips().isEmpty()) {
        writeCodeSnips(s, classTypeEntry->codeSnips(), TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode, metaClass);
        s << endl;
    }

    // Fill multiple inheritance data, if needed.
    const AbstractMetaClass* miClass = getMultipleInheritingClass(metaClass);
    if (miClass) {
        s << INDENT << "MultipleInheritanceInitFunction func = ";
        if (miClass == metaClass) {
            s << multipleInheritanceInitializerFunctionName(miClass) << ";" << endl;
        } else {
            s << "Shiboken::ObjectType::getMultipleIheritanceFunction(reinterpret_cast<SbkObjectType*>(";
            s << cpythonTypeNameExt(miClass->typeEntry()) << "));" << endl;
        }
        s << INDENT << "Shiboken::ObjectType::setMultipleIheritanceFunction(&";
        s << cpythonTypeName(metaClass) << ", func);" << endl;
        s << INDENT << "Shiboken::ObjectType::setCastFunction(&" << cpythonTypeName(metaClass);
        s << ", &" << cpythonSpecialCastFunctionName(metaClass) << ");" << endl;
    }

    // Set typediscovery struct or fill the struct of another one
    if (metaClass->isPolymorphic() && metaClass->baseClass()) {
        s << INDENT << "Shiboken::ObjectType::setTypeDiscoveryFunctionV2(&" << cpythonTypeName(metaClass);
        s << ", &" << cpythonBaseName(metaClass) << "_typeDiscovery);" << endl << endl;
    }

    AbstractMetaEnumList classEnums = metaClass->enums();
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses())
        lookForEnumsInClassesNotToBeGenerated(classEnums, innerClass);

    ErrorCode errorCode(QString::null);
    writeEnumsInitialization(s, classEnums);

    if (metaClass->hasSignals())
        writeSignalInitialization(s, metaClass);

    // Write static fields
    foreach (const AbstractMetaField* field, metaClass->fields()) {
        if (!field->isStatic())
            continue;
        s << INDENT << QLatin1String("PyDict_SetItemString(") + cpythonTypeName(metaClass) + QLatin1String(".super.ht_type.tp_dict, \"");
        s << field->name() << "\", ";
        writeToPythonConversion(s, field->type(), metaClass, metaClass->qualifiedCppName() + QLatin1String("::") + field->name());
        s << ");" << endl;
    }
    s << endl;

    // class inject-code target/end
    if (!classTypeEntry->codeSnips().isEmpty()) {
        s << endl;
        writeCodeSnips(s, classTypeEntry->codeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode, metaClass);
    }

    if (usePySideExtensions()) {
        if (avoidProtectedHack() && shouldGenerateCppWrapper(metaClass))
            s << INDENT << wrapperName(metaClass) << "::pysideInitQtMetaTypes();\n";
        else
            writeInitQtMetaTypeFunctionBody(s, classContext);
    }

    if (usePySideExtensions() && metaClass->isQObject()) {
        s << INDENT << "Shiboken::ObjectType::setSubTypeInitHook(&" << pyTypeName << ", &PySide::initQObjectSubType);" << endl;
        s << INDENT << "PySide::initDynamicMetaObject(&" << pyTypeName << ", &::" << metaClass->qualifiedCppName()
          << "::staticMetaObject, sizeof(::" << metaClass->qualifiedCppName() << "));" << endl;
    }

    s << '}' << endl;
}

void CppGenerator::writeInitQtMetaTypeFunctionBody(QTextStream &s, GeneratorContext &context) const
{
    const AbstractMetaClass* metaClass = context.metaClass();
    // Gets all class name variants used on different possible scopes
    QStringList nameVariants;
    if (!context.forSmartPointer())
        nameVariants << metaClass->name();
    else
        nameVariants << context.preciseType()->cppSignature();

    const AbstractMetaClass* enclosingClass = metaClass->enclosingClass();
    while (enclosingClass) {
        if (enclosingClass->typeEntry()->generateCode())
            nameVariants << (enclosingClass->name() + QLatin1String("::") + nameVariants.last());
        enclosingClass = enclosingClass->enclosingClass();
    }

    QString className;
    if (!context.forSmartPointer())
        className = metaClass->qualifiedCppName();
    else
        className = context.preciseType()->cppSignature();

    if (!metaClass->isNamespace() && !metaClass->isAbstract())  {
        // Qt metatypes are registered only on their first use, so we do this now.
        bool canBeValue = false;
        if (!isObjectType(metaClass)) {
            // check if there's a empty ctor
            foreach (AbstractMetaFunction* func, metaClass->functions()) {
                if (func->isConstructor() && !func->arguments().count()) {
                    canBeValue = true;
                    break;
                }
            }
        }

        if (canBeValue) {
            foreach (const QString &name, nameVariants) {
                if (name == QLatin1String("iterator")) {
                    qCWarning(lcShiboken).noquote().nospace()
                         << QString::fromLatin1("%1:%2 FIXME:\n"
                                                "    The code tried to qRegisterMetaType the unqualified name "
                                                "'iterator'. This is currently fixed by a hack(ct) and needs improvement!")
                                                .arg(QFile::decodeName(__FILE__)).arg(__LINE__);
                    continue;
                }
                s << INDENT << "qRegisterMetaType< ::" << className << " >(\"" << name << "\");" << endl;
            }
        }
    }

    foreach (AbstractMetaEnum* metaEnum, metaClass->enums()) {
        if (!metaEnum->isPrivate() && !metaEnum->isAnonymous()) {
            foreach (const QString &name, nameVariants)
                s << INDENT << "qRegisterMetaType< ::" << metaEnum->typeEntry()->qualifiedCppName() << " >(\"" << name << "::" << metaEnum->name() << "\");" << endl;

            if (metaEnum->typeEntry()->flags()) {
                QString n = metaEnum->typeEntry()->flags()->originalName();
                s << INDENT << "qRegisterMetaType< ::" << n << " >(\"" << n << "\");" << endl;
            }
        }
    }
}

void CppGenerator::writeTypeDiscoveryFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString polymorphicExpr = metaClass->typeEntry()->polymorphicIdValue();

    s << "static void* " << cpythonBaseName(metaClass) << "_typeDiscovery(void* cptr, SbkObjectType* instanceType)\n{" << endl;

    if (!polymorphicExpr.isEmpty()) {
        polymorphicExpr = polymorphicExpr.replace(QLatin1String("%1"),
                                                  QLatin1String(" reinterpret_cast< ::")
                                                  + metaClass->qualifiedCppName()
                                                  + QLatin1String("*>(cptr)"));
        s << INDENT << " if (" << polymorphicExpr << ")" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return cptr;" << endl;
        }
    } else if (metaClass->isPolymorphic()) {
        AbstractMetaClassList ancestors = getAllAncestors(metaClass);
        foreach (AbstractMetaClass* ancestor, ancestors) {
            if (ancestor->baseClass())
                continue;
            if (ancestor->isPolymorphic()) {
                s << INDENT << "if (instanceType == reinterpret_cast<SbkObjectType*>(Shiboken::SbkType< ::"
                            << ancestor->qualifiedCppName() << " >()))" << endl;
                Indentation indent(INDENT);
                s << INDENT << "return dynamic_cast< ::" << metaClass->qualifiedCppName()
                            << "*>(reinterpret_cast< ::"<< ancestor->qualifiedCppName() << "*>(cptr));" << endl;
            } else {
                qCWarning(lcShiboken).noquote().nospace()
                    << metaClass->qualifiedCppName() << " inherits from a non polymorphic type ("
                    << ancestor->qualifiedCppName() << "), type discovery based on RTTI is "
                       "impossible, write a polymorphic-id-expression for this type.";
            }

        }
    }
    s << INDENT << "return 0;" << endl;
    s << "}\n\n";
}

QString CppGenerator::writeSmartPointerGetterCast() {
    return QLatin1String("const_cast<char *>(" SMART_POINTER_GETTER ")");
}

void CppGenerator::writeSetattroFunction(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass* metaClass = context.metaClass();
    s << "static int " << cpythonSetattroFunctionName(metaClass) << "(PyObject* " PYTHON_SELF_VAR ", PyObject* name, PyObject* value)" << endl;
    s << '{' << endl;
    if (usePySideExtensions()) {
        s << INDENT << "Shiboken::AutoDecRef pp(reinterpret_cast<PyObject*>(PySide::Property::getObject(" PYTHON_SELF_VAR ", name)));" << endl;
        s << INDENT << "if (!pp.isNull())" << endl;
        Indentation indent(INDENT);
        s << INDENT << "return PySide::Property::setValue(reinterpret_cast<PySideProperty*>(pp.object()), " PYTHON_SELF_VAR ", value);" << endl;
    }

    if (context.forSmartPointer()) {
        s << INDENT << "// Try to find the 'name' attribute, by retrieving the PyObject for the corresponding C++ object held by the smart pointer." << endl;
        s << INDENT << "PyObject *rawObj = PyObject_CallMethod(" PYTHON_SELF_VAR ", "
          << writeSmartPointerGetterCast() << ", 0);" << endl;
        s << INDENT << "if (rawObj) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "int hasAttribute = PyObject_HasAttr(rawObj, name);" << endl;
            s << INDENT << "if (hasAttribute) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "return PyObject_GenericSetAttr(rawObj, name, value);" << endl;
            }
            s << INDENT << '}' << endl;
            s << INDENT << "Py_DECREF(rawObj);" << endl;
        }
        s << INDENT << '}' << endl;

    }

    s << INDENT << "return PyObject_GenericSetAttr(" PYTHON_SELF_VAR ", name, value);" << endl;
    s << '}' << endl;
}

static inline QString qObjectClassName() { return QStringLiteral("QObject"); }
static inline QString qMetaObjectClassName() { return QStringLiteral("QMetaObject"); }

void CppGenerator::writeGetattroFunction(QTextStream& s, GeneratorContext &context)
{
    const AbstractMetaClass* metaClass = context.metaClass();
    s << "static PyObject* " << cpythonGetattroFunctionName(metaClass) << "(PyObject* " PYTHON_SELF_VAR ", PyObject* name)" << endl;
    s << '{' << endl;

    QString getattrFunc;
    if (usePySideExtensions() && metaClass->isQObject()) {
        AbstractMetaClass *qobjectClass = AbstractMetaClass::findClass(classes(), qObjectClassName());
        getattrFunc = QString::fromLatin1("PySide::getMetaDataFromQObject(%1, " PYTHON_SELF_VAR ", name)")
                         .arg(cpythonWrapperCPtr(qobjectClass, QLatin1String(PYTHON_SELF_VAR)));
    } else {
        getattrFunc = QLatin1String("PyObject_GenericGetAttr(" PYTHON_SELF_VAR ", name)");
    }

    if (classNeedsGetattroFunction(metaClass)) {
        s << INDENT << "if (" PYTHON_SELF_VAR ") {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "// Search the method in the instance dict" << endl;
            s << INDENT << "if (reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR ")->ob_dict) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyObject* meth = PyDict_GetItem(reinterpret_cast<SbkObject*>(" PYTHON_SELF_VAR ")->ob_dict, name);" << endl;
                s << INDENT << "if (meth) {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "Py_INCREF(meth);" << endl;
                    s << INDENT << "return meth;" << endl;
                }
                s << INDENT << '}' << endl;
            }
            s << INDENT << '}' << endl;
            s << INDENT << "// Search the method in the type dict" << endl;
            s << INDENT << "if (Shiboken::Object::isUserType(" PYTHON_SELF_VAR ")) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyObject* meth = PyDict_GetItem(" PYTHON_SELF_VAR "->ob_type->tp_dict, name);" << endl;
                s << INDENT << "if (meth)" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "return PyFunction_Check(meth) ? SBK_PyMethod_New(meth, " PYTHON_SELF_VAR ") : " << getattrFunc << ';' << endl;
                }
            }
            s << INDENT << '}' << endl;

            foreach (const AbstractMetaFunction* func, getMethodsWithBothStaticAndNonStaticMethods(metaClass)) {
                QString defName = cpythonMethodDefinitionName(func);
                s << INDENT << "static PyMethodDef non_static_" << defName << " = {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << defName << ".ml_name," << endl;
                    s << INDENT << defName << ".ml_meth," << endl;
                    s << INDENT << defName << ".ml_flags & (~METH_STATIC)," << endl;
                    s << INDENT << defName << ".ml_doc," << endl;
                }
                s << INDENT << "};" << endl;
                s << INDENT << "if (Shiboken::String::compare(name, \"" << func->name() << "\") == 0)" << endl;
                Indentation indent(INDENT);
                s << INDENT << "return PyCFunction_NewEx(&non_static_" << defName << ", " PYTHON_SELF_VAR ", 0);" << endl;
            }
        }
        s << INDENT << '}' << endl;
    }

    if (context.forSmartPointer()) {
        s << INDENT << "PyObject *tmp = " << getattrFunc << ';' << endl;
        s << INDENT << "if (tmp) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return tmp;" << endl;
        }
        s << INDENT << "} else {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "if (!PyErr_ExceptionMatches(PyExc_AttributeError)) return NULL;" << endl;
            s << INDENT << "PyErr_Clear();" << endl;

            s << INDENT << "// Try to find the 'name' attribute, by retrieving the PyObject for "
                           "the corresponding C++ object held by the smart pointer." << endl;
            s << INDENT << "PyObject *rawObj = PyObject_CallMethod(" PYTHON_SELF_VAR ", "
              << writeSmartPointerGetterCast() << ", 0);" << endl;
            s << INDENT << "if (rawObj) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyObject *attribute = PyObject_GetAttr(rawObj, name);" << endl;
                s << INDENT << "if (attribute) {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT << "tmp = attribute;" << endl;
                }
                s << INDENT << '}' << endl;
                s << INDENT << "Py_DECREF(rawObj);" << endl;
            }
            s << INDENT << '}' << endl;
            s << INDENT << "if (!tmp) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyTypeObject *tp = Py_TYPE(self);" << endl;
                s << INDENT << "PyErr_Format(PyExc_AttributeError," << endl;
                s << INDENT << "             \"'%.50s' object has no attribute '%.400s'\"," << endl;
                s << INDENT << "             tp->tp_name, PyBytes_AS_STRING(name));" << endl;
                s << INDENT << "return NULL;" << endl;
            }
            s << INDENT << "} else {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "return tmp;" << endl;
            }
            s << INDENT << '}' << endl;

        }
        s << INDENT << '}' << endl;

    } else {
        s << INDENT << "return " << getattrFunc << ';' << endl;
    }
    s << '}' << endl;
}

bool CppGenerator::finishGeneration()
{
    //Generate CPython wrapper file
    QString classInitDecl;
    QTextStream s_classInitDecl(&classInitDecl);
    QString classPythonDefines;
    QTextStream s_classPythonDefines(&classPythonDefines);

    QSet<Include> includes;
    QString globalFunctionImpl;
    QTextStream s_globalFunctionImpl(&globalFunctionImpl);
    QString globalFunctionDecl;
    QTextStream s_globalFunctionDef(&globalFunctionDecl);
    QString signaturesString;
    QTextStream signatureStream(&signaturesString);

    Indentation indent(INDENT);

    const FunctionGroupMap &functionGroups = getFunctionGroups();
    for (FunctionGroupMapIt it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, it.value()) {
            if (!func->isModifiedRemoved()) {
                overloads.append(func);
                if (func->typeEntry())
                    includes << func->typeEntry()->include();
            }
        }

        if (overloads.isEmpty())
            continue;

        // Dummy context to satisfy the API.
        GeneratorContext classContext;
        writeMethodWrapper(s_globalFunctionImpl, overloads, classContext);
        writeSignatureInfo(signatureStream, overloads);
        writeMethodDefinition(s_globalFunctionDef, overloads);
    }

    //this is a temporary solution before new type revison implementation
    //We need move QMetaObject register before QObject
    Dependencies additionalDependencies;
    const AbstractMetaClassList &allClasses = classes();
    if (AbstractMetaClass::findClass(allClasses, qObjectClassName()) != Q_NULLPTR
        && AbstractMetaClass::findClass(allClasses, qMetaObjectClassName()) != Q_NULLPTR) {
        Dependency dependency;
        dependency.parent = qMetaObjectClassName();
        dependency.child = qObjectClassName();
        additionalDependencies.append(dependency);
    }
    const AbstractMetaClassList lst = classesTopologicalSorted(additionalDependencies);

    foreach (const AbstractMetaClass* cls, lst) {
        if (!shouldGenerate(cls))
            continue;

        s_classInitDecl << "void init_" << cls->qualifiedCppName().replace(QLatin1String("::"), QLatin1String("_")) << "(PyObject* module);" << endl;

        QString defineStr = QLatin1String("init_") + cls->qualifiedCppName().replace(QLatin1String("::"), QLatin1String("_"));

        if (cls->enclosingClass() && (cls->enclosingClass()->typeEntry()->codeGeneration() != TypeEntry::GenerateForSubclass))
            defineStr += QLatin1Char('(') + cpythonTypeNameExt(cls->enclosingClass()->typeEntry()) + QLatin1String("->tp_dict);");
        else
            defineStr += QLatin1String("(module);");
        s_classPythonDefines << INDENT << defineStr << endl;
    }

    // Initialize smart pointer types.
    foreach (const AbstractMetaType *metaType, instantiatedSmartPointers()) {
        GeneratorContext context(0, metaType, true);
        QString initFunctionName = getInitFunctionName(context);
        s_classInitDecl << "void init_" << initFunctionName << "(PyObject* module);" << endl;
        QString defineStr = QLatin1String("init_") + initFunctionName;
        defineStr += QLatin1String("(module);");
        s_classPythonDefines << INDENT << defineStr << endl;
    }

    QString moduleFileName(outputDirectory() + QLatin1Char('/') + subDirectoryForPackage(packageName()));
    moduleFileName += QLatin1Char('/') + moduleName().toLower() + QLatin1String("_module_wrapper.cpp");

    QFile file(moduleFileName);
    verifyDirectoryFor(file);
    if (!file.open(QFile::WriteOnly)) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Error writing file: " << QDir::toNativeSeparators(moduleFileName);
        return false;
    }

    QTextStream s(&file);

    // write license comment
    s << licenseComment() << endl;

    s << "#include <sbkpython.h>" << endl;
    s << "#include <shiboken.h>" << endl;
    s << "#include <algorithm>" << endl;
    if (usePySideExtensions()) {
        s << includeQDebug;
        s << "#include <pyside.h>" << endl;
        s << "#include <signature.h>" << endl;
        s << "#include <qapp_macro.h>" << endl;
    }

    s << "#include \"" << getModuleHeaderFileName() << '"' << endl << endl;
    foreach (const Include& include, includes)
        s << include;
    s << endl;

    // Global enums
    AbstractMetaEnumList globalEnums = this->globalEnums();
    foreach (const AbstractMetaClass* metaClass, classes()) {
        const AbstractMetaClass* encClass = metaClass->enclosingClass();
        if (encClass && encClass->typeEntry()->codeGeneration() != TypeEntry::GenerateForSubclass)
            continue;
        lookForEnumsInClassesNotToBeGenerated(globalEnums, metaClass);
    }

    TypeDatabase* typeDb = TypeDatabase::instance();
    TypeSystemTypeEntry* moduleEntry = reinterpret_cast<TypeSystemTypeEntry*>(typeDb->findType(packageName()));

    //Extra includes
    s << endl << "// Extra includes" << endl;
    QList<Include> extraIncludes;
    if (moduleEntry)
        extraIncludes = moduleEntry->extraIncludes();
    foreach (AbstractMetaEnum* cppEnum, globalEnums)
        extraIncludes.append(cppEnum->typeEntry()->extraIncludes());
    qSort(extraIncludes.begin(), extraIncludes.end());
    foreach (const Include& inc, extraIncludes)
        s << inc;
    s << endl;

    s << "// Current module's type array." << endl;
    s << "PyTypeObject** " << cppApiVariableName() << ';' << endl;

    s << "// Current module's converter array." << endl;
    s << "SbkConverter** " << convertersVariableName() << ';' << endl;

    CodeSnipList snips;
    if (moduleEntry)
        snips = moduleEntry->codeSnips();

    // module inject-code native/beginning
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode);
        s << endl;
    }

    // cleanup staticMetaObject attribute
    if (usePySideExtensions()) {
        s << "void cleanTypesAttributes(void) {" << endl;
        s << INDENT << "for (int i = 0, imax = SBK_" << moduleName() << "_IDX_COUNT; i < imax; i++) {" << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT << "PyObject *pyType = reinterpret_cast<PyObject*>(" << cppApiVariableName() << "[i]);" << endl;
            s << INDENT << "if (pyType && PyObject_HasAttrString(pyType, \"staticMetaObject\"))"<< endl;
            {
                Indentation indentation(INDENT);
                s << INDENT << "PyObject_SetAttrString(pyType, \"staticMetaObject\", Py_None);" << endl;
            }
        }
        s << INDENT << "}" << endl;
        s << "}" << endl;
    }

    s << "// Global functions ";
    s << "------------------------------------------------------------" << endl;
    s << globalFunctionImpl << endl;

    s << "static PyMethodDef " << moduleName() << "_methods[] = {" << endl;
    s << globalFunctionDecl;
    s << INDENT << "{0} // Sentinel" << endl << "};" << endl << endl;

    s << "// Classes initialization functions ";
    s << "------------------------------------------------------------" << endl;
    s << classInitDecl << endl;

    if (!globalEnums.isEmpty()) {
        QString converterImpl;
        QTextStream convImpl(&converterImpl);

        s << "// Enum definitions ";
        s << "------------------------------------------------------------" << endl;
        foreach (const AbstractMetaEnum* cppEnum, globalEnums) {
            if (cppEnum->isAnonymous() || cppEnum->isPrivate())
                continue;
            writeEnumConverterFunctions(s, cppEnum);
            s << endl;
        }

        if (!converterImpl.isEmpty()) {
            s << "// Enum converters ";
            s << "------------------------------------------------------------" << endl;
            s << "namespace Shiboken" << endl << '{' << endl;
            s << converterImpl << endl;
            s << "} // namespace Shiboken" << endl << endl;
        }
    }

    QStringList requiredModules = typeDb->requiredTargetImports();
    if (!requiredModules.isEmpty())
        s << "// Required modules' type and converter arrays." << endl;
    foreach (const QString& requiredModule, requiredModules) {
        s << "PyTypeObject** " << cppApiVariableName(requiredModule) << ';' << endl;
        s << "SbkConverter** " << convertersVariableName(requiredModule) << ';' << endl;
    }
    s << endl;

    s << "// Module initialization ";
    s << "------------------------------------------------------------" << endl;
    ExtendedConverterData extendedConverters = getExtendedConverters();
    if (!extendedConverters.isEmpty()) {
        s << endl << "// Extended Converters." << endl << endl;
        for (ExtendedConverterData::const_iterator it = extendedConverters.cbegin(), end = extendedConverters.cend(); it != end; ++it) {
            const TypeEntry *externalType = it.key();
            s << "// Extended implicit conversions for " << externalType->qualifiedTargetLangName() << '.' << endl;
            foreach (const AbstractMetaClass* sourceClass, extendedConverters[externalType]) {
                AbstractMetaType* sourceType = buildAbstractMetaTypeFromAbstractMetaClass(sourceClass);
                AbstractMetaType* targetType = buildAbstractMetaTypeFromTypeEntry(externalType);
                writePythonToCppConversionFunctions(s, sourceType, targetType);
            }
        }
    }

    QList<const CustomConversion*> typeConversions = getPrimitiveCustomConversions();
    if (!typeConversions.isEmpty()) {
        s << endl << "// Primitive Type converters." << endl << endl;
        foreach (const CustomConversion* conversion, typeConversions) {
            s << "// C++ to Python conversion for type '" << conversion->ownerType()->qualifiedCppName() << "'." << endl;
            writeCppToPythonFunction(s, conversion);
            writeCustomConverterFunctions(s, conversion);
        }
        s << endl;
    }

    QList<const AbstractMetaType*> containers = instantiatedContainers();
    if (!containers.isEmpty()) {
        s << "// Container Type converters." << endl << endl;
        foreach (const AbstractMetaType* container, containers) {
            s << "// C++ to Python conversion for type '" << container->cppSignature() << "'." << endl;
            writeContainerConverterFunctions(s, container);
        }
        s << endl;
    }

    s << "#if defined _WIN32 || defined __CYGWIN__" << endl;
    s << "    #define SBK_EXPORT_MODULE __declspec(dllexport)" << endl;
    s << "#elif __GNUC__ >= 4" << endl;
    s << "    #define SBK_EXPORT_MODULE __attribute__ ((visibility(\"default\")))" << endl;
    s << "#else" << endl;
    s << "    #define SBK_EXPORT_MODULE" << endl;
    s << "#endif" << endl << endl;

    s << "#ifdef IS_PY3K" << endl;
    s << "static struct PyModuleDef moduledef = {" << endl;
    s << "    /* m_base     */ PyModuleDef_HEAD_INIT," << endl;
    s << "    /* m_name     */ \"" << moduleName() << "\"," << endl;
    s << "    /* m_doc      */ 0," << endl;
    s << "    /* m_size     */ -1," << endl;
    s << "    /* m_methods  */ " << moduleName() << "_methods," << endl;
    s << "    /* m_reload   */ 0," << endl;
    s << "    /* m_traverse */ 0," << endl;
    s << "    /* m_clear    */ 0," << endl;
    s << "    /* m_free     */ 0" << endl;
    s << "};" << endl << endl;
    s << "#endif" << endl;
    s << "SBK_MODULE_INIT_FUNCTION_BEGIN(" << moduleName() << ")" << endl;

    ErrorCode errorCode(QLatin1String("SBK_MODULE_INIT_ERROR"));
    // module inject-code target/beginning
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode);
        s << endl;
    }

    foreach (const QString& requiredModule, typeDb->requiredTargetImports()) {
        s << INDENT << "{" << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT << "Shiboken::AutoDecRef requiredModule(Shiboken::Module::import(\"" << requiredModule << "\"));" << endl;
            s << INDENT << "if (requiredModule.isNull())" << endl;
            {
                Indentation indentation(INDENT);
                s << INDENT << "return SBK_MODULE_INIT_ERROR;" << endl;
            }
            s << INDENT << cppApiVariableName(requiredModule) << " = Shiboken::Module::getTypes(requiredModule);" << endl;
            s << INDENT << convertersVariableName(requiredModule) << " = Shiboken::Module::getTypeConverters(requiredModule);" << endl;
        }
        s << INDENT << "}" << endl << endl;
    }

    int maxTypeIndex = getMaxTypeIndex() + instantiatedSmartPointers().size();
    if (maxTypeIndex) {
        s << INDENT << "// Create an array of wrapper types for the current module." << endl;
        s << INDENT << "static PyTypeObject* cppApi[SBK_" << moduleName() << "_IDX_COUNT];" << endl;
        s << INDENT << cppApiVariableName() << " = cppApi;" << endl << endl;
    }

    s << INDENT << "// Create an array of primitive type converters for the current module." << endl;
    s << INDENT << "static SbkConverter* sbkConverters[SBK_" << moduleName() << "_CONVERTERS_IDX_COUNT" << "];" << endl;
    s << INDENT << convertersVariableName() << " = sbkConverters;" << endl << endl;

    s << "#ifdef IS_PY3K" << endl;
    s << INDENT << "PyObject* module = Shiboken::Module::create(\""  << moduleName() << "\", &moduledef);" << endl;
    s << "#else" << endl;
    s << INDENT << "PyObject* module = Shiboken::Module::create(\""  << moduleName() << "\", ";
    s << moduleName() << "_methods);" << endl;
    s << "#endif" << endl << endl;

    //s << INDENT << "// Initialize converters for primitive types." << endl;
    //s << INDENT << "initConverters();" << endl << endl;

    s << INDENT << "// Initialize classes in the type system" << endl;
    s << classPythonDefines;

    if (!typeConversions.isEmpty()) {
        s << endl;
        foreach (const CustomConversion* conversion, typeConversions) {
            writePrimitiveConverterInitialization(s, conversion);
            s << endl;
        }
    }

    if (!containers.isEmpty()) {
        s << endl;
        foreach (const AbstractMetaType* container, containers) {
            writeContainerConverterInitialization(s, container);
            s << endl;
        }
    }

    if (!extendedConverters.isEmpty()) {
        s << endl;
        for (ExtendedConverterData::const_iterator it = extendedConverters.cbegin(), end = extendedConverters.cend(); it != end; ++it) {
            writeExtendedConverterInitialization(s, it.key(), it.value());
            s << endl;
        }
    }

    writeEnumsInitialization(s, globalEnums);

    s << INDENT << "// Register primitive types converters." << endl;
    foreach(const PrimitiveTypeEntry* pte, primitiveTypes()) {
        if (!pte->generateCode() || !pte->isCppPrimitive())
            continue;
        const TypeEntry *referencedType = pte->basicReferencedTypeEntry();
        if (!referencedType)
            continue;
        QString converter = converterObject(referencedType);
        QStringList cppSignature = pte->qualifiedCppName().split(QLatin1String("::"), QString::SkipEmptyParts);
        while (!cppSignature.isEmpty()) {
            QString signature = cppSignature.join(QLatin1String("::"));
            s << INDENT << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << signature << "\");" << endl;
            cppSignature.removeFirst();
        }
    }
    // Register type resolver for all containers found in signals.
    QSet<QByteArray> typeResolvers;
    foreach (AbstractMetaClass* metaClass, classes()) {
        if (!metaClass->isQObject() || !metaClass->typeEntry()->generateCode())
            continue;
        foreach (AbstractMetaFunction* func, metaClass->functions()) {
            if (func->isSignal()) {
                foreach (AbstractMetaArgument* arg, func->arguments()) {
                    if (arg->type()->isContainer()) {
                        QString value = translateType(arg->type(), metaClass, ExcludeConst | ExcludeReference);
                        if (value.startsWith(QLatin1String("::")))
                            value.remove(0, 2);
                        typeResolvers << QMetaObject::normalizedType(value.toUtf8().constData());
                    }
                }
            }
        }
    }

    s << endl;
    if (maxTypeIndex)
        s << INDENT << "Shiboken::Module::registerTypes(module, " << cppApiVariableName() << ");" << endl;
    s << INDENT << "Shiboken::Module::registerTypeConverters(module, " << convertersVariableName() << ");" << endl;

    s << endl << INDENT << "if (PyErr_Occurred()) {" << endl;
    {
        Indentation indentation(INDENT);
        s << INDENT << "PyErr_Print();" << endl;
        s << INDENT << "Py_FatalError(\"can't initialize module " << moduleName() << "\");" << endl;
    }
    s << INDENT << '}' << endl;

    // module inject-code target/end
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode);
        s << endl;
    }

    // module inject-code native/end
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode);
        s << endl;
    }

    if (usePySideExtensions()) {
        foreach (AbstractMetaEnum* metaEnum, globalEnums)
            if (!metaEnum->isAnonymous()) {
                s << INDENT << "qRegisterMetaType< ::" << metaEnum->typeEntry()->qualifiedCppName() << " >(\"" << metaEnum->name() << "\");" << endl;
            }

        // cleanup staticMetaObject attribute
        s << INDENT << "PySide::registerCleanupFunction(cleanTypesAttributes);" << endl << endl;

        // PYSIDE-510: Create a signatures string for the introspection feature.
        s << "// The signatures string for the global functions." << endl;
        s << "// Multiple signatures have their index \"n:\" in front." << endl;
        s << "const char " << moduleName() << "_SignaturesString[] = \"\"" << endl;
        QString line;
        while (signatureStream.readLineInto(&line)) {
            s << INDENT << '"' << line << "\\n\"" << endl;
        }
        s << ';' << endl;
        // finish the rest of __signature__ initialization.
        s << INDENT << "FinishSignatureInitialization(module, " << moduleName()
            << "_SignaturesString);" << endl;
        // initialize the qApp module.
        s << INDENT << "NotifyModuleForQApp(module);" << endl << endl;
    }

    s << "SBK_MODULE_INIT_FUNCTION_END" << endl;

    return true;
}

static ArgumentOwner getArgumentOwner(const AbstractMetaFunction* func, int argIndex)
{
    ArgumentOwner argOwner = func->argumentOwner(func->ownerClass(), argIndex);
    if (argOwner.index == ArgumentOwner::InvalidIndex)
        argOwner = func->argumentOwner(func->declaringClass(), argIndex);
    return argOwner;
}

bool CppGenerator::writeParentChildManagement(QTextStream& s, const AbstractMetaFunction* func, int argIndex, bool useHeuristicPolicy)
{
    const int numArgs = func->arguments().count();
    bool ctorHeuristicEnabled = func->isConstructor() && useCtorHeuristic() && useHeuristicPolicy;

    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(OverloadData(getFunctionGroups(func->implementingClass())[func->name()], this));

    ArgumentOwner argOwner = getArgumentOwner(func, argIndex);
    ArgumentOwner::Action action = argOwner.action;
    int parentIndex = argOwner.index;
    int childIndex = argIndex;
    if (ctorHeuristicEnabled && argIndex > 0 && numArgs) {
        AbstractMetaArgument* arg = func->arguments().at(argIndex-1);
        if (arg->name() == QLatin1String("parent") && isObjectType(arg->type())) {
            action = ArgumentOwner::Add;
            parentIndex = argIndex;
            childIndex = -1;
        }
    }

    QString parentVariable;
    QString childVariable;
    if (action != ArgumentOwner::Invalid) {
        if (!usePyArgs && argIndex > 1)
            qCWarning(lcShiboken).noquote().nospace()
                << "Argument index for parent tag out of bounds: " << func->signature();

        if (action == ArgumentOwner::Remove) {
            parentVariable = QLatin1String("Py_None");
        } else {
            if (parentIndex == 0) {
                parentVariable = QLatin1String(PYTHON_RETURN_VAR);
            } else if (parentIndex == -1) {
                parentVariable = QLatin1String(PYTHON_SELF_VAR);
            } else {
                parentVariable = usePyArgs
                    ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(parentIndex - 1)
                    : QLatin1String(PYTHON_ARG);
            }
        }

        if (childIndex == 0) {
            childVariable = QLatin1String(PYTHON_RETURN_VAR);
        } else if (childIndex == -1) {
            childVariable = QLatin1String(PYTHON_SELF_VAR);
        } else {
            childVariable = usePyArgs
                ? QString::fromLatin1(PYTHON_ARGS "[%1]").arg(childIndex - 1)
                : QLatin1String(PYTHON_ARG);
        }

        s << INDENT << "Shiboken::Object::setParent(" << parentVariable << ", " << childVariable << ");\n";
        return true;
    }

    return false;
}

void CppGenerator::writeParentChildManagement(QTextStream& s, const AbstractMetaFunction* func, bool useHeuristicForReturn)
{
    const int numArgs = func->arguments().count();

    // -1    = return value
    //  0    = self
    //  1..n = func. args.
    for (int i = -1; i <= numArgs; ++i)
        writeParentChildManagement(s, func, i, useHeuristicForReturn);

    if (useHeuristicForReturn)
        writeReturnValueHeuristics(s, func);
}

void CppGenerator::writeReturnValueHeuristics(QTextStream& s, const AbstractMetaFunction* func, const QString& self)
{
    AbstractMetaType *type = func->type();
    if (!useReturnValueHeuristic()
        || !func->ownerClass()
        || !type
        || func->isStatic()
        || func->isConstructor()
        || !func->typeReplaced(0).isEmpty()) {
        return;
    }

    ArgumentOwner argOwner = getArgumentOwner(func, ArgumentOwner::ReturnIndex);
    if (argOwner.action == ArgumentOwner::Invalid || argOwner.index != ArgumentOwner::ThisIndex) {
        if (isPointerToWrapperType(type))
            s << INDENT << "Shiboken::Object::setParent(" << self << ", " PYTHON_RETURN_VAR ");" << endl;
    }
}

void CppGenerator::writeHashFunction(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    s << "static Py_hash_t " << cpythonBaseName(metaClass) << "_HashFunc(PyObject* self) {" << endl;
    writeCppSelfDefinition(s, context);
    s << INDENT << "return " << metaClass->typeEntry()->hashFunction() << '(';
    s << (isObjectType(metaClass) ? "" : "*") << CPP_SELF_VAR << ");" << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeStdListWrapperMethods(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    ErrorCode errorCode(0);

    // __len__
    s << "Py_ssize_t " << cpythonBaseName(metaClass->typeEntry()) << "__len__(PyObject* " PYTHON_SELF_VAR ")" << endl;
    s << '{' << endl;
    writeCppSelfDefinition(s, context);
    s << INDENT << "return " CPP_SELF_VAR "->size();" << endl;
    s << '}' << endl;

    // __getitem__
    s << "PyObject* " << cpythonBaseName(metaClass->typeEntry()) << "__getitem__(PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i)" << endl;
    s << '{' << endl;
    writeCppSelfDefinition(s, context);
    writeIndexError(s, QLatin1String("index out of bounds"));

    s << INDENT << metaClass->qualifiedCppName() << "::iterator _item = " CPP_SELF_VAR "->begin();" << endl;
    s << INDENT << "for (Py_ssize_t pos = 0; pos < _i; pos++) _item++;" << endl;

    const AbstractMetaType* itemType = metaClass->templateBaseClassInstantiations().first();

    s << INDENT << "return ";
    writeToPythonConversion(s, itemType, metaClass, QLatin1String("*_item"));
    s << ';' << endl;
    s << '}' << endl;

    // __setitem__
    ErrorCode errorCode2(-1);
    s << "int " << cpythonBaseName(metaClass->typeEntry()) << "__setitem__(PyObject* " PYTHON_SELF_VAR ", Py_ssize_t _i, PyObject* pyArg)" << endl;
    s << '{' << endl;
    writeCppSelfDefinition(s, context);
    writeIndexError(s, QLatin1String("list assignment index out of range"));

    s << INDENT << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << ';' << endl;
    s << INDENT << "if (!";
    writeTypeCheck(s, itemType, QLatin1String("pyArg"), isNumber(itemType->typeEntry()));
    s << ") {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"attributed value with wrong type, '";
        s << itemType->name() << "' or other convertible type expected\");" << endl;
        s << INDENT << "return -1;" << endl;
    }
    s << INDENT << '}' << endl;
    writeArgumentConversion(s, itemType, QLatin1String("cppValue"), QLatin1String("pyArg"), metaClass);

    s << INDENT << metaClass->qualifiedCppName() << "::iterator _item = " CPP_SELF_VAR "->begin();" << endl;
    s << INDENT << "for (Py_ssize_t pos = 0; pos < _i; pos++) _item++;" << endl;
    s << INDENT << "*_item = cppValue;" << endl;
    s << INDENT << "return 0;" << endl;
    s << '}' << endl;
}
void CppGenerator::writeIndexError(QTextStream& s, const QString& errorMsg)
{
    s << INDENT << "if (_i < 0 || _i >= (Py_ssize_t) " CPP_SELF_VAR "->size()) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_IndexError, \"" << errorMsg << "\");" << endl;
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << INDENT << '}' << endl;
}

QString CppGenerator::writeReprFunction(QTextStream &s, GeneratorContext &context)
{
    const AbstractMetaClass *metaClass = context.metaClass();
    QString funcName = cpythonBaseName(metaClass) + QLatin1String("__repr__");
    s << "extern \"C\"" << endl;
    s << '{' << endl;
    s << "static PyObject* " << funcName << "(PyObject* self)" << endl;
    s << '{' << endl;
    writeCppSelfDefinition(s, context);
    s << INDENT << "QBuffer buffer;" << endl;
    s << INDENT << "buffer.open(QBuffer::ReadWrite);" << endl;
    s << INDENT << "QDebug dbg(&buffer);" << endl;
    s << INDENT << "dbg << " << (metaClass->typeEntry()->isValue() ? "*" : "") << CPP_SELF_VAR ";" << endl;
    s << INDENT << "buffer.close();" << endl;
    s << INDENT << "QByteArray str = buffer.data();" << endl;
    s << INDENT << "int idx = str.indexOf('(');" << endl;
    s << INDENT << "if (idx >= 0)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "str.replace(0, idx, Py_TYPE(self)->tp_name);" << endl;
    }
    s << INDENT << "PyObject* mod = PyDict_GetItemString(Py_TYPE(self)->tp_dict, \"__module__\");" << endl;
    s << INDENT << "if (mod)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return Shiboken::String::fromFormat(\"<%s.%s at %p>\", Shiboken::String::toCString(mod), str.constData(), self);" << endl;
    }
    s << INDENT << "else" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return Shiboken::String::fromFormat(\"<%s at %p>\", str.constData(), self);" << endl;
    }
    s << '}' << endl;
    s << "} // extern C" << endl << endl;;
    return funcName;
}
