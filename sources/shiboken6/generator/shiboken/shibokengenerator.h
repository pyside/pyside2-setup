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
class TextStream;
struct GeneratorClassInfoCacheEntry;

QT_FORWARD_DECLARE_CLASS(TextStream)

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

    /// Returns true if the user enabled PySide extensions.
    bool usePySideExtensions() const;

protected:
    bool doSetup() override;

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
    static AbstractMetaFunctionList getFunctionAndInheritedOverloads(const AbstractMetaFunction *func,
                                                                     QSet<QString> *seen);

    /// Write user's custom code snippets at class or module level.
    void writeClassCodeSnips(TextStream &s,
                             const QVector<CodeSnip> & codeSnips,
                             TypeSystem::CodeSnipPosition position,
                             TypeSystem::Language language,
                             const GeneratorContext &context) const;
    void writeCodeSnips(TextStream &s,
                        const QVector<CodeSnip> & codeSnips,
                        TypeSystem::CodeSnipPosition position,
                        TypeSystem::Language language) const;
    /// Write user's custom code snippets at function level.
    void writeCodeSnips(TextStream &s,
                        const QVector<CodeSnip> & codeSnips,
                        TypeSystem::CodeSnipPosition position,
                        TypeSystem::Language language,
                        const AbstractMetaFunction *func,
                        const AbstractMetaArgument *lastArg = nullptr) const;

    /// Replaces variables for the user's custom code at global or class level.
    void processCodeSnip(QString &code) const;
    void processClassCodeSnip(QString &code, const GeneratorContext &context) const;

    /**
     *   Verifies if any of the function's code injections makes a call
     *   to the C++ method. This is used by the generator to avoid writing calls
     *   to C++ when the user custom code already does this.
     *   \param func the function to check
     *   \return true if the function's code snippets call the wrapped C++ function
     */
    static bool injectedCodeCallsCppFunction(const GeneratorContext &context,
                                             const AbstractMetaFunction *func);

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
    AbstractMetaFunctionList getMethodsWithBothStaticAndNonStaticMethods(const AbstractMetaClass *metaClass) const;

    void writeToPythonConversion(TextStream &s, const AbstractMetaType &type,
                                 const AbstractMetaClass *context, const QString &argumentName) const;
    void writeToCppConversion(TextStream &s, const AbstractMetaType &type, const AbstractMetaClass *context,
                              const QString &inArgName, const QString &outArgName) const;
    void writeToCppConversion(TextStream &s, const AbstractMetaClass *metaClass, const QString &inArgName,
                              const QString &outArgName) const;

    /// Returns true if the argument is a pointer that rejects nullptr values.
    bool shouldRejectNullPointerArgument(const AbstractMetaFunction *func, int argIndex) const;

    /// Verifies if the class should have a C++ wrapper generated for it, instead of only a Python wrapper.
    bool shouldGenerateCppWrapper(const AbstractMetaClass *metaClass) const;

    /// Condition to call WriteVirtualMethodNative. Was extracted because also used to count these calls.
    bool shouldWriteVirtualMethodNative(const AbstractMetaFunction *func) const;

    QString wrapperName(const AbstractMetaClass *metaClass) const;

    static QString fullPythonClassName(const AbstractMetaClass *metaClass);
    static QString fullPythonFunctionName(const AbstractMetaFunction *func, bool forceFunc);

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

    bool isValueTypeWithCopyConstructorOnly(const TypeEntry *type) const;
    bool isValueTypeWithCopyConstructorOnly(const AbstractMetaType &type) const;

    static bool isNullPtr(const QString &value);

    QString converterObject(const AbstractMetaType &type) const;
    QString converterObject(const TypeEntry *type) const;

    static QString cpythonBaseName(const AbstractMetaClass *metaClass);
    static QString cpythonBaseName(const TypeEntry *type);
    static QString cpythonBaseName(const AbstractMetaType &type);
    static QString cpythonTypeName(const AbstractMetaClass *metaClass);
    static QString cpythonTypeName(const TypeEntry *type);
    static QString cpythonTypeNameExt(const TypeEntry *type);
    QString cpythonTypeNameExt(const AbstractMetaType &type) const;
    QString cpythonCheckFunction(const TypeEntry *type, bool genericNumberType = false) const;
    QString cpythonCheckFunction(AbstractMetaType metaType, bool genericNumberType = false) const;
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
    CPythonCheckFunctionResult guessCPythonCheckFunction(const QString &type) const;
    QString cpythonIsConvertibleFunction(const TypeEntry *type, bool genericNumberType = false,
                                                bool checkExact = false) const;
    QString cpythonIsConvertibleFunction(AbstractMetaType metaType,
                                         bool genericNumberType = false) const;
    QString cpythonIsConvertibleFunction(const AbstractMetaArgument &metaArg,
                                         bool genericNumberType = false) const;

    QString cpythonToCppConversionFunction(const AbstractMetaClass *metaClass) const;
    QString cpythonToCppConversionFunction(const AbstractMetaType &type,
                                           const AbstractMetaClass *context = nullptr) const;
    QString cpythonToPythonConversionFunction(const AbstractMetaType &type,
                                              const AbstractMetaClass *context = nullptr) const;
    QString cpythonToPythonConversionFunction(const AbstractMetaClass *metaClass) const;
    QString cpythonToPythonConversionFunction(const TypeEntry *type) const;

    QString cpythonFunctionName(const AbstractMetaFunction *func) const;
    static QString cpythonMethodDefinitionName(const AbstractMetaFunction *func);
    static QString cpythonGettersSettersDefinitionName(const AbstractMetaClass *metaClass);
    static QString cpythonGetattroFunctionName(const AbstractMetaClass *metaClass);
    static QString cpythonSetattroFunctionName(const AbstractMetaClass *metaClass);
    static QString cpythonGetterFunctionName(const AbstractMetaField &metaField);
    static QString cpythonSetterFunctionName(const AbstractMetaField &metaField);
    static QString cpythonGetterFunctionName(const QPropertySpec &property,
                                             const AbstractMetaClass *metaClass);
    static QString cpythonSetterFunctionName(const QPropertySpec &property,
                                             const AbstractMetaClass *metaClass);
    static QString cpythonWrapperCPtr(const AbstractMetaClass *metaClass,
                               const QString &argName = QLatin1String("self"));
    QString cpythonWrapperCPtr(const AbstractMetaType &metaType, const QString &argName) const;
    static QString cpythonWrapperCPtr(const TypeEntry *type, const QString &argName);

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

    static QString getFormatUnitString(const AbstractMetaFunction *func, bool incRef = false);

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
    static QString cppApiVariableName(const QString &moduleName = QString());
    static QString pythonModuleObjectName(const QString &moduleName = QString());
    static QString convertersVariableName(const QString &moduleName = QString());
    /**
     *  Returns the type index variable name for a given class. If \p alternativeTemplateName is true
     *  and the class is a typedef for a template class instantiation, it will return an alternative name
     *  made of the template class and the instantiation values, or an empty string if the class isn't
     *  derived from a template class at all.
     */
    static QString getTypeIndexVariableName(const AbstractMetaClass *metaClass,
                                            bool alternativeTemplateName = false);
    static QString getTypeIndexVariableName(const TypeEntry *type);
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
                                        QString *errorMessage = nullptr) const;

    /// Creates an AbstractMetaType object from a TypeEntry.
    AbstractMetaType buildAbstractMetaTypeFromTypeEntry(const TypeEntry *typeEntry) const;
    /// Creates an AbstractMetaType object from an AbstractMetaClass.
    AbstractMetaType buildAbstractMetaTypeFromAbstractMetaClass(const AbstractMetaClass *metaClass) const;

    void writeMinimalConstructorExpression(TextStream &s, const AbstractMetaType &type,
                                           const QString &defaultCtor = QString()) const;
    void writeMinimalConstructorExpression(TextStream &s, const TypeEntry *type,
                                           const QString &defaultCtor = QString()) const;

    void collectContainerTypesFromConverterMacros(const QString &code, bool toPythonMacro);

    void clearTpFuncs();


    /// Initializes correspondences between primitive and Python types.
    static void initPrimitiveTypesCorrespondences();
    /// Initializes a list of Python known type names.
    static void initKnownPythonTypes();

    void writeFunctionCall(TextStream &s,
                           const AbstractMetaFunction *metaFunc,
                           Options options = NoOption) const;

    void writeUnusedVariableCast(TextStream &s, const QString &variableName) const;

    AbstractMetaFunctionList filterFunctions(const AbstractMetaClass *metaClass) const;

    // All data about extended converters: the type entries of the target type, and a
    // list of AbstractMetaClasses accepted as argument for the conversion.
    using ExtendedConverterData = QHash<const TypeEntry *, QVector<const AbstractMetaClass *> >;
    /// Returns all extended conversions for the current module.
    ExtendedConverterData getExtendedConverters() const;

    /// Returns a list of converters for the non wrapper types of the current module.
    QVector<const CustomConversion *> getPrimitiveCustomConversions() const;

    /// Returns true if the Python wrapper for the received OverloadData must accept a list of arguments.
    static bool pythonFunctionWrapperUsesListOfArguments(const OverloadData &overloadData);

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
    static AbstractMetaFunctionList getInheritedOverloads(const AbstractMetaFunction *func,
                                                          QSet<QString> *seen);

    /**
     *   Returns all overloads for a function named \p functionName.
     *   \param scope scope used to search for overloads.
     *   \param functionName the function name.
     */
    AbstractMetaFunctionList getFunctionOverloads(const AbstractMetaClass *scope,
                                                  const QString &functionName) const;
    /**
     *   Write a function argument in the C++ in the text stream \p s.
     *   This function just call \code s << argumentString(); \endcode
     *   \param s text stream used to write the output.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    void writeArgument(TextStream &s,
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
                                                      const AbstractMetaArgument *lastArg) const;

    /// Returns a string with the user's custom code snippets that comply with \p position and \p language.
    QString getCodeSnippets(const QVector<CodeSnip> & codeSnips,
                            TypeSystem::CodeSnipPosition position,
                            TypeSystem::Language language) const;

    enum TypeSystemConverterVariable {
        TypeSystemCheckFunction = 0,
        TypeSystemIsConvertibleFunction,
        TypeSystemToCppFunction,
        TypeSystemToPythonFunction,
        TypeSystemConverterVariables
    };
    void replaceConverterTypeSystemVariable(TypeSystemConverterVariable converterVariable,
                                            QString &code) const;

    /// Replaces the %CONVERTTOPYTHON type system variable.
    inline void replaceConvertToPythonTypeSystemVariable(QString &code) const
    {
        replaceConverterTypeSystemVariable(TypeSystemToPythonFunction, code);
    }
    /// Replaces the %CONVERTTOCPP type system variable.
    inline void replaceConvertToCppTypeSystemVariable(QString &code) const
    {
        replaceConverterTypeSystemVariable(TypeSystemToCppFunction, code);
    }
    /// Replaces the %ISCONVERTIBLE type system variable.
    inline void replaceIsConvertibleToCppTypeSystemVariable(QString &code) const
    {
        replaceConverterTypeSystemVariable(TypeSystemIsConvertibleFunction, code);
    }
    /// Replaces the %CHECKTYPE type system variable.
    inline void replaceTypeCheckTypeSystemVariable(QString &code) const
    {
        replaceConverterTypeSystemVariable(TypeSystemCheckFunction, code);
    }

    /// Return a prefix with '_' suitable for names in C++
    static QString moduleCppPrefix(const QString &moduleName = QString());

    /// Functions used to write the function arguments on the class buffer.
    /// \param s the class output buffer
    /// \param func the pointer to metafunction information
    /// \param count the number of function arguments
    /// \param options some extra options used during the parser
    void writeArgumentNames(TextStream &s,
                            const AbstractMetaFunction *func,
                            Options options = NoOption) const;

    void writeFunctionArguments(TextStream &s,
                                const AbstractMetaFunction *func,
                                Options options = NoOption) const;

    void replaceTemplateVariables(QString &code,
                                  const AbstractMetaFunction *func) const;

    bool m_useCtorHeuristic = false;
    bool m_userReturnValueHeuristic = false;
    bool m_usePySideExtensions = false;
    bool m_verboseErrorMessagesDisabled = false;
    bool m_useIsNullAsNbNonZero = false;
    bool m_avoidProtectedHack = false;
    bool m_wrapperDiagnostics = false;

    using AbstractMetaTypeCache = QHash<QString, AbstractMetaType>;
    mutable AbstractMetaTypeCache m_metaTypeFromStringCache;

    /// Type system converter variable replacement names and regular expressions.
    QString m_typeSystemConvName[TypeSystemConverterVariables];
    QRegularExpression m_typeSystemConvRegEx[TypeSystemConverterVariables];
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShibokenGenerator::AttroCheck);

#endif // SHIBOKENGENERATOR_H
