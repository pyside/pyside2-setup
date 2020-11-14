/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
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

#ifndef SHIBOKENGENERATOR_H
#define SHIBOKENGENERATOR_H

extern const char *CPP_ARG;
extern const char *CPP_ARG_REMOVED;
extern const char *CPP_RETURN_VAR;
extern const char *CPP_SELF_VAR;
extern const char *NULL_PTR;
extern const char *PYTHON_ARG;
extern const char *PYTHON_ARGS;
extern const char *PYTHON_OVERRIDE_VAR;
extern const char *PYTHON_RETURN_VAR;
extern const char *PYTHON_TO_CPP_VAR;
extern const char *SMART_POINTER_GETTER;

extern const char *CONV_RULE_OUT_VAR_SUFFIX;
extern const char *BEGIN_ALLOW_THREADS;
extern const char *END_ALLOW_THREADS;

#include <generator.h>

#include "typesystem.h"

#include <QtCore/QRegularExpression>

class DocParser;
class CodeSnip;
class QPropertySpec;
class OverloadData;
struct GeneratorClassInfoCacheEntry;

QT_FORWARD_DECLARE_CLASS(QTextStream)

/**
 * Abstract generator that contains common methods used in CppGenerator and HeaderGenerator.
 */
class ShibokenGenerator : public Generator
{
public:
    enum class AttroCheckFlag
    {
        None                   = 0x0,
        GetattroOverloads      = 0x01,
        GetattroSmartPointer   = 0x02,
        GetattroUser           = 0x04, // Injected code
        GetattroMask           = 0x0F,
        SetattroQObject        = 0x10,
        SetattroSmartPointer   = 0x20,
        SetattroMethodOverride = 0x40,
        SetattroUser           = 0x80, // Injected code
        SetattroMask           = 0xF0,
    };
    Q_DECLARE_FLAGS(AttroCheck, AttroCheckFlag);

    using FunctionGroups = QMap<QString, AbstractMetaFunctionList>; // Sorted

    ShibokenGenerator();
    ~ShibokenGenerator() override;

    const char *name() const override { return "Shiboken"; }

    /// Returns a list of all ancestor classes for the given class.
    AbstractMetaClassList getAllAncestors(const AbstractMetaClass *metaClass) const;

    /// Returns true if the user enabled PySide extensions.
    bool usePySideExtensions() const;

protected:
    bool doSetup() override;

    void writeArgumentNames(QTextStream &s,
                            const AbstractMetaFunction *func,
                            Options options = NoOption) const override;

    /**
     *   Function used to write the fucntion arguments on the class buffer.
     *   \param s the class output buffer
     *   \param func the pointer to metafunction information
     *   \param count the number of function arguments
     *   \param options some extra options used during the parser
     */
    void writeFunctionArguments(QTextStream &s,
                                const AbstractMetaFunction *func,
                                Options options = NoOption) const override;

    GeneratorContext contextForClass(const AbstractMetaClass *c) const override;

    /**
     *   Returns a map with all functions grouped, the function name is used as key.
     *   Example of return value: { "foo" -> ["foo(int)", "foo(int, long)], "bar" -> "bar(double)"}
     *   \param scope Where to search for functions, null means all global functions.
     */
    FunctionGroups getGlobalFunctionGroups() const;
    static FunctionGroups getFunctionGroups(const AbstractMetaClass *scope);

    /**
     *   Returns all different inherited overloads of func, and includes func as well.
     *   The function can be called multiple times without duplication.
     *   \param func the metafunction to be searched in subclasses.
     *   \param seen the function's minimal signatures already seen.
     */
    AbstractMetaFunctionList getFunctionAndInheritedOverloads(const AbstractMetaFunction *func, QSet<QString> *seen);

    /// Write user's custom code snippets at class or module level.
    void writeClassCodeSnips(QTextStream &s,
                             const QVector<CodeSnip> & codeSnips,
                             TypeSystem::CodeSnipPosition position,
                             TypeSystem::Language language,
                             const GeneratorContext &context);
    void writeCodeSnips(QTextStream &s,
                        const QVector<CodeSnip> & codeSnips,
                        TypeSystem::CodeSnipPosition position,
                        TypeSystem::Language language);
    /// Write user's custom code snippets at function level.
    void writeCodeSnips(QTextStream &s,
                        const QVector<CodeSnip> & codeSnips,
                        TypeSystem::CodeSnipPosition position,
                        TypeSystem::Language language,
                        const AbstractMetaFunction *func,
                        const AbstractMetaArgument *lastArg = nullptr);

    /// Replaces variables for the user's custom code at global or class level.
    void processCodeSnip(QString &code);
    void processClassCodeSnip(QString &code, const GeneratorContext &context);

    /**
     *   Verifies if any of the function's code injections of the "native"
     *   type needs the type system variable "%PYSELF".
     *   \param func the function to check
     *   \return true if the function's native code snippets use "%PYSELF"
     */
    bool injectedCodeUsesPySelf(const AbstractMetaFunction *func);

    /**
     *   Verifies if any of the function's code injections makes a call
     *   to the C++ method. This is used by the generator to avoid writing calls
     *   to C++ when the user custom code already does this.
     *   \param func the function to check
     *   \return true if the function's code snippets call the wrapped C++ function
     */
    bool injectedCodeCallsCppFunction(const GeneratorContext &context,
                                      const AbstractMetaFunction *func);

    /**
     *   Verifies if any of the function's code injections of the "native" class makes a
     *   call to the C++ method. This is used by the generator to avoid writing calls to
     *   Python overrides of C++ virtual methods when the user custom code already does this.
     *   \param func the function to check
     *   \return true if the function's code snippets call the Python override for a C++ virtual method
     */
    bool injectedCodeCallsPythonOverride(const AbstractMetaFunction *func);

    /**
     *   Verifies if any of the function's code injections attributes values to
     *   the return variable (%0 or %PYARG_0).
     *   \param func        the function to check
     *   \param language    the kind of code snip
     *   \return true if the function's code attributes values to "%0" or "%PYARG_0"
     */
    bool injectedCodeHasReturnValueAttribution(const AbstractMetaFunction *func, TypeSystem::Language language = TypeSystem::TargetLangCode);

    /**
     *   Verifies if any of the function's code injections uses the type system variable
     *   for function arguments of a given index.
     */
    bool injectedCodeUsesArgument(const AbstractMetaFunction *func, int argumentIndex);

    /**
     *   Function which parse the metafunction information
     *   \param func the function witch will be parserd
     *   \param option some extra options
     *   \param arg_count the number of function arguments
     */
    QString functionSignature(const AbstractMetaFunction *func,
                              const QString &prepend = QString(),
                              const QString &append = QString(),
                              Options options = NoOption,
                              int arg_count = -1) const;

    /// Returns the top-most class that has multiple inheritance in the ancestry.
    static const AbstractMetaClass *getMultipleInheritingClass(const AbstractMetaClass *metaClass);

    static bool useOverrideCaching(const AbstractMetaClass *metaClass);
    AttroCheck checkAttroFunctionNeeds(const AbstractMetaClass *metaClass) const;

    /// Returns a list of methods of the given class where each one is part of a different overload with both static and non-static method.
    AbstractMetaFunctionList getMethodsWithBothStaticAndNonStaticMethods(const AbstractMetaClass *metaClass);

    /// Returns a list of parent classes for a given class.
    AbstractMetaClassList getBaseClasses(const AbstractMetaClass *metaClass) const;

    void writeToPythonConversion(QTextStream &s, const AbstractMetaType &type,
                                 const AbstractMetaClass *context, const QString &argumentName);
    void writeToCppConversion(QTextStream &s, const AbstractMetaType &type, const AbstractMetaClass *context,
                              const QString &inArgName, const QString &outArgName);
    void writeToCppConversion(QTextStream &s, const AbstractMetaClass *metaClass, const QString &inArgName, const QString &outArgName);

    /// Returns true if the argument is a pointer that rejects nullptr values.
    bool shouldRejectNullPointerArgument(const AbstractMetaFunction *func, int argIndex);

    /// Verifies if the class should have a C++ wrapper generated for it, instead of only a Python wrapper.
    bool shouldGenerateCppWrapper(const AbstractMetaClass *metaClass) const;

    /// Condition to call WriteVirtualMethodNative. Was extracted because also used to count these calls.
    bool shouldWriteVirtualMethodNative(const AbstractMetaFunction *func);

    QString wrapperName(const AbstractMetaClass *metaClass) const;

    QString fullPythonClassName(const AbstractMetaClass *metaClass);
    QString fullPythonFunctionName(const AbstractMetaFunction *func, bool forceFunc=false);

    bool wrapperDiagnostics() const { return m_wrapperDiagnostics; }

    static QString protectedEnumSurrogateName(const AbstractMetaEnum &metaEnum);
    static QString protectedFieldGetterName(const AbstractMetaField &field);
    static QString protectedFieldSetterName(const AbstractMetaField &field);

    static QString pythonPrimitiveTypeName(const QString &cppTypeName);
    static QString pythonPrimitiveTypeName(const PrimitiveTypeEntry *type);

    static QString pythonOperatorFunctionName(const QString &cppOpFuncName);
    static QString pythonOperatorFunctionName(const AbstractMetaFunction *func);
    static QString pythonRichCompareOperatorId(const QString &cppOpFuncName);
    static QString pythonRichCompareOperatorId(const AbstractMetaFunction *func);

    static QString fixedCppTypeName(const CustomConversion::TargetToNativeConversion *toNative);
    static QString fixedCppTypeName(const AbstractMetaType &type);
    static QString fixedCppTypeName(const TypeEntry *type, QString typeName = QString());

    static bool isNumber(const QString &cpythonApiName);
    static bool isNumber(const TypeEntry *type);
    static bool isNumber(const AbstractMetaType &type);
    static bool isPyInt(const TypeEntry *type);
    static bool isPyInt(const AbstractMetaType &type);

    /**
     *  Returns true if the type passed has a Python wrapper for it.
     *  Although namespace has a Python wrapper, it's not considered a type.
     */
    static bool isWrapperType(const TypeEntry *type);
    static bool isWrapperType(const ComplexTypeEntry *type);
    static bool isWrapperType(const AbstractMetaType &metaType);

    /**
     *  Checks if the type is an Object/QObject or pointer to Value Type.
     *  In other words, tells if the type is "T*" and T has a Python wrapper.
     */
    static bool isPointerToWrapperType(const AbstractMetaType &type);

    /**
     *  Returns true if \p type is an Object Type used as a value.
     */
    static bool isObjectTypeUsedAsValueType(const AbstractMetaType &type);

    static bool isValueTypeWithCopyConstructorOnly(const AbstractMetaClass *metaClass);
    bool isValueTypeWithCopyConstructorOnly(const TypeEntry *type) const;
    bool isValueTypeWithCopyConstructorOnly(const AbstractMetaType &type) const;

    /// Returns true if the type is a primitive but not a C++ primitive.
    static bool isUserPrimitive(const TypeEntry *type);
    static bool isUserPrimitive(const AbstractMetaType &type);

    /// Returns true if the type is a C++ primitive, a void*, a const char*, or a std::string.
    static bool isCppPrimitive(const TypeEntry *type);
    static bool isCppPrimitive(const AbstractMetaType &type);

    /// Returns true if the type is a C++ integral primitive, i.e. bool, char, int, long, and their unsigned counterparts.
    static bool isCppIntegralPrimitive(const TypeEntry *type);
    static bool isCppIntegralPrimitive(const AbstractMetaType &type);

    /// Checks if an argument type should be dereferenced by the Python method wrapper before calling the C++ method.
    static bool shouldDereferenceArgumentPointer(const AbstractMetaArgument &arg);
    /// Checks if a meta type should be dereferenced by the Python method wrapper passing it to C++.
    static bool shouldDereferenceAbstractMetaTypePointer(const AbstractMetaType &metaType);

    static bool visibilityModifiedToPrivate(const AbstractMetaFunction *func);

    static bool isNullPtr(const QString &value);

    QString converterObject(const AbstractMetaType &type);
    QString converterObject(const TypeEntry *type);

    static QString cpythonBaseName(const AbstractMetaClass *metaClass);
    static QString cpythonBaseName(const TypeEntry *type);
    static QString cpythonBaseName(const AbstractMetaType &type);
    static QString cpythonTypeName(const AbstractMetaClass *metaClass);
    static QString cpythonTypeName(const TypeEntry *type);
    QString cpythonTypeNameExt(const TypeEntry *type) const;
    QString cpythonTypeNameExt(const AbstractMetaType &type) const;
    QString cpythonCheckFunction(const TypeEntry *type, bool genericNumberType = false);
    QString cpythonCheckFunction(AbstractMetaType metaType, bool genericNumberType = false);
    /**
     *  Receives the argument \p type and tries to find the appropriate AbstractMetaType for it
     *  or a custom type check.
     *  \param type     A string representing the type to be discovered.
     *  \param metaType A pointer to an AbstractMetaType pointer, to where write a new meta type object
     *                  if one is produced from the \p type string. This object must be deallocated by
     *                  the caller. It will set the target variable to nullptr, is \p type is a Python type.
     *  \return A custom check if \p type is a custom type, or an empty string if \p metaType
     *          receives an existing type object.
     */
    struct CPythonCheckFunctionResult
    {
        QString checkFunction;
        std::optional<AbstractMetaType> type;
    };
    CPythonCheckFunctionResult guessCPythonCheckFunction(const QString &type);
    QString cpythonIsConvertibleFunction(const TypeEntry *type, bool genericNumberType = false, bool checkExact = false);
    QString cpythonIsConvertibleFunction(AbstractMetaType metaType, bool genericNumberType = false);
    QString cpythonIsConvertibleFunction(const AbstractMetaArgument &metaArg, bool genericNumberType = false);

    QString cpythonToCppConversionFunction(const AbstractMetaClass *metaClass);
    QString cpythonToCppConversionFunction(const AbstractMetaType &type, const AbstractMetaClass *context = nullptr);
    QString cpythonToPythonConversionFunction(const AbstractMetaType &type, const AbstractMetaClass *context = nullptr);
    QString cpythonToPythonConversionFunction(const AbstractMetaClass *metaClass);
    QString cpythonToPythonConversionFunction(const TypeEntry *type);

    QString cpythonFunctionName(const AbstractMetaFunction *func);
    QString cpythonMethodDefinitionName(const AbstractMetaFunction *func);
    QString cpythonGettersSettersDefinitionName(const AbstractMetaClass *metaClass);
    static QString cpythonGetattroFunctionName(const AbstractMetaClass *metaClass);
    static QString cpythonSetattroFunctionName(const AbstractMetaClass *metaClass);
    static QString cpythonGetterFunctionName(const AbstractMetaField &metaField);
    static QString cpythonSetterFunctionName(const AbstractMetaField &metaField);
    static QString cpythonGetterFunctionName(const QPropertySpec &property,
                                             const AbstractMetaClass *metaClass);
    static QString cpythonSetterFunctionName(const QPropertySpec &property,
                                             const AbstractMetaClass *metaClass);
    QString cpythonWrapperCPtr(const AbstractMetaClass *metaClass,
                               const QString &argName = QLatin1String("self")) const;
    QString cpythonWrapperCPtr(const AbstractMetaType &metaType, const QString &argName) const;
    QString cpythonWrapperCPtr(const TypeEntry *type, const QString &argName) const;

    /// Guesses the scope to where belongs an argument's default value.
    QString guessScopeForDefaultValue(const AbstractMetaFunction *func,
                                      const AbstractMetaArgument &arg) const;
    QString guessScopeForDefaultFlagsValue(const AbstractMetaFunction *func,
                                           const AbstractMetaArgument &arg,
                                           const QString &value) const;

    static QString cpythonEnumName(const EnumTypeEntry *enumEntry);
    static QString cpythonEnumName(const AbstractMetaEnum &metaEnum);

    static QString cpythonFlagsName(const FlagsTypeEntry *flagsEntry);
    static QString cpythonFlagsName(const AbstractMetaEnum *metaEnum);
    /// Returns the special cast function name, the function used to proper cast class with multiple inheritance.
    static QString cpythonSpecialCastFunctionName(const AbstractMetaClass *metaClass);

    QString getFormatUnitString(const AbstractMetaFunction *func, bool incRef = false) const;

    /// Returns the file name for the module global header. If no module name is provided the current will be used.
    QString getModuleHeaderFileName(const QString &moduleName = QString()) const;

    OptionDescriptions options() const override;
    bool handleOption(const QString &key, const QString &value) override;

    /// Returns true if the user enabled the so called "parent constructor heuristic".
    bool useCtorHeuristic() const;
    /// Returns true if the user enabled the so called "return value heuristic".
    bool useReturnValueHeuristic() const;
    /// Returns true if the generator should use the result of isNull()const to compute boolean casts.
    bool useIsNullAsNbNonZero() const;
    /// Returns true if the generated code should use the "#define protected public" hack.
    bool avoidProtectedHack() const;
    QString cppApiVariableName(const QString &moduleName = QString()) const;
    QString pythonModuleObjectName(const QString &moduleName = QString()) const;
    QString convertersVariableName(const QString &moduleName = QString()) const;
    /**
     *  Returns the type index variable name for a given class. If \p alternativeTemplateName is true
     *  and the class is a typedef for a template class instantiation, it will return an alternative name
     *  made of the template class and the instantiation values, or an empty string if the class isn't
     *  derived from a template class at all.
     */
    QString getTypeIndexVariableName(const AbstractMetaClass *metaClass, bool alternativeTemplateName = false) const;
    QString getTypeIndexVariableName(const TypeEntry *type) const;
    QString getTypeIndexVariableName(const AbstractMetaType &type) const;

    /// Returns true if the user don't want verbose error messages on the generated bindings.
    bool verboseErrorMessagesDisabled() const;

    /**
     *   Builds an AbstractMetaType object from a QString.
     *   Returns nullptr if no type could be built from the string.
     *   \param typeSignature The string describing the type to be built.
     *   \return A new AbstractMetaType object that must be deleted by the caller,
     *           or a nullptr pointer in case of failure.
     */
    std::optional<AbstractMetaType>
        buildAbstractMetaTypeFromString(QString typeSignature,
                                        QString *errorMessage = nullptr);

    /// Creates an AbstractMetaType object from a TypeEntry.
    AbstractMetaType buildAbstractMetaTypeFromTypeEntry(const TypeEntry *typeEntry);
    /// Creates an AbstractMetaType object from an AbstractMetaClass.
    AbstractMetaType buildAbstractMetaTypeFromAbstractMetaClass(const AbstractMetaClass *metaClass);

    void writeMinimalConstructorExpression(QTextStream &s, const AbstractMetaType &type,
                                           const QString &defaultCtor = QString());
    void writeMinimalConstructorExpression(QTextStream &s, const TypeEntry *type, const QString &defaultCtor = QString());

    void collectContainerTypesFromConverterMacros(const QString &code, bool toPythonMacro);

    void clearTpFuncs();


    /// Initializes correspondences between primitive and Python types.
    static void initPrimitiveTypesCorrespondences();
    /// Initializes a list of Python known type names.
    static void initKnownPythonTypes();

    void writeFunctionCall(QTextStream &s,
                           const AbstractMetaFunction *metaFunc,
                           Options options = NoOption) const;

    void writeUnusedVariableCast(QTextStream &s, const QString &variableName);

    AbstractMetaFunctionList filterFunctions(const AbstractMetaClass *metaClass);

    // All data about extended converters: the type entries of the target type, and a
    // list of AbstractMetaClasses accepted as argument for the conversion.
    using ExtendedConverterData = QHash<const TypeEntry *, QVector<const AbstractMetaClass *> >;
    /// Returns all extended conversions for the current module.
    ExtendedConverterData getExtendedConverters() const;

    /// Returns a list of converters for the non wrapper types of the current module.
    QVector<const CustomConversion *> getPrimitiveCustomConversions();

    /// Returns true if the Python wrapper for the received OverloadData must accept a list of arguments.
    static bool pythonFunctionWrapperUsesListOfArguments(const OverloadData &overloadData);

    Indentor INDENT;

    const QRegularExpression &convertToCppRegEx() const
    { return m_typeSystemConvRegEx[TypeSystemToCppFunction]; }

    static QString pythonArgsAt(int i);

    static QHash<QString, QString> m_pythonPrimitiveTypeName;
    static QHash<QString, QString> m_pythonOperators;
    static QHash<QString, QString> m_formatUnits;
    static QHash<QString, QString> m_tpFuncs;
    static QStringList m_knownPythonTypes;

private:
    static QString cpythonGetterFunctionName(const QString &name,
                                             const AbstractMetaClass *enclosingClass);
    static QString cpythonSetterFunctionName(const QString &name,
                                             const AbstractMetaClass *enclosingClass);

    static const GeneratorClassInfoCacheEntry &getGeneratorClassInfo(const AbstractMetaClass *scope);
    static FunctionGroups getFunctionGroupsImpl(const AbstractMetaClass *scope);
    static bool classNeedsGetattroFunctionImpl(const AbstractMetaClass *metaClass);

    QString translateTypeForWrapperMethod(const AbstractMetaType &cType,
                                          const AbstractMetaClass *context,
                                          Options opt = NoOption) const;

    /**
     *   Returns all different inherited overloads of func.
     *   The function can be called multiple times without duplication.
     *   \param func the metafunction to be searched in subclasses.
     *   \param seen the function's minimal signatures already seen.
     */
    AbstractMetaFunctionList getInheritedOverloads(const AbstractMetaFunction *func,
                                                   QSet<QString> *seen);

    /**
     *   Returns all overloads for a function named \p functionName.
     *   \param scope scope used to search for overloads.
     *   \param functionName the function name.
     */
    AbstractMetaFunctionList getFunctionOverloads(const AbstractMetaClass *scope,
                                                  const QString &functionName);
    /**
     *   Write a function argument in the C++ in the text stream \p s.
     *   This function just call \code s << argumentString(); \endcode
     *   \param s text stream used to write the output.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    void writeArgument(QTextStream &s,
                       const AbstractMetaFunction *func,
                       const AbstractMetaArgument &argument,
                       Options options = NoOption) const;
    /**
     *   Create a QString in the C++ format to an function argument.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    QString argumentString(const AbstractMetaFunction *func,
                           const AbstractMetaArgument &argument,
                           Options options = NoOption) const;

    QString functionReturnType(const AbstractMetaFunction *func, Options options = NoOption) const;

    /// Utility function for writeCodeSnips.
    using ArgumentVarReplacementPair = QPair<AbstractMetaArgument, QString>;
    using ArgumentVarReplacementList = QVector<ArgumentVarReplacementPair>;
    ArgumentVarReplacementList getArgumentReplacement(const AbstractMetaFunction* func,
                                                      bool usePyArgs, TypeSystem::Language language,
                                                      const AbstractMetaArgument *lastArg);

    /// Returns a string with the user's custom code snippets that comply with \p position and \p language.
    QString getCodeSnippets(const QVector<CodeSnip> & codeSnips,
                            TypeSystem::CodeSnipPosition position,
                            TypeSystem::Language language);

    enum TypeSystemConverterVariable {
        TypeSystemCheckFunction = 0,
        TypeSystemIsConvertibleFunction,
        TypeSystemToCppFunction,
        TypeSystemToPythonFunction,
        TypeSystemConverterVariables
    };
    void replaceConverterTypeSystemVariable(TypeSystemConverterVariable converterVariable, QString &code);

    /// Replaces the %CONVERTTOPYTHON type system variable.
    inline void replaceConvertToPythonTypeSystemVariable(QString &code)
    {
        replaceConverterTypeSystemVariable(TypeSystemToPythonFunction, code);
    }
    /// Replaces the %CONVERTTOCPP type system variable.
    inline void replaceConvertToCppTypeSystemVariable(QString &code)
    {
        replaceConverterTypeSystemVariable(TypeSystemToCppFunction, code);
    }
    /// Replaces the %ISCONVERTIBLE type system variable.
    inline void replaceIsConvertibleToCppTypeSystemVariable(QString &code)
    {
        replaceConverterTypeSystemVariable(TypeSystemIsConvertibleFunction, code);
    }
    /// Replaces the %CHECKTYPE type system variable.
    inline void replaceTypeCheckTypeSystemVariable(QString &code)
    {
        replaceConverterTypeSystemVariable(TypeSystemCheckFunction, code);
    }

    /// Return a prefix with '_' suitable for names in C++
    QString moduleCppPrefix(const QString &moduleName = QString()) const;

    bool m_useCtorHeuristic = false;
    bool m_userReturnValueHeuristic = false;
    bool m_usePySideExtensions = false;
    bool m_verboseErrorMessagesDisabled = false;
    bool m_useIsNullAsNbNonZero = false;
    bool m_avoidProtectedHack = false;
    bool m_wrapperDiagnostics = false;

    using AbstractMetaTypeCache = QHash<QString, AbstractMetaType>;
    AbstractMetaTypeCache m_metaTypeFromStringCache;

    /// Type system converter variable replacement names and regular expressions.
    QString m_typeSystemConvName[TypeSystemConverterVariables];
    QRegularExpression m_typeSystemConvRegEx[TypeSystemConverterVariables];
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShibokenGenerator::AttroCheck);

#endif // SHIBOKENGENERATOR_H
