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

#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include "boostpythongenerator.h"

/**
*   The CppGenerator generate the implementation of boost::python bindings classes.
*/
class CppGenerator : public BoostPythonGenerator
{
public:
    const char* name() const
    {
        return "CppGenerator";
    }

    QMap<QString, QString> options() const;
    bool prepareGeneration(const QMap<QString, QString>& args);

protected:
    QString fileNameForClass(const AbstractMetaClass *cppClass) const;
    void generateClass(QTextStream &s, const AbstractMetaClass *cppClass);
    void finishGeneration();

private:
    void writePrelude(QTextStream &s, const AbstractMetaClass *cppClass);
    void writeBoostDeclaration(QTextStream &s, const AbstractMetaClass *cppClass);

    // method declaration writers
    void writeConstructor(QTextStream &s, const AbstractMetaFunction *func);
    void writeConstructorInitialization(QTextStream &s, const AbstractMetaFunction *func);
    void writeNormalMethodDef(QTextStream &s, const AbstractMetaFunction *func);
    void writeModifiedMethodDef(QTextStream &s, const AbstractMetaFunction *func);
    void writeOperatorOverload(QTextStream &s, const AbstractMetaFunction *func);
    void writeGlobalOperatorOverload(QTextStream &s, const AbstractMetaFunction *func);
    void writeFunctionArgsDef(QTextStream &s_out, const AbstractMetaFunction *func);
    void writeGlobalFunctions();
    void writeDestructor(QTextStream &s, const AbstractMetaClass *cppClass);

    // method implementation writers
    void writeModifiedConstructorImpl(QTextStream &s, const AbstractMetaFunction *func);
    void writeConstructorImpl(QTextStream &s, const AbstractMetaFunction *func);
    void writeVirtualMethodImpl(QTextStream &s, const AbstractMetaFunction *func);
    void writeVirtualMethodImplHead(QTextStream &s, const AbstractMetaFunction *func);
    void writeVirtualMethodImplFoot(QTextStream &s, const AbstractMetaFunction *func);
    void writePureVirtualMethodImplFoot(QTextStream &s, const AbstractMetaFunction *func);
    void writeNonVirtualModifiedFunctionImpl(QTextStream &s, const AbstractMetaFunction *func);
    void writeGlobalOperatorOverloadImpl(QTextStream& s, const AbstractMetaFunction* func);

    // helper functions
    QString writeFunctionCast(QTextStream& s, const AbstractMetaFunction* func, const QString& castNameSuffix = QString(), const QString& className = QString());
    QString getFuncTypedefName(const AbstractMetaFunction *func) const;
    QString getFunctionReturnType(const AbstractMetaFunction *func);
    AbstractMetaFunction* findMainConstructor(const AbstractMetaClass *clazz);
    QString getArgumentType(const AbstractMetaClass *cppClass, const AbstractMetaFunction *func, int idx);
    QString operatorFunctionName(const AbstractMetaFunction *func);
    QString getOperatorArgumentTypeName(const AbstractMetaFunction *func, int argumentIndex);

    // call policy related
    QString verifyDefaultReturnPolicy(const AbstractMetaFunction *func, const QString &callPolicy);
    QString getFunctionCallPolicy(const AbstractMetaFunction *func);

    // enum related
    void writeEnums(QTextStream &s, const AbstractMetaClass *cppClass, bool useNamespace);
    void writeEnum(QTextStream &s, const AbstractMetaEnum *cppEnum, const QString &nameSpace);
    // write implicitly conversions
    void writeImplicitlyConversion(QTextStream &s, const AbstractMetaClass *cppClass);
    void writeVirtualDefaultFunction(QTextStream &s, const AbstractMetaFunction *arg2);

    void writeHashFunction(QTextStream &s, const AbstractMetaClass *cppClass);
    QString baseClassName(const QString &name);

    bool m_disableNamedArgs;
};

#endif // CPPGENERATOR_H

