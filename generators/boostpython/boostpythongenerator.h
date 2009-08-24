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

#ifndef BOOSTPYTHONGENERATOR_H
#define BOOSTPYTHONGENERATOR_H

#include <QtCore/QTextStream>
#include "generator.h"

class DocParser;

/**
* Abstract generator that contains common methods used in CppGenerator and HppGenerator.
*/
class BoostPythonGenerator : public Generator
{
public:
    /**
    *   Write a function argument in the boost::python format in the text stream \p s.
    *   This function just call \code s << argumentString(); \endcode
    *   \param s text stream used to write the output.
    *   \param boost_fuction the current metafunction.
    *   \param boost_argument metaargument information to be parsed.
    *   \param options some extra options.
    */
    void writeArgument(QTextStream &s,
                       const AbstractMetaFunction *boost_function,
                       const AbstractMetaArgument *boost_argument,
                       uint options = 0) const;
    /**
    *   Create a QString in the boost::python format to an function argument.
    *   \param boost_fuction the current metafunction.
    *   \param boost_argument metaargument information to be parsed.
    *   \param options some extra options.
    */
    QString argumentString(const AbstractMetaFunction *boost_function,
                           const AbstractMetaArgument *boost_argument,
                           uint options = 0) const;

    void writeArgumentNames(QTextStream &s,
                            const AbstractMetaFunction *cpp_function,
                            uint options = 0) const;

    /**
    *   Function used to write the fucntion arguments on the class buffer.
    *   \param s the class output buffer
    *   \param boost_function the pointer to metafunction information
    *   \param count the number of function arguments
    *   \param options some extra options used during the parser
    */
    void writeFunctionArguments(QTextStream &s,
                                const AbstractMetaFunction *boost_function,
                                uint options = 0) const;
    QString functionReturnType(const AbstractMetaFunction* func, int option = NoOption);
    /**
    *   Write a code snip into the buffer \p s.
    *   CodeSnip are codes inside inject-code tags.
    *   \param s    the buffer
    *   \param cpp_function the cpp function
    *   \param code_snips   a list of code snips
    *   \param position     the position to insert the code snip
    *   \param language     the kind of code snip
    */
    void writeCodeSnips(QTextStream &s,
                        const CodeSnipList &code_snips,
                        CodeSnip::Position position,
                        TypeSystem::Language language,
                        const AbstractMetaFunction *cpp_function = 0);
    static bool canCreateWrapperFor(const AbstractMetaClass* cppClass);
    /**
    *   Function witch parse the metafunction information
    *   \param cpp_function the function witch will be parserd
    *   \param option some extra options
    *   \param arg_count the number of function arguments
    */
    QString functionSignature(const AbstractMetaFunction *boost_function,
                              QString prepend = "",
                              QString append = "",
                              int option = NoOption,
                              int arg_count = -1);

    QString signatureForDefaultVirtualMethod(const AbstractMetaFunction *cpp_function,
                                             QString prepend = "",
                                             QString append = "_default",
                                             int option = NoOption,
                                             int arg_count = -1);

    virtual QString subDirectoryForClass(const AbstractMetaClass* metaClass) const
    {
        return subDirectoryForPackage(metaClass->package());
    }

    QStringList getBaseClasses(const AbstractMetaClass* cppClass);

    static QString getWrapperName(const AbstractMetaClass* clazz);


    virtual bool doSetup(const QMap<QString, QString>& args);

protected:
    // verify if the class is copyalbe
    bool isCopyable(const AbstractMetaClass *cpp_class);

    void writeFunctionCall(QTextStream &s, const AbstractMetaFunction *cpp_func, uint options = 0);
};


#endif // BOOSTPYTHONGENERATOR_H

