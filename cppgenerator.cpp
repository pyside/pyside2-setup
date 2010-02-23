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

#include "cppgenerator.h"
#include <reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

inline CodeSnipList getConversionRule(TypeSystem::Language lang, const AbstractMetaFunction *function)
{
    CodeSnipList list;

    foreach(AbstractMetaArgument *arg, function->arguments()) {
        QString convRule = function->conversionRule(lang, arg->argumentIndex() + 1);
        if (!convRule.isEmpty()) {
            CodeSnip snip(TypeSystem::TargetLangCode);
            snip.position = CodeSnip::Beginning;

            convRule.replace("%in", arg->argumentName());
            convRule.replace("%out", arg->argumentName() + "_out");

            snip.addCode(convRule);
            list << snip;
        }

    }
    return list;
}

// utiliy functions
inline CodeSnipList getReturnConversionRule(TypeSystem::Language lang,
                                            const AbstractMetaFunction *function,
                                            const QString& inputName,
                                            const QString& outputName)
{
    CodeSnipList list;

    QString convRule = function->conversionRule(lang, 0);
    if (!convRule.isEmpty()) {
        CodeSnip snip(lang);
        snip.position = CodeSnip::Beginning;

        convRule.replace("%in", inputName);
        convRule.replace("%out", outputName);

        snip.addCode(convRule);
        list << snip;
    }

    return list;
}


CppGenerator::CppGenerator() : m_currentErrorCode(0)
{
    // sequence protocol functions
    typedef QPair<QString, QString> StrPair;
    m_sequenceProtocol.insert("__len__", StrPair("PyObject* self", "Py_ssize_t"));
    m_sequenceProtocol.insert("__getitem__", StrPair("PyObject* self, Py_ssize_t _i", "PyObject*"));
    m_sequenceProtocol.insert("__setitem__", StrPair("PyObject* self, Py_ssize_t _i, PyObject* _value", "int"));
    m_sequenceProtocol.insert("__getslice__", StrPair("PyObject* self, Py_ssize_t _i1, Py_ssize_t _i2", "PyObject*"));
    m_sequenceProtocol.insert("__setslice__", StrPair("PyObject* self, Py_ssize_t _i1, Py_ssize_t _i2, PyObject* _value", "int"));
    m_sequenceProtocol.insert("__contains__", StrPair("PyObject* self, PyObject* _value", "int"));
    m_sequenceProtocol.insert("__concat__", StrPair("PyObject* self, PyObject* _other", "PyObject*"));
}

QString CppGenerator::fileNameForClass(const AbstractMetaClass *metaClass) const
{
    return metaClass->qualifiedCppName().toLower().replace("::", "_") + QLatin1String("_wrapper.cpp");
}

QList<AbstractMetaFunctionList> CppGenerator::filterGroupedOperatorFunctions(const AbstractMetaClass* metaClass,
                                                                             uint query)
{
    // ( func_name, num_args ) => func_list
    QMap<QPair<QString, int >, AbstractMetaFunctionList> results;
    foreach (AbstractMetaFunction* func, metaClass->operatorOverloads(query)) {
        if (func->isModifiedRemoved() || func->name() == "operator[]" || func->name() == "operator->")
            continue;
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

/*!
    Function used to write the class generated binding code on the buffer
    \param s the output buffer
    \param metaClass the pointer to metaclass information
*/
void CppGenerator::generateClass(QTextStream &s, const AbstractMetaClass *metaClass)
{
    ReportHandler::debugSparse("Generating wrapper implementation for " + metaClass->fullName());

    // write license comment
    s << licenseComment() << endl;

#ifndef AVOID_PROTECTED_HACK
    if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        s << "//workaround to access protected functions" << endl;
        s << "#define protected public" << endl << endl;
    }
#endif

    // headers
    s << "// default includes" << endl;
    s << "#include <shiboken.h>" << endl;
    s << "#include <typeresolver.h>\n";
    s << "#include <typeinfo>\n";
    if (usePySideExtensions()) {
        if (metaClass->isQObject()) {
            s << "#include <signalmanager.h>\n";
            s << "#include <dynamicqmetaobject.h>\n";
        }
    }

    // The multiple inheritance initialization function
    // needs the 'set' class from C++ STL.
    if (hasMultipleInheritanceInAncestry(metaClass))
        s << "#include <set>" << endl;

    s << "#include \"" << getModuleHeaderFileName() << '"' << endl << endl;

    QString converterImpl;
    QTextStream convImpl(&converterImpl);
    QString copyCppObjectImpl;
    QTextStream copyImpl(&copyCppObjectImpl);

    if (!metaClass->isNamespace()) {
        Indentation indentation(INDENT);
        writeSbkCopyCppObjectFunction(copyImpl, metaClass);
    }

    QString headerfile = fileNameForClass(metaClass);
    headerfile.replace("cpp", "h");
    s << "#include \"" << headerfile << '"' << endl;
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses()) {
        if (shouldGenerate(innerClass)) {
            QString headerfile = fileNameForClass(innerClass);
            headerfile.replace("cpp", "h");
            s << "#include \"" << headerfile << '"' << endl;
        }
    }

    //Extra includes
    s << endl << "// Extra includes" << endl;
    QList<Include> includes = metaClass->typeEntry()->extraIncludes();
    qSort(includes.begin(), includes.end());
    foreach (Include inc, includes)
        s << inc.toString() << endl;
    s << endl;

    if (metaClass->typeEntry()->typeFlags() & ComplexTypeEntry::Deprecated)
        s << "#Deprecated" << endl;

    s << "using namespace Shiboken;" << endl << endl;

    // class inject-code native/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Beginning, TypeSystem::NativeCode, 0, 0, metaClass);
        s << endl;
    }

    if (metaClass->isPolymorphic())
        writeTypeNameFunction(s, metaClass);

    if (shouldGenerateCppWrapper(metaClass)) {
        s << "// Native ---------------------------------------------------------" << endl;
        s << endl;

        foreach (const AbstractMetaFunction* func, filterFunctions(metaClass)) {
            if (func->isPrivate() || (func->isModifiedRemoved() && !func->isAbstract()))
                continue;
            if (func->isConstructor() && !func->isCopyConstructor() && !func->isUserAdded())
                writeConstructorNative(s, func);
            else if (func->isVirtual() || func->isAbstract())
                writeVirtualMethodNative(s, func);
        }

        if (usePySideExtensions() && metaClass->isQObject())
            writeMetaObjectMethod(s, metaClass);

        writeDestructorNative(s, metaClass);

        s << endl << "// Target ---------------------------------------------------------" << endl;
        s << endl;
    }

    Indentation indentation(INDENT);

    QString methodsDefinitions;
    QTextStream md(&methodsDefinitions);
    QString singleMethodDefinitions;
    QTextStream smd(&singleMethodDefinitions);

    bool hasComparisonOperator = metaClass->hasComparisonOperatorOverload();
    bool typeAsNumber = metaClass->hasArithmeticOperatorOverload() || metaClass->hasLogicalOperatorOverload() || metaClass->hasBitwiseOperatorOverload();

    foreach (AbstractMetaFunctionList allOverloads, getFunctionGroups(metaClass).values()) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, allOverloads) {
            if (!func->isAssignmentOperator() && !func->isCastOperator() && !func->isModifiedRemoved() && !func->isPrivate() &&
                func->ownerClass() == func->implementingClass())
                overloads.append(func);
        }

        if (overloads.isEmpty())
            continue;

        const AbstractMetaFunction* rfunc = overloads.first();
        if (m_sequenceProtocol.contains(rfunc->name()))
            continue;

        if (rfunc->isConstructor())
            writeConstructorWrapper(s, overloads);

        if (!rfunc->isConstructor() && !rfunc->isOperatorOverload()) {
            writeMethodWrapper(s, overloads);
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

    QString className = cpythonTypeName(metaClass).replace(QRegExp("_Type$"), "");

    // Write single method definitions
    s << singleMethodDefinitions;

    // Write methods definition
    s << "static PyMethodDef " << className << "_methods[] = {" << endl;
    s << methodsDefinitions << INDENT << "{0} // Sentinel" << endl;
    s << "};" << endl << endl;

    // Write tp_getattro function
    if (classNeedsGetattroFunction(metaClass)) {
        writeGetattroFunction(s, metaClass);
        s << endl;
    }

    if (typeAsNumber) {
        QList<AbstractMetaFunctionList> opOverloads = filterGroupedOperatorFunctions(
                metaClass,
                AbstractMetaClass::ArithmeticOp
                | AbstractMetaClass::LogicalOp
                | AbstractMetaClass::BitwiseOp);

        foreach (AbstractMetaFunctionList allOverloads, opOverloads) {
            AbstractMetaFunctionList overloads;
            foreach (AbstractMetaFunction* func, allOverloads) {
                if (!func->isModifiedRemoved()
                    && !func->isPrivate()
                    && (func->ownerClass() == func->implementingClass() || func->isAbstract()))
                    overloads.append(func);
            }

            if (overloads.isEmpty())
                continue;

            writeMethodWrapper(s, overloads);
        }

        s << "// type has number operators" << endl;
        writeTypeAsNumberDefinition(s, metaClass);
    }

    if (supportsSequenceProtocol(metaClass)) {
        writeSequenceMethods(s, metaClass);
        writeTypeAsSequenceDefinition(s, metaClass);
    }

    if (hasComparisonOperator) {
        s << "// Rich comparison" << endl;
        writeRichCompareFunction(s, metaClass);
    }

    if (shouldGenerateGetSetList(metaClass)) {
        foreach (const AbstractMetaField* metaField, metaClass->fields()) {
            writeGetterFunction(s, metaField);
            writeSetterFunction(s, metaField);
            s << endl;
        }

        s << "// Getters and Setters for " << metaClass->name() << endl;
        s << "static PyGetSetDef " << cpythonGettersSettersDefinitionName(metaClass) << "[] = {" << endl;
        foreach (const AbstractMetaField* metaField, metaClass->fields()) {
            s << INDENT << "{const_cast<char*>(\"" << metaField->name() << "\"), ";
            s << "(getter)" << cpythonGetterFunctionName(metaField);
            s << ", (setter)" << cpythonSetterFunctionName(metaField);
            s << "}," << endl;
        }
        s << INDENT << "{0}  // Sentinel" << endl;
        s << "};" << endl << endl;
    }

    s << "extern \"C\"" << endl << '{' << endl << endl;
    writeClassDefinition(s, metaClass);
    s << endl;

    foreach (AbstractMetaEnum* cppEnum, metaClass->enums()) {
        bool hasFlags = cppEnum->typeEntry()->flags();
        if (hasFlags) {
            writeFlagsMethods(s, cppEnum);
            writeFlagsNumberMethodsDefinition(s, cppEnum);
            s << endl;
        }

        writeEnumDefinition(s, cppEnum);

        if (hasFlags) {
            // Write Enum as Flags definition (at the moment used only by QFlags<enum>)
            writeFlagsDefinition(s, cppEnum);
            s << endl;
        }
    }
    s << endl;

    writeClassRegister(s, metaClass);
    s << endl << "} // extern \"C\"" << endl << endl;

    s << "namespace Shiboken" << endl << '{' << endl;
    s << "// Copy C++ object implementation" << endl;
    s << copyCppObjectImpl;
    s << "// Converter implementations" << endl;
    s << converterImpl;
    s << "} // namespace Shiboken" << endl << endl;

    // class inject-code native/end
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::End, TypeSystem::NativeCode, 0, 0, metaClass);
        s << endl;
    }
}

void CppGenerator::writeConstructorNative(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    s << functionSignature(func, wrapperName(func->ownerClass()) + "::", "",
                           OriginalTypeDescription | SkipDefaultValues);
    s << " : ";
    writeFunctionCall(s, func);
    if (usePySideExtensions() && func->ownerClass()->isQObject())
        s << ", m_metaObject(0)";
    s << " {" << endl;
    const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
    writeCodeSnips(s, func->injectedCodeSnips(), CodeSnip::Beginning, TypeSystem::NativeCode, func, lastArg);
    s << INDENT << "// ... middle" << endl;
    writeCodeSnips(s, func->injectedCodeSnips(), CodeSnip::End, TypeSystem::NativeCode, func, lastArg);
    s << '}' << endl << endl;
}

void CppGenerator::writeDestructorNative(QTextStream &s, const AbstractMetaClass *metaClass)
{
    Indentation indentation(INDENT);
    s << wrapperName(metaClass) << "::~" << wrapperName(metaClass) << "()" << endl << '{' << endl;
    s << INDENT << "BindingManager::instance().invalidateWrapper(this);" << endl;
    if (usePySideExtensions() && metaClass->isQObject())
        s << INDENT << "delete m_metaObject;\n";
    s << '}' << endl;
}

void CppGenerator::writeVirtualMethodNative(QTextStream &s, const AbstractMetaFunction* func)
{
    //skip metaObject function, this will be write manually ahead
    if (usePySideExtensions() && func->ownerClass() && func->ownerClass()->isQObject() &&
        ((func->name() == "metaObject") || (func->name() == "qt_metacall")))
        return;

    const TypeEntry* type = func->type() ? func->type()->typeEntry() : 0;

    QString prefix = wrapperName(func->ownerClass()) + "::";
    s << functionSignature(func, prefix, "", Generator::SkipDefaultValues|Generator::OriginalTypeDescription) << endl;
    s << "{" << endl;

    Indentation indentation(INDENT);

    if (func->isAbstract() && func->isModifiedRemoved()) {
        s << INDENT << "#warning Pure virtual method \"" << func->ownerClass()->name() << "::" << func->minimalSignature();
        s << "\" must be implement but was completely removed on typesystem." << endl;
        if (func->type()) {
            s << INDENT << "return ";
            writeMinimalConstructorCallArguments(s, func->type());
            s << ';' << endl;
        }
        s << '}' << endl << endl;
        return;
    }

    s << INDENT << "Shiboken::GilState gil;" << endl;

    s << INDENT << "Shiboken::AutoDecRef py_override(BindingManager::instance().getOverride(this, \"";
    s << func->name() << "\"));" << endl;

    s << INDENT << "if (py_override.isNull()) {" << endl;
    {
        Indentation indentation(INDENT);
        if (func->isAbstract()) {
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
            s << func->ownerClass()->name() << '.' << func->name();
            s << "()' not implemented.\");" << endl;
            s << INDENT << "return";
            if (func->type()) {
                s << ' ';
                writeMinimalConstructorCallArguments(s, func->type());
            }
        } else {
            if (func->allowThread()) {
                s << INDENT << "Shiboken::ThreadStateSaver " << THREAD_STATE_SAVER_VAR << ';' << endl;
                s << INDENT << THREAD_STATE_SAVER_VAR << ".save();" << endl;
            }

            s << INDENT << "return this->" << func->implementingClass()->qualifiedCppName() << "::";
            writeFunctionCall(s, func, Generator::VirtualCall);
        }
    }
    s << ';' << endl;
    s << INDENT << '}' << endl << endl;

    CodeSnipList convRules = getConversionRule(TypeSystem::TargetLangCode, func);
    if (convRules.size())
        writeCodeSnips(s, convRules, CodeSnip::Beginning, TypeSystem::TargetLangCode, func);

    s << INDENT << "Shiboken::AutoDecRef pyargs(";

    if (func->arguments().isEmpty()) {
        s << "PyTuple_New(0));" << endl;
    } else {
        QStringList argConversions;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (func->argumentRemoved(arg->argumentIndex() + 1))
                continue;


            QString argConv;
            QTextStream ac(&argConv);
            bool convert = arg->type()->isObject()
                            || arg->type()->isQObject()
                            || arg->type()->isValue()
                            || arg->type()->isValuePointer()
                            || arg->type()->isFlags()
                            || arg->type()->isEnum()
                            || arg->type()->isReference()
                            || (arg->type()->isPrimitive()
                                && !m_formatUnits.contains(arg->type()->typeEntry()->name()));

            bool hasConversionRule = !func->conversionRule(TypeSystem::TargetLangCode, arg->argumentIndex() + 1).isEmpty();

            Indentation indentation(INDENT);
            ac << INDENT;
            if (convert && !hasConversionRule) {
                writeToPythonConversion(ac, arg->type(), func->ownerClass());
                ac << '(';
            }

            if (hasConversionRule)
                ac << arg->argumentName() << "_out";
            else
                ac << arg->argumentName() << (convert ? ")" : "");

            argConversions << argConv;
        }

        s << "Py_BuildValue(\"(" << getFormatUnitString(func) << ")\"," << endl;
        s << argConversions.join(",\n") << endl;
        s << INDENT << "));" << endl;
    }
    s << endl;

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = func->injectedCodeSnips();

        if (injectedCodeUsesPySelf(func))
            s << INDENT << "PyObject* pySelf = BindingManager::instance().retrieveWrapper(this);" << endl;

        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::NativeCode, func, lastArg);
        s << endl;
    }

    if (!injectedCodeCallsPythonOverride(func)) {
        s << INDENT;
        s << "Shiboken::AutoDecRef " PYTHON_RETURN_VAR "(PyObject_Call(py_override, pyargs, NULL));" << endl;
        if (type) {
            s << INDENT << "// An error happened in python code!" << endl;
            s << INDENT << "if (" PYTHON_RETURN_VAR ".isNull()) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "PyErr_Print();" << endl;
                s << INDENT << "return ";
                writeMinimalConstructorCallArguments(s, func->type());
                s << ';' << endl;
            }
            s << INDENT << '}' << endl;

            s << INDENT << "// Check return type" << endl;
            s << INDENT << "bool typeIsValid = " << cpythonCheckFunction(func->type()) << "(" << PYTHON_RETURN_VAR << ");" << endl;
            if (func->type()->isQObject() || func->type()->isObject() || func->type()->isValuePointer()) {
                s << INDENT << "typeIsValid = typeIsValid || (" << PYTHON_RETURN_VAR << " == Py_None);" << endl;
            }
            s << INDENT << "if (!typeIsValid) {" << endl;
            s << INDENT << INDENT << "PyErr_SetString(PyExc_TypeError, \"Invalid return value in function " << func->ownerClass()->name() << "." << func->name() << "\");" << endl;
            s << INDENT << INDENT << "return ";
            writeMinimalConstructorCallArguments(s, func->type());
            s << INDENT << INDENT << ";" << endl;
            s << INDENT << "}" << endl;

            s << INDENT;
            s << translateTypeForWrapperMethod(func->type(), func->implementingClass()) << ' ' << CPP_RETURN_VAR << "(";
            writeToCppConversion(s, func->type(), func->implementingClass(), PYTHON_RETURN_VAR);
            s << ')';
            s << ';' << endl;
        }
    }

    foreach (FunctionModification func_mod, func->modifications()) {
        foreach (ArgumentModification arg_mod, func_mod.argument_mods) {
            if (!arg_mod.resetAfterUse)
                continue;
            s << INDENT << "BindingManager::instance().invalidateWrapper(PyTuple_GET_ITEM(pyargs, ";
            s << (arg_mod.index - 1) << "));" << endl;
        }
    }

    if (func->hasInjectedCode()) {
        s << endl;
        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::NativeCode, func, lastArg);
    }

    // write ownership rules
    TypeSystem::Ownership ownership = func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0);
    if (ownership == TypeSystem::CppOwnership) {
        s << INDENT << "// Return value ownership transference" << endl;
        s << INDENT << "SbkBaseWrapper_setOwnership("PYTHON_RETURN_VAR".object(), 0);" << endl;
    } else
        writeReturnValueHeuristics(s, func, "BindingManager::instance().retrieveWrapper(this)");

    if (type)
        s << INDENT << "return "CPP_RETURN_VAR";" << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeMetaObjectMethod(QTextStream& s, const AbstractMetaClass* metaClass)
{
    Indentation indentation(INDENT);
    QString wrapperClassName = wrapperName(metaClass);
    QString prefix = wrapperClassName + "::";
    s << "const QMetaObject* " << wrapperClassName << "::metaObject() const\n{\n";
    s << INDENT << "if (!m_metaObject) {\n";
    {
        Indentation indentation(INDENT);
        s << INDENT << "PyObject *pySelf = BindingManager::instance().retrieveWrapper(this);\n";
        s << INDENT << "m_metaObject = new PySide::DynamicQMetaObject(pySelf->ob_type->tp_name, &" << metaClass->qualifiedCppName() << "::staticMetaObject);\n";
        s << INDENT << "}\n";
    }
    s << INDENT << "return m_metaObject;\n";
    s << "}\n\n";

    s << "int " << wrapperClassName << "::qt_metacall(QMetaObject::Call call, int id, void** args)\n";
    s << "{\n";
    s << INDENT << "int result = " << metaClass->qualifiedCppName() << "::qt_metacall(call, id, args);\n";
    s << INDENT << "return result < 0 ? result : PySide::SignalManager::qt_metacall(this, call, id, args);\n";
    s << "}\n\n";
}

void CppGenerator::writeConstructorWrapper(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    OverloadData overloadData(overloads, this);
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    const AbstractMetaClass* metaClass = rfunc->ownerClass();
    QString className = cpythonTypeName(metaClass);
    m_currentErrorCode = -1;

    s << "static int" << endl;
    s << cpythonFunctionName(rfunc) << "(PyObject* self, PyObject* args, PyObject* kwds)" << endl;
    s << '{' << endl;

    s << INDENT;
    bool hasCppWrapper = shouldGenerateCppWrapper(metaClass);
    s << (hasCppWrapper ? wrapperName(metaClass) : metaClass->qualifiedCppName());
    s << "* cptr = 0;" << endl;


    bool hasCodeInjectionsAtEnd = false;
    foreach(AbstractMetaFunction* func, overloads) {
        foreach (CodeSnip cs, func->injectedCodeSnips()) {
            if (cs.position == CodeSnip::End) {
                hasCodeInjectionsAtEnd = true;
                break;
            }
        }
    }
    if (hasCodeInjectionsAtEnd)
        s << INDENT << "int overloadId = -1;" << endl;

    if (overloadData.hasAllowThread())
        s << INDENT << "Shiboken::ThreadStateSaver " << THREAD_STATE_SAVER_VAR << ';' << endl;
    s << INDENT << "SbkBaseWrapper* sbkSelf = reinterpret_cast<SbkBaseWrapper*>(self);" << endl;
    s << INDENT << "assert(!sbkSelf->cptr);\n"; // FIXME: object reinitialization not supported

    if (metaClass->isAbstract() || metaClass->baseClassNames().size() > 1) {
        s << INDENT << "SbkBaseWrapperType* type = reinterpret_cast<SbkBaseWrapperType*>(self->ob_type);" << endl;
        s << INDENT << "SbkBaseWrapperType* myType = reinterpret_cast<SbkBaseWrapperType*>(" << cpythonTypeNameExt(metaClass->typeEntry()) << ");" << endl;
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
            s << "' represents a C++ abstract class and cannot be instanciated\");" << endl;
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
            s << INDENT << "type->mi_init = myType->mi_init;" << endl;
            s << INDENT << "type->mi_offsets = myType->mi_offsets;" << endl;
            s << INDENT << "type->mi_specialcast = myType->mi_specialcast;" << endl;
        }
        if (!metaClass->isAbstract())
            s << INDENT << '}' << endl << endl;
    }

    if (overloadData.maxArgs() > 0) {
        s  << endl << INDENT << "int numArgs = ";
        writeArgumentsInitializer(s, overloadData);
    }

    writeOverloadedMethodDecisor(s, &overloadData);
    s << endl;

    if (overloadData.maxArgs() > 0) {
        s << INDENT << "if (PyErr_Occurred()) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "delete cptr;" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
        s << INDENT << '}' << endl;
        s << INDENT << "if (!cptr) goto " << cpythonFunctionName(rfunc) << "_TypeError;" << endl;
        s << endl;
    }

    s << INDENT << "sbkSelf->cptr = cptr;" << endl;
    s << INDENT << "sbkSelf->validCppObject = 1;" << endl;
    // If the created C++ object has a C++ wrapper the ownership is assigned to Python
    // (first "1") and the flag indicating that the Python wrapper holds an C++ wrapper
    // is marked as true (the second "1"). Otherwise the default values apply:
    // Python owns it and C++ wrapper is false.
    if (shouldGenerateCppWrapper(overloads.first()->ownerClass()))
        s << INDENT << "sbkSelf->containsCppWrapper = 1;" << endl;
    if (needsReferenceCountControl(metaClass))
        s << INDENT << "sbkSelf->referredObjects = new Shiboken::RefCountMap;" << endl;
    s << INDENT << "BindingManager::instance().registerWrapper(sbkSelf);" << endl;

    // Constructor code injections, position=end
    if (hasCodeInjectionsAtEnd) {
        // FIXME: C++ arguments are not available in code injection on constructor when position = end.
        s << INDENT << "switch(overloadId) {" << endl;
        foreach(AbstractMetaFunction* func, overloads) {
            Indentation indent(INDENT);
            foreach (CodeSnip cs, func->injectedCodeSnips()) {
                if (cs.position == CodeSnip::End) {
                    s << INDENT << "case " << metaClass->functions().indexOf(func) << ':' << endl;
                    s << INDENT << '{' << endl;
                    {
                        Indentation indent(INDENT);
                        writeCodeSnips(s, func->injectedCodeSnips(), CodeSnip::End, TypeSystem::TargetLangCode, func);
                    }
                    s << INDENT << '}' << endl;
                    break;
                }
            }
        }
        s << '}' << endl;
    }

    s << endl << INDENT << "return 1;" << endl;
    if (overloadData.maxArgs() > 0)
        writeErrorSection(s, overloadData);
    s << '}' << endl << endl;
    m_currentErrorCode = 0;
}

void CppGenerator::writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return;

    AbstractMetaFunctionList ctors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
    const AbstractMetaFunction* ctor = 0;

    foreach (const AbstractMetaFunction* candidate, ctors) {
        if (candidate->arguments().size() == 0) {
            ctor = candidate;
            break;
        }

        bool allPrimitives = true;
        foreach (const AbstractMetaArgument* arg, candidate->arguments()) {
            if (!arg->type()->isPrimitive()) {
                allPrimitives = false;
                break;
            }
        }
        if (allPrimitives) {
            if (!ctor || candidate->arguments().size() < ctor->arguments().size())
                ctor = candidate;
        }
    }

    if (!ctor) {
        ReportHandler::warning("Class "+metaClass->name()+" does not have a default ctor.");
        return;
    }

    QStringList argValues;
    AbstractMetaArgumentList args = ctor->arguments();
    for (int i = 0; i < args.size(); i++)
        argValues << args[i]->type()->name()+"(0)";
    s << metaClass->qualifiedCppName() << '(' << argValues.join(QLatin1String(", ")) << ')';
}

void CppGenerator::writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaType* metaType)
{
    Q_ASSERT(metaType);
    const TypeEntry* type = metaType->typeEntry();

    if (type->isObject()) {
        s << "0";
    } else if (type->isPrimitive()) {
        s << type->name() << "(0)";
    } else if (type->isContainer() || type->isFlags() || type->isEnum()){
        s << metaType->cppSignature() << "()";
    } else {
        // this is slowwwww, FIXME: Fix the API od APIExtractor, these things should be easy!
        foreach (AbstractMetaClass* metaClass, classes()) {
            if (metaClass->typeEntry() == type) {
                writeMinimalConstructorCallArguments(s, metaClass);
                return;
            }
        }
        ReportHandler::warning("Could not find a AbstractMetaClass for type "+metaType->name());
    }
}

void CppGenerator::writeMethodWrapper(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    OverloadData overloadData(overloads, this);
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();

    //DEBUG
//     if (rfunc->name() == "operator+" && rfunc->ownerClass()->name() == "Str") {
//         QString dumpFile = QString("/tmp/%1_%2.dot").arg(moduleName()).arg(pythonOperatorFunctionName(rfunc)).toLower();
//         overloadData.dumpGraph(dumpFile);
//     }
    //DEBUG

    int minArgs = overloadData.minArgs();
    int maxArgs = overloadData.maxArgs();

    s << "static PyObject*" << endl;
    s << cpythonFunctionName(rfunc) << "(PyObject* self";
    if (maxArgs > 0) {
        s << ", PyObject* arg";
        if (maxArgs > 1)
            s << 's';
    }
    s << ')' << endl << '{' << endl;

    if (rfunc->implementingClass() &&
        (!rfunc->implementingClass()->isNamespace() && !rfunc->isStatic())) {

        if (rfunc->isOperatorOverload() && rfunc->isBinaryOperator()) {
            QString checkFunc = cpythonCheckFunction(rfunc->ownerClass()->typeEntry());
            s << INDENT << "// FIXME: Optimize this: Only do this when there is a reverse operator in this function group\n";
            s << INDENT << "bool isReverse = " << checkFunc << "(arg) && !" << checkFunc << "(self);\n"
                << INDENT << "if (isReverse)\n"
                << INDENT << INDENT << "std::swap(self, arg);\n\n";
        }

        // Checks if the underlying C++ object is valid.
        if (OverloadData::hasStaticAndInstanceFunctions(overloads)) {
            s << INDENT << "if (self) {" << endl;
            {
                Indentation indent(INDENT);
                writeInvalidCppObjectCheck(s);
            }
            s << INDENT << '}' << endl;
        } else {
            writeInvalidCppObjectCheck(s);
        }
        s << endl;
    }

    bool hasReturnValue = overloadData.hasNonVoidReturnType();

    if (hasReturnValue && !rfunc->isInplaceOperator())
        s << INDENT << "PyObject* " << PYTHON_RETURN_VAR << " = 0;" << endl;
    if (overloadData.hasAllowThread())
        s << INDENT << "Shiboken::ThreadStateSaver " << THREAD_STATE_SAVER_VAR << ';' << endl;
    s << endl;

    if (minArgs != maxArgs || maxArgs > 1) {
        s << INDENT << "int numArgs = ";
        if (minArgs == 0 && maxArgs == 1)
            s << "(arg == 0 ? 0 : 1);" << endl;
        else
            writeArgumentsInitializer(s, overloadData);
    }

    writeOverloadedMethodDecisor(s, &overloadData);

    s << endl << INDENT << "if (PyErr_Occurred()";
    if (hasReturnValue && !rfunc->isInplaceOperator())
        s << " || !" << PYTHON_RETURN_VAR;
    s << ") {" << endl;
    {
        Indentation indent(INDENT);
        if (hasReturnValue  && !rfunc->isInplaceOperator())
            s << INDENT << "Py_XDECREF(" << PYTHON_RETURN_VAR << ");" << endl;
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << INDENT << '}' << endl;

    s << INDENT;
    if (hasReturnValue) {
        if (rfunc->isInplaceOperator()) {
            s << INDENT << "Py_INCREF(self);\n";
            s << INDENT << "return self;\n";
        } else {
            s << INDENT << "return " << PYTHON_RETURN_VAR << ";\n";
        }
    } else {
        s << "Py_RETURN_NONE";
    }
    s << ';' << endl;

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

    s << INDENT << "PyObject* pyargs[] = {";
    s << QString(maxArgs, '0').split("", QString::SkipEmptyParts).join(", ");
    s << "};" << endl << endl;

    if (overloadData.hasVarargs()) {
        maxArgs--;
        if (minArgs > maxArgs)
            minArgs = maxArgs;
    }

    if (overloadData.hasVarargs()) {
        s << INDENT << "PyObject* nonvarargs = PyTuple_GetSlice(args, 0, " << maxArgs << ");" << endl;
        s << INDENT << "Shiboken::AutoDecRef auto_nonvarargs(nonvarargs);" << endl;
        s << INDENT << "pyargs[" << maxArgs << "] = PyTuple_GetSlice(args, " << maxArgs << ", numArgs);" << endl;
        s << INDENT << "Shiboken::AutoDecRef auto_varargs(pyargs[" << maxArgs << "]);" << endl;
        s << endl;
    }

    QStringList palist;
    for (int i = 0; i < maxArgs; i++)
        palist << QString("&(pyargs[%1])").arg(i);
    QString pyargs = palist.join(", ");

    QList<int> invalidArgsLength = overloadData.invalidArgumentLengths();
    if (!invalidArgsLength.isEmpty()) {
        QStringList invArgsLen;
        foreach (int i, invalidArgsLength)
            invArgsLen << QString("numArgs == %1").arg(i);
        s << INDENT << "// invalid argument lengths" << endl;
        s << INDENT << "if (" << invArgsLen.join(" || ") << ")" << endl;
        s << INDENT << INDENT << "goto " << cpythonFunctionName(rfunc) << "_TypeError;" << endl << endl;
    }

    QString funcName;
    if (rfunc->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
    else
        funcName = rfunc->name();

    s << INDENT << "if (!PyArg_UnpackTuple(" << (overloadData.hasVarargs() ?  "nonvarargs" : "args");
    s << ", \"" << funcName << "\", " << minArgs << ", " << maxArgs  << ", " << pyargs << "))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
    s << endl;
}

void CppGenerator::writeCppSelfDefinition(QTextStream& s, const AbstractMetaFunction* func)
{
    if (!func->ownerClass() || func->isStatic() || func->isConstructor())
        return;

    s << INDENT;
#ifdef AVOID_PROTECTED_HACK
    bool hasProtectedFunctions = func->ownerClass()->hasProtectedFunctions();
    QString _wrapperName = wrapperName(func->ownerClass());
    s << (hasProtectedFunctions ? _wrapperName : func->ownerClass()->qualifiedCppName()) << "* " << CPP_SELF_VAR << " = ";
    s << (hasProtectedFunctions ? QString("(%1*)").arg(_wrapperName) : "");
#else
    s << func->ownerClass()->qualifiedCppName() << "* " << CPP_SELF_VAR << " = ";
#endif
    s << cpythonWrapperCPtr(func->ownerClass(), "self") << ';' << endl;
    if (func->isUserAdded())
        s << INDENT << "(void)" << CPP_SELF_VAR << "; // avoid warnings about unused variables" << endl;
}

void CppGenerator::writeErrorSection(QTextStream& s, OverloadData& overloadData)
{
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    s << endl << INDENT << cpythonFunctionName(rfunc) << "_TypeError:" << endl;
    Indentation indentation(INDENT);
    QString funcName;
    if (rfunc->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
    else
        funcName = rfunc->name();
    if (rfunc->ownerClass()) {
        QString fullName = rfunc->ownerClass()->fullName();
        if (rfunc->isConstructor())
            funcName = fullName;
        else
            funcName.prepend(fullName + '.');
    }

    QString argsVar = !rfunc->isConstructor() && overloadData.maxArgs() == 1 ? "arg" : "args";
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
                    strArg = "str";
                } else if (argType->isPrimitive()) {
                    const PrimitiveTypeEntry* ptp = reinterpret_cast<const PrimitiveTypeEntry*>(argType->typeEntry());
                    while (ptp->aliasedTypeEntry())
                        ptp = ptp->aliasedTypeEntry();
                    strArg = ptp->name().replace(QRegExp("^signed\\s+"), "");
                    if (strArg == "double")
                        strArg = "float";
                } else {
                    strArg = argType->fullName();
                }
                if (!arg->defaultValueExpression().isEmpty()) {
                    strArg += " = ";
                    if ((isCString(argType) || argType->isValuePointer() || argType->typeEntry()->isObject())
                        && arg->defaultValueExpression() == "0")
                        strArg += "None";
                    else
                        strArg += arg->defaultValueExpression().replace("::", ".").replace("\"", "\\\"");
                }
                args << strArg;
            }
            overloadSignatures << "\""+args.join(", ")+"\"";
        }
        s << INDENT << "const char* overloads[] = {" << overloadSignatures.join(", ") << ", 0};" << endl;
        s << INDENT << "Shiboken::setErrorAboutWrongArguments(" << argsVar << ", \"" << funcName << "\", overloads);" << endl;
    }
    s << INDENT << "return " << m_currentErrorCode << ';' << endl;
}

void CppGenerator::writeInvalidCppObjectCheck(QTextStream& s, QString pyArgName, const TypeEntry* type)
{
    s << INDENT << "if (";
    if (type) {
        QString implicitChecks;
        QTextStream ic(&implicitChecks);
        foreach (const AbstractMetaFunction* ctor, implicitConversions(type)) {
            const TypeEntry* te = ctor->arguments().first()->type()->typeEntry();
            if (te->isValue() || te->isObject())
                ic << " || " << cpythonCheckFunction(te) << '(' << pyArgName << ')';
        }
        s << (!implicitChecks.isEmpty() ? "(" : "");
        s << cpythonCheckFunction(type) << '(' << pyArgName << ')';
        if (!implicitChecks.isEmpty())
            s << implicitChecks << ')';
        s << " && ";
    }
    s << "Shiboken::cppObjectIsInvalid(" << pyArgName << "))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return " << m_currentErrorCode << ';' << endl;
    }
}

void CppGenerator::writeTypeCheck(QTextStream& s, const AbstractMetaType* argType, QString argumentName, bool isNumber, QString customType)
{
    bool writeIsConvertibleCheck = !implicitConversions(argType).isEmpty()
                                    || argType->typeEntry()->isObject()
                                    || argType->isValuePointer();

    if (writeIsConvertibleCheck || isCString(argType) || isPairContainer(argType))
        s << '(';

    if (!customType.isEmpty())
        s << guessCPythonCheckFunction(customType);
    else if (argType->typeEntry()->isFlags())
        s << cpythonCheckFunction(((FlagsTypeEntry*) argType->typeEntry())->originator(), true);
    else if (argType->isEnum())
        s << cpythonCheckFunction(argType, false, true);
    else
        s << cpythonCheckFunction(argType, isNumber);

    s << '(' << argumentName << ')';

    if (isPairContainer(argType))
        s << " && PySequence_Size(" << argumentName << ") == 2)";
    else if (isCString(argType))
        s << " || " << argumentName << " == Py_None)";
    else if (writeIsConvertibleCheck)
        s << " || " << cpythonIsConvertibleFunction(argType) << '(' << argumentName << "))";
}

void CppGenerator::writeTypeCheck(QTextStream& s, const OverloadData* overloadData, QString argumentName)
{
    QSet<const TypeEntry*> numericTypes;

    foreach (OverloadData* od, overloadData->previousOverloadData()->nextOverloadData()) {
        foreach (const AbstractMetaFunction* func, od->overloads()) {
            const AbstractMetaArgument* arg = od->argument(func);

            if (!arg->type()->isPrimitive())
                continue;
            if (ShibokenGenerator::isNumber(arg->type()->typeEntry()))
                numericTypes << arg->type()->typeEntry();
        }
    }

    // This condition trusts that the OverloadData object will arrange for
    // PyInt type to come after the more precise numeric types (e.g. float and bool)
    const AbstractMetaType* argType = overloadData->argType();
    bool numberType = numericTypes.count() == 1 || ShibokenGenerator::isPyInt(argType);
    QString customType = (overloadData->hasArgumentTypeReplace() ? overloadData->argumentTypeReplaced() : "");
    writeTypeCheck(s, argType, argumentName, numberType, customType);
}

void CppGenerator::writeArgumentConversion(QTextStream& s,
                                           const AbstractMetaType* argType,
                                           QString argName, QString pyArgName,
                                           const AbstractMetaClass* context)
{
    const TypeEntry* type = argType->typeEntry();

    if (type->isCustom() || type->isVarargs())
        return;

    QString typeName;
    QString baseTypeName = type->name();
    bool isWrappedCppClass = type->isValue() || type->isObject();
    if (isWrappedCppClass)
        typeName = baseTypeName + '*';
    else
        typeName = translateTypeForWrapperMethod(argType, context);

    if (type->isContainer() || type->isPrimitive()) {
        // If the type is a const char*, we don't remove the "const".
        if (typeName.startsWith("const ") && !(isCString(argType)))
            typeName.remove(0, sizeof("const ") / sizeof(char) - 1);
        if (typeName.endsWith("&"))
            typeName.chop(1);
    }

    bool hasImplicitConversions = !implicitConversions(argType).isEmpty();

    if (isWrappedCppClass) {
        const TypeEntry* type = (hasImplicitConversions ? argType->typeEntry() : 0);
        writeInvalidCppObjectCheck(s, pyArgName, type);
    }

    if (hasImplicitConversions) {
        s << INDENT << "std::auto_ptr<" << baseTypeName << " > ";
        s << argName << "_auto_ptr;" << endl;
    }

    s << INDENT << typeName << ' ' << argName << " = ";
    s << "Shiboken::Converter<" << typeName << " >::toCpp(" << pyArgName << ");" << endl;

    if (hasImplicitConversions) {
        s << INDENT << "if (!" << cpythonCheckFunction(argType) << '(' << pyArgName << "))";
        s << endl;
        Indentation indent(INDENT);
        s << INDENT << argName << "_auto_ptr = std::auto_ptr<" << baseTypeName;
        s << " >(" << argName << ");" << endl;
    }
}

void CppGenerator::writeNoneReturn(QTextStream& s, const AbstractMetaFunction* func, bool thereIsReturnValue)
{
    if (thereIsReturnValue && (!func->type() || func->argumentRemoved(0)) && !injectedCodeHasReturnValueAttribution(func)) {
        s << INDENT << PYTHON_RETURN_VAR << " = Py_None;" << endl;
        s << INDENT << "Py_INCREF(Py_None);" << endl;
    }
}

void CppGenerator::writeOverloadedMethodDecisor(QTextStream& s, OverloadData* parentOverloadData)
{
    bool hasDefaultCall = parentOverloadData->nextArgumentHasDefaultValue();
    const AbstractMetaFunction* referenceFunction = parentOverloadData->referenceFunction();

    // If the next argument has not an argument with a default value, it is still possible
    // that one of the overloads for the current overload data has its final occurrence here.
    // If found, the final occurrence of a method is attributed to the referenceFunction
    // variable to be used further on this method on the conditional that writes default
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
    bool manyArgs = maxArgs > 1 || referenceFunction->isConstructor();

    // Functions without arguments are written right away.
    if (maxArgs == 0) {
        writeMethodCall(s, referenceFunction);
        return;

    // To decide if a method call is possible at this point the current overload
    // data object cannot be the head, since it is just an entry point, or a root,
    // for the tree of arguments and it does not represent a valid method call.
    } else if (!parentOverloadData->isHeadOverloadData()) {
        bool isLastArgument = parentOverloadData->nextOverloadData().isEmpty();
        bool signatureFound = parentOverloadData->overloads().size() == 1;

        // The current overload data describes the last argument of a signature,
        // so the method can be called right now.
        if (isLastArgument || (signatureFound && !hasDefaultCall)) {
            const AbstractMetaFunction* func = parentOverloadData->referenceFunction();
            int numRemovedArgs = OverloadData::numberOfRemovedArguments(func);
            writeMethodCall(s, func, func->arguments().size() - numRemovedArgs);
            if (!func->isConstructor())
                writeNoneReturn(s, func, parentOverloadData->headOverloadData()->hasNonVoidReturnType());
            return;
        }
    }

    s << INDENT;

    // If the next argument has a default value the decisor can perform a method call;
    // it just need to check if the number of arguments received from Python are equal
    // to the number of parameters preceding the argument with the default value.
    if (hasDefaultCall) {
        int numArgs = parentOverloadData->argPos() + 1;
        s << "if (numArgs == " << numArgs << ") {" << endl;
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
            writeMethodCall(s, func, numArgs);
            if (!func->isConstructor())
                writeNoneReturn(s, func, parentOverloadData->headOverloadData()->hasNonVoidReturnType());
        }
        s << INDENT << "} else ";
    }

    foreach (OverloadData* overloadData, parentOverloadData->nextOverloadData()) {
        bool signatureFound = overloadData->overloads().size() == 1
                                && !overloadData->getFunctionWithDefaultValue()
                                && !overloadData->findNextArgWithDefault();

        const AbstractMetaFunction* refFunc = overloadData->referenceFunction();

        s << "if (";
        if (manyArgs && signatureFound) {
            AbstractMetaArgumentList args = refFunc->arguments();
            int lastArgIsVarargs = (int) (args.size() > 1 && args.last()->type()->isVarargs());
            int numArgs = args.size() - OverloadData::numberOfRemovedArguments(refFunc) - lastArgIsVarargs;
            s << "numArgs " << (lastArgIsVarargs ? ">=" : "==") << " " << numArgs << " && ";
        }

        if (refFunc->isOperatorOverload())
            s << (refFunc->isReverseOperator() ? "" : "!") << "isReverse && ";

        QString typeChecks;
        QTextStream tck(&typeChecks);
        QString typeConversions;
        QTextStream tcv(&typeConversions);

        QString pyArgName = manyArgs ? QString("pyargs[%1]").arg(overloadData->argPos()) : "arg";

        OverloadData* od = overloadData;
        while (od && !od->argType()->isVarargs()) {
            if (manyArgs)
                pyArgName = QString("pyargs[%1]").arg(od->argPos());

            writeTypeCheck(tck, od, pyArgName);

            Indentation indent(INDENT);
            if (od->argumentTypeReplaced().isEmpty()) {
                writeArgumentConversion(tcv, od->argType(),
                                        QString("cpp_arg%1").arg(od->argPos()),
                                        pyArgName,
                                        refFunc->implementingClass());
            }

            if (od->nextOverloadData().isEmpty()
                || od->nextArgumentHasDefaultValue()
                || od->nextOverloadData().size() != 1
                || od->overloads().size() != od->nextOverloadData().first()->overloads().size()) {
                overloadData = od;
                od = 0;
            } else {
                od = od->nextOverloadData().first();
                if (!od->argType()->isVarargs())
                    tck << " && ";
            }
        }

        s << typeChecks << ") {" << endl;
        s << typeConversions;

        {
            Indentation indent(INDENT);
            writeOverloadedMethodDecisor(s, overloadData);
        }

        s << INDENT << "} else ";
    }

    if (maxArgs > 0)
        s << "goto " << cpythonFunctionName(referenceFunction) << "_TypeError;" << endl;
}

QString CppGenerator::argumentNameFromIndex(const AbstractMetaFunction* func, int argIndex, const AbstractMetaClass** wrappedClass)
{
    *wrappedClass = 0;
    QString pyArgName;
    if (argIndex == -1) {
        pyArgName = QString("self");
        *wrappedClass = func->implementingClass();
    } else if (argIndex == 0) {
        pyArgName = PYTHON_RETURN_VAR;
        *wrappedClass = classes().findClass(func->type()->typeEntry()->name());
    } else {
        int real_index = OverloadData::numberOfRemovedArguments(func, argIndex - 1);
        *wrappedClass = classes().findClass(func->arguments().at(real_index)->type()->typeEntry()->name());
        if ((argIndex == 1)
            && OverloadData::isSingleArgument(getFunctionGroups(func->implementingClass())[func->name()]))
            pyArgName = QString("arg");
        else
            pyArgName = QString("pyargs[%1]").arg(argIndex - 1);
    }
    return pyArgName;
}

void CppGenerator::writeMethodCall(QTextStream& s, const AbstractMetaFunction* func, int maxArgs)
{
    s << INDENT << "// " << func->minimalSignature() << (func->isReverseOperator() ? " [reverse operator]": "") << endl;
    if (func->isConstructor()) {
        foreach (CodeSnip cs, func->injectedCodeSnips()) {
            if (cs.position == CodeSnip::End) {
                s << INDENT << "overloadId = " << func->ownerClass()->functions().indexOf(const_cast<AbstractMetaFunction* const>(func)) << ';' << endl;
                break;
            }
        }
    }

    if (func->isAbstract()) {
        s << INDENT << "if (SbkBaseWrapper_containsCppWrapper(self)) {\n";
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
            s << func->ownerClass()->name() << '.' << func->name() << "()' not implemented.\");" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
        s << INDENT << "}\n";
    }

    writeCppSelfDefinition(s, func);

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
                lastArg = func->arguments()[i];
                if (func->argumentRemoved(i + 1))
                    removedArgs++;
            }
        } else if (maxArgs != 0 && !func->arguments().isEmpty()) {
            lastArg = func->arguments().last();
        }

        writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::TargetLangCode, func, lastArg);
        s << endl;
    }

    CodeSnipList convRules = getConversionRule(TypeSystem::NativeCode, func);
    if (convRules.size())
        writeCodeSnips(s, convRules, CodeSnip::Beginning, TypeSystem::TargetLangCode, func);

    if (!func->isUserAdded()) {
        bool badModifications = false;
        QStringList userArgs;
        if (!func->isCopyConstructor()) {
            int removedArgs = 0;
            for (int i = 0; i < maxArgs + removedArgs; i++) {
                const AbstractMetaArgument* arg = func->arguments()[i];
                if (func->argumentRemoved(i + 1)) {

                    // If some argument with default value is removed from a
                    // method signature, the said value must be explicitly
                    // added to the method call.
                    removedArgs++;

                    // If have conversion rules I will use this for removed args
                    bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, arg->argumentIndex() + 1).isEmpty();
                    if (hasConversionRule) {
                        userArgs << arg->argumentName() + "_out";
                    } else {
                       if (arg->defaultValueExpression().isEmpty())
                           badModifications = true;
                       else
                           userArgs << arg->defaultValueExpression();
                    }
                } else {
                    int idx = arg->argumentIndex() - removedArgs;
                    QString argName;

                    bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, arg->argumentIndex() + 1).isEmpty();
                    if (hasConversionRule) {
                        argName = arg->argumentName() + "_out";
                    } else {
                        argName = QString("cpp_arg%1").arg(idx);
                        if (shouldDereferenceArgumentPointer(arg))
                            argName.prepend('*');
                    }
                    userArgs << argName;
                }
            }

            // If any argument's default value was modified the method must be called
            // with this new value whenever the user doesn't pass an explicit value to it.
            // Also, any unmodified default value coming after the last user specified
            // argument and before the modified argument must be splicitly stated.
            QStringList otherArgs;
            bool defaultModified = false;
            bool argsClear = true;
            for (int i = func->arguments().size() - 1; i >= maxArgs + removedArgs; i--) {
                const AbstractMetaArgument* arg = func->arguments().at(i);
                bool defValModified = arg->defaultValueExpression() != arg->originalDefaultValueExpression();
                if (argsClear && !defValModified)
                    continue;
                else
                    argsClear = false;

                defaultModified |=  defValModified || func->argumentRemoved(i + 1);

                if (arg->defaultValueExpression().isEmpty())
                    badModifications = true;
                else
                    otherArgs.prepend(arg->defaultValueExpression());
            }
            if (defaultModified)
                userArgs << otherArgs;
        }

        bool isCtor = false;
        QString methodCall;
        QTextStream mc(&methodCall);

        if (badModifications) {
            // When an argument is removed from a method signature and no other
            // means of calling the method is provided (as with code injection)
            // the generator must write a compiler error line stating the situation.
            if (func->injectedCodeSnips(CodeSnip::Any, TypeSystem::TargetLangCode).isEmpty()) {
                s << INDENT << "#error No way to call \"" << func->ownerClass()->name();
                s << "::" << func->minimalSignature();
                s << "\" with the modifications described on the type system file" << endl;
            }
        } else if (func->isOperatorOverload()) {
            QString firstArg = QString("(*%1)").arg(CPP_SELF_VAR);
            QString secondArg("cpp_arg0");
            if (!func->isUnaryOperator() && shouldDereferenceArgumentPointer(func->arguments().first())) {
                secondArg.prepend("(*");
                secondArg.append(')');
            }

            if (func->isUnaryOperator())
                std::swap(firstArg, secondArg);

            QString op = func->originalName();
            op = op.right(op.size() - (sizeof("operator")/sizeof(char)-1));

            if (func->isBinaryOperator()) {
                if (func->isReverseOperator())
                    std::swap(firstArg, secondArg);
                mc << firstArg << ' ' << op << ' ' << secondArg;
            } else {
                mc << op << ' ' << secondArg;
            }
        } else if (!injectedCodeCallsCppFunction(func)) {
            if (func->isConstructor() || func->isCopyConstructor()) {
                isCtor = true;
                mc << "new " << wrapperName(func->ownerClass());
                mc << '(';
                if (func->isCopyConstructor() && maxArgs == 1)
                    mc << "*cpp_arg0";
                else
                    mc << userArgs.join(", ");
                mc << ')';
            } else {
                if (func->ownerClass()) {
#ifndef AVOID_PROTECTED_HACK
                    if (!func->isStatic())
                        mc << CPP_SELF_VAR << "->";
                    if (!func->isAbstract())
                        mc << func->ownerClass()->name() << "::";
                    mc << func->originalName();
#else
                    if (!func->isStatic()) {
                        if (func->isProtected())
                            mc << "((" << wrapperName(func->ownerClass()) << "*) ";
                        mc << CPP_SELF_VAR << (func->isProtected() ? ")" : "") << "->";
                    }
                    if (!func->isAbstract())
                        mc << (func->isProtected() ? wrapperName(func->ownerClass()) : func->ownerClass()->name()) << "::";
                    mc << func->originalName() << (func->isProtected() ? "_protected" : "");
#endif
                } else {
                    mc << func->originalName();
                }
                mc << '(' << userArgs.join(", ") << ')';
            }
        }

        if (!badModifications) {
            if (!injectedCodeCallsCppFunction(func)) {
                if (func->allowThread())
                    s << INDENT << THREAD_STATE_SAVER_VAR << ".save();" << endl;
                s << INDENT;
                if (isCtor)
                    s << "cptr = ";
                else if (func->type() && !func->isInplaceOperator())
                    s << func->type()->cppSignature() << ' ' << CPP_RETURN_VAR << " = ";
                s << methodCall << ';' << endl;
                if (func->allowThread())
                    s << INDENT << THREAD_STATE_SAVER_VAR << ".restore();" << endl;

                if (!isCtor && !func->isInplaceOperator() && func->type()) {
                    s << INDENT << PYTHON_RETURN_VAR << " = ";
                    writeToPythonConversion(s, func->type(), func->ownerClass(), CPP_RETURN_VAR);
                    s << ';' << endl;
                }
            }
        }
    }

    if (func->hasInjectedCode() && !func->isConstructor()) {
        s << endl;
        writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::TargetLangCode, func, lastArg);
    }

    writeParentChildManagement(s, func);

    // Ownership transference between C++ and Python.
    QList<ArgumentModification> ownership_mods;
    // Python object reference management.
    QList<ArgumentModification> refcount_mods;
    foreach (FunctionModification func_mod, func->modifications()) {
        foreach (ArgumentModification arg_mod, func_mod.argument_mods) {
            if (!arg_mod.ownerships.isEmpty() && arg_mod.ownerships.contains(TypeSystem::TargetLangCode))
                ownership_mods.append(arg_mod);
            else if (!arg_mod.referenceCounts.isEmpty())
                refcount_mods.append(arg_mod);
        }
    }

    if (!ownership_mods.isEmpty()) {
        s << endl << INDENT << "// Ownership transferences." << endl;
        foreach (ArgumentModification arg_mod, ownership_mods) {
            const AbstractMetaClass* wrappedClass = 0;
            QString pyArgName = argumentNameFromIndex(func, arg_mod.index, &wrappedClass);
            if (!wrappedClass) {
                s << "#error Invalid ownership modification for argument " << arg_mod.index << endl << endl;
                break;
            }

            // The default ownership does nothing. This is useful to avoid automatic heuristically
            // based generation of code defining parenting.
            if (arg_mod.ownerships[TypeSystem::TargetLangCode] == TypeSystem::DefaultOwnership)
                continue;

            s << INDENT;
            if (arg_mod.ownerships[TypeSystem::TargetLangCode] == TypeSystem::TargetLangOwnership) {
                s << "SbkBaseWrapper_setOwnership(" << pyArgName << ", true);";
            } else if (wrappedClass->hasVirtualDestructor()) {
                if (arg_mod.index == 0) {
                    s << "SbkBaseWrapper_setOwnership(" << PYTHON_RETURN_VAR << ", 0);";
                } else {
                    s << "BindingManager::instance().transferOwnershipToCpp(" << pyArgName << ");";
                }
            } else {
                s << "BindingManager::instance().invalidateWrapper(" << pyArgName << ");";
            }
            s << endl;
        }

    } else if (!refcount_mods.isEmpty()) {
        foreach (ArgumentModification arg_mod, refcount_mods) {
            if (arg_mod.referenceCounts.first().action != ReferenceCount::Add)
                continue;
            const AbstractMetaClass* wrappedClass = 0;
            QString pyArgName = argumentNameFromIndex(func, arg_mod.index, &wrappedClass);
            if (!wrappedClass) {
                s << "#error Invalid reference count modification for argument " << arg_mod.index << endl << endl;
                break;
            }
            s << INDENT << "Shiboken::SbkBaseWrapper_keepReference(reinterpret_cast<SbkBaseWrapper*>(self), \"";
            s << func->minimalSignature() << arg_mod.index << "\", " << pyArgName << ");" << endl;
        }
    }
}

QStringList CppGenerator::getAncestorMultipleInheritance(const AbstractMetaClass* metaClass)
{
    QStringList result;
    if (!metaClass->baseClassNames().isEmpty()) {
        foreach (QString base, metaClass->baseClassNames()) {
            result.append(QString("((size_t) static_cast<const %1*>(class_ptr)) - base").arg(base));
            result.append(QString("((size_t) static_cast<const %1*>((%2*)((void*)class_ptr))) - base").arg(base).arg(metaClass->name()));
        }
        foreach (const AbstractMetaClass* pClass, getBaseClasses(metaClass))
            result.append(getAncestorMultipleInheritance(pClass));
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

        foreach (QString ancestor, ancestors)
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
    s << "static void* " << cpythonSpecialCastFunctionName(metaClass) << "(PyObject* obj, SbkBaseWrapperType* desiredType)\n";
    s << "{\n";
    s << INDENT << metaClass->qualifiedCppName() << "* me = " << cpythonWrapperCPtr(metaClass, "obj") << ";\n";
    AbstractMetaClassList bases = getBaseClasses(metaClass);
    bool firstClass = true;
    foreach(const AbstractMetaClass* baseClass, bases) {
        s << INDENT << (!firstClass ? "else " : "") << "if (desiredType == reinterpret_cast<SbkBaseWrapperType*>(" << cpythonTypeNameExt(baseClass->typeEntry()) << "))\n";
        Indentation indent(INDENT);
        s << INDENT << "return static_cast<" << baseClass->qualifiedCppName() << "*>(me);\n";
        firstClass = false;
    }
    s << INDENT << "else\n";
    Indentation indent(INDENT);
    s << INDENT << "return me;\n";
    s << "}\n\n";
}

QString CppGenerator::multipleInheritanceInitializerFunctionName(const AbstractMetaClass* metaClass)
{
    if (!hasMultipleInheritanceInAncestry(metaClass))
        return QString();
    return QString("%1_mi_init").arg(cpythonBaseName(metaClass->typeEntry()));
}

bool CppGenerator::supportsSequenceProtocol(const AbstractMetaClass* metaClass)
{
    foreach(QString funcName, m_sequenceProtocol.keys()) {
        if (metaClass->hasFunction(funcName))
            return true;
    }
    return false;
}

bool CppGenerator::shouldGenerateGetSetList(const AbstractMetaClass* metaClass)
{
    return !metaClass->fields().isEmpty();
}

void CppGenerator::writeClassDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString tp_flags;
    QString tp_init;
    QString tp_new;
    QString tp_getattro('0');
    QString tp_dealloc;
    QString tp_as_number('0');
    QString tp_as_sequence('0');
    QString mi_init('0');
    QString type_name_func('0');
    QString mi_specialcast('0');
    QString cppClassName = metaClass->qualifiedCppName();
    QString className = cpythonTypeName(metaClass).replace(QRegExp("_Type$"), "");
    QString baseClassName('0');

    if (metaClass->hasArithmeticOperatorOverload()
        || metaClass->hasLogicalOperatorOverload()
        || metaClass->hasBitwiseOperatorOverload()) {
        tp_as_number = QString("&%1_as_number").arg(cpythonBaseName(metaClass));
    }

    // sequence protocol check
    if (supportsSequenceProtocol(metaClass))
        tp_as_sequence = QString("&Py%1_as_sequence").arg(cppClassName);

    if (!metaClass->baseClass())
        baseClassName = "reinterpret_cast<PyTypeObject*>(&Shiboken::SbkBaseWrapper_Type)";

    if (metaClass->isNamespace() || metaClass->hasPrivateDestructor()) {
        tp_flags = "Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES";
        tp_dealloc = metaClass->hasPrivateDestructor() ?
                     "(destructor)Shiboken::SbkBaseWrapper_Dealloc_PrivateDtor" : "0";
        tp_init = "0";
    } else {
        tp_flags = "Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES";//|Py_TPFLAGS_HAVE_GC";

        QString deallocClassName;
        if (shouldGenerateCppWrapper(metaClass))
            deallocClassName = wrapperName(metaClass);
        else
            deallocClassName = cppClassName;
        tp_dealloc = QString("(destructor)&(Shiboken::SbkBaseWrapper_Dealloc< %1 >)").arg(deallocClassName);

        AbstractMetaFunctionList ctors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
        tp_init = ctors.isEmpty() ? "0" : cpythonFunctionName(ctors.first());
    }

    if (classNeedsGetattroFunction(metaClass))
        tp_getattro = cpythonGetattroFunctionName(metaClass);

    if (metaClass->isPolymorphic())
        type_name_func = cpythonBaseName(metaClass) + "_typeName";

    if (metaClass->hasPrivateDestructor())
        tp_new = "0";
    else
        tp_new = "Shiboken::SbkBaseWrapper_TpNew";

    QString tp_richcompare = QString('0');
    if (metaClass->hasComparisonOperatorOverload())
        tp_richcompare = cpythonBaseName(metaClass) + "_richcompare";

    QString tp_getset = QString('0');
    if (shouldGenerateGetSetList(metaClass))
        tp_getset = cpythonGettersSettersDefinitionName(metaClass);

    // search for special functions
    ShibokenGenerator::clearTpFuncs();
    foreach (AbstractMetaFunction* func, metaClass->functions()) {
        if (m_tpFuncs.contains(func->name()))
            m_tpFuncs[func->name()] = cpythonFunctionName(func);
    }

    // class or some ancestor has multiple inheritance
    const AbstractMetaClass* miClass = getMultipleInheritingClass(metaClass);
    if (miClass) {
        mi_init = QString("(Shiboken::MultipleInheritanceInitFunction)%1")
                            .arg(multipleInheritanceInitializerFunctionName(miClass));
        if (metaClass == miClass) {
            writeMultipleInheritanceInitializerFunction(s, metaClass);
        } else {
            s << "extern int* " << multipleInheritanceInitializerFunctionName(miClass);
            s << "(const void* cptr);" << endl;
        }
        mi_specialcast = '&'+cpythonSpecialCastFunctionName(metaClass);
        writeSpecialCastFunction(s, metaClass);
        s << endl;
    }

    s << "// Class Definition -----------------------------------------------" << endl;

    s << "static SbkBaseWrapperType " << className + "_Type" << " = { { {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&Shiboken::SbkBaseWrapperType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << cppClassName << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::SbkBaseWrapper)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          " << tp_dealloc << ',' << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             " << m_tpFuncs["__repr__"] << "," << endl;
    s << INDENT << "/*tp_as_number*/        " << tp_as_number << ',' << endl;
    s << INDENT << "/*tp_as_sequence*/      " << tp_as_sequence << ',' << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             0," << endl;
    s << INDENT << "/*tp_call*/             0," << endl;
    s << INDENT << "/*tp_str*/              " << m_tpFuncs["__str__"] << ',' << endl;
    s << INDENT << "/*tp_getattro*/         " << tp_getattro << ',' << endl;
    s << INDENT << "/*tp_setattro*/         0," << endl;
    s << INDENT << "/*tp_as_buffer*/        0," << endl;
    s << INDENT << "/*tp_flags*/            " << tp_flags << ',' << endl;
    s << INDENT << "/*tp_doc*/              0," << endl;
    s << INDENT << "/*tp_traverse*/         0," << endl;
    s << INDENT << "/*tp_clear*/            0," << endl;
    s << INDENT << "/*tp_richcompare*/      " << tp_richcompare << ',' << endl;
    s << INDENT << "/*tp_weaklistoffset*/   0," << endl;
    s << INDENT << "/*tp_iter*/             0," << endl;
    s << INDENT << "/*tp_iternext*/         0," << endl;
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
    s << INDENT << "/*mi_offsets*/          0," << endl;
    s << INDENT << "/*mi_init*/             " << mi_init << ',' << endl;
    s << INDENT << "/*mi_specialcast*/      " << mi_specialcast << ',' << endl;
    s << INDENT << "/*type_name_func*/      " << type_name_func << endl;
    s << "};" << endl;
}


void CppGenerator::writeSequenceMethods(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QMap<QString, QString> funcs;

    QHash< QString, QPair< QString, QString > >::const_iterator it = m_sequenceProtocol.begin();
    for (; it != m_sequenceProtocol.end(); ++it) {
        const AbstractMetaFunction* func = metaClass->findFunction(it.key());
        if (!func)
            continue;
        QString funcName = cpythonFunctionName(func);
        QString funcArgs = it.value().first;
        QString funcRetVal = it.value().second;

        CodeSnipList snips = func->injectedCodeSnips(CodeSnip::Any, TypeSystem::TargetLangCode);
        s << funcRetVal << ' ' << funcName << '(' << funcArgs << ')' << endl << '{' << endl;
        writeInvalidCppObjectCheck(s);

        writeCppSelfDefinition(s, func);

        const AbstractMetaArgument* lastArg = func->arguments().isEmpty() ? 0 : func->arguments().last();
        writeCodeSnips(s, snips,CodeSnip::Any, TypeSystem::TargetLangCode, func, lastArg);
        s << '}' << endl << endl;
    }
}

void CppGenerator::writeTypeAsSequenceDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString className = metaClass->qualifiedCppName();
    QMap<QString, QString> funcs;

    foreach(QString funcName, m_sequenceProtocol.keys()) {
        const AbstractMetaFunction* func = metaClass->findFunction(funcName);
        funcs[funcName] = func ? cpythonFunctionName(func).prepend("&") : "0";
    }

    s << "static PySequenceMethods Py" << className << "_as_sequence = {\n"
      << INDENT << "/*sq_length*/ " << funcs["__len__"] << ",\n"
      << INDENT << "/*sq_concat*/ " << funcs["__concat__"] << ",\n"
      << INDENT << "/*sq_repeat*/ 0,\n"
      << INDENT << "/*sq_item*/ " << funcs["__getitem__"] << ",\n"
      << INDENT << "/*sq_slice*/ " << funcs["__getslice__"] << ",\n"
      << INDENT << "/*sq_ass_item*/ " << funcs["__setitem__"] << ",\n"
      << INDENT << "/*sq_ass_slice*/ " << funcs["__setslice__"] << ",\n"
      << INDENT << "/*sq_contains*/ " << funcs["__contains__"] << ",\n"
      << INDENT << "/*sq_inplace_concat*/ 0,\n"
      << INDENT << "/*sq_inplace_repeat*/ 0\n"
      << "};\n\n";
}

void CppGenerator::writeTypeAsNumberDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QMap<QString, QString> nb;

    nb["__add__"] = QString('0');
    nb["__sub__"] = QString('0');
    nb["__mul__"] = QString('0');
    nb["__div__"] = QString('0');
    nb["__mod__"] = QString('0');
    nb["__neg__"] = QString('0');
    nb["__pos__"] = QString('0');
    nb["__invert__"] = QString('0');
    nb["__lshift__"] = QString('0');
    nb["__rshift__"] = QString('0');
    nb["__and__"] = QString('0');
    nb["__xor__"] = QString('0');
    nb["__or__"] = QString('0');
    nb["__iadd__"] = QString('0');
    nb["__isub__"] = QString('0');
    nb["__imul__"] = QString('0');
    nb["__idiv__"] = QString('0');
    nb["__imod__"] = QString('0');
    nb["__ilshift__"] = QString('0');
    nb["__irshift__"] = QString('0');
    nb["__iand__"] = QString('0');
    nb["__ixor__"] = QString('0');
    nb["__ior__"] = QString('0');

    QList<AbstractMetaFunctionList> opOverloads =
            filterGroupedOperatorFunctions(metaClass,
                                           AbstractMetaClass::ArithmeticOp
                                           | AbstractMetaClass::LogicalOp
                                           | AbstractMetaClass::BitwiseOp);

    foreach (AbstractMetaFunctionList opOverload, opOverloads) {
        const AbstractMetaFunction* rfunc = opOverload[0];
        QString opName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
        nb[opName] = cpythonFunctionName(rfunc);
    }

    s << "static PyNumberMethods " << cpythonBaseName(metaClass);
    s << "_as_number = {" << endl;
    s << INDENT << "/*nb_add*/                  (binaryfunc)" << nb["__add__"] << ',' << endl;
    s << INDENT << "/*nb_subtract*/             (binaryfunc)" << nb["__sub__"] << ',' << endl;
    s << INDENT << "/*nb_multiply*/             (binaryfunc)" << nb["__mul__"] << ',' << endl;
    s << INDENT << "/*nb_divide*/               (binaryfunc)" << nb["__div__"] << ',' << endl;
    s << INDENT << "/*nb_remainder*/            (binaryfunc)" << nb["__mod__"] << ',' << endl;
    s << INDENT << "/*nb_divmod*/               0," << endl;
    s << INDENT << "/*nb_power*/                0," << endl;
    s << INDENT << "/*nb_negative*/             (unaryfunc)" << nb["__neg__"] << ',' << endl;
    s << INDENT << "/*nb_positive*/             (unaryfunc)" << nb["__pos__"] << ',' << endl;
    s << INDENT << "/*nb_absolute*/             0," << endl;
    s << INDENT << "/*nb_nonzero*/              0," << endl;
    s << INDENT << "/*nb_invert*/               (unaryfunc)" << nb["__invert__"] << ',' << endl;
    s << INDENT << "/*nb_lshift*/               (binaryfunc)" << nb["__lshift__"] << ',' << endl;
    s << INDENT << "/*nb_rshift*/               (binaryfunc)" << nb["__rshift__"] << ',' << endl;
    s << INDENT << "/*nb_and*/                  (binaryfunc)" << nb["__and__"] << ',' << endl;
    s << INDENT << "/*nb_xor*/                  (binaryfunc)" << nb["__xor__"] << ',' << endl;
    s << INDENT << "/*nb_or*/                   (binaryfunc)" << nb["__or__"] << ',' << endl;
    s << INDENT << "/*nb_coerce*/               0," << endl;
    s << INDENT << "/*nb_int*/                  0," << endl;
    s << INDENT << "/*nb_long*/                 0," << endl;
    s << INDENT << "/*nb_float*/                0," << endl;
    s << INDENT << "/*nb_oct*/                  0," << endl;
    s << INDENT << "/*nb_hex*/                  0," << endl;
    s << INDENT << "/*nb_inplace_add*/          (binaryfunc)" << nb["__iadd__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_subtract*/     (binaryfunc)" << nb["__isub__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_multiply*/     (binaryfunc)" << nb["__imul__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_divide*/       (binaryfunc)" << nb["__idiv__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_remainder*/    (binaryfunc)" << nb["__imod__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_power*/        0," << endl;
    s << INDENT << "/*nb_inplace_lshift*/       (binaryfunc)" << nb["__ilshift__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_rshift*/       (binaryfunc)" << nb["__irshift__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_and*/          (binaryfunc)" << nb["__iand__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_xor*/          (binaryfunc)" << nb["__ixor__"] << ',' << endl;
    s << INDENT << "/*nb_inplace_or*/           (binaryfunc)" << nb["__ior__"] << ',' << endl;
    s << INDENT << "/*nb_floor_divide*/         0," << endl;
    s << INDENT << "/*nb_true_divide*/          0," << endl;
    s << INDENT << "/*nb_inplace_floor_divide*/ 0," << endl;
    s << INDENT << "/*nb_inplace_true_divide*/  0," << endl;
    s << INDENT << "/*nb_index*/                0" << endl;
    s << "};" << endl << endl;
}

void CppGenerator::writeGetterFunction(QTextStream& s, const AbstractMetaField* metaField)
{
    s << "static PyObject* " << cpythonGetterFunctionName(metaField) << "(SbkBaseWrapper* self)" << endl;
    s << '{' << endl;
    s << INDENT << "return ";
    QString cppField= QString("%1->%2").arg(cpythonWrapperCPtr(metaField->enclosingClass(), "self")).arg(metaField->name());
    writeToPythonConversion(s, metaField->type(), metaField->enclosingClass(), cppField);
    s << ';' << endl;
    s << '}' << endl;
}

void CppGenerator::writeSetterFunction(QTextStream& s, const AbstractMetaField* metaField)
{
    s << "static int " << cpythonSetterFunctionName(metaField) << "(SbkBaseWrapper* self, PyObject* value)" << endl;
    s << '{' << endl;

    s << INDENT << "if (value == 0) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"'";
        s << metaField->name() << "' may not be deleted\");" << endl;
        s << INDENT << "return -1;" << endl;
    }
    s << INDENT << '}' << endl;

    s << INDENT << "if (!";
    writeTypeCheck(s, metaField->type(), "value", isNumber(metaField->type()->typeEntry()));
    s << ") {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"wrong type attributed to '";
        s << metaField->name() << "', '" << metaField->type()->name() << "' or convertible type expected\");" << endl;
        s << INDENT << "return -1;" << endl;
    }
    s << INDENT << '}' << endl << endl;

    QString fieldStr = QString("%1->%2").arg(cpythonWrapperCPtr(metaField->enclosingClass(), "self")).arg(metaField->name());


    s << INDENT << fieldStr << " = ";
    writeToCppConversion(s, metaField->type(), metaField->enclosingClass(), "value");
    s << ';' << endl << endl;

    bool pythonWrapperRefCounting = metaField->type()->typeEntry()->isObject()
                                    || metaField->type()->isValuePointer();
    if (pythonWrapperRefCounting) {
        s << INDENT << "Shiboken::SbkBaseWrapper_keepReference(reinterpret_cast<SbkBaseWrapper*>(self), \"";
        s << metaField->name() << "\", value);" << endl;
        //s << INDENT << "Py_XDECREF(oldvalue);" << endl;
        s << endl;
    }

    s << INDENT << "return 0;" << endl;
    s << '}' << endl;
}

void CppGenerator::writeRichCompareFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString baseName = cpythonBaseName(metaClass);
    s << "static PyObject*" << endl;
    s << baseName << "_richcompare(PyObject* self, PyObject* other, int op)" << endl;
    s << '{' << endl;
    QList<AbstractMetaFunctionList> cmpOverloads = filterGroupedOperatorFunctions(metaClass, AbstractMetaClass::ComparisonOp);
    s << INDENT << "bool result;" << endl;
    s << INDENT << metaClass->qualifiedCppName() << "& cpp_self = *" << cpythonWrapperCPtr(metaClass) << ';' << endl;
    s << endl;

    s << INDENT << "switch (op) {" << endl;
    {
        Indentation indent(INDENT);
        foreach (AbstractMetaFunctionList overloads, cmpOverloads) {
            OverloadData overloadData(overloads, this);
            const AbstractMetaFunction* rfunc = overloads[0];

            s << INDENT << "case " << ShibokenGenerator::pythonRichCompareOperatorId(rfunc) << ':' << endl;

            Indentation indent(INDENT);

            QString op = rfunc->originalName();
            op = op.right(op.size() - QString("operator").size());

            int alternativeNumericTypes = 0;
            foreach (const AbstractMetaFunction* func, overloads) {
                if (!func->isStatic() &&
                    ShibokenGenerator::isNumber(func->arguments()[0]->type()->typeEntry()))
                    alternativeNumericTypes++;
            }

            bool first = true;
            bool comparesWithSameType = false;
            foreach (const AbstractMetaFunction* func, overloads) {
                if (func->isStatic())
                    continue;

                const AbstractMetaType* type = func->arguments()[0]->type();
                bool numberType = alternativeNumericTypes == 1 || ShibokenGenerator::isPyInt(type);

                if (!comparesWithSameType)
                    comparesWithSameType = type->typeEntry() == metaClass->typeEntry();

                if (!first) {
                    s << " else ";
                } else {
                    first = false;
                    s << INDENT;
                }

                s << "if (" << cpythonCheckFunction(type, numberType) << "(other)) {" << endl;
                {
                    Indentation indent(INDENT);
                    s << INDENT;
                    if (type->typeEntry()->isValue()) {
                        AbstractMetaClass* clz = classes().findClass(type->typeEntry());
                        Q_ASSERT(clz);
                        s << clz->qualifiedCppName() << '*';
                    } else
                        s << translateTypeForWrapperMethod(type, metaClass);
                    s << " cpp_other = ";
                    if (type->typeEntry()->isValue())
                        s << cpythonWrapperCPtr(type, "other");
                    else
                        writeToCppConversion(s, type, metaClass, "other");
                    s << ';' << endl;
                    s << INDENT << "result = (cpp_self " << op << ' ' << (type->typeEntry()->isValue() ? "(*" : "");
                    s << "cpp_other" << (type->typeEntry()->isValue() ? ")" : "") << ");" << endl;
                }
                s << INDENT << '}';
            }

            // Compares with implicit conversions
            if (comparesWithSameType && !metaClass->implicitConversions().isEmpty()) {
                AbstractMetaType temporaryType;
                temporaryType.setTypeEntry(metaClass->typeEntry());
                temporaryType.setConstant(true);
                temporaryType.setReference(false);
                temporaryType.setTypeUsagePattern(AbstractMetaType::ValuePattern);
                s << " else if (" << cpythonIsConvertibleFunction(metaClass->typeEntry());
                s << "(other)) {" << endl;
                {
                    Indentation indent(INDENT);
                    writeArgumentConversion(s, &temporaryType, "cpp_other", "other", metaClass);
                    s << INDENT << "result = (cpp_self " << op << " (*cpp_other));" << endl;
                }
                s << INDENT << '}';
            }

            s << " else goto " << baseName << "_RichComparison_TypeError;" << endl;
            s << endl;

            s << INDENT << "break;" << endl;
        }
        s << INDENT << "default:" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"operator not implemented.\");" << endl;
            s << INDENT << "return " << m_currentErrorCode << ';' << endl;
        }
    }
    s << INDENT << '}' << endl << endl;

    s << INDENT << "if (result)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "Py_RETURN_TRUE;" << endl;
    }
    s << INDENT << "Py_RETURN_FALSE;" << endl << endl;
    s << INDENT << baseName << "_RichComparison_TypeError:" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "Py_RETURN_FALSE;" << endl;
    }
    s << '}' << endl << endl;
}

void CppGenerator::writeMethodDefinitionEntry(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    Q_ASSERT(!overloads.isEmpty());
    QPair<int, int> minMax = OverloadData::getMinMaxArguments(overloads);
    const AbstractMetaFunction* func = overloads.first();
    s << '"' << func->name() << "\", (PyCFunction)" << cpythonFunctionName(func) << ", ";
    if (minMax.second < 2) {
        if (minMax.first == 0)
            s << "METH_NOARGS";
        if (minMax.first != minMax.second)
            s << '|';
        if (minMax.second == 1)
            s << "METH_O";
    } else {
        s << "METH_VARARGS";
    }
    if (func->ownerClass() && OverloadData::hasStaticFunction(overloads))
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

void CppGenerator::writeEnumInitialization(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);
    QString addFunction;
    if (cppEnum->enclosingClass()) {
        addFunction = QString("PyDict_SetItemString(Sbk")
                      + cppEnum->enclosingClass()->name()
                      + "_Type.super.ht_type.tp_dict,";
    } else {
        addFunction = "PyModule_AddObject(module,";
    }

    s << INDENT << "// init enum class: " << cppEnum->name() << endl;
    s << INDENT << cpythonTypeNameExt(cppEnum->typeEntry()) << " = &" << cpythonTypeName(cppEnum->typeEntry()) << ';' << endl;
    s << INDENT << "if (PyType_Ready((PyTypeObject*)&" << cpythonName << "_Type) < 0)" << endl;
    s << INDENT << INDENT << "return;" << endl;

    s << INDENT << "Py_INCREF(&" << cpythonName << "_Type);" << endl;

    s << INDENT << addFunction << endl;
    s << INDENT << INDENT << INDENT << '\"' << cppEnum->name() << "\",";
    s << "((PyObject*)&" << cpythonName << "_Type));" << endl << endl;

    FlagsTypeEntry* flags = cppEnum->typeEntry()->flags();
    if (flags) {
        QString flagsName = cpythonFlagsName(flags);
        s << INDENT << "// init flags class: " << flags->name() << endl;
        s << INDENT << cpythonTypeNameExt(flags) << " = &" << cpythonTypeName(flags) << ';' << endl;

        s << INDENT << "if (PyType_Ready((PyTypeObject*)&" << flagsName << "_Type) < 0)" << endl;
        s << INDENT << INDENT << "return;" << endl;

        s << INDENT << "Py_INCREF(&" << flagsName << "_Type);" << endl;

        s << INDENT << addFunction << endl;
        s << INDENT << INDENT << INDENT << '\"' << flags->flagsName() << "\",";
        s << "((PyObject*)&" << flagsName << "_Type));" << endl << endl;
    }


    foreach (const AbstractMetaEnumValue* enumValue, cppEnum->values()) {
        if (cppEnum->typeEntry()->isEnumValueRejected(enumValue->name()))
            continue;

        s << INDENT << "enum_item = Shiboken::SbkEnumObject_New(&";
        s << cpythonName << "_Type," << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "(long) ";
            if (cppEnum->enclosingClass())
                s << cppEnum->enclosingClass()->qualifiedCppName() << "::";
            s << enumValue->name() << ", \"" << enumValue->name() << "\");" << endl;
        }

        s << INDENT << addFunction << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << '"' << enumValue->name() << "\", enum_item);" << endl;
        }
        s << INDENT << "PyDict_SetItemString(" << cpythonName << "_Type.tp_dict," << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << '"' << enumValue->name() << "\", enum_item);" << endl;
        }

    }

    // TypeResolver stuff
    s << INDENT << "Shiboken::TypeResolver::createValueTypeResolver<int>(\"";
    if (cppEnum->enclosingClass())
        s << cppEnum->enclosingClass()->qualifiedCppName() << "::";
    s << cppEnum->name() << "\");\n";


    s << endl;
}

void CppGenerator::writeFlagsNewMethod(QTextStream& s, const FlagsTypeEntry* cppFlags)
{
    QString cpythonName = cpythonFlagsName(cppFlags);
    s << "static PyObject*" << endl;
    s << cpythonName << "_New(PyTypeObject* type, PyObject* args, PyObject* kwds)" << endl;
    s << '{' << endl;
    s << INDENT << "if (!PyType_IsSubtype(type, &" << cpythonName << "_Type))" << endl;
    s << INDENT << INDENT << "return 0;" << endl << endl;
    s << INDENT << "int item_value;" << endl;
    s << INDENT << "if (!PyArg_ParseTuple(args, \"i:__new__\", &item_value))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << "PyObject* self = Shiboken::SbkEnumObject_New(type, item_value);" << endl << endl;
    s << INDENT << "if (!self)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << "return self;" << endl << '}' << endl;
}

void CppGenerator::writeEnumNewMethod(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);
    s << "static PyObject*" << endl;
    s << cpythonName << "_New(PyTypeObject* type, PyObject* args, PyObject* kwds)" << endl;
    s << '{' << endl;
    s << INDENT << "int item_value;" << endl;
    s << INDENT << "if (!PyArg_ParseTuple(args, \"i:__new__\", &item_value))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << "PyObject* self = Shiboken::SbkEnumObject_New(type, item_value);" << endl << endl;
    s << INDENT << "if (!self)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << "return self;" << endl << '}' << endl;
}

void CppGenerator::writeEnumDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);
    QString tp_as_number("0");
    if (cppEnum->typeEntry()->flags())
        tp_as_number = QString("&%1_as_number").arg(cpythonName);


    s << "static PyGetSetDef " << cpythonName << "_getsetlist[] = {" << endl;
    s << INDENT << "{const_cast<char*>(\"name\"), (getter)Shiboken::SbkEnumObject_name}," << endl;
    s << INDENT << "{0}  // Sentinel" << endl;
    s << "};" << endl << endl;

    QString newFunc = cpythonName + "_New";

    s << "// forward declaration of new function" << endl;
    s << "static PyObject* " << newFunc << "(PyTypeObject*, PyObject*, PyObject*);" << endl << endl;

    s << "static PyTypeObject " << cpythonName << "_Type = {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&PyType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << cppEnum->name() << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::SbkEnumObject)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          0," << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             Shiboken::SbkEnumObject_repr," << endl;
    s << INDENT << "/*tp_as_number*/        " << tp_as_number << ',' << endl;
    s << INDENT << "/*tp_as_sequence*/      0," << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             0," << endl;
    s << INDENT << "/*tp_call*/             0," << endl;
    s << INDENT << "/*tp_str*/              Shiboken::SbkEnumObject_repr," << endl;
    s << INDENT << "/*tp_getattro*/         0," << endl;
    s << INDENT << "/*tp_setattro*/         0," << endl;
    s << INDENT << "/*tp_as_buffer*/        0," << endl;
    s << INDENT << "/*tp_flags*/            Py_TPFLAGS_DEFAULT," << endl;
    s << INDENT << "/*tp_doc*/              0," << endl;
    s << INDENT << "/*tp_traverse*/         0," << endl;
    s << INDENT << "/*tp_clear*/            0," << endl;
    s << INDENT << "/*tp_richcompare*/      0," << endl;
    s << INDENT << "/*tp_weaklistoffset*/   0," << endl;
    s << INDENT << "/*tp_iter*/             0," << endl;
    s << INDENT << "/*tp_iternext*/         0," << endl;
    s << INDENT << "/*tp_methods*/          0," << endl;
    s << INDENT << "/*tp_members*/          0," << endl;
    s << INDENT << "/*tp_getset*/           " << cpythonName << "_getsetlist," << endl;
    s << INDENT << "/*tp_base*/             &PyInt_Type," << endl;
    s << INDENT << "/*tp_dict*/             0," << endl;
    s << INDENT << "/*tp_descr_get*/        0," << endl;
    s << INDENT << "/*tp_descr_set*/        0," << endl;
    s << INDENT << "/*tp_dictoffset*/       0," << endl;
    s << INDENT << "/*tp_init*/             0," << endl;
    s << INDENT << "/*tp_alloc*/            0," << endl;
    s << INDENT << "/*tp_new*/              " << newFunc << ',' << endl;
    s << INDENT << "/*tp_free*/             0," << endl;
    s << INDENT << "/*tp_is_gc*/            0," << endl;
    s << INDENT << "/*tp_bases*/            0," << endl;
    s << INDENT << "/*tp_mro*/              0," << endl;
    s << INDENT << "/*tp_cache*/            0," << endl;
    s << INDENT << "/*tp_subclasses*/       0," << endl;
    s << INDENT << "/*tp_weaklist*/         0" << endl;
    s << "};" << endl << endl;

    writeEnumNewMethod(s, cppEnum);
    s << endl;
}

void CppGenerator::writeFlagsMethods(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    writeFlagsBinaryOperator(s, cppEnum, "and", "&");
    writeFlagsBinaryOperator(s, cppEnum, "or", "|");
    writeFlagsBinaryOperator(s, cppEnum, "xor", "^");

    writeFlagsInplaceOperator(s, cppEnum, "iand", "&=");
    writeFlagsInplaceOperator(s, cppEnum, "ior", "|=");
    writeFlagsInplaceOperator(s, cppEnum, "ixor", "^=");

    writeFlagsUnaryOperator(s, cppEnum, "neg", "~");
    writeFlagsUnaryOperator(s, cppEnum, "not", "!", true);
    s << endl;
}

void CppGenerator::writeFlagsNumberMethodsDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);

    s << "static PyNumberMethods " << cpythonName << "_as_number = {" << endl;
    s << INDENT << "/*nb_add*/                  0," << endl;
    s << INDENT << "/*nb_subtract*/             0," << endl;
    s << INDENT << "/*nb_multiply*/             0," << endl;
    s << INDENT << "/*nb_divide*/               0," << endl;
    s << INDENT << "/*nb_remainder*/            0," << endl;
    s << INDENT << "/*nb_divmod*/               0," << endl;
    s << INDENT << "/*nb_power*/                0," << endl;
    s << INDENT << "/*nb_negative*/             (unaryfunc)" << cpythonName << "___neg__," << endl;
    s << INDENT << "/*nb_positive*/             0," << endl;
    s << INDENT << "/*nb_absolute*/             0," << endl;
    s << INDENT << "/*nb_nonzero*/              0," << endl;
    s << INDENT << "/*nb_invert*/               0," << endl;
    s << INDENT << "/*nb_lshift*/               0," << endl;
    s << INDENT << "/*nb_rshift*/               0," << endl;
    s << INDENT << "/*nb_and*/                  (binaryfunc)" << cpythonName  << "___and__" << ',' << endl;
    s << INDENT << "/*nb_xor*/                  (binaryfunc)" << cpythonName  << "___xor__" << ',' << endl;
    s << INDENT << "/*nb_or*/                   (binaryfunc)" << cpythonName  << "___or__" << ',' << endl;
    s << INDENT << "/*nb_coerce*/               0," << endl;
    s << INDENT << "/*nb_int*/                  0," << endl;
    s << INDENT << "/*nb_long*/                 0," << endl;
    s << INDENT << "/*nb_float*/                0," << endl;
    s << INDENT << "/*nb_oct*/                  0," << endl;
    s << INDENT << "/*nb_hex*/                  0," << endl;
    s << INDENT << "/*nb_inplace_add*/          0," << endl;
    s << INDENT << "/*nb_inplace_subtract*/     0," << endl;
    s << INDENT << "/*nb_inplace_multiply*/     0," << endl;
    s << INDENT << "/*nb_inplace_divide*/       0," << endl;
    s << INDENT << "/*nb_inplace_remainder*/    0," << endl;
    s << INDENT << "/*nb_inplace_power*/        0," << endl;
    s << INDENT << "/*nb_inplace_lshift*/       0," << endl;
    s << INDENT << "/*nb_inplace_rshift*/       0," << endl;
    s << INDENT << "/*nb_inplace_and*/          (binaryfunc)" << cpythonName  << "___iand__" << ',' << endl;
    s << INDENT << "/*nb_inplace_xor*/          (binaryfunc)" << cpythonName  << "___ixor__" << ',' << endl;
    s << INDENT << "/*nb_inplace_or*/           (binaryfunc)" << cpythonName  << "___ior__" << ',' << endl;
    s << INDENT << "/*nb_floor_divide*/         0," << endl;
    s << INDENT << "/*nb_true_divide*/          0," << endl;
    s << INDENT << "/*nb_inplace_floor_divide*/ 0," << endl;
    s << INDENT << "/*nb_inplace_true_divide*/  0," << endl;
    s << INDENT << "/*nb_index*/                0" << endl;
    s << "};" << endl << endl;
}

void CppGenerator::writeFlagsDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    if (!flagsEntry)
        return;
    QString cpythonName = cpythonFlagsName(flagsEntry);
    QString enumName = cpythonEnumName(cppEnum);

    QString newFunc = cpythonName + "_New";

    s << "// forward declaration of new function" << endl;
    s << "static PyObject* " << newFunc << "(PyTypeObject*, PyObject*, PyObject*);" << endl << endl;
    s << "static PyTypeObject " << cpythonName << "_Type = {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&PyType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << flagsEntry->flagsName() << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::SbkEnumObject)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          0," << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             Shiboken::SbkEnumObject_repr," << endl;
    s << INDENT << "/*tp_as_number*/        0," << endl;
    s << INDENT << "/*tp_as_sequence*/      0," << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             0," << endl;
    s << INDENT << "/*tp_call*/             0," << endl;
    s << INDENT << "/*tp_str*/              Shiboken::SbkEnumObject_repr," << endl;
    s << INDENT << "/*tp_getattro*/         0," << endl;
    s << INDENT << "/*tp_setattro*/         0," << endl;
    s << INDENT << "/*tp_as_buffer*/        0," << endl;
    s << INDENT << "/*tp_flags*/            Py_TPFLAGS_DEFAULT," << endl;
    s << INDENT << "/*tp_doc*/              0," << endl;
    s << INDENT << "/*tp_traverse*/         0," << endl;
    s << INDENT << "/*tp_clear*/            0," << endl;
    s << INDENT << "/*tp_richcompare*/      0," << endl;
    s << INDENT << "/*tp_weaklistoffset*/   0," << endl;
    s << INDENT << "/*tp_iter*/             0," << endl;
    s << INDENT << "/*tp_iternext*/         0," << endl;
    s << INDENT << "/*tp_methods*/          0," << endl;
    s << INDENT << "/*tp_members*/          0," << endl;
    s << INDENT << "/*tp_getset*/           0," << endl;
    s << INDENT << "/*tp_base*/             &" << enumName << "_Type," << endl;
    s << INDENT << "/*tp_dict*/             0," << endl;
    s << INDENT << "/*tp_descr_get*/        0," << endl;
    s << INDENT << "/*tp_descr_set*/        0," << endl;
    s << INDENT << "/*tp_dictoffset*/       0," << endl;
    s << INDENT << "/*tp_init*/             0," << endl;
    s << INDENT << "/*tp_alloc*/            0," << endl;
    s << INDENT << "/*tp_new*/              " << newFunc << ',' << endl;
    s << INDENT << "/*tp_free*/             0," << endl;
    s << INDENT << "/*tp_is_gc*/            0," << endl;
    s << INDENT << "/*tp_bases*/            0," << endl;
    s << INDENT << "/*tp_mro*/              0," << endl;
    s << INDENT << "/*tp_cache*/            0," << endl;
    s << INDENT << "/*tp_subclasses*/       0," << endl;
    s << INDENT << "/*tp_weaklist*/         0" << endl;
    s << "};" << endl << endl;

    writeFlagsNewMethod(s, flagsEntry);
    s << endl;
}

void CppGenerator::writeFlagsBinaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                            QString pyOpName, QString cppOpName)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject*" << endl;
    s << cpythonEnumName(cppEnum) << "___" << pyOpName << "__(PyObject* self, PyObject* arg)" << endl;
    s << '{' << endl;
    s << INDENT << "return Shiboken::Converter< " << flagsEntry->originalName() << " >::toPython(" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "((" << flagsEntry->originalName() << ") ((SbkEnumObject*)self)->ob_ival)" << endl;
        s << INDENT << cppOpName << " Shiboken::Converter< ";
        s << flagsEntry->originalName() << " >::toCpp(arg)" << endl;
    }
    s << INDENT << ");" << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeFlagsInplaceOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                             QString pyOpName, QString cppOpName)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject*" << endl;
    s << cpythonEnumName(cppEnum) << "___" << pyOpName << "__(PyObject* self, PyObject* arg)" << endl;
    s << '{' << endl;

    s << INDENT << "((" << flagsEntry->originalName() << ") ((SbkEnumObject*)self)->ob_ival) " << cppOpName << endl;
    s << INDENT << "Shiboken::Converter< " << flagsEntry->originalName() << " >::toCpp(arg);" << endl;

    s << INDENT << "Py_INCREF(self);" << endl;
    s << INDENT << "return self;" << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeFlagsUnaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                           QString pyOpName, QString cppOpName, bool boolResult)
{
    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject*" << endl;
    s << cpythonEnumName(cppEnum) << "___" << pyOpName << "__(PyObject* self, PyObject* arg)" << endl;
    s << '{' << endl;
    s << INDENT << "return Shiboken::Converter< " << (boolResult ? "bool" : flagsEntry->originalName());
    s << " >::toPython(" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << cppOpName << " Shiboken::Converter< ";
        s << flagsEntry->originalName() << " >::toCpp(arg)" << endl;
    }
    s << INDENT << ");" << endl;
    s << '}' << endl << endl;
}

void CppGenerator::writeClassRegister(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString pyTypeName = cpythonTypeName(metaClass);
    s << "PyAPI_FUNC(void)" << endl;
    s << "init_" << metaClass->qualifiedCppName().replace("::", "_") << "(PyObject* module)" << endl;
    s << '{' << endl;

    if (!metaClass->isNamespace())
        s << INDENT << cpythonTypeNameExt(metaClass->typeEntry()) << " = reinterpret_cast<PyTypeObject*>(&" << cpythonTypeName(metaClass->typeEntry()) << ");" << endl << endl;

    // class inject-code target/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Beginning, TypeSystem::TargetLangCode, 0, 0, metaClass);
        s << endl;
    }

    if (metaClass->baseClass())
        s << INDENT << pyTypeName << ".super.ht_type.tp_base = " << cpythonTypeNameExt(metaClass->baseClass()->typeEntry()) << ';' << endl;
    // Multiple inheritance
    if (metaClass->baseClassNames().size() > 1) {
        AbstractMetaClassList baseClasses = getBaseClasses(metaClass);
        s << INDENT << pyTypeName << ".super.ht_type.tp_bases = PyTuple_Pack(";
        s << baseClasses.size();
        s << ',' << endl;
        QStringList bases;
        foreach (const AbstractMetaClass* base, baseClasses)
            bases << "(PyTypeObject*)"+cpythonTypeNameExt(base->typeEntry());
        Indentation indent(INDENT);
        s << INDENT << bases.join(", ") << ");" << endl << endl;
    }

    s << INDENT << "if (PyType_Ready((PyTypeObject*)&" << pyTypeName << ") < 0)" << endl;
    s << INDENT << INDENT << "return;" << endl << endl;
    s << INDENT << "Py_INCREF(reinterpret_cast<PyObject*>(&" << pyTypeName << "));" << endl;
    s << INDENT << "PyModule_AddObject(module, \"" << metaClass->name() << "\"," << endl;
    s << INDENT << INDENT << "((PyObject*)&" << pyTypeName << "));" << endl << endl;

    if (!metaClass->enums().isEmpty()) {
        s << INDENT << "// Initialize enums" << endl;
        s << INDENT << "PyObject* enum_item;" << endl << endl;
    }
    foreach (const AbstractMetaEnum* cppEnum, metaClass->enums())
        writeEnumInitialization(s, cppEnum);

    // class inject-code target/end
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        s << endl;
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::End, TypeSystem::TargetLangCode, 0, 0, metaClass);
    }

    if (!metaClass->isNamespace()) {
        bool isObjectType = metaClass->typeEntry()->isObject();
        QString typeName = metaClass->qualifiedCppName();
        QString registeredTypeName = typeName + (isObjectType ? "*" : "");
        QString functionSufix = isObjectType ? "Object" : "Value";
        s << INDENT << "Shiboken::TypeResolver::create" << functionSufix;
        s << "TypeResolver<" << typeName << " >" << "(\"" << registeredTypeName << "\");\n";
        s << INDENT << "Shiboken::TypeResolver::create" << functionSufix;
        s << "TypeResolver<" << typeName << " >" << "(typeid(" << typeName << ").name());\n";
    }

    s << '}' << endl << endl;
}

void CppGenerator::writeTypeNameFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    Indentation indent(INDENT);
    s << "static const char* " << cpythonBaseName(metaClass) << "_typeName(const void* cptr)\n{\n";
    s << INDENT << "return typeid(*reinterpret_cast<const " << metaClass->qualifiedCppName() << "*>(cptr)).name();\n";
    s << "}\n\n";
}

void CppGenerator::writeSbkCopyCppObjectFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    if (!metaClass->typeEntry()->isValue() || !shouldGenerateCppWrapper(metaClass))
        return;
    QString className = metaClass->qualifiedCppName();
    s << className << "* CppObjectCopier<" << className << " >::copy(const " << className << "& cppobj)" << endl;
    s << '{' << endl;
    s << INDENT << "return new " << wrapperName(metaClass) << "(cppobj);" << endl;
    s << '}' << endl;
}

void CppGenerator::writeGetattroFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    s << "static PyObject* " << cpythonGetattroFunctionName(metaClass) << "(PyObject* self, PyObject* name)" << endl;
    s << '{' << endl;
    s << INDENT << "if (self) {" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "if (SbkBaseWrapper_instanceDict(self)) {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyObject* meth = PyDict_GetItem(SbkBaseWrapper_instanceDict(self), name);" << endl;
            s << INDENT << "if (meth) {" << endl;
            {
                Indentation indent(INDENT);
                s << INDENT << "Py_INCREF(meth);" << endl;
                s << INDENT << "return meth;" << endl;
            }
            s << INDENT << '}' << endl;
        }
        s << INDENT << '}' << endl;
        s << INDENT << "const char* cname = PyString_AS_STRING(name);" << endl;
        foreach (const AbstractMetaFunction* func, getMethodsWithBothStaticAndNonStaticMethods(metaClass)) {
            s << INDENT << "if (strcmp(cname, \"" << func->name() << "\") == 0)" << endl;
            Indentation indent(INDENT);
            s << INDENT << "return PyCFunction_NewEx(&" << cpythonMethodDefinitionName(func) << ", self, 0);" << endl;
        }
    }
    s << INDENT << '}' << endl;
    s << INDENT << "return PyObject_GenericGetAttr(self, name);" << endl;
    s << '}' << endl;
}

void CppGenerator::finishGeneration()
{
    //Generate CPython wrapper file
    QString classInitDecl;
    QTextStream s_classInitDecl(&classInitDecl);
    QString classPythonDefines;
    QTextStream s_classPythonDefines(&classPythonDefines);
    QString namespaceDefines;
    QTextStream s_namespaceDefines(&namespaceDefines);

    QSet<QString> includes;
    QString globalFunctionImpl;
    QTextStream s_globalFunctionImpl(&globalFunctionImpl);
    QString globalFunctionDecl;
    QTextStream s_globalFunctionDef(&globalFunctionDecl);

    Indentation indent(INDENT);

    foreach (AbstractMetaFunctionList globalOverloads, getFunctionGroups().values()) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, globalOverloads) {
            // TODO: this is an ugly hack to avoid binding global
            // functions from outside the library beign processed.
            // The decent solution is to expand API Extractor so
            // that it support global function declarations on
            // type system files.
            QString incFile = func->includeFile();
            QRegExp regex("\\b(?:lib)?" + moduleName() + "\\b");
            if (!func->isUserAdded() && regex.indexIn(incFile) == -1)
                continue;

            if (!func->isModifiedRemoved())
                overloads.append(func);
        }

        if (overloads.isEmpty())
            continue;

        QString includeFile = overloads.first()->includeFile();
        if (!includeFile.isEmpty())
            includes << includeFile;

        writeMethodWrapper(s_globalFunctionImpl, overloads);
        writeMethodDefinition(s_globalFunctionDef, overloads);
    }

    foreach (const AbstractMetaClass *cls, classes()) {
        if (!shouldGenerate(cls) || cls->enclosingClass())
            continue;

        s_classInitDecl << "extern \"C\" PyAPI_FUNC(void) init_"
                        << cls->qualifiedCppName().replace("::", "_") << "(PyObject* module);" << endl;

        QString defineStr = "init_" + cls->qualifiedCppName().replace("::", "_") + "(module);";
        if (cls->isNamespace())
            s_namespaceDefines << INDENT << defineStr << endl;
        else
            s_classPythonDefines << INDENT << defineStr << endl;
    }

    QString moduleFileName(outputDirectory() + "/" + subDirectoryForPackage(packageName()));
    moduleFileName += "/" + moduleName().toLower() + "_module_wrapper.cpp";

    QFile file(moduleFileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);

        // write license comment
        s << licenseComment() << endl;

        s << "#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */" << endl;
        s << "#define PyMODINIT_FUNC void" << endl << "#endif" << endl << endl;

        s << "#include <Python.h>" << endl;
        s << "#include <shiboken.h>" << endl;
        s << "#include \"" << getModuleHeaderFileName() << '"' << endl << endl;
        foreach (const QString& include, includes)
            s << "#include \"" << include << '\"' << endl;
        s << endl;

        TypeSystemTypeEntry* moduleEntry = reinterpret_cast<TypeSystemTypeEntry*>(TypeDatabase::instance()->findType(packageName()));
        CodeSnipList snips = moduleEntry->codeSnips();

        // module inject-code native/beginning
        if (!snips.isEmpty()) {
            writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::NativeCode);
            s << endl;
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

        if (!globalEnums().isEmpty()) {
            QString converterImpl;
            QTextStream convImpl(&converterImpl);

            s << "// Enum definitions ";
            s << "------------------------------------------------------------" << endl;
            foreach (const AbstractMetaEnum* cppEnum, globalEnums()) {
                writeEnumDefinition(s, cppEnum);
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

        s << "PyTypeObject** " << cppApiVariableName() << ";" << endl << endl;;
        foreach (const QString& requiredModule, TypeDatabase::instance()->requiredTargetImports())
            s << "PyTypeObject** " << cppApiVariableName(requiredModule) << ";" << endl << endl;;

        s << "// Module initialization ";
        s << "------------------------------------------------------------" << endl;
        s << "extern \"C\" {" << endl << endl;

        s << getApiExportMacro() << " PyMODINIT_FUNC" << endl << "init" << moduleName() << "()" << endl;
        s << '{' << endl;

        // module inject-code target/beginning
        if (!snips.isEmpty()) {
            writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::TargetLangCode);
            s << endl;
        }

        foreach (const QString& requiredModule, TypeDatabase::instance()->requiredTargetImports()) {
            s << INDENT << "if (!Shiboken::importModule(\"" << requiredModule << "\", &" << cppApiVariableName(requiredModule) << ")) {" << endl;
            s << INDENT << INDENT << "PyErr_SetString(PyExc_ImportError," << "\"could not import ";
            s << requiredModule << "\");" << endl << INDENT << INDENT << "return;" << endl;
            s << INDENT << "}" << endl << endl;
        }

        s << INDENT << "Shiboken::init_shiboken();" << endl;
        s << INDENT << "PyObject* module = Py_InitModule(\""  << moduleName() << "\", ";
        s << moduleName() << "_methods);" << endl << endl;

        s << INDENT << "// Create a CObject containing the API pointer array's address" << endl;
        s << INDENT << "static PyTypeObject* cppApi[" << "SBK_" << moduleName() << "_IDX_COUNT" << "];" << endl;
        s << INDENT << cppApiVariableName() << " = cppApi;" << endl;
        s << INDENT << "PyObject* cppApiObject = PyCObject_FromVoidPtr(reinterpret_cast<void*>(cppApi), 0);" << endl;
        s << INDENT << "PyModule_AddObject(module, \"_Cpp_Api\", cppApiObject);" << endl << endl;
        s << INDENT << "// Initialize classes in the type system" << endl;
        s << classPythonDefines << endl;

        s << INDENT << "// Initialize namespaces as uninstantiable classes in the type system" << endl;
        s << namespaceDefines << endl;

        if (!globalEnums().isEmpty()) {
            s << INDENT << "// Initialize enums" << endl;
            s << INDENT << "PyObject* enum_item;" << endl << endl;
        }

        foreach (const AbstractMetaEnum* cppEnum, globalEnums())
           writeEnumInitialization(s, cppEnum);

        // module inject-code target/end
        if (!snips.isEmpty()) {
            writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::TargetLangCode);
            s << endl;
        }

        s << INDENT << "if (PyErr_Occurred()) {" << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT << "PyErr_Print();" << endl;
            s << INDENT << "Py_FatalError(\"can't initialize module " << moduleName() << "\");" << endl;
        }
        s << INDENT << '}' << endl;
        s << '}' << endl << endl;
        s << "} // extern \"C\"" << endl << endl;

        // module inject-code native/end
        if (!snips.isEmpty()) {
            writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::NativeCode);
            s << endl;
        }
    }
}

void CppGenerator::writeParentChildManagement(QTextStream& s, const AbstractMetaFunction* func)
{
    const int numArgs = func->arguments().count();
    const AbstractMetaClass* cppClass = func->ownerClass();
    bool ctorHeuristicEnabled = func->isConstructor() && useCtorHeuristic();

    // -1   = return value
    //  0    = self
    //  1..n = func. args.
    for (int i = -1; i <= numArgs; ++i) {
        QString parentVariable;
        QString childVariable;
        ArgumentOwner argOwner = func->argumentOwner(cppClass, i);
        bool usePyArgs = getMinMaxArguments(func).second > 1 || func->isConstructor();

        ArgumentOwner::Action action = argOwner.action;
        int parentIndex = argOwner.index;
        int childIndex = i;
        if (ctorHeuristicEnabled && i > 0 && numArgs) {
            AbstractMetaArgument* arg = func->arguments().at(i-1);
            if (arg->argumentName() == "parent" && (arg->type()->isObject() || arg->type()->isQObject())) {
                action = ArgumentOwner::Add;
                parentIndex = i;
                childIndex = -1;
            }
        }

        if (action != ArgumentOwner::Invalid) {
            if (!usePyArgs && i > 1)
                ReportHandler::warning("Argument index for parent tag out of bounds: "+func->signature());

            if (parentIndex == 0)
                parentVariable = PYTHON_RETURN_VAR;
            else if (parentIndex == -1)
                parentVariable = "self";
            else
                parentVariable = usePyArgs ? "pyargs["+QString::number(parentIndex-1)+"]" : "arg";

            if (argOwner.action == ArgumentOwner::Remove)
                childVariable = "0";
            else if (childIndex == 0)
                childVariable = PYTHON_RETURN_VAR;
            else if (childIndex == -1)
                childVariable = "self";
            else
                childVariable = usePyArgs ? "pyargs["+QString::number(childIndex-1)+"]" : "arg";

            s << INDENT << "Shiboken::setParent(" << parentVariable << ", " << childVariable << ");\n";

        }
    }
    writeReturnValueHeuristics(s, func);
}

void CppGenerator::writeReturnValueHeuristics(QTextStream& s, const AbstractMetaFunction* func, const QString& self)
{
    AbstractMetaType *type = func->type();
    if (!useReturnValueHeuristic()
        || !func->ownerClass()
        || func->ownership(func->ownerClass(), TypeSystem::TargetLangCode, 0) != TypeSystem::InvalidOwnership
        || !type
        || func->isStatic()
        || !func->typeReplaced(0).isEmpty()) {
        return;
    }

    if (type->isQObject() || type->isObject() || type->isValuePointer())
        s << INDENT << "Shiboken::setParent(" << self << ", " PYTHON_RETURN_VAR ");" << endl;
}
