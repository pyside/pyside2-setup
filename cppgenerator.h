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
protected:
    QString fileNameForClass(const AbstractMetaClass* metaClass) const;
    QList<AbstractMetaFunctionList> filterGroupedFunctions(const AbstractMetaClass* metaClass = 0);
    QList<AbstractMetaFunctionList> filterGroupedOperatorFunctions(const AbstractMetaClass* metaClass,
                                                                   uint query);
    QString cpythonWrapperCPtr(const AbstractMetaClass* metaClass, QString argName = "self");

    void generateClass(QTextStream& s, const AbstractMetaClass* metaClass);
    void finishGeneration();

private:
    void writeNonVirtualModifiedFunctionNative(QTextStream& s, const AbstractMetaFunction* func);
    void writeConstructorNative(QTextStream& s, const AbstractMetaFunction* func);
    void writeDestructorNative(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeVirtualMethodNative(QTextStream& s, const AbstractMetaFunction* func);

    void writeConstructorWrapper(QTextStream &s, const AbstractMetaFunctionList overloads);
    void writeDestructorWrapper(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMinimalConstructorCallArguments(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMethodWrapper(QTextStream &s, const AbstractMetaFunctionList overloads);
    void writeArgumentsInitializer(QTextStream& s, OverloadData& overloadData);

    void writeErrorSection(QTextStream& s, OverloadData& overloadData);
    void writeTypeCheck(QTextStream& s, const OverloadData* overloadData, QString argumentName);

    void writeOverloadedMethodDecisor(QTextStream& s, OverloadData* parentOverloadData);
    void writeMethodCall(QTextStream& s, const AbstractMetaFunction* func, int maxArgs = 0);

    void writeClassRegister(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeClassDefinition(QTextStream& s, const AbstractMetaClass* metaClass);
    void writeMethodDefinition(QTextStream& s, const AbstractMetaFunctionList overloads);
    void writeTypeAsNumberDefinition(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeRichCompareFunction(QTextStream& s, const AbstractMetaClass* metaClass);

    void writeEnumNewMethod(QTextStream& s, const AbstractMetaEnum* metaEnum);
    void writeEnumDefinition(QTextStream& s, const AbstractMetaEnum* metaEnum);
    void writeEnumInitialization(QTextStream& s, const AbstractMetaEnum* metaEnum);
};

#endif // CPPGENERATOR_H

