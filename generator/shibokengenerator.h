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

#ifndef SHIBOKENGENERATOR_H
#define SHIBOKENGENERATOR_H

#define PYTHON_RETURN_VAR         "pyResult"
#define CPP_RETURN_VAR            "cppResult"
#define THREAD_STATE_SAVER_VAR    "threadStateSaver"
#define CPP_SELF_VAR              "cppSelf"
#define CPP_ARG                   "cppArg"
#define CPP_ARG0                  (CPP_ARG "0")

#include <generator.h>
#include <QtCore/QTextStream>

#include "overloaddata.h"

class DocParser;

/**
 * Abstract generator that contains common methods used in CppGenerator and HeaderGenerator.
 */
class ShibokenGenerator : public Generator
{
public:
    ShibokenGenerator();

    QString translateTypeForWrapperMethod(const AbstractMetaType* cType,
                                          const AbstractMetaClass* context, Options opt = NoOption) const;

    /**
    *   Returns a map with all functions grouped, the function name is used as key.
    *   Example ofg return value: { "foo" -> ["foo(int)", "foo(int, long)], "bar" -> "bar(double)"}
    * \param scope Where to search for functions, null means all global functions.
    */
    QMap<QString, AbstractMetaFunctionList> getFunctionGroups(const AbstractMetaClass* scope = 0);
    /**
    *   Returns all overloads for a function named \p functionName.
    *   \param scope scope used to search for overloads.
    *   \param functionName the function name.
    */
    AbstractMetaFunctionList getFunctionOverloads(const AbstractMetaClass* scope, const QString& functionName);
    /**
    *   Returns the minimun and maximun number of arguments which this function and all overloads
    *   can accept. Arguments removed by typesystem are considered as well.
    */
    QPair<int, int> getMinMaxArguments(const AbstractMetaFunction* metaFunction);
    /**
     *   Write a function argument in the C++ in the text stream \p s.
     *   This function just call \code s << argumentString(); \endcode
     *   \param s text stream used to write the output.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    void writeArgument(QTextStream &s,
                       const AbstractMetaFunction* func,
                       const AbstractMetaArgument* argument,
                       Options options = NoOption) const;
    /**
     *   Create a QString in the C++ format to an function argument.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    QString argumentString(const AbstractMetaFunction* func,
                           const AbstractMetaArgument* argument,
                           Options options = NoOption) const;

    void writeArgumentNames(QTextStream &s,
                            const AbstractMetaFunction* func,
                            Options options = NoOption) const;

    /**
     *   Function used to write the fucntion arguments on the class buffer.
     *   \param s the class output buffer
     *   \param func the pointer to metafunction information
     *   \param count the number of function arguments
     *   \param options some extra options used during the parser
     */
    void writeFunctionArguments(QTextStream &s,
                                const AbstractMetaFunction* func,
                                Options options = NoOption) const;
    QString functionReturnType(const AbstractMetaFunction* func, Options options = NoOption) const;
    /**
     *   Write a code snip into the buffer \p s.
     *   CodeSnip are codes inside inject-code tags.
     *   \param s    the buffer
     *   \param code_snips   a list of code snips
     *   \param position     the position to insert the code snip
     *   \param language     the kind of code snip
     *   \param func the cpp function
     *   \param lastArg last argument whose value is available, usually the last;
     *                  a NULL pointer indicates that no argument will be available,
     *                  i.e. a call without arguments.
     *   \param context the class context for the place where the code snip will be written
     */
    void writeCodeSnips(QTextStream &s,
                        const CodeSnipList &code_snips,
                        CodeSnip::Position position,
                        TypeSystem::Language language,
                        const AbstractMetaFunction* func = 0,
                        const AbstractMetaArgument* lastArg = 0,
                        const AbstractMetaClass* context = 0);

    /**
     *   Verifies if any of the function's code injections of the "target"
     *   type needs the type system variable "%CPPSELF".
     *   \param func the function to check
     *   \return true if the function's target code snippets use "%CPPSELF"
     */
    bool injectedCodeUsesCppSelf(const AbstractMetaFunction* func);

    /**
     *   Verifies if any of the function's code injections of the "native"
     *   type needs the type system variable "%PYSELF".
     *   \param func the function to check
     *   \return true if the function's native code snippets use "%PYSELF"
     */
    bool injectedCodeUsesPySelf(const AbstractMetaFunction* func);

    /**
     *   Verifies if any of the function's code injections makes a call
     *   to the C++ method. This is used by the generator to avoid writing calls
     *   to C++ when the user custom code already does this.
     *   \param func the function to check
     *   \return true if the function's code snippets call the wrapped C++ function
     */
    bool injectedCodeCallsCppFunction(const AbstractMetaFunction* func);

    /**
     *   Verifies if any of the function's code injections of the "native" class makes a
     *   call to the C++ method. This is used by the generator to avoid writing calls to
     *   Python overrides of C++ virtual methods when the user custom code already does this.
     *   \param func the function to check
     *   \return true if the function's code snippets call the Python override for a C++ virtual method
     */
    bool injectedCodeCallsPythonOverride(const AbstractMetaFunction* func);

    /**
     *   Verifies if any of the function's code injections attributes values to
     *   the return variable (%0 or %PYARG_0).
     *   \param func        the function to check
     *   \param language    the kind of code snip
     *   \return true if the function's code attributes values to "%0" or "%PYARG_0"
     */
    bool injectedCodeHasReturnValueAttribution(const AbstractMetaFunction* func, TypeSystem::Language language = TypeSystem::TargetLangCode);

    /**
     *   Function which parse the metafunction information
     *   \param func the function witch will be parserd
     *   \param option some extra options
     *   \param arg_count the number of function arguments
     */
    QString functionSignature(const AbstractMetaFunction* func,
                              QString prepend = "",
                              QString append = "",
                              Options options = NoOption,
                              int arg_count = -1) const;

    /// Returns true if there are cases of multiple inheritance in any of its ancestors.
    bool hasMultipleInheritanceInAncestry(const AbstractMetaClass* metaClass);

    /// Returns true if the class needs to have a getattro function.
    bool classNeedsGetattroFunction(const AbstractMetaClass* metaClass);

    /// Returns a list of methods of the given class where each one is part of a different overload with both static and non-static method.
    AbstractMetaFunctionList getMethodsWithBothStaticAndNonStaticMethods(const AbstractMetaClass* metaClass);

    /// Returns a list of parent classes for a given class.
    AbstractMetaClassList getBaseClasses(const AbstractMetaClass* metaClass);

    /// Returns a list of all ancestor classes for the given class.
    AbstractMetaClassList getAllAncestors(const AbstractMetaClass* metaClass);

    const AbstractMetaClass* getMultipleInheritingClass(const AbstractMetaClass* metaClass);

    void writeBaseConversion(QTextStream& s, const AbstractMetaType* type,
                             const AbstractMetaClass* context, Options options = NoOption);
    /// Simpler version of writeBaseConversion, uses only the base name of the type.
    void writeBaseConversion(QTextStream& s, const TypeEntry* type);
    void writeToPythonConversion(QTextStream& s, const AbstractMetaType* type,
                                 const AbstractMetaClass* context, const QString& argumentName = QString());
    void writeToCppConversion(QTextStream& s, const AbstractMetaType* type,
                              const AbstractMetaClass* context, const QString& argumentName, Options options = NoOption);
    void writeToCppConversion(QTextStream& s, const AbstractMetaClass* metaClass, const QString& argumentName);

    /// Verifies if the class should have a C++ wrapper generated for it, instead of only a Python wrapper.
    static bool shouldGenerateCppWrapper(const AbstractMetaClass* metaClass);

    /// Adds enums eligible for generation from classes/namespaces marked not to be generated.
    static void lookForEnumsInClassesNotToBeGenerated(AbstractMetaEnumList& enumList, const AbstractMetaClass* metaClass);
    /// Returns the enclosing class for an enum, or NULL if it should be global.
    const AbstractMetaClass* getProperEnclosingClassForEnum(const AbstractMetaEnum* metaEnum);

    static QString wrapperName(const AbstractMetaClass* metaClass);

    static QString fullPythonFunctionName(const AbstractMetaFunction* func);
    static QString protectedEnumSurrogateName(const AbstractMetaEnum* metaEnum);
    static QString protectedFieldGetterName(const AbstractMetaField* field);
    static QString protectedFieldSetterName(const AbstractMetaField* field);

    static QString pythonPrimitiveTypeName(const QString& cppTypeName);
    static QString pythonPrimitiveTypeName(const PrimitiveTypeEntry* type);

    static QString pythonOperatorFunctionName(QString cppOpFuncName);
    static QString pythonOperatorFunctionName(const AbstractMetaFunction* func);
    static QString pythonRichCompareOperatorId(QString cppOpFuncName);
    static QString pythonRichCompareOperatorId(const AbstractMetaFunction* func);

    static QString cpythonOperatorFunctionName(const AbstractMetaFunction* func);

    static bool isNumber(QString cpythonApiName);
    static bool isNumber(const TypeEntry* type);
    static bool isNumber(const AbstractMetaType* type);
    static bool isPyInt(const TypeEntry* type);
    static bool isPyInt(const AbstractMetaType* type);
    static bool isCString(const AbstractMetaType* type);
    static bool isPairContainer(const AbstractMetaType* type);

    /// Checks if an argument type should be dereferenced by the Python method wrapper before calling the C++ method.
    static bool shouldDereferenceArgumentPointer(const AbstractMetaArgument* arg);
    /// Checks if a meta type should be dereferenced by the Python method wrapper passing it to C++.
    static bool shouldDereferenceAbstractMetaTypePointer(const AbstractMetaType* metaType);

    static bool visibilityModifiedToPrivate(const AbstractMetaFunction* func);

    QString cpythonBaseName(const AbstractMetaClass* metaClass);
    QString cpythonBaseName(const TypeEntry* type);
    QString cpythonBaseName(const AbstractMetaType* type);
    QString cpythonTypeName(const AbstractMetaClass* metaClass);
    QString cpythonTypeName(const TypeEntry* type);
    QString cpythonTypeNameExt(const TypeEntry* type);
    QString cpythonCheckFunction(const TypeEntry* type, bool genericNumberType = false);
    QString cpythonCheckFunction(const AbstractMetaType* metaType, bool genericNumberType = false);
    QString guessCPythonCheckFunction(const QString& type);
    QString cpythonIsConvertibleFunction(const TypeEntry* type, bool genericNumberType = false, bool checkExact = false);
    QString cpythonIsConvertibleFunction(const AbstractMetaType* metaType, bool genericNumberType = false);
    QString cpythonIsConvertibleFunction(const AbstractMetaArgument* metaArg, bool genericNumberType = false)
    {
        return cpythonIsConvertibleFunction(metaArg->type(), genericNumberType);
    }
    QString guessCPythonIsConvertible(const QString& type);
    QString cpythonFunctionName(const AbstractMetaFunction* func);
    QString cpythonMethodDefinitionName(const AbstractMetaFunction* func);
    QString cpythonGettersSettersDefinitionName(const AbstractMetaClass* metaClass);
    QString cpythonGetattroFunctionName(const AbstractMetaClass* metaClass);
    QString cpythonSetattroFunctionName(const AbstractMetaClass* metaClass);
    QString cpythonGetterFunctionName(const AbstractMetaField* metaField);
    QString cpythonSetterFunctionName(const AbstractMetaField* metaField);
    QString cpythonWrapperCPtr(const AbstractMetaClass* metaClass, QString argName = "self");
    QString cpythonWrapperCPtr(const AbstractMetaType* metaType, QString argName);
    QString cpythonWrapperCPtr(const TypeEntry* type, QString argName);

    /// Guesses the scope to where belongs an argument's default value.
    QString guessScopeForDefaultValue(const AbstractMetaFunction* func, const AbstractMetaArgument* arg);

    QString cpythonEnumName(const EnumTypeEntry* enumEntry);
    QString cpythonEnumName(const AbstractMetaEnum* metaEnum)
    {
        return cpythonEnumName(metaEnum->typeEntry());
    }

    QString cpythonFlagsName(const FlagsTypeEntry* flagsEntry);
    QString cpythonFlagsName(const AbstractMetaEnum* metaEnum)
    {
        FlagsTypeEntry* flags = metaEnum->typeEntry()->flags();
        if (!flags)
            return QString();
        return cpythonFlagsName(flags);
    }
    /// Returns the special cast function name, the function used to proper cast class with multiple inheritance.
    QString cpythonSpecialCastFunctionName(const AbstractMetaClass* metaClass);

    QString getFunctionReturnType(const AbstractMetaFunction* func, Options options = NoOption) const;
    QString getFormatUnitString(const AbstractMetaFunction* func, bool incRef = false) const;

    /// Returns the file name for the module global header. If no module name is provided the current will be used.
    QString getModuleHeaderFileName(const QString& moduleName = QString()) const;

    QString extendedIsConvertibleFunctionName(const TypeEntry* targetType) const;
    QString extendedToCppFunctionName(const TypeEntry* targetType) const;

    QMap< QString, QString > options() const;

    /// Returns true if the user enabled the so called "parent constructor heuristic".
    bool useCtorHeuristic() const;
    /// Returns true if the user enabled the so called "return value heuristic".
    bool useReturnValueHeuristic() const;
    /// Returns true if the user enabled PySide extensions.
    bool usePySideExtensions() const;
    QString cppApiVariableName(const QString& moduleName = QString()) const;
    QString getTypeIndexVariableName(const TypeEntry* metaType);
    /// Returns true if the user don't want verbose error messages on the generated bindings.
    bool verboseErrorMessagesDisabled() const;

    /**
     *   Builds an AbstractMetaType object from a QString.
     *   Returns NULL if no type could be built from the string.
     *   \param typeString The string describing the type to be built.
     *   \return A new AbstractMetaType object that must be deleted by the caller, or a NULL pointer in case of failure.
     */
    AbstractMetaType* buildAbstractMetaTypeFromString(QString typeString);

    /**
     *  Helper function to return the flags to be used by a meta type when
     * it needs to write some converter code.
     */
    static Options getConverterOptions(const AbstractMetaType* metaType);
protected:
    bool doSetup(const QMap<QString, QString>& args);
    // verify whether the class is copyable
    bool isCopyable(const AbstractMetaClass* metaClass);

    bool m_native_jump_table;
    static QHash<QString, QString> m_pythonPrimitiveTypeName;
    static QHash<QString, QString> m_pythonOperators;
    static QHash<QString, QString> m_formatUnits;
    static QHash<QString, QString> m_tpFuncs;

    void clearTpFuncs();

    const char* name() const { return "Shiboken"; }

    /**
     *   Initialize correspondences between primitive and Python types
     */
    static void initPrimitiveTypesCorrespondences();

    void writeFunctionCall(QTextStream& s,
                           const AbstractMetaFunction* metaFunc,
                           Options options = NoOption) const;

    static AbstractMetaFunctionList filterFunctions(const AbstractMetaClass* metaClass);

    // All data about extended converters: the type entries of the target type, and a
    // list of AbstractMetaClasses accepted as argument for the conversion.
    typedef QHash<const TypeEntry*, QList<const AbstractMetaClass*> > ExtendedConverterData;
    /// Returns all extended conversions for the current module.
    ExtendedConverterData getExtendedConverters() const;

    /// Returns true if the Python wrapper for the received OverloadData must accept a list of arguments.
    static bool pythonFunctionWrapperUsesListOfArguments(const OverloadData& overloadData);

    Indentor INDENT;
private:
    bool m_useCtorHeuristic;
    bool m_userReturnValueHeuristic;
    bool m_usePySideExtensions;
    bool m_verboseErrorMessagesDisabled;
};


#endif // SHIBOKENGENERATOR_H

