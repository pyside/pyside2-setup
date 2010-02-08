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

#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include "shibokengenerator.h"
#include "overloaddata.h"

/**
 *   The CppGenerator generate the implementations of C++ bindings classes.
 */
class CppGenerator : public ShibokenGenerator
{
public:
    CppGenerator();
    void clazclasses(QString arg1);
    void AbstractMeclasses(QString arg1);
protected:
    QString fileNameForClass(const AbstractMetaClass* metaClass) const;
    QList<AbstractMetaFunctionList> filterGroupedOperatorFunctions(const AbstractMetaClass* metaClass,
                                                                   uint query);
    void generateClass(QTextStream& s, const AbstractMetaClass* metaClass);
    void finishGeneration();

private:
    void writeConstructorNative(QTextStream& s, const AbstractMetaFunction* func);
    void writeDestructorNative(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeVirtualMethodNative(QTextStream& s, const AbstractMetaFunction* func);
    void writeMetaObjectMethod(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeConstructorWrapper(QTextStream &s, const AbstractMetaFunctionList overloads);
    void writeDestructorWrapper(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaType* type);
    void writeMethodWrapper(QTextStream &s, const AbstractMetaFunctionList overloads);
    void writeArgumentsInitializer(QTextStream& s, OverloadData& overloadData);
    void writeCppSelfDefinition(QTextStream& s, const AbstractMetaFunction* func);

    void writeErrorSection(QTextStream& s, OverloadData& overloadData);
    /**
     *   Writes the check section for the validity of wrapped C++ objects.
     *   \param s text stream to write
     *   \param argName Python argument name
     *   \param type the TypeEntry passed when the validity check must confirm the type of the Python wrapper to be checked
     */
    void writeInvalidCppObjectCheck(QTextStream& s, QString pyArgName = "self", const TypeEntry* type = 0);
    void writeTypeCheck(QTextStream& s, const AbstractMetaType* argType, QString argumentName, QString customCheck = "");
    void writeTypeCheck(QTextStream& s, const OverloadData* overloadData, QString argumentName);

    void writeTypeConverterImpl(QTextStream& s, const TypeEntry* type);
    void writeTypeNameFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeSbkCopyCppObjectFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeGetattroFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    /**
     *   Writes Python to C++ conversions for arguments on Python wrappers.
     *   If implicit conversions, and thus new object allocation, are needed,
     *   code to deallocate a possible new instance is also generated.
     *   \param s text stream to write
     *   \param metatype a pointer to the argument type to be converted
     *   \param context the current meta class
     *   \param argName C++ argument name
     *   \param argName Python argument name
     */
    void writeArgumentConversion(QTextStream& s, const AbstractMetaType* argType,
                                 QString argName, QString pyArgName,
                                 const AbstractMetaClass* context = 0);
    /// Convenience method to call writeArgumentConversion with an AbstractMetaArgument
    /// instead of an AbstractMetaType.
    void writeArgumentConversion(QTextStream& s, const AbstractMetaArgument* arg,
                                 QString argName, QString pyArgName,
                                 const AbstractMetaClass* context = 0)
    {
        writeArgumentConversion(s, arg->type(), argName, pyArgName, context);
    }

    /**
     *   Set the Python method wrapper return value variable to Py_None if
     *   there are return types different from void in any of the other overloads
     *   for the function passed as parameter.
     *   \param s text stream to write
     *   \param func a pointer to the function that will possibly return Py_None
     *   \param thereIsReturnValue indicates if the return type of any of the other overloads
     *                             for this function is different from 'void'
     */
    void writeNoneReturn(QTextStream& s, const AbstractMetaFunction* func, bool thereIsReturnValue);

    /**
     *   Writes the Python method wrapper overload decisor that selects which C++
     *   method/function to call with the received Python arguments.
     *   \param s text stream to write
     *   \param parentOverloadData a pointer to overload data describing the argument being evaluated
     */
    void writeOverloadedMethodDecisor(QTextStream& s, OverloadData* parentOverloadData);
    /// Returns a string containing the name of an argument for the given function and argument index.
    QString argumentNameFromIndex(const AbstractMetaFunction* func, int argIndex, const AbstractMetaClass** wrappedClass);
    void writeMethodCall(QTextStream& s, const AbstractMetaFunction* func, int maxArgs = 0);

    void writeClassRegister(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeClassDefinition(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMethodDefinitionEntry(QTextStream& s, const AbstractMetaFunctionList overloads);
    void writeMethodDefinition(QTextStream& s, const AbstractMetaFunctionList overloads);
    /// Writes the implementation of all methods part of python sequence protocol
    void writeSequenceMethods(QTextStream& s, const AbstractMetaClass* metaClass);
    /// Writes the struct PySequenceMethods for types thats supports the python sequence protocol
    void writeTypeAsSequenceDefinition(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeTypeAsNumberDefinition(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeGetterFunction(QTextStream& s, const AbstractMetaField* metaField);
    void writeSetterFunction(QTextStream& s, const AbstractMetaField* metaField);

    void writeRichCompareFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeFlagsNewMethod(QTextStream& s, const FlagsTypeEntry* cppFlags);
    void writeEnumDefinition(QTextStream& s, const AbstractMetaEnum* metaEnum);
    void writeEnumInitialization(QTextStream& s, const AbstractMetaEnum* metaEnum);

    void writeFlagsDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum);
    void writeFlagsMethods(QTextStream& s, const AbstractMetaEnum* cppEnum);
    void writeFlagsNumberMethodsDefinition(QTextStream& s, const AbstractMetaEnum* cppEnum);
    void writeFlagsBinaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                  QString pyOpName, QString cppOpName);
    void writeFlagsInplaceOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                   QString pyOpName, QString cppOpName);
    void writeFlagsUnaryOperator(QTextStream& s, const AbstractMetaEnum* cppEnum,
                                 QString pyOpName, QString cppOpName, bool boolResult = false);

    /// Writes the function that registers the multiple inheritance information for the classes that need it.
    void writeMultipleInheritanceInitializerFunction(QTextStream& s, const AbstractMetaClass* metaClass);
    /// Writes the implementation of special cast functions, used when we need to cast a class with mulltiple inheritance.
    void writeSpecialCastFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeParentChildManagement(QTextStream& s, const AbstractMetaFunction* func);
    void writeReturnValueHeuristics(QTextStream& s, const AbstractMetaFunction* func, const QString& self = "self");
    /**
     *   Returns the multiple inheritance initializer function for the given class.
     *   \param metaClass the class for whom the function name must be generated.
     *   \return name of the multiple inheritance information initializer function or
     *           an empty string if there is no multiple inheritance in its ancestry.
     */
    QString multipleInheritanceInitializerFunctionName(const AbstractMetaClass* metaClass);

    /// Returns a list of all classes to which the given class could be casted.
    QStringList getAncestorMultipleInheritance(const AbstractMetaClass* metaClass);

    /// Returns true if the given class supports the python sequence protocol
    bool supportsSequenceProtocol(const AbstractMetaClass* metaClass);

    /// Returns true if generator should produce getters and setters for the given class.
    bool shouldGenerateGetSetList(const AbstractMetaClass* metaClass);

    // Maps special function names to function parameters and return types
    // used by CPython API in the sequence protocol.
    QHash<QString, QPair<QString, QString> > m_sequenceProtocol;
    int m_currentErrorCode;
};

#endif // CPPGENERATOR_H

