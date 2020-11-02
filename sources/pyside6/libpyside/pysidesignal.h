/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef PYSIDE_SIGNAL_H
#define PYSIDE_SIGNAL_H

#include <pysidemacros.h>

#include <sbkpython.h>
#include <basewrapper.h>

#include <QtCore/QList>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE
struct QMetaObject;
class QObject;
QT_END_NAMESPACE

extern "C"
{
    extern PYSIDE_API PyTypeObject *PySideSignalTypeF(void);
    extern PYSIDE_API PyTypeObject *PySideSignalInstanceTypeF(void);

    // Internal object
    struct PYSIDE_API PySideSignal;

    struct PySideSignalInstancePrivate;
    struct PYSIDE_API PySideSignalInstance
    {
        PyObject_HEAD
        PySideSignalInstancePrivate *d;
    };
}; // extern "C"

namespace PySide {
namespace Signal {

PYSIDE_API bool checkType(PyObject *type);

/**
 * Register all C++ signals of a QObject on Python type.
 */
PYSIDE_API void registerSignals(SbkObjectType *pyObj, const QMetaObject *metaObject);

/**
 * This function creates a Signal object which stays attached to QObject class based on a list of QMetaMethods
 *
 * @param   source of the Signal to be registered on meta object
 * @param   methods a list of QMetaMethod wich contains the supported signature
 * @return  Return a new reference to PyObject* of type  PySideSignal
 **/
PYSIDE_API PySideSignalInstance *newObjectFromMethod(PyObject *source, const QList<QMetaMethod> &methods);

/**
 * This function initializes the Signal object by creating a PySideSignalInstance
 *
 * @param   self a Signal object used as base to PySideSignalInstance
 * @param   name the name to be used on PySideSignalInstance
 * @param   object the PyObject where the signal will be attached
 * @return  Return a new reference to PySideSignalInstance
 **/
PYSIDE_API PySideSignalInstance *initialize(PySideSignal *signal, PyObject *name, PyObject *object);

/**
 * This function is used to retrieve the object in which the signal is attached
 *
 * @param   self The Signal object
 * @return  Return the internal reference to the parent object of the signal
 **/
PYSIDE_API PyObject *getObject(PySideSignalInstance *signal);

/**
 * This function is used to retrieve the signal signature
 *
 * @param   self The Signal object
 * @return  Return the signal signature
 **/
PYSIDE_API const char *getSignature(PySideSignalInstance *signal);

/**
 * This function is used to retrieve the signal signature
 *
 * @param   self The Signal object
 * @return  Return the signal signature
 **/
PYSIDE_API void updateSourceObject(PyObject *source);

/**
 * This function verifies if the signature is a QtSignal base on SIGNAL flag
 * @param   signature   The signal signature
 * @return  Return true if this is a Qt Signal, otherwise return false
 **/
PYSIDE_API bool isQtSignal(const char *signature);

/**
 * This function is similar to isQtSignal, however if it fails, it'll raise a Python error instead.
 *
 * @param   signature   The signal signature
 * @return  Return true if this is a Qt Signal, otherwise return false
 **/
PYSIDE_API bool checkQtSignal(const char *signature);

/**
 * This function is used to retrieve the signature base on Signal and receiver callback
 * @param   signature   The signal signature
 * @param   receiver    The QObject which will receive the signal
 * @param   callback    Callback function which will connect to the signal
 * @param   encodeName  Used to specify if the returned signature will be encoded with Qt signal/slot style
 * @return  Return the callback signature
 **/
PYSIDE_API QString getCallbackSignature(const char *signal, QObject *receiver, PyObject *callback, bool encodeName);

/**
 * This function parses the signature and then returns a list of argument types.
 *
 * @param   signature       The signal signature
 * @param   isShortCircuit  If this is a shortCircuit(python<->python) signal
 * @return  Return true if this is a Qt Signal, otherwise return false
 * @todo    replace return type by QList<QByteArray>
 **/
QStringList getArgsFromSignature(const char *signature, bool *isShortCircuit = 0);

} // namespace Signal
} // namespace PySide

#endif
