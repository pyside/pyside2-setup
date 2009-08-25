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

#ifndef HPPGENERATOR_H
#define HPPGENERATOR_H

#include "boostpythongenerator.h"

/**
*   The HppGenerator generate the declarations of boost::python bindings classes.
*/
class HppGenerator : public BoostPythonGenerator
{
protected:
    QString fileNameForClass(const AbstractMetaClass* cppClass) const;
    void generateClass(QTextStream& s, const AbstractMetaClass* cppClass);
    void finishGeneration() {}
    const char* name() const
    {
        return "HppGenerator";
    }
private:
    void writeFunction(QTextStream& s, const AbstractMetaFunction* func);
    void writePureVirtualEmptyImpl(QTextStream& , const AbstractMetaFunction* func);
    void writeBaseClass(QTextStream& s, const AbstractMetaClass* cppClass);
    void writeCopyCtor(QTextStream &s, const AbstractMetaClass* cppClass);
    void writeDefaultImplementation(QTextStream& s, const AbstractMetaFunction* func);
};

#endif // HPPGENERATOR_H

