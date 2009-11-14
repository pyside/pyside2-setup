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
#include <apiextractor/reporthandler.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

static Indentor INDENT;


CppGenerator::CppGenerator()
{
    // sequence protocol functions
    typedef QPair<QString, QString> StrPair;
    m_sequenceProtocol.insert("__len__", StrPair("PyObject* self", "Py_ssize_t"));
    m_sequenceProtocol.insert("__getitem__", StrPair("PyObject* self, Py_ssize_t _i", "PyObject*"));
    m_sequenceProtocol.insert("__setitem__", StrPair("PyObject* self, Py_ssize_t _i, PyObject* _value", "int"));
    m_sequenceProtocol.insert("__contains__", StrPair("PyObject* self, PyObject* _value", "int"));
    m_sequenceProtocol.insert("__concat__", StrPair("PyObject* self, PyObject* _other", "PyObject*"));
}

QString CppGenerator::fileNameForClass(const AbstractMetaClass *metaClass) const
{
    return metaClass->qualifiedCppName().toLower().replace("::", "_") + QLatin1String("_wrapper.cpp");
}

QList<AbstractMetaFunctionList> CppGenerator::filterGroupedFunctions(const AbstractMetaClass* metaClass)
{
    AbstractMetaFunctionList lst;
    if (metaClass)
        lst = queryFunctions(metaClass, true);
    else
        lst = globalFunctions();

    QMap<QString, AbstractMetaFunctionList> results;
    foreach (AbstractMetaFunction* func, lst) {
        //skip signals
        if (func->isSignal() || func->isDestructor() || (func->isModifiedRemoved() && !func->isAbstract()))
            continue;
        // weird operator overloads
        if (func->name() == "operator[]" || func->name() == "operator->")  // FIXME: what about cast operators?
            continue;
        results[func->name()].append(func);
    }

    //TODO: put these lines back to work

    //append global operators
    //lst += queryGlobalOperators(metaClass);

    return results.values();
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

    if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        //workaround to access protected functions
        s << "//workaround to access protected functions" << endl;
        s << "#define protected public" << endl << endl;
    }

    // headers
    s << "// default includes" << endl;
    s << "#include <shiboken.h>" << endl;
    s << "#include \"" << moduleName().toLower() << "_python.h\"" << endl << endl;

    QString converterImpl;
    QTextStream convImpl(&converterImpl);
    if (!metaClass->isNamespace()) {
        Indentation indentation(INDENT);
        writeTypeConverterImpl(convImpl, metaClass->typeEntry());
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
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Beginning, TypeSystem::NativeCode);
        s << endl;
    }

    if (metaClass->isPolymorphic() && !metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        s << "// Native ---------------------------------------------------------" << endl;
        s << endl;

        foreach (const AbstractMetaFunction* func, filterFunctions(metaClass)) {
            if (func->isPrivate() || (func->isModifiedRemoved() && !func->isAbstract()))
                continue;
            if (func->isConstructor() && !func->isCopyConstructor())
                writeConstructorNative(s, func);
            else if (func->isVirtual() || func->isAbstract())
                writeVirtualMethodNative(s, func);
        }

        writeDestructorNative(s, metaClass);

        s << endl << "// Target ---------------------------------------------------------" << endl;
        s << endl;
    }

    Indentation indentation(INDENT);

    QString methodsDefinitions;
    QTextStream md(&methodsDefinitions);

    bool hasComparisonOperator = false;
    bool typeAsNumber = false;

    foreach (AbstractMetaFunctionList allOverloads, filterGroupedFunctions(metaClass)) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, allOverloads) {
            if (!func->isAssignmentOperator() && !func->isCastOperator() && !func->isModifiedRemoved() && !func->isPrivate() &&
                func->ownerClass() == func->implementingClass())
                overloads.append(func);
        }

        if (overloads.isEmpty())
            continue;

        const AbstractMetaFunction* rfunc = overloads[0];
        if (m_sequenceProtocol.contains(rfunc->name()))
            continue;

        if (rfunc->isConstructor())
            writeConstructorWrapper(s, overloads);
        else if (rfunc->isArithmeticOperator()
                 || rfunc->isLogicalOperator()
                 || rfunc->isBitwiseOperator())
            typeAsNumber = true;
        else if (rfunc->isComparisonOperator())
            hasComparisonOperator = true;
        else
            writeMethodWrapper(s, overloads);

        if (!rfunc->isConstructor() && !rfunc->isOperatorOverload())
            writeMethodDefinition(md, overloads);
    }

    QString className = cpythonTypeName(metaClass).replace(QRegExp("_Type$"), "");

    // Write methods definition
    s << "static PyMethodDef " << className << "_methods[] = {" << endl;
    s << methodsDefinitions << INDENT << "{0} // Sentinel" << endl << "};" << endl << endl;

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

    s << "extern \"C\"" << endl << '{' << endl << endl;
    writeClassDefinition(s, metaClass);
    s << endl;

    foreach (AbstractMetaEnum* cppEnum, metaClass->enums()) {
        bool hasFlags = cppEnum->typeEntry()->flags();
        if (hasFlags) {
            writeTypeConverterImpl(convImpl, cppEnum->typeEntry()->flags());
            writeFlagsMethods(s, cppEnum);
            writeFlagsNumberMethodsDefinition(s, cppEnum);
            s << endl;
        }

        writeEnumDefinition(s, cppEnum);
        writeTypeConverterImpl(convImpl, cppEnum->typeEntry());

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
    s << "// Converter implementations" << endl;
    s << converterImpl;
    s << "} // namespace Shiboken" << endl << endl;

    // class inject-code native/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Beginning, TypeSystem::NativeCode);
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
    s << " {" << endl;
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::Beginning, TypeSystem::All, func);
    s << INDENT << "// ... middle" << endl;
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::End, TypeSystem::All, func);
    s << '}' << endl << endl;
}

void CppGenerator::writeDestructorNative(QTextStream &s, const AbstractMetaClass *metaClass)
{
    s << wrapperName(metaClass) << "::~" << wrapperName(metaClass) << "()" << endl << '{' << endl;
    s << '}' << endl;
}

void CppGenerator::writeVirtualMethodNative(QTextStream &s, const AbstractMetaFunction* func)
{
    QString returnKeyword = func->type() ? QLatin1String("return ") : QString();
    QString prefix = wrapperName(func->ownerClass()) + "::";
    s << functionSignature(func, prefix, "", Generator::SkipDefaultValues) << endl << "{" << endl;

    Indentation indentation(INDENT);

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = getCodeSnips(func);
        writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::NativeCode, func);
        s << endl;
    }

    if (func->isAbstract() && func->isModifiedRemoved()) {
        s << INDENT << "#error Pure virtual method \"" << func->ownerClass()->name();
        s << "::" << func->minimalSignature();
        s << "\" must be implement but was completely removed on typesystem." << endl;
    } else {
        if (func->allowThread())
            s << INDENT << "// how to say to Python to allow threads?" << endl;

        s << INDENT << "PyObject* method = BindingManager::instance().getOverride(this, \"";
        s << func->name() << "\");" << endl;

        s << INDENT << "if (!method) {" << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT;
            if (func->isAbstract()) {
                s << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
                s << func->ownerClass()->name() << '.' << func->name();
                s << "()' not implemented.\");" << endl;
                s << INDENT << "return";
                if (func->type()) {
                    s << ' ';
                    writeMinimalConstructorCallArguments(s, func->type());
                }
            } else {
                s << "return this->" << func->implementingClass()->qualifiedCppName() << "::";
                writeFunctionCall(s, func);
            }
        }
        s << ';' << endl << INDENT << '}' << endl << endl;

        s << INDENT << "PyObject* args = ";
        if (func->arguments().isEmpty()) {
            s << "PyTuple_New(0);" << endl;
        } else {
            s << "Py_BuildValue(\"(" << getFormatUnitString(func) << ")\"," << endl;
            foreach (const AbstractMetaArgument* arg, func->arguments()) {
                Indentation indentation(INDENT);
                bool convert = arg->type()->isObject()
                                || arg->type()->isQObject()
                                || arg->type()->isValue()
                                || arg->type()->isFlags()
                                || arg->type()->isReference()
                                || (arg->type()->isPrimitive()
                                    && !m_formatUnits.contains(arg->type()->typeEntry()->name()));
                s << INDENT;
                if (convert) {
                    writeToPythonConversion(s, arg->type(), func->ownerClass());
                    s << '(';
                }
                s << arg->argumentName();
                if (convert)
                    s << ")";
                if (arg->argumentIndex() != func->arguments().size() - 1)
                    s << ',';
                s << endl;
            }
            s << INDENT << ");" << endl;
        }
        s << endl;

        s << INDENT << "PyGILState_STATE gil_state = PyGILState_Ensure();" << endl;

        s << INDENT;
        if (!returnKeyword.isEmpty())
            s << "PyObject* method_result = ";
        s << "PyObject_Call(method, args, NULL);" << endl;
        s << INDENT << "PyGILState_Release(gil_state);" << endl << endl;
        s << INDENT << "Py_XDECREF(args);" << endl;
        s << INDENT << "Py_XDECREF(method);" << endl;

        s << endl << INDENT << "// check and set Python error here..." << endl;
    }

    if (func->hasInjectedCode()) {
        s << endl;
        writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::NativeCode, func);
    }

    if (!returnKeyword.isEmpty()) {
        s << INDENT << returnKeyword;
        writeToCppConversion(s, func->type(), func->implementingClass(), "method_result");
        s << ';' << endl;
    }
    s << '}' << endl << endl;
}

void CppGenerator::writeNonVirtualModifiedFunctionNative(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);

    s << getFunctionReturnType(func) << ' ';
    s << func->ownerClass()->name() << '_' << func->originalName() << "_modified(";

    Options options = Options(SkipRemovedArguments) | SkipDefaultValues;
    if (!func->isStatic())
        options |= WriteSelf;

    writeFunctionArguments(s, func, options);
    s << ")" << endl << "{" << endl;

    if (func->isThread())
        s << INDENT << "thread_locker lock;" << endl;

    if (func->allowThread())
        s << INDENT << "py_allow_threads allow_threads;" << endl;

    if (!getCodeSnips(func).isEmpty()) {
        writeCodeSnips(s, getCodeSnips(func), CodeSnip::Beginning, TypeSystem::All, func);
        writeCodeSnips(s, getCodeSnips(func), CodeSnip::End, TypeSystem::All, func);
    } else {
        s << INDENT;
        if (func->type())
            s << "return ";
        if (func->isStatic())
            s << func->declaringClass()->name() << "::";
        else
            s << "self.";
        writeFunctionCall(s, func);
        s << ";" << endl;
    }

    s << '}' << endl << endl;
}

void CppGenerator::writeConstructorWrapper(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    OverloadData overloadData(overloads, this);
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    QString className = cpythonTypeName(rfunc->ownerClass());

    s << "PyObject*" << endl;
    s << cpythonFunctionName(rfunc) << "(PyTypeObject *type, PyObject *args, PyObject *kwds)" << endl;
    s << '{' << endl;

    s << INDENT << "PyObject* self;" << endl;
    s << INDENT << getFunctionReturnType(rfunc) << " cptr;" << endl << endl;

    if (rfunc->ownerClass()->isAbstract()) {
        s << INDENT << "if (type == &" << className << ") {" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError," << endl;
            s << INDENT << INDENT << "\"'" << rfunc->ownerClass()->qualifiedCppName();
            s << "' represents a C++ abstract class and cannot be instanciated\");" << endl;
            s << INDENT << "return 0;" << endl;
        }
        s << INDENT << '}' << endl << endl;
    }

    s << INDENT << "if (!PyType_IsSubtype(type, &" << className << "))" << endl;
    s << INDENT << INDENT << "return 0;" << endl << endl;

    if (overloadData.maxArgs() > 0) {
        s  << endl << INDENT << "int numArgs = ";
        writeArgumentsInitializer(s, overloadData);
    }

    writeOverloadedMethodDecisor(s, &overloadData);
    s << endl;

    s << INDENT << "self = Shiboken::PyBaseWrapper_New(type, &" << className << ", cptr);" << endl;
    s << endl << INDENT << "if (!self) {" << endl;
    {
        Indentation indentation(INDENT);
        s << INDENT << "if (cptr) delete cptr;" << endl;
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << '}' << endl;

    s << endl << INDENT << "return self;" << endl;
    if (overloadData.maxArgs() > 0)
        writeErrorSection(s, overloadData);
    s << '}' << endl << endl;
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
    for (int i = 0; i < ctor->arguments().size(); i++)
        argValues << QLatin1String("0");
    s << metaClass->qualifiedCppName() << '(' << argValues.join(QLatin1String(", ")) << ')';
}

void CppGenerator::writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaType* metaType)
{
    Q_ASSERT(metaType);
    const TypeEntry* type = metaType->typeEntry();

    if (type->isPrimitive() || type->isObject()) {
        s << "0";
    } else if (type->isContainer()){
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
//         QString dumpFile = QString("/tmp/%1_%2.dot").arg(m_packageName).arg(pythonOperatorFunctionName(rfunc)).toLower();
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

    if (overloads.count() == 1 && rfunc->isAbstract()) {
        s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
        s << rfunc->ownerClass()->name() << '.' << rfunc->name();
        s << "()' not implemented.\");" << endl;
        s << INDENT << "return 0;" << endl;
    } else {
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
            // If the wrapped C++ library have no function that steals ownership and
            // deletes the C++ object this check would not be needed.
            // Value type objects are always valid
            if (!rfunc->ownerClass()->typeEntry()->isValue()) {
                s << INDENT << "if (!Shiboken::cppObjectIsValid((Shiboken::PyBaseWrapper*)self)) {\n";
                s << INDENT << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"C++ object is invalid.\");\n";
                s << INDENT << INDENT << "return 0;\n";
                s << INDENT << "}\n";
            }
        }

        if (rfunc->type() && !rfunc->argumentRemoved(0) && !rfunc->isInplaceOperator())
            s << INDENT << "PyObject* " << retvalVariableName() << " = 0;" << endl;

        if (minArgs != maxArgs || maxArgs > 1) {
            s << INDENT << "int numArgs = ";
            if (minArgs == 0 && maxArgs == 1)
                s << "(arg == 0 ? 0 : 1);" << endl;
            else
                writeArgumentsInitializer(s, overloadData);
        }

        writeOverloadedMethodDecisor(s, &overloadData);

        s << endl << INDENT << "if (PyErr_Occurred()";
        if (rfunc->type() && !rfunc->argumentRemoved(0) && !rfunc->isInplaceOperator())
            s << " || !" << retvalVariableName();
        s << ')' << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return 0;" << endl;
        }

        s << endl << INDENT;
        if (rfunc->type() && !rfunc->argumentRemoved(0)) {
            s << "return ";
            if (rfunc->isInplaceOperator())
                s << "self";
            else
                s << retvalVariableName();
//                 writeToPythonConversion(s, rfunc->type(), rfunc->ownerClass(), retvalVariableName());
        } else {
            s << "Py_RETURN_NONE";
        }
        s << ';' << endl;

        if (maxArgs > 0)
            writeErrorSection(s, overloadData);
    }
    s << '}' << endl << endl;
}

void CppGenerator::writeArgumentsInitializer(QTextStream& s, OverloadData& overloadData)
{
    const AbstractMetaFunction* rfunc = overloadData.referenceFunction();
    s << "PyTuple_GET_SIZE(args);" << endl;

    s << INDENT << "PyObject* pyargs[] = {";
    s << QString(overloadData.maxArgs(), '0').split("", QString::SkipEmptyParts).join(", ");
    s << "};" << endl << endl;

    QStringList palist;
    for (int i = 0; i < overloadData.maxArgs(); i++)
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

    s << INDENT << "if (!PyArg_UnpackTuple(args, \"" << funcName << "\", ";
    s << overloadData.minArgs() << ", " << overloadData.maxArgs();
    s  << ", " << pyargs << "))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << endl;
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
    s << INDENT << "PyErr_SetString(PyExc_TypeError, \"'" << funcName;
    s << "()' called with wrong parameters.\");" << endl;
    s << INDENT << "return 0;" << endl;
}

void CppGenerator::writeTypeCheck(QTextStream& s, const OverloadData* overloadData, QString argumentName)
{
    const AbstractMetaType* argType = overloadData->argType();
    AbstractMetaFunctionList implicitConvs = implicitConversions(argType);

    int alternativeNumericTypes = 0;
    foreach (OverloadData* pd, overloadData->overloadDataOnPosition(overloadData->argPos())) {
        if (!pd->argType()->isPrimitive())
            continue;
        if (ShibokenGenerator::isNumber(pd->argType()->typeEntry()))
            alternativeNumericTypes++;
    }

    // This condition trusts that the OverloadData object will arrange for
    // PyInt type to come after the more precise numeric types (e.g. float)
    bool numberType = alternativeNumericTypes == 1 || ShibokenGenerator::isPyInt(argType);

    bool isPairContainer = overloadData->argType()->isContainer()
                            && ((ContainerTypeEntry*)overloadData->argType()->typeEntry())->type()
                                == ContainerTypeEntry::PairContainer;

    if (!implicitConvs.isEmpty())
        s << '(';

    if (isPairContainer)
        s << '(';

    if (argType->typeEntry()->isFlags())
        s << cpythonCheckFunction(((FlagsTypeEntry*) argType->typeEntry())->originator(), true);
    else if (argType->isEnum())
        s << cpythonCheckFunction(argType, false, true);
    else
        s << cpythonCheckFunction(argType, numberType);

    s << '(' << argumentName << ')';

    if (isPairContainer)
        s << " && PySequence_Size(" << argumentName << ") == 2)";

    if (!implicitConvs.isEmpty())
        s << " || " << cpythonIsConvertibleFunction(argType) << '(' << argumentName << "))";
}

void CppGenerator::writeArgumentConversion(QTextStream& s,
                                           const AbstractMetaType* argType,
                                           QString argName, QString pyArgName,
                                           const AbstractMetaClass* context)
{
    QString typeName;
    QString baseTypeName = argType->typeEntry()->name();
    if (argType->typeEntry()->isValue() || argType->typeEntry()->isObject())
        typeName = baseTypeName + '*';
    else
        typeName = translateTypeForWrapperMethod(argType, context);

    if (argType->typeEntry()->isContainer()) {
        if (typeName.startsWith("const "))
            typeName.remove(0, 6);
        if (typeName.endsWith("&"))
            typeName.chop(1);
    }

    bool hasImplicitConversions = !implicitConversions(argType).isEmpty();

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
            const AbstractMetaFunction* func = parentOverloadData->overloads()[0];
            int numRemovedArgs = OverloadData::numberOfRemovedArguments(func);
            writeMethodCall(s, func, func->arguments().size() - numRemovedArgs);
            return;
        }
    }

    s << INDENT;

    // If the next argument has a default value the decisor can perform a method call;
    // it just need to check if the number of arguments received from Python are equal
    // to the number of parameters preceding the argument with the default value.
    if (hasDefaultCall) {
        s << "if (numArgs == " << parentOverloadData->argPos() + 1 << ") {" << endl;
        {
            Indentation indent(INDENT);
            writeMethodCall(s, referenceFunction, parentOverloadData->argPos() + 1);
        }
        s << INDENT << "} else ";
    }

    foreach (OverloadData* overloadData, parentOverloadData->nextOverloadData()) {
        if (maxArgs > 0) {
            bool signatureFound = overloadData->overloads().size() == 1
                                  && !overloadData->hasDefaultValue();

            OverloadData* nextArgWithDefault = 0;
            if (signatureFound) {
                nextArgWithDefault = overloadData->findNextArgWithDefault();
                signatureFound = !nextArgWithDefault;
            }

            const AbstractMetaFunction* func = overloadData->overloads()[0];
            int numRemovedArgs = OverloadData::numberOfRemovedArguments(func);
            QString pyArgName = manyArgs ? QString("pyargs[%1]").arg(overloadData->argPos()) : "arg";

            s << "if (";
            int numArgs = 0;
            if (manyArgs) {
                if (signatureFound) {
                    numArgs = func->arguments().size() - numRemovedArgs;
                    s << "numArgs == " << numArgs << " && ";
                } else if (nextArgWithDefault) {
                    numArgs = nextArgWithDefault->argPos();
                }
            }

            if (referenceFunction->isOperatorOverload()) {
                s << (overloadData->overloads().first()->isReverseOperator() ? "" : "!") << "isReverse && ";
            }
            writeTypeCheck(s, overloadData, pyArgName);

            if (overloadData->overloads().size() == 1) {
                OverloadData* tmp = overloadData;
                for (int i = overloadData->argPos() + 1; i < numArgs; i++) {
                    tmp = tmp->nextOverloadData()[0];
                    s << " && ";
                    writeTypeCheck(s, tmp, QString("pyargs[%1]").arg(i));
                }
            }

            s << ") {" << endl;
            {
                if (!func->isAbstract()) {
                    Indentation indent(INDENT);
                    int maxArgs;
                    if (signatureFound)
                        maxArgs = func->arguments().size() - numRemovedArgs;
                    else if (nextArgWithDefault)
                        maxArgs = nextArgWithDefault->argPos();
                    else
                        maxArgs = overloadData->argPos() + 1;

                    int removed = 0;
                    for (int i = overloadData->argPos(); i < maxArgs; i++) {
                        if (func->argumentRemoved(i + 1))
                            removed++;
                        if (manyArgs)
                            pyArgName = QString("pyargs[%1]").arg(i);
                        writeArgumentConversion(s, func->arguments().at(i + removed),
                                                QString("cpp_arg%1").arg(i), pyArgName,
                                                func->implementingClass());
                    }
                }
            }

            if (nextArgWithDefault) {
                while (overloadData->nextOverloadData()[0] != nextArgWithDefault)
                    overloadData = overloadData->nextOverloadData()[0];
            }
        }

        {
            Indentation indent(INDENT);
            writeOverloadedMethodDecisor(s, overloadData);
        }

        s << INDENT << "} else ";
    }

    if (maxArgs > 0)
        s << "goto " << cpythonFunctionName(referenceFunction) << "_TypeError;" << endl;
}

void CppGenerator::writeMethodCall(QTextStream& s, const AbstractMetaFunction* func, int maxArgs)
{
    s << INDENT << "// " << func->minimalSignature() << (func->isReverseOperator() ? " [reverse operator]": "") << endl;

    if (func->isAbstract()) {
        s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '"
          << func->ownerClass()->name() << '.' << func->name() << "()' not implemented.\");" << endl;
        return;
    }

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = getCodeSnips(func);
        if (injectedCodeUsesCppSelf(func)) {
            s << INDENT << func->ownerClass()->name() << "* cppSelf = ";
            s << cpythonWrapperCPtr(func->ownerClass(), "self") << ';' << endl;
        }
        writeCodeSnips(s, snips, CodeSnip::Beginning, TypeSystem::TargetLangCode, func);
        s << endl;
    }

    if (func->functionType() != AbstractMetaFunction::UserAddedFunction) {
        bool badModifications = false;
        QStringList userArgs;
        if (!func->isCopyConstructor()) {
            int removed = 0;
            for (int i = 0; i < maxArgs + removed; i++) {
                const AbstractMetaArgument* arg = func->arguments()[i];
                if (func->argumentRemoved(i + 1)) {
                    // If some argument with default value is removed from a
                    // method signature, the said value must be explicitly
                    // added to the method call.
                    removed++;
                    if (arg->defaultValueExpression().isEmpty())
                        badModifications = true;
                    else
                        userArgs << arg->defaultValueExpression();
                } else {
                    QString argName = QString("cpp_arg%1").arg(arg->argumentIndex() - removed);
                    if (shouldDereferenceArgumentPointer(arg))
                        argName.prepend('*');
                    userArgs << argName;
                }
            }

            // If any argument's default value was modified the method must be called
            // with this new value whenever the user doesn't pass an explicit value to it.
            // Also, any unmodified default value coming after the last user specified
            // argument and before the modified argument must be splicitly stated.
            QStringList otherArgs;
            bool defaultModified = false;
            for (int i = func->arguments().size() - 1; i >= maxArgs; i--) {
                const AbstractMetaArgument* arg = func->arguments()[i];
                defaultModified = defaultModified || arg->defaultValueExpression() != arg->originalDefaultValueExpression();
                if (defaultModified) {
                    if (arg->defaultValueExpression().isEmpty())
                        badModifications = true;
                    else
                        otherArgs.prepend(arg->defaultValueExpression());
                }
            }

            userArgs += otherArgs;
        }

        bool isCtor = false;
        QString methodCall;
        QTextStream mc(&methodCall);

        // This indentation is here for aesthetical reasons concerning the generated code.
        if (func->type() && !func->isInplaceOperator()) {
            Indentation indent(INDENT);
            mc << endl << INDENT;
        }

        if (badModifications) {
            // When an argument is removed from a method signature and no other
            // means of calling the method is provided (as with code injection)
            // the generator must write a compiler error line stating the situation.
            if (func->injectedCodeSnips(CodeSnip::Any, TypeSystem::TargetLangCode).isEmpty()) {
                s << INDENT << "#error No way to call \"" << func->ownerClass()->name();
                s << "::" << func->minimalSignature();
                s << "\" with the modifications provided on typesystem file" << endl;
            }
        } else if (func->isOperatorOverload()) {
            QString firstArg = QString("(*%1)").arg(cpythonWrapperCPtr(func->ownerClass()));
            QString secondArg("cpp_arg0");
            if (!func->isUnaryOperator() && shouldDereferenceArgumentPointer(func->arguments().at(0))) {
                secondArg.prepend("(*");
                secondArg.append(')');
            }

            if (func->isUnaryOperator())
                std::swap(firstArg, secondArg);

            QString op = func->originalName();
            op = op.right(op.size() - (sizeof("operator")/sizeof(char)-1));

            s << INDENT;
            if (!func->isInplaceOperator())
                s << retvalVariableName() << " = ";

            if (func->isBinaryOperator()) {
                if (func->isReverseOperator())
                    std::swap(firstArg, secondArg);
                mc << firstArg << ' ' << op << ' ' << secondArg;
            } else {
                mc << op << ' ' << secondArg;
            }
        } else if (func->isConstructor() || func->isCopyConstructor()) {
            s << INDENT;
            isCtor = true;
            s << "cptr = new " << wrapperName(func->ownerClass());
            s << '(';
            if (func->isCopyConstructor() && maxArgs == 1)
                s << "*cpp_arg0";
            else
                s << userArgs.join(", ");
            s << ')';
        } else if (!injectedCodeCallsCppFunction(func)) {
            s << INDENT;
            if (func->type())
                s << retvalVariableName() << " = ";
            if (func->ownerClass()) {
                if (!func->isStatic())
                    mc << cpythonWrapperCPtr(func->ownerClass()) << "->";
                mc << func->ownerClass()->name() << "::";
            }
            mc << func->originalName() << '(' << userArgs.join(", ") << ')';
        }

        if (!injectedCodeCallsCppFunction(func)) {
            if (!func->type() || func->isInplaceOperator()) {
                s << methodCall;
            } else if (!isCtor) {
                mc << endl << INDENT;
                writeToPythonConversion(s, func->type(), func->ownerClass(), methodCall);
            }
            s << ';' << endl;
        }
    }

    if (func->hasInjectedCode()) {
        s << endl;
        writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::TargetLangCode, func);
    }
}

bool CppGenerator::supportsSequenceProtocol(const AbstractMetaClass* metaClass)
{
    foreach(QString funcName, m_sequenceProtocol.keys()) {
        if (metaClass->hasFunction(funcName))
            return true;
    }
    return false;
}

void CppGenerator::writeClassDefinition(QTextStream& s, const AbstractMetaClass* metaClass)
{
    QString tp_flags;
    QString tp_new;
    QString tp_dealloc;
    QString tp_as_number('0');
    QString tp_as_sequence('0');
    QString cppClassName = metaClass->qualifiedCppName();
    QString className = cpythonTypeName(metaClass).replace(QRegExp("_Type$"), "");
    QString baseClassName;

    if (metaClass->hasArithmeticOperatorOverload()
        || metaClass->hasLogicalOperatorOverload()
        || metaClass->hasBitwiseOperatorOverload()) {
        tp_as_number = QString("&Py%1_as_number").arg(cppClassName);
    }

    // sequence protocol check
    if (supportsSequenceProtocol(metaClass))
        tp_as_sequence = QString("&Py%1_as_sequence").arg(cppClassName);

    if (metaClass->baseClass())
        baseClassName = QString("&") + cpythonTypeName(metaClass->baseClass()->typeEntry());
    else
        baseClassName = QString("0");

    if (metaClass->isNamespace() || metaClass->hasPrivateDestructor()) {
        tp_flags = "Py_TPFLAGS_HAVE_CLASS";
        tp_dealloc = metaClass->hasPrivateDestructor() ?
                     "(destructor)Shiboken::PyBaseWrapper_Dealloc_PrivateDtor" : "0";
        tp_new = "0";
    } else {
        tp_flags = "Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES";
        tp_dealloc = QString("(destructor)&(Shiboken::PyBaseWrapper_Dealloc< %1 >)").arg(cppClassName);

        AbstractMetaFunctionList ctors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
        tp_new = ctors.isEmpty() ? "0" : className + "_New";
    }

    QString tp_richcompare = QString('0');
    if (metaClass->hasComparisonOperatorOverload())
        tp_richcompare = cpythonBaseName(metaClass->typeEntry()) + "_richcompare";

    // search for special functions
    ShibokenGenerator::clearTpFuncs();
    foreach (AbstractMetaFunction* func, metaClass->functions()) {
        if (m_tpFuncs.contains(func->name()))
            m_tpFuncs[func->name()] = cpythonFunctionName(func);
    }

    s << "// Class Definition -----------------------------------------------" << endl;

    s << "PyTypeObject " << className + "_Type" << " = {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&PyType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << cppClassName << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::PyBaseWrapper)," << endl;
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
    s << INDENT << "/*tp_getattro*/         0," << endl;
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
    s << INDENT << "/*tp_getset*/           0," << endl;
    s << INDENT << "/*tp_base*/             " << baseClassName << ',' << endl;
    s << INDENT << "/*tp_dict*/             0," << endl;
    s << INDENT << "/*tp_descr_get*/        0," << endl;
    s << INDENT << "/*tp_descr_set*/        0," << endl;
    s << INDENT << "/*tp_dictoffset*/       0," << endl;
    s << INDENT << "/*tp_init*/             0," << endl;
    s << INDENT << "/*tp_alloc*/            PyType_GenericAlloc," << endl;
    s << INDENT << "/*tp_new*/              " << tp_new << ',' << endl;
    s << INDENT << "/*tp_free*/             PyObject_Del," << endl;
    s << INDENT << "/*tp_is_gc*/            0," << endl;
    s << INDENT << "/*tp_bases*/            0," << endl;
    s << INDENT << "/*tp_mro*/              0," << endl;
    s << INDENT << "/*tp_cache*/            0," << endl;
    s << INDENT << "/*tp_subclasses*/       0," << endl;
    s << INDENT << "/*tp_weaklist*/         0" << endl;
    s << "};" << endl << endl;
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
        s << funcRetVal << ' ' << funcName << '(' << funcArgs << ')'
          << "\n{\n"
          << INDENT << "if (!Shiboken::cppObjectIsValid((Shiboken::PyBaseWrapper*)self)) {\n"
          << INDENT << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"C++ object is invalid.\");\n"
          << INDENT << INDENT << "return 0;\n"
          << INDENT << "}\n"
          << INDENT << func->ownerClass()->name() << "* cppSelf = " << cpythonWrapperCPtr(func->ownerClass(), "self") << ";\n"
          << INDENT << "(void)cppSelf; // avoid warnings about unused variables\n";
          writeCodeSnips(s, snips,CodeSnip::Any, TypeSystem::TargetLangCode, func);
        s << "}\n\n";
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
      << INDENT << "/*sq_slice*/ 0,\n"
      << INDENT << "/*sq_ass_item*/ " << funcs["__setitem__"] << ",\n"
      << INDENT << "/*sq_ass_slice*/ 0,\n"
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

    s << "static PyNumberMethods Py" << metaClass->qualifiedCppName();
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


void CppGenerator::writeRichCompareFunction(QTextStream& s, const AbstractMetaClass* metaClass)
{
    s << "static PyObject*" << endl;
    s << cpythonBaseName(metaClass->typeEntry()) << "_richcompare(PyObject* self, PyObject* other, int op)" << endl;
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
                    if (type->typeEntry()->isValue())
                        s << type->name() << '*';
                    else
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

            s << " else goto Py" << metaClass->name() << "_RichComparison_TypeError;" << endl;
            s << endl;

            s << INDENT << "break;" << endl;
        }
        s << INDENT << "default:" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \"operator not implemented.\");" << endl;
            s << INDENT << "return 0;" << endl;
        }
    }
    s << INDENT << '}' << endl << endl;

    s << INDENT << "if (result)" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "Py_RETURN_TRUE;" << endl;
    }
    s << INDENT << "Py_RETURN_FALSE;" << endl << endl;
    s << INDENT << "Py" << metaClass->name() << "_RichComparison_TypeError:" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_TypeError, \"operator called with wrong parameters.\");" << endl;
        s << INDENT << "return 0;" << endl;
    }
    s << '}' << endl << endl;
}

void CppGenerator::writeMethodDefinition(QTextStream& s, const AbstractMetaFunctionList overloads)
{
    Q_ASSERT(!overloads.isEmpty());
    QPair<int, int> minMax = OverloadData::getMinMaxArguments(overloads);
    const AbstractMetaFunction* func = overloads[0];
    if (m_tpFuncs.contains(func->name()))
        return;

    s << INDENT << "{\"" << func->name() << "\", (PyCFunction)";
    s << cpythonFunctionName(func) << ", ";

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
    if (func->ownerClass() && func->isStatic())
        s << "|METH_STATIC";
    s << "}," << endl;
}

void CppGenerator::writeEnumInitialization(QTextStream& s, const AbstractMetaEnum* cppEnum)
{
    QString cpythonName = cpythonEnumName(cppEnum);
    QString addFunction;
    if (cppEnum->enclosingClass()) {
        addFunction = QString("PyDict_SetItemString(Py")
                      + cppEnum->enclosingClass()->name()
                      + "_Type.tp_dict,";
    } else {
        addFunction = "PyModule_AddObject(module,";
    }

    s << INDENT << "// init enum class: " << cppEnum->name() << endl;

    s << INDENT << "if (PyType_Ready(&" << cpythonName << "_Type) < 0)" << endl;
    s << INDENT << INDENT << "return;" << endl;

    s << INDENT << "Py_INCREF(&" << cpythonName << "_Type);" << endl;

    s << INDENT << addFunction << endl;
    s << INDENT << INDENT << INDENT << '\"' << cppEnum->name() << "\",";
    s << "((PyObject*)&" << cpythonName << "_Type));" << endl << endl;

    FlagsTypeEntry* flags = cppEnum->typeEntry()->flags();
    if (flags) {
        QString flagsName = cpythonFlagsName(flags);
        s << INDENT << "// init flags class: " << flags->name() << endl;

        s << INDENT << "if (PyType_Ready(&" << flagsName << "_Type) < 0)" << endl;
        s << INDENT << INDENT << "return;" << endl;

        s << INDENT << "Py_INCREF(&" << flagsName << "_Type);" << endl;

        s << INDENT << addFunction << endl;
        s << INDENT << INDENT << INDENT << '\"' << flags->flagsName() << "\",";
        s << "((PyObject*)&" << flagsName << "_Type));" << endl << endl;
    }


    foreach (const AbstractMetaEnumValue* enumValue, cppEnum->values()) {
        if (cppEnum->typeEntry()->isEnumValueRejected(enumValue->name()))
            continue;

        s << INDENT << "enum_item = Shiboken::PyEnumObject_New(&";
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
    }
    s << endl;
}

void CppGenerator::writeFlagsNewMethod(QTextStream& s, const FlagsTypeEntry* cppFlags)
{
    QString cpythonName = cpythonFlagsName(cppFlags);
    s << "static PyObject*" << endl;
    s << cpythonName << "_New(PyTypeObject *type, PyObject *args, PyObject *kwds)" << endl;
    s << '{' << endl;
    s << INDENT << "if (!PyType_IsSubtype(type, &" << cpythonName << "_Type))" << endl;
    s << INDENT << INDENT << "return 0;" << endl << endl;
    s << INDENT << "int item_value;" << endl;
    s << INDENT << "if (!PyArg_ParseTuple(args, \"i:__new__\", &item_value))" << endl;
    {
        Indentation indent(INDENT);
        s << INDENT << "return 0;" << endl;
    }
    s << INDENT << "PyObject* self = Shiboken::PyEnumObject_New(type, item_value);" << endl << endl;
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
    s << INDENT << "{const_cast<char*>(\"name\"), (getter)Shiboken::PyEnumObject_name}," << endl;
    s << INDENT << "{0}  // Sentinel" << endl;
    s << "};" << endl << endl;

    s << "PyTypeObject " << cpythonName << "_Type = {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&PyType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << cppEnum->name() << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::PyEnumObject)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          0," << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             Shiboken::PyEnumObject_repr," << endl;
    s << INDENT << "/*tp_as_number*/        " << tp_as_number << ',' << endl;
    s << INDENT << "/*tp_as_sequence*/      0," << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             0," << endl;
    s << INDENT << "/*tp_call*/             0," << endl;
    s << INDENT << "/*tp_str*/              Shiboken::PyEnumObject_repr," << endl;
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
    s << INDENT << "/*tp_new*/              Shiboken::PyEnumObject_NonExtensibleNew," << endl;
    s << INDENT << "/*tp_free*/             0," << endl;
    s << INDENT << "/*tp_is_gc*/            0," << endl;
    s << INDENT << "/*tp_bases*/            0," << endl;
    s << INDENT << "/*tp_mro*/              0," << endl;
    s << INDENT << "/*tp_cache*/            0," << endl;
    s << INDENT << "/*tp_subclasses*/       0," << endl;
    s << INDENT << "/*tp_weaklist*/         0" << endl;
    s << "};" << endl << endl;
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

    QString newFunc = QString("(newfunc)") + cpythonName + "_New";
    writeFlagsNewMethod(s, flagsEntry);
    s << endl;

    s << "PyTypeObject " << cpythonName << "_Type = {" << endl;
    s << INDENT << "PyObject_HEAD_INIT(&PyType_Type)" << endl;
    s << INDENT << "/*ob_size*/             0," << endl;
    s << INDENT << "/*tp_name*/             \"" << flagsEntry->flagsName() << "\"," << endl;
    s << INDENT << "/*tp_basicsize*/        sizeof(Shiboken::PyEnumObject)," << endl;
    s << INDENT << "/*tp_itemsize*/         0," << endl;
    s << INDENT << "/*tp_dealloc*/          0," << endl;
    s << INDENT << "/*tp_print*/            0," << endl;
    s << INDENT << "/*tp_getattr*/          0," << endl;
    s << INDENT << "/*tp_setattr*/          0," << endl;
    s << INDENT << "/*tp_compare*/          0," << endl;
    s << INDENT << "/*tp_repr*/             Shiboken::PyEnumObject_repr," << endl;
    s << INDENT << "/*tp_as_number*/        0," << endl;
    s << INDENT << "/*tp_as_sequence*/      0," << endl;
    s << INDENT << "/*tp_as_mapping*/       0," << endl;
    s << INDENT << "/*tp_hash*/             0," << endl;
    s << INDENT << "/*tp_call*/             0," << endl;
    s << INDENT << "/*tp_str*/              Shiboken::PyEnumObject_repr," << endl;
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
        s << INDENT << "((" << flagsEntry->originalName() << ") ((PyEnumObject*)self)->ob_ival)" << endl;
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

    s << INDENT << "((" << flagsEntry->originalName() << ") ((PyEnumObject*)self)->ob_ival) " << cppOpName << endl;
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
    s << "init_" << metaClass->name().toLower() << "(PyObject *module)" << endl;
    s << '{' << endl;

    // class inject-code target/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::Beginning, TypeSystem::TargetLangCode);
        s << endl;
    }

    // Multiple inheritance
    if (metaClass->baseClassNames().size() > 1) {
        s << INDENT << pyTypeName << ".tp_bases = PyTuple_Pack(";
        s << metaClass->baseClassNames().size();
        s << ',' << endl;
        QStringList bases;
        foreach (QString baseName, metaClass->baseClassNames()) {
            const AbstractMetaClass* base = classes().findClass(baseName);
            bases << QString("&%1").arg(cpythonTypeName(base->typeEntry()));
        }
        Indentation indent(INDENT);
        s << INDENT << bases.join(", ") << ");" << endl << endl;
    }

    s << INDENT << "if (PyType_Ready(&" << pyTypeName << ") < 0)" << endl;
    s << INDENT << INDENT << "return;" << endl << endl;
    s << INDENT << "Py_INCREF(&" << pyTypeName << ");" << endl;
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
        writeCodeSnips(s, metaClass->typeEntry()->codeSnips(), CodeSnip::End, TypeSystem::TargetLangCode);
    }

    s << '}' << endl << endl;
}

void CppGenerator::writeTypeConverterImpl(QTextStream& s, const TypeEntry* type)
{
    if (type->hasConversionRule())
        return;

    QString pyTypeName = cpythonTypeName(type);

    const AbstractMetaClass* metaClass = classes().findClass(type->name());
    bool isAbstractOrObjectType = (metaClass &&  metaClass->isAbstract()) || type->isObject();

    // Write Converter<T>::createWrapper function
    s << "PyObject* Converter<" << type->name() << (isAbstractOrObjectType ? "*" : "");
    s << " >::createWrapper(";
    QString convArg = type->name();
    if (!type->isEnum() && !type->isFlags()) {
        convArg.prepend("const ");
        convArg.append('*');
    }
    s << convArg << " cppobj)" << endl;

    s << '{' << endl;
    s << INDENT << "return " << "Shiboken::";
    if (type->isObject() || type->isValue()) {
        s << "PyBaseWrapper_New(&" << pyTypeName << ", &" << pyTypeName << ',';
    } else {
        // Type is enum or flag
        s << "PyEnumObject_New(&" << pyTypeName << ", (long)";
    }
    s << " cppobj);" << endl;
    s << '}' << endl << endl;

    AbstractMetaFunctionList implicitConvs = implicitConversions(type);
    bool hasImplicitConversions = !implicitConvs.isEmpty();

    if (hasImplicitConversions) {
        // Write Converter<T>::isConvertible
        s << "bool Converter<" << type->name() << " >::isConvertible(PyObject* pyobj)" << endl;
        s << '{' << endl;
        s << INDENT << "return ";
        bool isFirst = true;
        foreach (const AbstractMetaFunction* ctor, implicitConvs) {
            Indentation indent(INDENT);
            if (isFirst)
                isFirst = false;
            else
                s << endl << INDENT << " || ";
            s << cpythonCheckFunction(ctor->arguments().first()->type());
            s << "(pyobj)";
        }
        s << ';' << endl;
        s << '}' << endl << endl;
    }

    if (!type->isValue())
        return;

    // Write Converter<T>::toPython function
    s << "PyObject* Converter<" << type->name() << " >::toPython(const ";
    s << type->name() << "& cppobj)" << endl;
    s << '{' << endl;
    s << INDENT << "return Converter<" << type->name() << " >::createWrapper(new ";
    s << type->name() << "(cppobj));" << endl;
    s << '}' << endl << endl;

    if (!hasImplicitConversions)
        return;

    // Write Converter<T>::toCpp function
    s << type->name() << " Converter<" << type->name() << " >::toCpp(PyObject* pyobj)" << endl;
    s << '{' << endl << INDENT;

    bool firstImplicitIf = true;
    foreach (const AbstractMetaFunction* ctor, implicitConvs) {
        if (ctor->isModifiedRemoved())
            continue;

        const AbstractMetaType* argType = ctor->arguments().first()->type();
        if (firstImplicitIf)
            firstImplicitIf = false;
        else
            s << INDENT << "else ";
        s << "if (" << cpythonCheckFunction(argType) << "(pyobj))" << endl;
        {
            Indentation indent(INDENT);
            s << INDENT << "return " << type->name() << '(';
            writeBaseConversion(s, argType, 0);
            s << "toCpp(pyobj));" << endl;
        }
    }

    s << INDENT << "return *Converter<" << type->name() << "* >::toCpp(pyobj);" << endl;
    s << '}' << endl << endl;

    // Write Converter<T>::copyCppObject function
    s << type->name() << "* Converter<" << type->name();
    s << " >::copyCppObject(const " << type->name() << "& cppobj)" << endl;
    s << '{' << endl;
    s << INDENT << "return new " << type->name() << "(cppobj);" << endl;
    s << '}' << endl << endl;
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

    foreach (AbstractMetaFunctionList globalOverloads, filterGroupedFunctions()) {
        AbstractMetaFunctionList overloads;
        foreach (AbstractMetaFunction* func, globalOverloads) {
            // TODO: this is an ugly hack to avoid binding global
            // functions from outside the library beign processed.
            // The decent solution is to expand API Extractor so
            // that it support global function declarations on
            // type system files.
            QString incFile = func->includeFile();
            QRegExp regex("\\b(?:lib)?" + moduleName() + "\\b");
            if (regex.indexIn(incFile) == -1)
                continue;

            if (!func->isModifiedRemoved())
                overloads.append(func);
        }

        if (overloads.isEmpty())
            continue;

        includes << overloads.first()->includeFile();

        writeMethodWrapper(s_globalFunctionImpl, overloads);
        writeMethodDefinition(s_globalFunctionDef, overloads);
    }

    foreach (const AbstractMetaClass *cls, classes()) {
        if (!shouldGenerate(cls) || cls->enclosingClass())
            continue;

        if (m_packageName.isEmpty())
            m_packageName = cls->package();

        s_classInitDecl << "extern \"C\" PyAPI_FUNC(void) init_"
                        << cls->name().toLower() << "(PyObject* module);" << endl;

        QString defineStr = "init_" + cls->name().toLower() + "(module);";
        if (cls->isNamespace())
            s_namespaceDefines << INDENT << defineStr << endl;
        else
            s_classPythonDefines << INDENT << defineStr << endl;
    }

    QString moduleFileName(outputDirectory() + "/" + subDirectoryForPackage(m_packageName));
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
        s << "#include \"" << moduleName().toLower() << "_python.h\"" << endl << endl;
        foreach (const QString& include, includes)
            s << "#include \"" << include << '\"' << endl;
        s << endl;

        TypeSystemTypeEntry* moduleEntry = reinterpret_cast<TypeSystemTypeEntry*>(TypeDatabase::instance()->findType(m_packageName));
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
                writeTypeConverterImpl(convImpl, cppEnum->typeEntry());
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
            s << INDENT << "if (PyImport_ImportModule(\"" << requiredModule << "\") == NULL) {" << endl;
            s << INDENT << INDENT << "PyErr_SetString(PyExc_ImportError," << "\"could not import ";
            s << requiredModule << "\");" << endl << INDENT << INDENT << "return;" << endl;
            s << INDENT << "}" << endl << endl;
        }

        s << INDENT << "PyObject* module = Py_InitModule(\""  << moduleName() << "\", ";
        s << moduleName() << "_methods);" << endl << endl;

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

        s << INDENT << "if (PyErr_Occurred())" << endl;
        s << INDENT << INDENT << "Py_FatalError(\"can't initialize module ";
        s << moduleName() << "\");" << endl << '}' << endl << endl;
        s << "} // extern \"C\"" << endl << endl;

        // module inject-code native/end
        if (!snips.isEmpty()) {
            writeCodeSnips(s, snips, CodeSnip::End, TypeSystem::NativeCode);
            s << endl;
        }
    }
}
