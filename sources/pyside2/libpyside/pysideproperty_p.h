/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PYSIDE_QPROPERTY_P_H
#define PYSIDE_QPROPERTY_P_H

#include <sbkpython.h>
#include <QtCore/QByteArray>
#include <QMetaObject>
#include "pysideproperty.h"

struct PySideProperty;

struct PySidePropertyPrivate
{
    QByteArray typeName;
    PySide::Property::MetaCallHandler metaCallHandler = nullptr;
    PyObject *fget = nullptr;
    PyObject *fset = nullptr;
    PyObject *freset = nullptr;
    PyObject *fdel = nullptr;
    PyObject *notify = nullptr;
    bool getter_doc = false;
    QByteArray notifySignature;
    QByteArray doc;
    bool designable = true;
    bool scriptable = true;
    bool stored = true;
    bool user = false;
    bool constant = false;
    bool final = false;
    void *userData = nullptr;
};

namespace PySide { namespace Property {

/**
 * Init PySide QProperty support system
 */
void init(PyObject* module);

/**
 * This function call reset property function
 * This function does not check the property object type
 *
 * @param   self The property object
 * @param   source The QObject witch has the property
 * @return  Return 0 if ok or -1 if this function fail
 **/
int reset(PySideProperty* self, PyObject* source);


/**
 * This function return the property type
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return the property type name
 **/
const char* getTypeName(const PySideProperty* self);

/**
 * This function check if property has read function
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isReadable(const PySideProperty* self);

/**
 * This function check if property has write function
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isWritable(const PySideProperty* self);

/**
 * This function check if property has reset function
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool hasReset(const PySideProperty* self);

/**
 * This function check if property has the flag DESIGNABLE setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isDesignable(const PySideProperty* self);

/**
 * This function check if property has the flag SCRIPTABLE setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isScriptable(const PySideProperty* self);

/**
 * This function check if property has the flag STORED setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isStored(const PySideProperty* self);

/**
 * This function check if property has the flag USER setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isUser(const PySideProperty* self);

/**
 * This function check if property has the flag CONSTANT setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isConstant(const PySideProperty* self);

/**
 * This function check if property has the flag FINAL setted
 * This function does not check the property object type
 *
 * @param   self The property object
 * @return  Return a boolean value
 **/
bool isFinal(const PySideProperty* self);

} // namespace Property
} // namespace PySide

#endif
