/*
 * This file is part of the Shiboken Python Binding Generator project.
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

#include <apiextractor/generator.h>
#include <QtCore/QTextStream>

class DocParser;

/**
 * Abstract generator that contains common methods used in CppGenerator and HeaderGenerator.
 */
class ShibokenGenerator : public Generator
{
public:
    ShibokenGenerator();

    /**
     *   Translate metatypes to C++ format.
     *   \param metaType a pointer to metatype
     *   \param context the current meta class
     *   \param option some extra options
     *   \return the metatype translated to C++ format
     */
    virtual QString translateType(const AbstractMetaType* metaType,
                                  const AbstractMetaClass* context,
                                  int option = NoOption) const;
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
                       uint options = 0) const;
    /**
     *   Create a QString in the C++ format to an function argument.
     *   \param func the current metafunction.
     *   \param argument metaargument information to be parsed.
     *   \param options some extra options.
     */
    QString argumentString(const AbstractMetaFunction* func,
                           const AbstractMetaArgument* argument,
                           uint options = 0) const;

    void writeArgumentNames(QTextStream &s,
                            const AbstractMetaFunction* func,
                            uint options = 0) const;

    /**
     *   Function used to write the fucntion arguments on the class buffer.
     *   \param s the class output buffer
     *   \param func the pointer to metafunction information
     *   \param count the number of function arguments
     *   \param options some extra options used during the parser
     */
    void writeFunctionArguments(QTextStream &s,
                                const AbstractMetaFunction* func,
                                uint options = 0) const;
    QString functionReturnType(const AbstractMetaFunction* func, int option = NoOption) const;
    /**
     *   Write a code snip into the buffer \p s.
     *   CodeSnip are codes inside inject-code tags.
     *   \param s    the buffer
     *   \param func the cpp function
     *   \param code_snips   a list of code snips
     *   \param position     the position to insert the code snip
     *   \param language     the kind of code snip
     */
    void writeCodeSnips(QTextStream &s,
                        const CodeSnipList &code_snips,
                        CodeSnip::Position position,
                        TypeSystem::Language language,
                        const AbstractMetaFunction* func = 0);
    /// returns the code snips of a function
    CodeSnipList getCodeSnips(const AbstractMetaFunction* func);
    static bool canCreateWrapperFor(const AbstractMetaClass* metaClass);
    /**
     *   Function witch parse the metafunction information
     *   \param func the function witch will be parserd
     *   \param option some extra options
     *   \param arg_count the number of function arguments
     */
    QString functionSignature(const AbstractMetaFunction* func,
                              QString prepend = "",
                              QString append = "",
                              int option = NoOption,
                              int arg_count = -1) const;

    QString signatureForDefaultVirtualMethod(const AbstractMetaFunction* func,
                                             QString prepend = "",
                                             QString append = "_default",
                                             int option = NoOption,
                                             int arg_count = -1) const;

    virtual QString subDirectoryForClass(const AbstractMetaClass* metaClass) const
    {
        return subDirectoryForPackage(metaClass->package());
    }

    bool hasInjectedCodeOrSignatureModification(const AbstractMetaFunction* func);
    QStringList getBaseClasses(const AbstractMetaClass* metaClass);

    QString writeBaseConversion(QTextStream& s, const AbstractMetaType* type,
                                const AbstractMetaClass* context);
    void writeToPythonConversion(QTextStream& s, const AbstractMetaType* type,
                                 const AbstractMetaClass* context, QString argumentName);
    void writeToCppConversion(QTextStream& s, const AbstractMetaType* type,
                              const AbstractMetaClass* context, QString argumentName);

    static QString wrapperName(const AbstractMetaClass* metaClass);

    static QString pythonPrimitiveTypeName(QString cppTypeName);
    static QString pythonPrimitiveTypeName(const PrimitiveTypeEntry* type);

    static QString pythonOperatorFunctionName(QString cppOpFuncName)
    {
        return QString("__%1__").arg(m_pythonOperators.value(cppOpFuncName));
    }
    static QString pythonOperatorFunctionName(const AbstractMetaFunction* func);
    static QString pythonRichCompareOperatorId(QString cppOpFuncName)
    {
        return QString("Py_%1").arg(m_pythonOperators.value(cppOpFuncName).toUpper());
    }
    static QString pythonRichCompareOperatorId(const AbstractMetaFunction* func)
    {
        return pythonRichCompareOperatorId(func->originalName());
    }

    static QString cpythonOperatorFunctionName(const AbstractMetaFunction* func);

    static bool isNumber(QString cpythonApiName);
    static bool isNumber(const TypeEntry* type);
    static bool isNumber(const AbstractMetaType* type);
    static bool isPyInt(const TypeEntry* type);
    static bool isPyInt(const AbstractMetaType* type);
    static bool isReverseOperator(const AbstractMetaFunction* func);

    QString cpythonBaseName(const TypeEntry* type);
    QString cpythonBaseName(const AbstractMetaType* type);
    QString cpythonTypeName(const TypeEntry* type);
    QString cpythonCheckFunction(const TypeEntry* type, bool genericNumberType = false);
    QString cpythonCheckFunction(const AbstractMetaType* type, bool genericNumberType = false);
    QString cpythonFunctionName(const AbstractMetaFunction* func);
    QString cpythonEnumName(const EnumTypeEntry* enumEntry);
    QString cpythonEnumName(const AbstractMetaEnum* metaEnum)
    {
        return cpythonEnumName(metaEnum->typeEntry());
    }

    QString getFunctionReturnType(const AbstractMetaFunction* func) const;
    QString getFormatUnitString(const AbstractMetaArgumentList arguments) const;

    virtual bool prepareGeneration(const QMap<QString, QString>& args)
    {
        return true;
    }

protected:
    bool m_native_jump_table;
    static QHash<QString, QString> m_pythonPrimitiveTypeName;
    static QHash<QString, QString> m_pythonOperators;
    static QHash<QString, QString> m_formatUnits;

    const char* name() const { return "Shiboken"; }

    /**
     *   Initialize correspondences between primitive and Python types
     */
    static void initPrimitiveTypesCorrespondences();

    static QString retvalVariableName() { return QString("cpp_result"); }

    static FunctionModificationList functionModifications(const AbstractMetaFunction* func);
    AbstractMetaFunctionList queryFunctions(const AbstractMetaClass* metaClass, bool allFunction = false);
    void writeFunctionCall(QTextStream& s,
                           const AbstractMetaFunction* metaFunc,
                           uint options = 0) const;

    AbstractMetaFunctionList filterFunctions(const AbstractMetaClass* metaClass);
    AbstractMetaFunctionList queryGlobalOperators(const AbstractMetaClass* metaClass);
    AbstractMetaFunctionList sortContructor(AbstractMetaFunctionList list);
};


#endif // SHIBOKENGENERATOR_H

