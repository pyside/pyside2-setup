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
#include "cppgenerator.h"
#include <apiextractor/reporthandler.h>
#include <apiextractor/fileout.h>
#include <apiextractor/abstractmetalang.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QListIterator>

static Indentor INDENT;

// utiliy functions
inline QString getMethodPointerString(const AbstractMetaFunction* func)
{
    QString className;
    if (!func->declaringClass()->isAbstract())
        className = func->declaringClass()->qualifiedCppName();
    else
        className = func->ownerClass()->qualifiedCppName();

    return '&' + className + "::" + func->originalName();
}

static QString nameForModifiedCtorFunction(const AbstractMetaFunction* func) {
    QString res = func->ownerClass()->name().toLower().replace("::", "_");
    res += "_constructor";
    foreach (AbstractMetaArgument* arg, func->arguments()) {
        res += '_';
        res += arg->type()->name().toLower();
    }
    return res;
}

static QString createStaticFunctionName(const AbstractMetaFunction* func)
{
    QString funcName;
    QString originalName(func->name());


    funcName = func->ownerClass()->name().toLower();

    //remove initial 'Q'
    if (funcName.startsWith('q'))
        funcName = funcName.remove(0, 1);

    //upercase first letter
    funcName += originalName[0].toUpper() + originalName.mid(1);

    return funcName;
}

QString CppGenerator::fileNameForClass(const AbstractMetaClass* cppClass) const
{
    return getWrapperName(cppClass) + QLatin1String(".cpp");
}

QString CppGenerator::getFuncTypedefName(const AbstractMetaFunction* func) const
{
    return func->name() + QLatin1String("_type");
}

void CppGenerator::writeConstructorInitialization(QTextStream &s, const AbstractMetaFunction *function)
{
    QStringList nonOpts;
    QStringList opts;

    Options options = Options(SkipName) | SkipDefaultValues;
    foreach (AbstractMetaArgument *arg, function->arguments()) {
        QString argType = argumentString(function, arg, options);
        if (arg->defaultValueExpression().isEmpty())
            nonOpts << argType;
        else
            opts << argType;
    }

    bool hasModifications = function->allowThread() || function->hasInjectedCode();

    if (hasModifications) {
        s << "\"__init__\", python::make_constructor("
          << nameForModifiedCtorFunction(function);
    } else {
        s << "python::init< ";

        if (nonOpts.size() > 0)
            s << nonOpts.join(", ");

        if (opts.size() > 0) {
            if (nonOpts.size() > 0)
                s << ", ";

            s << "python::optional< " << opts.join(",") << " > ";
        }

        s << " > ()";
    }

    QString callPolicy = getFunctionCallPolicy(function);
    QString parentType;
    const AbstractMetaClass *cppClass = function->ownerClass();
    uint closePolicy = 0;
    bool hasPolicy = false;

    if (
        !hasModifications &&
        (!cppClass->isPolymorphic() || cppClass->hasPrivateDestructor() || cppClass->isNamespace())
       ) {
        closePolicy++;
        hasPolicy = true;
        s << "[ PySide::register_wrapper_object< "
          << function->ownerClass()->qualifiedCppName();
    }

    if (callPolicy.isEmpty()) {
        int parentIndex = -1;
        //try find for parent arg to create callPolicy
        foreach (AbstractMetaArgument *arg, function->arguments()) {
            if (arg->argumentName() == "parent") {
                parentIndex = arg->argumentIndex();
                parentType = translateType(arg->type(), function->ownerClass(),
                                           Options(ExcludeConst) | ExcludeReference).replace("*", "");
                break;
            }
        }
        if (parentIndex != -1) {
            if (!closePolicy)
                s << (hasModifications ? ", " : "[ ");
            else
                s << ", ";

            s << "parent_policy_add< " << parentIndex + 2 << ", 1, "
              << parentType << " , " << function->ownerClass()->qualifiedCppName();

            hasPolicy = true;
            closePolicy++;
        }
    } else {
        if (!closePolicy)
            s << (hasModifications ? ", " : "[ ");
        else
            s << ", ";

        if (callPolicy.endsWith("()"))
            callPolicy = callPolicy.remove(callPolicy.size() - 2, 2);

        s << callPolicy;
        hasPolicy = true;
    }

    while(closePolicy) {
        s << " > ";
        closePolicy--;
    }

    if (hasModifications)
        s << ')';
    else if (hasPolicy)
        s << "() ]";
}

QString CppGenerator::getFunctionReturnType(const AbstractMetaFunction* func)
{
    QString modifiedReturnType = QString(func->typeReplaced(0));

    return modifiedReturnType.isNull() ? translateType(func->type(), func->implementingClass()) : modifiedReturnType;
}

QString CppGenerator::writeFunctionCast(QTextStream &s,
                                        const AbstractMetaFunction* func,
                                        const QString& castNameSuffix,
                                        const QString& className)
{
    QString castName = getFuncTypedefName(func) + castNameSuffix;
    const AbstractMetaClass* cppClass = func->ownerClass();
    bool isWrapped = !func->isVirtual() &&
                     (func->hasInjectedCode() || func->isThread() || func->allowThread());
    bool isVirtualMethodDefault = castNameSuffix == "_default";

    s << INDENT << "typedef ";
    s << getFunctionReturnType(func);
    s << " (";
    if (cppClass && !func->isStatic() && func->ownerClass() && !isVirtualMethodDefault) {
        if (!isWrapped) {
            // pointer to a class method
            if (!className.isEmpty())
                s << className;
            else if (func->isVirtual() && !func->declaringClass()->isAbstract())
                s << func->declaringClass()->qualifiedCppName();
            else
                s << cppClass->qualifiedCppName();

            s << "::";
        }
    }

    s << '*' << castName << ") (";
    if (isVirtualMethodDefault) {
        if (func->isConstant())
            s << "const ";

        s << func->implementingClass()->qualifiedCppName() << "&";
        if (func->arguments().size() > 0)
            s << ", ";
    }
    Options options = Options(SkipName) | SkipDefaultValues | SkipRemovedArguments;
    if (isWrapped && !func->isStatic())
        options |= WriteSelf;

    writeFunctionArguments(s, func, options);
    s << ')';

    if (func->isConstant() && !isWrapped && !isVirtualMethodDefault)
        s << " const";

    s << ';' << endl;

    return castName;
}

QString CppGenerator::verifyDefaultReturnPolicy(const AbstractMetaFunction *cppFunction, const QString& callPolicy)
{
    AbstractMetaType *type = cppFunction->type();

    //If return type replaced, the return policy need be set manually.
    if (!type || !cppFunction->typeReplaced(0).isEmpty()) {
        return QString();
    }

    QString returnPolicy;

    if (type->isReference()) {
        QString detail;
        if (type->isConstant()) {
            detail = "copy_const_reference";
        } else {
            detail = "copy_non_const_reference";
        }

        returnPolicy = "python::return_value_policy<python::" + detail;
        if (!callPolicy.isEmpty())
            returnPolicy += ", " + callPolicy;
        returnPolicy += " >()";
    } else if (type->isQObject() || type->isObject() || type->isValuePointer()) {
        bool cppOwnership = type->isConstant();
        if (cppFunction->isStatic() || cppOwnership) {
            returnPolicy = QString("python::return_value_policy<PySide::return_ptr_object<")
                         + (cppOwnership ? "true" : "false") + QString("> >()");
        } else {
            returnPolicy = QString("PySide::return_object<1, 0, %1, %2 %3 %4 >()")
                            .arg(getArgumentType(cppFunction->ownerClass(), cppFunction, -1))
                            .arg(getArgumentType(cppFunction->ownerClass(), cppFunction, 0))
                            .arg(callPolicy.isEmpty() ? "" : ",")
                            .arg(callPolicy);
        }
    } else if (!callPolicy.isEmpty()) {
        returnPolicy = callPolicy + "()";
    }

    return returnPolicy;
}

static int boost_parent_policy_index(int i, const AbstractMetaFunction* func = 0)
{
    if (func && func->isStatic())
        return i;

    if (i == -1)
        return 1;
    else if (i > 0)
        return i + 1;
    else
        return i;
}

QString CppGenerator::getArgumentType(const AbstractMetaClass *cppClass, const AbstractMetaFunction *func, int idx)
{
    QString retval;
    if (idx == -1) {
        retval = cppClass->qualifiedCppName();
    } else if (idx == 0 && func->type()) {
        retval = translateType(func->type(), cppClass,
                             Options(Generator::ExcludeConst) | Generator::ExcludeReference);
    } else if (idx > 0) {
        retval = argumentString(func, func->arguments()[idx-1],
                              Options(SkipDefaultValues) | ExcludeConst |
                              ExcludeReference | SkipName);
    }

    retval = retval.trimmed();
    if (retval.endsWith('*'))
        retval.chop(1);
    return retval;
}

QString CppGenerator::getFunctionCallPolicy(const AbstractMetaFunction *func)
{
    QString callPolicy;
    QStringList callPolicies;
    bool returnChild = false;
    const AbstractMetaClass* cppClass = func->implementingClass();

    const int numArgs = func->arguments().count();

    if (func->type() && (func->type()->name() == "HANDLE")) {
        return "python::return_value_policy<PySide::return_QHANDLE>()";
    }

    for (int i = -1; i <= numArgs; ++i) {
        ArgumentOwner ao = func->argumentOwner(cppClass, i);
        //Parent Policy
        if ((ao.index != -2) && (ao.index != i)) {
            switch (ao.action) {
                case ArgumentOwner::Add:
                    if (!i) {
                        callPolicy = "return_object<";
                        returnChild = true;
                    } else {
                        callPolicy = "parent_policy_add<";
                    }
                    break;
                case ArgumentOwner::Remove:
                    callPolicy = "parent_policy_remove<";
                    break;
                default:
                    continue;
            }

            callPolicy += QString("%1, %2, %3, %4")
                          .arg(boost_parent_policy_index(ao.index, func))
                          .arg(boost_parent_policy_index(i, func))
                          .arg(getArgumentType(cppClass, func, ao.index))
                          .arg(getArgumentType(cppClass, func, i));

            callPolicies << callPolicy;
        } else if (i) { //only function args ignore return value
            //Ownership policy
            bool changeOwnership = false;
            bool releaseOwnership = false;
            TypeSystem::Ownership owner = func->ownership(cppClass,
                                                          TypeSystem::TargetLangCode, i);

            switch(owner)
            {
                case TypeSystem::CppOwnership:
                    releaseOwnership = true;
                case TypeSystem::TargetLangOwnership:
                    changeOwnership = true;
                    break;
                default:
                    changeOwnership = false;
            }

            if (changeOwnership)
            {
                QString ownershipPolicy = QString("transfer_ownership<%1, %2, %3")
                                          .arg(boost_parent_policy_index(i, func))
                                          .arg(releaseOwnership ? "true" : "false")
                                          .arg(getArgumentType(cppClass, func, i));
                callPolicies << ownershipPolicy;
            }
        }
    }

    if (callPolicies.size() > 0) {
        callPolicy = callPolicies.join(", ");
        for (int i = 0; i < callPolicies.count(); ++i)
            callPolicy += " >";
    }

    QString returnPolicy;

    //return value
    bool cppOwnership = false;

    if (!returnChild) {
        switch (func->ownership(cppClass, TypeSystem::TargetLangCode, 0))
        {
            case TypeSystem::CppOwnership:
                cppOwnership = true;
            case TypeSystem::TargetLangOwnership:
            {
                QString cppOwnershipFlag = (cppOwnership ? "true" : "false");
                returnPolicy = "python::return_value_policy< PySide::return_ptr_object<" + cppOwnershipFlag + "> ";
                if (!callPolicy.isEmpty())
                    returnPolicy += ", " + callPolicy;
                returnPolicy += " >()";
                break;
            }
            default:
                returnPolicy = verifyDefaultReturnPolicy(func, callPolicy);
                break;
        }
    }

    //return policy
    if (func->shouldReturnThisObject())
        return "python::return_self< " + callPolicy + " >()";
    else if (!returnPolicy.isEmpty())
        return returnPolicy;
    else if (!callPolicy.isEmpty())
        return callPolicy + "()";

    return QString();
}

/*!\internal
    Function used to write the enum boost code on the buffer
    \param s the output buffer
    \param cpp_enum the pointer to metaenum information to be translated to boost
*/
void CppGenerator::writeEnum(QTextStream &s,
                             const AbstractMetaEnum *cppEnum,
                             const QString &nameSpace)
{
    s << INDENT << "python::enum_<" << nameSpace << cppEnum->name();
    s << ">(\"" << cppEnum->name() << "\")" << endl;
    const AbstractMetaEnumValueList &values = cppEnum->values();
    EnumTypeEntry *ete = cppEnum->typeEntry();

    foreach (const AbstractMetaEnumValue* enumValue, values) {
        Indentation indent(INDENT);
        if (ete->isEnumValueRejected(enumValue->name()))
            continue;

        s << INDENT << ".value(\"" << enumValue->name() << "\", ";
        s << nameSpace << enumValue->name() << ")" << endl;
    }

    //Export values to current scope
    s << INDENT << INDENT << ".export_values()" << endl;
    s << INDENT << ";" << endl << endl;

    FlagsTypeEntry* flagsEntry = cppEnum->typeEntry()->flags();

    if (flagsEntry) {
        s << INDENT << "PySide::declare_" << (cppEnum->typeEntry()->forceInteger() ?  "int_" : "") << "qflags< "
        << flagsEntry->originalName() << " >(\"" << flagsEntry->flagsName() << "\");" << endl;
    }

    //register enum in typemanager
    s << INDENT
    << "type_manager::instance().register_native_type<int>(\""
    <<  nameSpace << cppEnum->name() << "\");\n\n";
}

void CppGenerator::writeEnums(QTextStream &s, const AbstractMetaClass *cppClass, bool useNamespace)
{
    AbstractMetaEnumList enums = cppClass->enums();
    if (!enums.size())
        return;

    s << INDENT << "// Enums" << endl;
    QString name_space;
    if (useNamespace || !cppClass->isPolymorphic() || cppClass->hasPrivateDestructor())
        name_space = cppClass->qualifiedCppName() + "::";

    foreach (AbstractMetaEnum *cpp_enum, enums)
        writeEnum(s, cpp_enum, name_space);
}

void CppGenerator::writeImplicitlyConversion(QTextStream &s, const AbstractMetaClass *cppClass)
{
#if 0
    if (cppClass->isNamespace())
        return;
    s << endl << "// Implicitly conversions" << endl;
    QStringList interfaces = getBaseClasses(cppClass);

    if (!interfaces.size()) {
        s << INDENT << "python::implicitly_convertible< " << endl;
        s << INDENT << INDENT << "std::auto_ptr< " << getWrapperName(cppClass->name()) << " >," << endl;
        s << INDENT << INDENT << "std::auto_ptr< " << cppClass->name() << " > >();" << endl;
    } else {
        foreach (QString base_class, interfaces) {
            s << INDENT << "python::implicitly_convertible< " << endl;
            s << INDENT << INDENT << "std::auto_ptr< " << cppClass->name() << " >," << endl;
            s << INDENT << INDENT << "std::auto_ptr< " << base_class << " > >();" << endl;
        }
    }
#endif
}


void CppGenerator::writeDestructor(QTextStream &s, const AbstractMetaClass *cppClass)
{
    Indentation indentation(INDENT);
    QString wrapperName = getWrapperName(cppClass);
    s << wrapperName << "::~" << wrapperName << "()" << endl << "{" << endl
    << INDENT << "PySide::qptr_base::invalidate(this);" << endl << "}" << endl;
}

/*!
    Function used to write the class generated boost code on the buffer
    \param s the output buffer
    \param cppClass the pointer to metaclass information
*/
void CppGenerator::generateClass(QTextStream &s, const AbstractMetaClass *cppClass)
{
    ReportHandler::debugSparse("Generating wrapper implementation for " + cppClass->fullName());

    // write license comment
    s << licenseComment() << endl;

    QString localStr, globalStr;
    QTextStream includesLocal(&localStr);
    QTextStream includesGlobal(&globalStr);

    bool canCreateWrapper = canCreateWrapperFor(cppClass);

    QList<Include> includes = cppClass->typeEntry()->extraIncludes();
    qSort(includes.begin(), includes.end());

    foreach (Include inc, includes) {
        if (inc.type == Include::IncludePath)
            includesGlobal << inc.toString() << endl;
        else
            includesLocal << inc.toString() << endl;
    }

    //workaround to access protected functions
    s << "//workaround to access protected functions" << endl;
    s << "#define protected public" << endl;

    s << "//Base Class" << endl;
    if (cppClass->typeEntry()->include().isValid())
        s << cppClass->typeEntry()->include().toString() << endl << endl;

    s << "//Extra includes [global]" << endl;
    s << globalStr << endl;

    s << "#undef protected" << endl;
    s << "//Base include" << endl;
    s << "#include \"pyside.hpp\"" << endl;
    s << "#include \"" << getWrapperName(cppClass) << ".hpp\"" << endl;
    foreach (AbstractMetaClass* innerClass, cppClass->innerClasses()) {
        if (shouldGenerate(innerClass))
            s << "#include \"" << getWrapperName(innerClass) << ".hpp\"" << endl;
    }
    s << endl << "//Extra includes [local]" << endl;
    s << localStr << endl;

    s << endl << "using namespace boost;" << endl;
    s << "using namespace PySide;" << endl;
    s << endl;

    if (cppClass->typeEntry()->typeFlags() & ComplexTypeEntry::Deprecated)
        s << "#Deprecated" << endl;

    if (canCreateWrapper) {
        writePrelude(s, cppClass);
        if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor())
            writeDestructor(s, cppClass);
    }

    writeFieldsAccessFunctions(s, cppClass);

    //inject code native end
    writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                   CodeSnip::End, TypeSystem::NativeCode);

    writeBoostDeclaration(s, cppClass);
}

void CppGenerator::writeFieldsAccessFunctions(QTextStream& s, const AbstractMetaClass* cppClass)
{
    //Fields
    foreach (AbstractMetaField *field, cppClass->fields()) {
        if (field->isPublic()) {
            writeFieldAccess(s, cppClass, field);
        }
    }
}

void CppGenerator::writePrelude(QTextStream& s, const AbstractMetaClass* cppClass)
{
    //inject code native beginner
    writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                   CodeSnip::Beginning, TypeSystem::NativeCode);

    foreach (AbstractMetaFunction *func, filterFunctions(cppClass)) {
        if ((func->isPrivate() || func->isModifiedRemoved()) && !func->isAbstract())
            continue;

        if (func->isConstructor() && (func->allowThread() || func->hasInjectedCode())) {
            writeModifiedConstructorImpl(s, func);
        } else if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor() &&
            func->isConstructor() && !func->isCopyConstructor()) {
            writeConstructorImpl(s, func);
        } else if (func->isVirtual() || func->isAbstract()) {
            writeVirtualMethodImpl(s, func);
        } else if (func->hasInjectedCode() || func->isThread() || func->allowThread()) {
            writeNonVirtualModifiedFunctionImpl(s, func);
        } else if (func->isInGlobalScope() && func->isOperatorOverload()) {
            writeGlobalOperatorOverloadImpl(s, func);
        }
    }
}


void CppGenerator::writeModifiedConstructorImpl ( QTextStream& s, const AbstractMetaFunction* func )
{
    Indentation indentation(INDENT);
    const AbstractMetaClass* clazz = func->ownerClass();
    s << "static " << clazz->name() << "* " << nameForModifiedCtorFunction(func) << '(';
    writeFunctionArguments(s, func, SkipDefaultValues);
    s << ")\n{" << endl;

    s << INDENT << clazz->name() << "* _self = 0;" << endl;
    s << INDENT << '{' << endl;
    {
        Indentation indentation(INDENT);
        if (func->allowThread())
            s << INDENT << "py_allow_threads allow_threads;" << endl;

        s << INDENT << "_self = new ";
        writeFunctionCall(s, func);
        s << ';' << endl;
    }
    s << INDENT << '}' << endl;
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::Beginning, TypeSystem::All, func);
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::End, TypeSystem::All, func);
    s << INDENT << "python::object _obj(PySide::ptr(_self));" << endl;
    s << INDENT << "return _self;" << endl;
    s << '}' << endl;
}

void CppGenerator::writeConstructorImpl(QTextStream& s, const AbstractMetaFunction* func)
{
    QString wrapperName = getWrapperName(func->ownerClass());
    s << wrapperName << "::" << wrapperName << "(PyObject *py_self" << (func->arguments().size() ? ", " : "");
    writeFunctionArguments(s, func, Options(OriginalTypeDescription) | SkipDefaultValues);
    s << ")" << endl;
    s << INDENT << " : ";
    writeFunctionCall(s, func);
    s << ", wrapper(py_self)" << endl << "{" << endl;
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::Beginning, TypeSystem::All, func);
    writeCodeSnips(s, getCodeSnips(func), CodeSnip::End, TypeSystem::All, func);
    s << '}' << endl << endl;
}

void CppGenerator::writeVirtualMethodImplHead(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    s << INDENT << "thread_locker lock;" << endl;

    if (func->hasInjectedCode()) {
        writeCodeSnips(s, getCodeSnips(func),
                       CodeSnip::Beginning, TypeSystem::NativeCode, func);
    }

    s << INDENT << "python::object method = get_override(\"" << func->implementingClass()->name();
    if (func->implementingClass()->typeEntry()->isObject() || func->implementingClass()->typeEntry()->isQObject())
        s << '*';

    s << "\", \"" << func->name() << "\");" << endl
    << INDENT << "if (method)" << endl <<  INDENT << "{" << endl;

    {
        Indentation indentation(INDENT);
        s << INDENT;
        if (func->type())
            s << "python::object __result = ";

        s << "method(";
        writeArgumentNames(s, func, BoxedPrimitive);
        s << ");" << endl;

        QString typeName = getFunctionReturnType(func);
        if (!typeName.isEmpty()) {

            CodeSnipList codeSnips = getCodeSnips(func);
            bool hasVirtualBeginningCode = false;
            foreach(CodeSnip cs, codeSnips) {
                if ((cs.position == CodeSnip::Beginning) && (cs.language == TypeSystem::TargetLangCode)) {
                    hasVirtualBeginningCode = true;
                    break;
                }
            }

            if (hasVirtualBeginningCode) {
                writeCodeSnips(s, codeSnips, CodeSnip::Beginning, TypeSystem::TargetLangCode, func);
            } else if (func->type()) {
                s << INDENT << typeName << " __return_value = " << "python::extract<" << typeName << " >(__result);" << endl;
                bool boxedPointer = false;
                if (func->type() && !func->type()->isConstant() &&
                    (func->type()->isObject() || func->type()->isQObject())) {

                    s << INDENT << "PySide::qptr<" << QString(typeName).replace("*", "") << " > __ptr(__result.ptr());" << endl
                      << INDENT << "python::incref(__result.ptr());" << endl
                      << INDENT << "__ptr.release_ownership();" << endl;
                }

                s << INDENT << "return __return_value;" << endl;
            }
        }
    }
    s << INDENT << "}" << endl;
}

void CppGenerator::writeVirtualMethodImpl(QTextStream& s, const AbstractMetaFunction* func)
{
    if (func->isModifiedRemoved())
        return;

    if (!func->isAbstract() && !func->ownerClass()->hasPrivateDestructor() &&
        func->implementingClass() == func->ownerClass()) {
        writeVirtualDefaultFunction(s, func);
    }


    QString prefix = getWrapperName(func->ownerClass()) + "::";
    s << functionSignature(func, prefix, "",
                           Options(Generator::OriginalTypeDescription) | Generator::SkipDefaultValues)
      << endl << "{" << endl;

    writeVirtualMethodImplHead(s, func);

    if (func->isAbstract())
        writePureVirtualMethodImplFoot(s, func);
    else
        writeVirtualMethodImplFoot(s, func);

    s << '}' << endl << endl;
}


void CppGenerator::writePureVirtualMethodImplFoot(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    s << INDENT << "else" << endl
      << INDENT << "{" << endl;
    {
        Indentation indentation(INDENT);
        s << INDENT << "PyErr_SetString(PyExc_NotImplementedError, \""
          << func->ownerClass()->name() << "." << func->name() << " : "
          << "You need to implement pure virtual functions in python\");" << endl
          << INDENT << "throw python::error_already_set();" << endl;
    }
    s << INDENT << "}" << endl;
}

void CppGenerator::writeVirtualMethodImplFoot(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    s << INDENT << "else" << endl << INDENT <<  "{" << endl;
    {
        Indentation indentation(INDENT);
        QString returnKeyword = func->type() ? QLatin1String("return ") : QString();

        if (func->allowThread())
            s << INDENT << "py_allow_threads allow_threads;" << endl;

        s << INDENT << returnKeyword << func->implementingClass()->qualifiedCppName() << "::";
        writeFunctionCall(s, func);
        s << ';' << endl;
    }
    s << INDENT << '}' << endl;
}

void CppGenerator::writeVirtualDefaultFunction(QTextStream &s, const AbstractMetaFunction *func)
{
    Indentation indentation(INDENT);
    QString returnKeyword = func->type() ? QLatin1String("return ") : QString();
    QString defaultMethodSignature = signatureForDefaultVirtualMethod(func, getWrapperName(func->ownerClass()) + "::", "_default", Generator::SkipDefaultValues);
    s << defaultMethodSignature << endl << '{' << endl;

    if (func->allowThread())
        s << INDENT << "py_allow_threads allow_threads;" << endl;

    CodeSnipList codeSnips = getCodeSnips(func);
    bool hasVirtualEndCode = false;
    foreach(CodeSnip cs, codeSnips) {
        if ((cs.position == CodeSnip::End) && (cs.language == TypeSystem::TargetLangCode)) {
            hasVirtualEndCode = true;
            break;
        }
    }

    if (!hasVirtualEndCode) {
        s << INDENT << returnKeyword << "self." << func->implementingClass()->qualifiedCppName() << "::";
        writeFunctionCall(s, func);
        s << ";" << endl;
    } else  {
        writeCodeSnips(s, getCodeSnips(func),
                       CodeSnip::End, TypeSystem::TargetLangCode, func);
    }
    s << '}' << endl << endl;

}



void CppGenerator::writeNonVirtualModifiedFunctionImpl(QTextStream& s, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);

    s << "static " << getFunctionReturnType(func) << ' ';
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

    if (getCodeSnips(func).size() > 0) {
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

AbstractMetaFunction* CppGenerator::findMainConstructor(const AbstractMetaClass* clazz)
{
    foreach (AbstractMetaFunction* func, filterFunctions(clazz)) {
        if (func->isConstructor() &&
            func->isPublic() &&
            !func->isModifiedRemoved() &&
            !func->isPrivate()) {

            //do not use copy constructor here
            if (func->isCopyConstructor())
                continue;
            return func;
        }
    }
    return 0;
}

void CppGenerator::writeGetterFieldFunction(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaField *field)
{
    s << "static ";

    bool pointer = false;
    if (field->type()->isQObject() || field->type()->isObject())
        pointer = true;

    if (pointer)
        s << "python::object";
    else
        s << field->type()->cppSignature();

    s << " getter_" << cppClass->name() << "_" << field->name() << "(";

    if (!field->isStatic())
        s << cppClass->qualifiedCppName() << " &self";

    s  << ")" << endl << "{" << endl
       << INDENT << "return ";

    if (pointer)
        s << "python::object(PySide::ptr(";

    if (!field->isStatic())
        s << "self.";
    else
        s << field->enclosingClass()->typeEntry()->qualifiedCppName() << "::";

    s << field->name();

    if (pointer)
        s << "))";

    s << ";" << endl << "}" << endl;
}

void CppGenerator::writeSetterFieldFunction(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaField *field)
{
    s << "static void setter_" << cppClass->name() << "_" << field->name() << "(";

    if (!field->isStatic())
        s << cppClass->qualifiedCppName() << " &self, ";

    s  << field->type()->cppSignature() << " _value)" << endl << "{" << endl
       << INDENT;

    if (!field->isStatic())
        s << "self.";
    else
        s << field->enclosingClass()->typeEntry()->qualifiedCppName() << "::";

    s << field->name() << " = _value;" << endl << "}" << endl;
}


void CppGenerator::writeFieldAccess(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaField *field)
{
    Indentation indent(INDENT);

    writeGetterFieldFunction(s, cppClass, field);
    if (!field->type()->isConstant())
        writeSetterFieldFunction(s, cppClass, field);
}


void CppGenerator::writeHashFunction(QTextStream& s, const AbstractMetaClass* cppClass)
{
    QString argType;

    //WORKAROUND: diferent way to QChar
    if (cppClass->name() == "QChar")
        argType = "QChar";
    else
        argType = "const " + cppClass->name() + "&";

    s << "// Hash function" << endl
    << "{" << endl
    << INDENT << INDENT << "typedef uint (*hash_type) ( " << argType << " );"
    << INDENT << INDENT <<  "python_cls.def(\"__hash__\", hash_type(&"
    << cppClass->typeEntry()->hashFunction() << "));" << endl
    << "}" << endl;
}

QString CppGenerator::baseClassName(const QString& name)
{
    QStringList lst = name.split("::");
    return lst.last();
}

void CppGenerator::writeBoostDeclaration(QTextStream& s, const AbstractMetaClass* cppClass)
{
    Indentation indent(INDENT);
    QString wrapperName = getWrapperName(cppClass);

    s << "void " << wrapperName << "::define_python_class() throw() {" << endl;

    const AbstractMetaFunction* mainCtor = 0;
    bool mainCtorHasModifications = false;
    if (!cppClass->isNamespace()) {
        // python_cls declaration
        mainCtor = findMainConstructor(cppClass);
        if (mainCtor)
            mainCtorHasModifications = mainCtor->allowThread() || mainCtor->hasInjectedCode();

        s << INDENT;
        if (!cppClass->isPolymorphic() || cppClass->hasPrivateDestructor())
            s << wrapperName << "::";

        s << "class_type python_cls(\""
          <<  baseClassName(cppClass->name()) << "\", ";

        if (!mainCtor || mainCtorHasModifications)
            s << "python::no_init";
        else
            writeConstructorInitialization(s, mainCtor);

        s << ");" << endl << endl;
    } else {
        QRegExp reg("(?:\\w+::)*(\\w+)");
        reg.indexIn(cppClass->name());
        s << INDENT << "python::class_<Namespace> python_cls(\"" << reg.cap(1) << "\");" << endl;
    }
    // scope declaration
    s << INDENT << "python::scope " << wrapperName << "_scope(python_cls);" << endl;

    if (cppClass->templateBaseClass() && cppClass->templateBaseClass()->typeEntry()->isContainer()) {
        s << endl << INDENT << "//Index suite for QContainer" << endl
          << INDENT << "python_cls.def(qcontainer_indexing_suite< " << cppClass->qualifiedCppName() << " >());" << endl << endl;
    }


    if (cppClass->isPolymorphic() && !cppClass->hasPrivateDestructor() && canCreateWrapperFor(cppClass)) {
        QString heldType = cppClass->typeEntry()->heldTypeValue();
        if (heldType.isEmpty())
            heldType = "PySide::qptr";

        s << INDENT << "python::implicitly_convertible< "
          << heldType << "<" << wrapperName << ">, "
          << heldType << "<" << cppClass->qualifiedCppName() << "> >();" << endl;
    }

    //Enums
    writeEnums(s, cppClass, cppClass->hasPrivateDestructor() || cppClass->isNamespace());

    if (cppClass->innerClasses().count()) {
        s << endl << INDENT << "// Inner classes" << endl;
        foreach (AbstractMetaClass* innerClass, cppClass->innerClasses()) {
            if (!innerClass->typeEntry()->generateCode())
                continue;
            s << INDENT << getWrapperName(innerClass) << "::define_python_class();" << endl;
        }
    }

    //Fields
    foreach (AbstractMetaField *field, cppClass->fields()) {
        QString strAccess;

        if (field->isPublic()) {

            s << INDENT << "python_cls.add_property("
              << "\"" << field->name() << "\""
              << ", getter_" << cppClass->name() << "_" << field->name();
            if (!field->type()->isConstant())
                s << ", setter_" << cppClass->name() << "_" << field->name();

            s << ");" << endl;

        }
    }

    writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                   CodeSnip::Beginning, TypeSystem::TargetLangCode);

    QSet<QString> staticMethods;
    AbstractMetaFunctionList functionList = filterFunctions(cppClass);

    if (!cppClass->isNamespace()) {
        //search for all static methods to match with normal functions
        //to rename when match with one member function
        foreach (AbstractMetaFunction *func, functionList) {
            if (func->isStatic() && !func->isOperatorOverload())
                staticMethods << func->name();
        }
    }

    foreach (AbstractMetaFunction *func, functionList) {
        if (func->isModifiedRemoved() || func->isPrivate() || func->isSignal())
            continue;

        //rename static function when is the same name as member function
        if (!cppClass->isNamespace() && func->isStatic()) {
            QString staticName(createStaticFunctionName(func));
            QSet<QString>::iterator staticFuncInter = staticMethods.find(staticName);
            if (staticFuncInter != staticMethods.end())
                func->setName(staticName);
        } else {
            QSet<QString>::iterator staticFuncInter = staticMethods.find(func->name());
            if (staticFuncInter != staticMethods.end()) {
                staticMethods.erase(staticFuncInter);
                staticMethods << createStaticFunctionName(func);
            }
        }

        if (func->isOperatorOverload()) {
            // Do not join the ifs -- isOperatorOverload must be checked alone
            if (func->originalName() == func->name())
                writeOperatorOverload(s, func);
        } else if (func->isConstructor()) {
            //Use same rule as hpp genenrator for copy constructor
            if ((mainCtorHasModifications || func != mainCtor) && !func->isCopyConstructor()) {
                writeConstructor(s, func);
            }
        } else if (!func->isVirtual() &&
                   (func->hasInjectedCode() ||
                    func->isThread() || func->allowThread())) {
            writeModifiedMethodDef(s, func);
        } else if (func->implementingClass() == func->ownerClass()) {
            writeNormalMethodDef(s, func);
        }

        //if is namespace all methothds is stattic
        if (cppClass->isNamespace())
            s << INDENT << "python_cls.staticmethod(\"" << func->name() << "\");" << endl;
    }

    //write copy constructor here
    if (isCopyable(cppClass) && !cppClass->isNamespace()) {
        s << INDENT << "python_cls.def(python::init<const ";
        s << cppClass->qualifiedCppName() << "&>());" << endl;
    }

    writeCodeSnips(s, cppClass->typeEntry()->codeSnips(),
                   CodeSnip::End, TypeSystem::TargetLangCode);

    if (!cppClass->isNamespace()) {
        // Static methods
        if (!staticMethods.isEmpty())
            s << INDENT << "// Static methods" << endl;

        foreach (QString funcName, staticMethods)
            s << INDENT << "python_cls.staticmethod(\"" << funcName << "\");" << endl;
    }

    // qHash usage
    if (!cppClass->typeEntry()->hashFunction().isEmpty())
        writeHashFunction(s, cppClass);

    // implicity conversions
    writeImplicitlyConversion(s, cppClass);

    // register object/value type
    if (!cppClass->isNamespace()) {
        QString className = cppClass->qualifiedCppName();
        const char* funcName = (cppClass->typeEntry()->isObject() || !isCopyable(cppClass)) ? "object" : "value";
        s << INDENT
          << "type_manager::instance().register_"
          << funcName
          << "_type<" << className << " >(\""
          << cppClass->qualifiedCppName() << (cppClass->typeEntry()->isObject() ? "*" : "") << "\");\n";
    }
    s << '}' << endl;
}

void CppGenerator::writeConstructor(QTextStream& s, const AbstractMetaFunction* func)
{
    s << INDENT << "python_cls.def(";
    writeConstructorInitialization(s, func);
    s << ");" << endl;
}

void CppGenerator::writeFunctionArgsDef(QTextStream &sOut,
                                        const AbstractMetaFunction *cppFunction)
{
    bool hasDefaultValue = false;
    int argUsed = 0;
    QString aux;
    QTextStream s(&aux);

    foreach (const AbstractMetaArgument *arg, cppFunction->arguments()) {
        if (cppFunction->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        if (argUsed > 0)
            s << ", ";

        if (!m_disableNamedArgs)
            s << "python::arg(\"" << arg->argumentName() << "\")";
        else
            s << "python::arg(0)";

        if (!arg->defaultValueExpression().isEmpty()) {
            QString defaultValue = arg->defaultValueExpression();
            bool isPointer = arg->type()->isObject() ||
                              arg->type()->isQObject() ||
                              arg->type()->isValuePointer() ||
                              arg->type()->isNativePointer();

            if (isPointer && defaultValue == "0") {
                defaultValue = "python::object()";
            } else if (arg->type()->isFlags()) {
                defaultValue = " (int) " + defaultValue;
            } else if (arg->type()->isEnum()) {
                QString enumName = arg->type()->minimalSignature();
                QRegExp reg("(.*::)");
                reg.indexIn(enumName);
                if (!defaultValue.startsWith(reg.cap(1)))
                    defaultValue =  reg.cap(1) + defaultValue;
            }

            s << "=" << defaultValue;
            hasDefaultValue = true;
        }
        argUsed++;
    }

    if (hasDefaultValue || ((argUsed > 0) && !m_disableNamedArgs))
        sOut << "," << endl << INDENT << INDENT << "(" << aux << ")";
}

void CppGenerator::writeNormalMethodDef(QTextStream& s, const AbstractMetaFunction* func)
{
    s << INDENT << '{' << endl;
    {
        Indentation indentation(INDENT);
        QString wrapperClassName = getWrapperName(func->ownerClass());
        bool needDefaultFunction = func->isVirtual() && !func->isAbstract() && !func->ownerClass()->hasPrivateDestructor();
        QString castName;

        if (needDefaultFunction)
            castName = writeFunctionCast(s, func, "_default", func->implementingClass()->qualifiedCppName());
        else
            castName = writeFunctionCast(s, func);

        s << INDENT << "python_cls.def(\"" << func->name() << "\", ";

        if (needDefaultFunction) { // add the default function
            s << castName << "(&" << wrapperClassName << "::" << func->originalName() << "_default)";
        } else {
            if (func->isAbstract())
                s << "python::pure_virtual";
            s << '(' << castName << '(' << getMethodPointerString(func) << "))";
        }

        QString functionPolicy = getFunctionCallPolicy(func);
        if (!functionPolicy.isEmpty())
            s << ", " << functionPolicy;

        writeFunctionArgsDef(s, func);
        s << ");" << endl;
    }
    s << INDENT << '}' << endl;
}

void CppGenerator::writeModifiedMethodDef(QTextStream& s, const AbstractMetaFunction* func)
{
    s << INDENT << '{' << endl;
    {
        Indentation indentation(INDENT);
        QString castName = writeFunctionCast(s, func);
        s << INDENT
          << "python_cls.def(\""
          << func->name() << "\", "
          << castName
          << "(&" << func->implementingClass()->name()
          << "_" << func->originalName()
          << "_modified)";
        QString functionPolicy = getFunctionCallPolicy(func);
        if (!functionPolicy.isEmpty())
            s << ", " << functionPolicy;

        writeFunctionArgsDef(s, func);
        s << ");" << endl;
    }
    s << INDENT << '}' << endl;
}

QString CppGenerator::operatorFunctionName(const AbstractMetaFunction *cppFunction)
{
    QString funcName = QString("%1_operator_%2_")
                       .arg(cppFunction->arguments()[0]->type()->name())
                       .arg(cppFunction->arguments()[1]->type()->name());

    if (cppFunction->name().contains(">>")) {
        funcName += "rshift";
    } else if (cppFunction->name().contains("<<")) {
        funcName += "lshift";
    } else {
        //TODO: implemente support to others operators
        return QString();
    }

    return funcName;
}

void CppGenerator::writeGlobalOperatorOverloadImpl(QTextStream& s, const AbstractMetaFunction* cppFunction)
{
    Indentation indent(INDENT);
    QString operatorStr;

    if (cppFunction->name().contains(">>")) {
        operatorStr = " >> ";
    } else if (cppFunction->name().contains("<<")) {
        operatorStr = " << ";
    } else {
        //TODO: implemente support to others operators
        return;
    }

    QString funcName = operatorFunctionName(cppFunction);
    bool reverse = cppFunction->isReverseOperator();

    const AbstractMetaClass *klass = cppFunction->ownerClass();
    s << "python::object " << funcName << "(";
    writeFunctionArguments(s, cppFunction, Options(SkipDefaultValues) | SkipRemovedArguments);
    s << ")" << endl << "{" << endl
      << INDENT << cppFunction->arguments()[reverse]->argumentName()
      << operatorStr << cppFunction->arguments()[!reverse]->argumentName() << ";" << endl
      << INDENT << "return python::object(PySide::ptr(&"
      << cppFunction->arguments()[reverse]->argumentName() << "));" << endl
      << "}" << endl;
}

void CppGenerator::writeGlobalOperatorOverload(QTextStream &s, const AbstractMetaFunction *cppFunction)
{
    QString funcName = operatorFunctionName(cppFunction);
    if (funcName.isEmpty())
        return;

    bool reverse = cppFunction->isReverseOperator();
    QString operatorStr;
    if (cppFunction->name().contains(">>")) {
        operatorStr = QString("__%1rshift__").arg(reverse ? "r" : "");
    } else if (cppFunction->name().contains("<<")) {
        operatorStr = QString("__%1lshift__").arg(reverse ? "r" : "");
    } else {
        //TODO: implemente support to others operators
        return;
    }

    s << INDENT << "python_cls.def(\"" << operatorStr << "\", " << funcName << ");\n";
}

QString CppGenerator::getOperatorArgumentTypeName(const AbstractMetaFunction *cppFunction, int argumentIndex)
{
    AbstractMetaType* type = cppFunction->arguments()[argumentIndex]->type();
    if (type->name() == cppFunction->implementingClass()->name())
        return QLatin1String("python::self");

    QString typeName = translateType(type, cppFunction->implementingClass(),
                                     (Option)(ExcludeReference));
    return type->isPrimitive() ? "(" + typeName + ")(0)" : "python::other<" + typeName + " >()";
}

void CppGenerator::writeOperatorOverload(QTextStream& s, const AbstractMetaFunction* cppFunction)
{
    static QRegExp operatorRegex("operator(.+)");

    if (!operatorRegex.exactMatch(cppFunction->originalName())) {
        qWarning("What kind of operator is that!? %s",
                 cppFunction->originalName().toLocal8Bit().data());
        return;
    }

    QString op(operatorRegex.cap(1));
    if (op == "=" || op == "[]") {
        // = is handled by type boost and type conversions, [] by someone...
        return;
    }

    // no args == member unary operator
    if (!cppFunction->arguments().count()) {
        // check if it is a name instead of an operator symbol
        // this means it is a conversion operator that will be ignored for now
        static QRegExp ConversionOperatorRegex("[A-Za-z]+");
        if (ConversionOperatorRegex.indexIn(op) < 0)
            s << INDENT << "python_cls.def(" << op << "python::self);" << endl;
        return;
    }

    //this because global operators use first arg with current class
    if (cppFunction->isInGlobalScope()) {
        writeGlobalOperatorOverload(s, cppFunction);
        return;
    }

    QString operand1, operand2;
    if (cppFunction->arguments().count() == 1) {
        operand1 = "python::self";
        operand2 = getOperatorArgumentTypeName(cppFunction, 0);
    } else {
        operand1 = getOperatorArgumentTypeName(cppFunction, 0);
        operand2 = getOperatorArgumentTypeName(cppFunction, 1);
    }
    s << INDENT << "python_cls.def(" << operand1 << ' ' << op << ' ' << operand2 << ");\n";
}

void CppGenerator::finishGeneration()
{
    //Generate boost wrapper file
    QString classFiles;
    QTextStream sClassFiles(&classFiles);
    QString classPythonDefines;
    QTextStream sClassPythonDefines(&classPythonDefines);

    Indentation indent(INDENT);

    foreach (AbstractMetaClass *cls, classes()) {
        if (!shouldGenerate(cls) || cls->enclosingClass())
            continue;

        if (m_packageName.isEmpty())
            m_packageName = cls->package();

        QString wrapperName = getWrapperName(cls);
        QString boostFilename;
        boostFilename += wrapperName + ".hpp";
        sClassFiles << "#include \"" << boostFilename << "\"" << endl;

        QString define_str = wrapperName + "::";
        define_str += "define_python_class();";

        sClassPythonDefines << INDENT << define_str << endl;
    }

    QString moduleFileName(outputDirectory() + "/" + subDirectoryForPackage(m_packageName));
    moduleFileName += "/" + moduleName().toLower() + "_module_wrapper.cpp";

    QFile file(moduleFileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);

        // write license comment
        s << licenseComment() << endl;

        s << "#include \"converter_register_" << moduleName().toLower();
        s << ".hpp\"" << endl << endl;

        s << classFiles << endl;

        s << "using namespace boost;" << endl << endl;
        s << "using namespace PySide;" << endl << endl;

        s << "// forward decl. for global func. register\n";
        s << "void register_global_functions_" << moduleName().toLower() << "();\n\n";

        s << "BOOST_PYTHON_MODULE(" << moduleName() << ")" << endl;
        s << "{" << endl;

        foreach (QString requiredModule, TypeDatabase::instance()->requiredTargetImports()) {
            s << INDENT << "if (";
            s << "PyImport_ImportModule(\"" << requiredModule << "\") == NULL) {" << endl;
            s << INDENT << INDENT << "PyErr_SetString(PyExc_ImportError,";
            s << "\"could not import " << requiredModule << "\");" << endl;
            s << INDENT << INDENT << "return;" << endl;
            s << INDENT << "}" << endl;
        }
        s << endl;



        s << INDENT << "register_type_converters_" << moduleName().toLower() << "();" << endl << endl
          << classPythonDefines << endl
          << INDENT << "register_global_functions_" << moduleName().toLower() << "();" << endl
          << INDENT << "//Namespaces" << endl;

        s << "}" << endl << endl;
    }

    writeGlobalFunctions();
}

void CppGenerator::writeGlobalFunctions()
{
    QString fileName = moduleName().toLower() + "_globals_wrapper.cpp";

    FileOut fileOut(outputDirectory() + "/" + subDirectoryForPackage(m_packageName) + "/" + fileName);

    QSet<QString> includes;
    QString defsStr;
    QTextStream defsStream(&defsStr);

    foreach (AbstractMetaFunction* func, globalFunctions()) {
        QString incFile = func->includeFile();
        QRegExp regex("\\b" + moduleName() + "\\b");
        //FIXME: this regex doesn't work with all cases, e.g.:
        //          moduleName() = local
        //          incFile = /usr/local/include/local
        if (regex.indexIn(incFile) == -1)
            continue;

        int idx = incFile.indexOf(moduleName());
        QString cleanPath = QDir::cleanPath(incFile.mid(idx));
        if (!cleanPath.startsWith(moduleName()))
            continue;

        includes << cleanPath;
        defsStream << INDENT << "{\n" << INDENT;
        QString castName = writeFunctionCast(defsStream, func);
        defsStream << INDENT << INDENT << "python::def(\"" << func->name();
        defsStream << "\", " << castName << '(' << func->name() << ')';
        if (func->type() && func->type()->isReference())
            defsStream << ", python::return_internal_reference<>()";
        defsStream << ");\n";
        defsStream << INDENT << "}\n";
    }

    QTextStream& s = fileOut.stream;

    // write license comment
    s << licenseComment() << endl;

    s << "#include \"pyside.hpp\"" << endl;

    foreach (QString include, includes)
        s << "#include <" << include << ">\n";

    s << "using namespace boost;\n\n";
    s << "using namespace PySide;\n\n";

    // Add module level code snippets to 'Global' class
    TypeSystemTypeEntry *moduleEntry = dynamic_cast<TypeSystemTypeEntry *>(
                                            TypeDatabase::instance()->findType(m_packageName));
    QString sEnd;
    QTextStream snipEnd(&sEnd);
    if (moduleEntry && moduleEntry->codeSnips().size() > 0) {
        foreach (CodeSnip snip, moduleEntry->codeSnips()) {
            if (snip.position == CodeSnip().Beginning)
                formatCode(s, snip.code(), INDENT);
            else
                formatCode(snipEnd, snip.code(), INDENT);
        }
    }

    s << "\nvoid register_global_functions_" << moduleName().toLower() << "() {\n";
    { //global enums
        QString name_space;

        foreach (AbstractMetaEnum *cppEnum, globalEnums()) {
            if (cppEnum)
                writeEnum(s, cppEnum, name_space);
        }
    }
    s << sEnd;
    s << defsStr;
    s << "}\n";
}

QMap<QString, QString> CppGenerator::options() const
{
    QMap<QString, QString> res;
    res.insert("disable-named-arg", "Disable Python names arguments.");
    return res;
}

bool CppGenerator::doSetup(const QMap<QString, QString>& args )
{
    m_disableNamedArgs = args.contains("disable-named-arg");
    return BoostPythonGenerator::doSetup(args);
}
