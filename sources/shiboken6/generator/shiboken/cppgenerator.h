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

#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include "shibokengenerator.h"

/**
 *   The CppGenerator generate the implementations of C++ bindings classes.
 */
class CppGenerator : public ShibokenGenerator
{
public:
    CppGenerator();

    const char *name() const override { return "Source generator"; }

protected:
    QString fileNameSuffix() const override;
    QString fileNameForContext(const GeneratorContext &context) const override;
    static QVector<AbstractMetaFunctionList>
        filterGroupedOperatorFunctions(const AbstractMetaClass *metaClass, uint query);
    void generateClass(TextStream &s, const GeneratorContext &classContext) override;
    bool finishGeneration() override;

private:
    void writeInitFunc(TextStream &declStr, TextStream &callStr,
                       const QString &initFunctionName,
                       const TypeEntry *enclosingEntry = nullptr) const;
    void writeCacheResetNative(TextStream &s, const GeneratorContext &classContext) const;
    void writeConstructorNative(TextStream &s, const GeneratorContext &classContext,
                                const AbstractMetaFunction *func) const;
    void writeDestructorNative(TextStream &s, const GeneratorContext &classContext) const;

    QString getVirtualFunctionReturnTypeName(const AbstractMetaFunction *func) const;
    void writeVirtualMethodNative(TextStream &s, const AbstractMetaFunction *func,
                                  int cacheIndex) const;
    void writeVirtualMethodCppCall(TextStream &s, const AbstractMetaFunction *func,
                                   const QString &funcName, const CodeSnipList &snips,
                                   const AbstractMetaArgument *lastArg, const TypeEntry *retType,
                                   const QString &returnStatement) const;
    QString virtualMethodReturn(TextStream &s,
                                const AbstractMetaFunction *func,
                                const FunctionModificationList &functionModifications) const;
    void writeMetaObjectMethod(TextStream &s, const GeneratorContext &classContext) const;
    void writeMetaCast(TextStream &s, const GeneratorContext &classContext) const;

    void writeEnumConverterFunctions(TextStream &s, const TypeEntry *enumType) const;
    void writeEnumConverterFunctions(TextStream &s, const AbstractMetaEnum &metaEnum) const;
    void writeConverterFunctions(TextStream &s, const AbstractMetaClass *metaClass,
                                 const GeneratorContext &classContext) const;
    void writeCustomConverterFunctions(TextStream &s,
                                       const CustomConversion *customConversion) const;
    void writeConverterRegister(TextStream &s, const AbstractMetaClass *metaClass,
                                const GeneratorContext &classContext) const;
    void writeCustomConverterRegister(TextStream &s, const CustomConversion *customConversion,
                                      const QString &converterVar) const;

    void writeContainerConverterFunctions(TextStream &s,
                                          const AbstractMetaType &containerType) const;

    void writeSmartPointerConverterFunctions(TextStream &s,
                                             const AbstractMetaType &smartPointerType) const;

    void writeMethodWrapperPreamble(TextStream &s, OverloadData &overloadData,
                                    const GeneratorContext &context) const;
    void writeConstructorWrapper(TextStream &s, const AbstractMetaFunctionList &overloads,
                                 const GeneratorContext &classContext) const;
    void writeMethodWrapper(TextStream &s, const AbstractMetaFunctionList &overloads,
                            const GeneratorContext &classContext) const;
    void writeArgumentsInitializer(TextStream &s, OverloadData &overloadData) const;
    void writeCppSelfConversion(TextStream &s, const GeneratorContext &context,
                                const QString &className, bool useWrapperClass) const;
    void writeCppSelfDefinition(TextStream &s,
                                const AbstractMetaFunction *func,
                                const GeneratorContext &context,
                                bool hasStaticOverload = false) const;
    void writeCppSelfDefinition(TextStream &s,
                                const GeneratorContext &context,
                                bool hasStaticOverload = false,
                                bool cppSelfAsReference = false) const;

    void writeErrorSection(TextStream &s, OverloadData &overloadData) const;
    static void writeFunctionReturnErrorCheckSection(TextStream &s, bool hasReturnValue = true);

    /// Writes the check section for the validity of wrapped C++ objects.
    static void writeInvalidPyObjectCheck(TextStream &s, const QString &pyObj);

    void writeTypeCheck(TextStream &s, AbstractMetaType argType, const QString &argumentName,
                        bool isNumber = false, const QString &customType = QString(),
                        bool rejectNull = false) const;
    void writeTypeCheck(TextStream& s, const OverloadData *overloadData,
                        QString argumentName) const;

    static void writeTypeDiscoveryFunction(TextStream &s, const AbstractMetaClass *metaClass);

    void writeSetattroDefinition(TextStream &s, const AbstractMetaClass *metaClass) const;
    static void writeSetattroDefaultReturn(TextStream &s);
    void writeSmartPointerSetattroFunction(TextStream &s, const GeneratorContext &context) const;
    void writeSetattroFunction(TextStream &s, AttroCheck attroCheck,
                               const GeneratorContext &context) const;
    static void writeGetattroDefinition(TextStream &s, const AbstractMetaClass *metaClass);
    static void writeSmartPointerGetattroFunction(TextStream &s, const GeneratorContext &context);
    void writeGetattroFunction(TextStream &s, AttroCheck attroCheck,
                               const GeneratorContext &context) const;
    static QString writeSmartPointerGetterCast();
    QString qObjectGetAttroFunction() const;

    /**
     *   Writes Python to C++ conversions for arguments on Python wrappers.
     *   If implicit conversions, and thus new object allocation, are needed,
     *   code to deallocate a possible new instance is also generated.
     *   \param s                    text stream to write
     *   \param argType              a pointer to the argument type to be converted
     *   \param argName              C++ argument name
     *   \param pyArgName            Python argument name
     *   \param context              the current meta class
     *   \param defaultValue         an optional default value to be used instead of the conversion result
     *   \param castArgumentAsUnused if true the converted argument is cast as unused to avoid compiler warnings
     */
    void writeArgumentConversion(TextStream &s, const AbstractMetaType &argType,
                                 const QString &argName, const QString &pyArgName,
                                 const AbstractMetaClass *context = nullptr,
                                 const QString &defaultValue = QString(),
                                 bool castArgumentAsUnused = false) const;

    /**
     *  Returns the AbstractMetaType for a function argument.
     *  If the argument type was modified in the type system, this method will
     *  try to build a new type based on the type name defined in the type system.
     *  \param  func    The function which owns the argument.
     *  \param  argPos  Argument position in the function signature.
     *                  Note that the position 0 represents the return value, and the function
     *                  parameters start counting on 1.
     *  \param  newType It is set to true if the type returned is a new object that must be deallocated.
     *  \return The type of the argument indicated by \p argPos.
     */
    std::optional<AbstractMetaType>
        getArgumentType(const AbstractMetaFunction *func, int argPos) const;

    void writePythonToCppTypeConversion(TextStream &s,
                                        const AbstractMetaType &type,
                                        const QString &pyIn,
                                        const QString &cppOut,
                                        const AbstractMetaClass *context = nullptr,
                                        const QString &defaultValue = QString()) const;

    /// Writes the conversion rule for arguments of regular and virtual methods.
    void writeConversionRule(TextStream &s, const AbstractMetaFunction *func, TypeSystem::Language language) const;
    /// Writes the conversion rule for the return value of a method.
    void writeConversionRule(TextStream &s, const AbstractMetaFunction *func, TypeSystem::Language language,
                             const QString &outputVar) const;

    /**
     *   Set the Python method wrapper return value variable to Py_None if
     *   there are return types different from void in any of the other overloads
     *   for the function passed as parameter.
     *   \param s text stream to write
     *   \param func a pointer to the function that will possibly return Py_None
     *   \param thereIsReturnValue indicates if the return type of any of the other overloads
     *                             for this function is different from 'void'
     */
    static void writeNoneReturn(TextStream &s, const AbstractMetaFunction *func, bool thereIsReturnValue);

    /**
     *   Writes the Python function wrapper overload decisor that selects which C++
     *   method/function to call with the received Python arguments.
     *   \param s text stream to write
     *   \param overloadData the overload data describing all the possible overloads for the function/method
     */
    void writeOverloadedFunctionDecisor(TextStream &s, const OverloadData &overloadData) const;
    /// Recursive auxiliar method to the other writeOverloadedFunctionDecisor.
    void writeOverloadedFunctionDecisorEngine(TextStream &s,
                                              const OverloadData *parentOverloadData) const;

    /// Writes calls to all the possible method/function overloads.
    void writeFunctionCalls(TextStream &s,
                            const OverloadData &overloadData,
                            const GeneratorContext &context) const;

    /// Writes the call to a single function usually from a collection of overloads.
    void writeSingleFunctionCall(TextStream &s,
                                 const OverloadData &overloadData,
                                 const AbstractMetaFunction *func,
                                 const GeneratorContext &context) const;

    /// Returns the name of a C++ to Python conversion function.
    static QString cppToPythonFunctionName(const QString &sourceTypeName, QString targetTypeName = QString());

    /// Returns the name of a Python to C++ conversion function.
    static QString pythonToCppFunctionName(const QString &sourceTypeName, const QString &targetTypeName);
    static QString pythonToCppFunctionName(const AbstractMetaType &sourceType, const AbstractMetaType &targetType);
    static QString pythonToCppFunctionName(const CustomConversion::TargetToNativeConversion *toNative, const TypeEntry *targetType);

    /// Returns the name of a Python to C++ convertible check function.
    static QString convertibleToCppFunctionName(const QString &sourceTypeName, const QString &targetTypeName);
    static QString convertibleToCppFunctionName(const AbstractMetaType &sourceType, const AbstractMetaType &targetType);
    static QString convertibleToCppFunctionName(const CustomConversion::TargetToNativeConversion *toNative, const TypeEntry *targetType);

    /// Writes a C++ to Python conversion function.
    void writeCppToPythonFunction(TextStream &s, const QString &code, const QString &sourceTypeName,
                                  QString targetTypeName = QString()) const;
    void writeCppToPythonFunction(TextStream &s, const CustomConversion *customConversion) const;
    void writeCppToPythonFunction(TextStream &s, const AbstractMetaType &containerType) const;

    /// Writes a Python to C++ conversion function.
    void writePythonToCppFunction(TextStream &s, const QString &code, const QString &sourceTypeName,
                                  const QString &targetTypeName) const;

    /// Writes a Python to C++ convertible check function.
    void writeIsPythonConvertibleToCppFunction(TextStream &s,
                                               const QString &sourceTypeName,
                                               const QString &targetTypeName,
                                               const QString &condition,
                                               QString pythonToCppFuncName = QString(),
                                               bool acceptNoneAsCppNull = false) const;

    /// Writes a pair of Python to C++ conversion and check functions.
    void writePythonToCppConversionFunctions(TextStream &s,
                                             const AbstractMetaType &sourceType,
                                             const AbstractMetaType &targetType,
                                             QString typeCheck = QString(),
                                             QString conversion = QString(),
                                             const QString &preConversion = QString()) const;
    /// Writes a pair of Python to C++ conversion and check functions for implicit conversions.
    void writePythonToCppConversionFunctions(TextStream &s,
                                             const CustomConversion::TargetToNativeConversion *toNative,
                                             const TypeEntry *targetType) const;

    /// Writes a pair of Python to C++ conversion and check functions for instantiated container types.
    void writePythonToCppConversionFunctions(TextStream &s,
                                             const AbstractMetaType &containerType) const;

    void writeAddPythonToCppConversion(TextStream &s, const QString &converterVar,
                                       const QString &pythonToCppFunc,
                                       const QString &isConvertibleFunc) const;

    void writeNamedArgumentResolution(TextStream &s, const AbstractMetaFunction *func,
                                      bool usePyArgs, const OverloadData &overloadData) const;

    /// Returns a string containing the name of an argument for the given function and argument index.
    QString argumentNameFromIndex(const AbstractMetaFunction *func, int argIndex,
                                  const AbstractMetaClass **wrappedClass,
                                  QString *errorMessage = nullptr) const;
    void writeMethodCall(TextStream &s, const AbstractMetaFunction *func,
                         const GeneratorContext &context, int maxArgs = 0) const;

    QString getInitFunctionName(const GeneratorContext &context) const;
    QString getSimpleClassInitFunctionName(const AbstractMetaClass *metaClass) const;

    void writeSignatureStrings(TextStream &s, const QString &signatures,
                               const QString &arrayName,
                               const char *comment) const;
    void writeClassRegister(TextStream &s,
                            const AbstractMetaClass *metaClass,
                            const GeneratorContext &classContext,
                            const QString &signatures) const;
    void writeClassDefinition(TextStream &s,
                              const AbstractMetaClass *metaClass,
                              const GeneratorContext &classContext);
    void writeMethodDefinitionEntry(TextStream &s, const AbstractMetaFunctionList &overloads) const;
    void writeMethodDefinition(TextStream &s, const AbstractMetaFunctionList &overloads) const;
    void writeSignatureInfo(TextStream &s, const AbstractMetaFunctionList &overloads) const;
    /// Writes the implementation of all methods part of python sequence protocol
    void writeSequenceMethods(TextStream &s,
                              const AbstractMetaClass *metaClass,
                              const GeneratorContext &context) const;
    void writeTypeAsSequenceDefinition(TextStream &s, const AbstractMetaClass *metaClass) const;

    /// Writes the PyMappingMethods structure for types that supports the python mapping protocol.
    void writeTypeAsMappingDefinition(TextStream &s, const AbstractMetaClass *metaClass) const;
    void writeMappingMethods(TextStream &s,
                             const AbstractMetaClass *metaClass,
                             const GeneratorContext &context) const;

    void writeTypeAsNumberDefinition(TextStream &s, const AbstractMetaClass *metaClass) const;

    void writeTpTraverseFunction(TextStream &s, const AbstractMetaClass *metaClass) const;
    void writeTpClearFunction(TextStream &s, const AbstractMetaClass *metaClass) const;

    void writeCopyFunction(TextStream &s, const GeneratorContext &context) const;

    void writeGetterFunction(TextStream &s,
                             const AbstractMetaField &metaField,
                             const GeneratorContext &context) const;
    void writeGetterFunction(TextStream &s,
                             const QPropertySpec &property,
                             const GeneratorContext &context) const;
    void writeSetterFunctionPreamble(TextStream &s,
                                     const QString &name,
                                     const QString &funcName,
                                     const AbstractMetaType &type,
                                     const GeneratorContext &context) const;
    void writeSetterFunction(TextStream &s,
                             const AbstractMetaField &metaField,
                             const GeneratorContext &context) const;
    void writeSetterFunction(TextStream &s,
                             const QPropertySpec &property,
                             const GeneratorContext &context) const;

    void writeRichCompareFunction(TextStream &s, const GeneratorContext &context) const;

    void writeEnumsInitialization(TextStream &s, AbstractMetaEnumList &enums) const;
    void writeEnumInitialization(TextStream &s, const AbstractMetaEnum &metaEnum) const;

    static void writeSignalInitialization(TextStream &s, const AbstractMetaClass *metaClass);

    void writeFlagsMethods(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeFlagsToLong(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeFlagsNonZero(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeFlagsNumberMethodsDefinition(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeFlagsNumberMethodsDefinitions(TextStream &s, const AbstractMetaEnumList &enums) const;
    void writeFlagsBinaryOperator(TextStream &s, const AbstractMetaEnum &cppEnum,
                                  const QString &pyOpName, const QString &cppOpName) const;
    void writeFlagsUnaryOperator(TextStream &s, const AbstractMetaEnum &cppEnum,
                                 const QString &pyOpName, const QString &cppOpName,
                                 bool boolResult = false) const;

    /// Writes the function that registers the multiple inheritance information for the classes that need it.
    static void writeMultipleInheritanceInitializerFunction(TextStream &s, const AbstractMetaClass *metaClass);
    /// Writes the implementation of special cast functions, used when we need to cast a class with multiple inheritance.
    static void writeSpecialCastFunction(TextStream &s, const AbstractMetaClass *metaClass);

    void writePrimitiveConverterInitialization(TextStream &s,
                                               const CustomConversion *customConversion) const;
    void writeEnumConverterInitialization(TextStream &s, const TypeEntry *enumType) const;
    void writeEnumConverterInitialization(TextStream &s, const AbstractMetaEnum &metaEnum) const;
    void writeContainerConverterInitialization(TextStream &s, const AbstractMetaType &type) const;
    void writeSmartPointerConverterInitialization(TextStream &s, const AbstractMetaType &ype) const;
    void writeExtendedConverterInitialization(TextStream &s, const TypeEntry *externalType,
                                              const QVector<const AbstractMetaClass *>& conversions) const;

    void writeParentChildManagement(TextStream &s, const AbstractMetaFunction *func, bool userHeuristicForReturn) const;
    bool writeParentChildManagement(TextStream &s, const AbstractMetaFunction *func, int argIndex, bool userHeuristicPolicy) const;
    void writeReturnValueHeuristics(TextStream &s, const AbstractMetaFunction *func) const;
    void writeInitQtMetaTypeFunctionBody(TextStream &s, const GeneratorContext &context) const;

    /**
     *   Returns the multiple inheritance initializer function for the given class.
     *   \param metaClass the class for whom the function name must be generated.
     *   \return name of the multiple inheritance information initializer function or
     *           an empty string if there is no multiple inheritance in its ancestry.
     */
    static QString multipleInheritanceInitializerFunctionName(const AbstractMetaClass *metaClass);

    /// Returns a list of all classes to which the given class could be cast.
    static QStringList getAncestorMultipleInheritance(const AbstractMetaClass *metaClass);

    /// Returns true if the given class supports the python number protocol
    bool supportsNumberProtocol(const AbstractMetaClass *metaClass) const;

    /// Returns true if the given class supports the python sequence protocol
    bool supportsSequenceProtocol(const AbstractMetaClass *metaClass) const;

    /// Returns true if the given class supports the python mapping protocol
    bool supportsMappingProtocol(const AbstractMetaClass *metaClass) const;

    /// Returns true if generator should produce getters and setters for the given class.
    bool shouldGenerateGetSetList(const AbstractMetaClass *metaClass) const;

    void writeHashFunction(TextStream &s, const GeneratorContext &context) const;

    /// Write default implementations for sequence protocol
    void writeDefaultSequenceMethods(TextStream &s, const GeneratorContext &context) const;
    /// Helper function for writeStdListWrapperMethods.
    static void writeIndexError(TextStream &s, const QString &errorMsg);

    QString writeReprFunction(TextStream &s, const GeneratorContext &context,
                              uint indirections) const;

    const AbstractMetaFunction *boolCast(const AbstractMetaClass *metaClass) const;
    bool hasBoolCast(const AbstractMetaClass *metaClass) const
    { return boolCast(metaClass) != nullptr; }

    std::optional<AbstractMetaType>
        findSmartPointerInstantiation(const TypeEntry *entry) const;

    // Number protocol structure members names.
    static QHash<QString, QString> m_nbFuncs;

    // Maps special function names to function parameters and return types
    // used by CPython API in the sequence protocol.
    QHash<QString, QPair<QString, QString> > m_sequenceProtocol;
    // Sequence protocol structure members names.
    static QHash<QString, QString> m_sqFuncs;

    // Maps special function names to function parameters and return types
    // used by CPython API in the mapping protocol.
    QHash<QString, QPair<QString, QString> > m_mappingProtocol;
    // Mapping protocol structure members names.
    static QHash<QString, QString> m_mpFuncs;

    static QString m_currentErrorCode;

    /// Helper class to set and restore the current error code.
    class ErrorCode {
    public:
        explicit ErrorCode(QString errorCode) {
            m_savedErrorCode = CppGenerator::m_currentErrorCode;
            CppGenerator::m_currentErrorCode = errorCode;
        }
        explicit ErrorCode(int errorCode) {
            m_savedErrorCode = CppGenerator::m_currentErrorCode;
            CppGenerator::m_currentErrorCode = QString::number(errorCode);
        }
        ~ErrorCode() {
            CppGenerator::m_currentErrorCode = m_savedErrorCode;
        }
    private:
        QString m_savedErrorCode;
    };
};

#endif // CPPGENERATOR_H
