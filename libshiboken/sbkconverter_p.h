/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SBK_CONVERTER_P_H
#define SBK_CONVERTER_P_H

#include <Python.h>
#include <list>
#include "sbkconverter.h"

extern "C"
{

typedef std::pair<IsConvertibleToCppFunc, PythonToCppFunc> ToCppConversion;
typedef std::list<ToCppConversion> ToCppConversionList;

/**
 *  \internal
 *  Private structure of SbkConverter.
 */
struct SbkConverter
{
    /**
     *  TODO: it certainly will be empty in some cases, like with PyDate.
     *  TODO: probably a setPythonType(SbkConverter*, PyTypeObject*) function will be required.
     *  Python type associated with this converter. If the type is a Shiboken
     *  wrapper, then it must be a SbkObjectType; otherwise it will be the
     *  Python type to which the C++ value will be converted (note that the
     *  C++ type could be produced from various Python types).
     */
    PyTypeObject*           pythonType;
    /**
     *  This function converts a C++ object to a Python object of the type
     *  indicated in pythonType. The identity of the C++ object is kept,
     *  because it looks for an already existing Python wrapper associated
     *  with the C++ instance.
     *  It is used to convert C++ pointers and references to Python objects.
     */
    CppToPythonFunc         pointerToPython;
    /**
     *  This function converts a C++ object to a Python object of the type
     *  indicated in pythonType. The identity of the is not kept, because a
     *  new instance of the C++ object is created.
     *  It is used to convert objects passed by value, or reference, if said
     *  reference can't be traced to an object that already has a Python
     *  wrapper assigned for it.
     */
    CppToPythonFunc         copyToPython;
    /**
     *  This is a special case of a Python to C++ conversion. It returns
     *  the underlying C++ pointer of a Python wrapper passed as parameter
     *  or NULL if the Python object is a None value.
     *  It comes separated from the other toCppConversions because if you
     *  have a Python object representing a Value Type the type checking
     *  for both ValueType and ValueType* would be the same, thus the first
     *  check would be true and the second would never be reached.
     */
    ToCppConversion         toCppPointerConversion;
    /**
     *  This is a list of type checking functions that return the
     *  proper Python to C++ conversion function, for the given Python
     *  object.
     *  For Object Types, that never have implicit conversions, this
     *  list is always empty.
     */
    ToCppConversionList     toCppConversions;
};

} // extern "C"

#endif // SBK_CONVERTER_P_H
