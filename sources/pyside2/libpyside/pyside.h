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

#ifndef PYSIDE_H
#define PYSIDE_H

#include <sbkpython.h>

#include <pysidemacros.h>

#ifdef PYSIDE_QML_SUPPORT
#  include <QtQml/qqml.h>
#endif

#include <QtCore/QMetaType>
#include <QtCore/QHash>

struct SbkObjectType;

namespace PySide
{

PYSIDE_API void init(PyObject *module);

/**
 * Hash function used to enable hash on objects not supported on native Qt library which has toString function.
 */
template<class T>
inline uint hash(const T& value)
{
    return qHash(value.toString());
}

/**
 * Fill QObject properties and do signal connections using the values found in \p kwds dictonary.
 * \param qObj PyObject fot the QObject.
 * \param metaObj QMetaObject of \p qObj.
 * \param blackList keys to be ignored in kwds dictionary, this string list MUST be sorted.
 * \param blackListSize numbe rof elements in blackList.
 * \param kwds key->value dictonary.
 * \return True if everything goes well, false with a Python error setted otherwise.
 */
PYSIDE_API bool fillQtProperties(PyObject *qObj, const QMetaObject *metaObj, PyObject *kwds, const char **blackList, unsigned int blackListSize);

/**
*   If the type \p T was registered on Qt meta type system with Q_DECLARE_METATYPE macro, this class will initialize
*   the meta type.
*
*   Initialize a meta type means register it on Qt meta type system, Qt itself only do this on the first call of
*   qMetaTypeId, and this is exactly what we do to init it. If we don't do that, calls to QMetaType::type("QMatrix2x2")
*   could return zero, causing QVariant to not recognize some C++ types, like QMatrix2x2.
*/
template<typename T, bool OK = QMetaTypeId<T>::Defined >
struct initQtMetaType {
    initQtMetaType()
    {
        qMetaTypeId<T>();
    }
};

// Template specialization to do nothing when the type wasn't registered on Qt meta type system.
template<typename T>
struct initQtMetaType<T, false> {
};

PYSIDE_API void initDynamicMetaObject(SbkObjectType *type, const QMetaObject *base,
                                      std::size_t cppObjSize);
PYSIDE_API void initQObjectSubType(SbkObjectType *type, PyObject *args, PyObject *kwds);
PYSIDE_API void initQApp();

/// Return the size in bytes of a type that inherits QObject.
PYSIDE_API std::size_t getSizeOfQObject(SbkObjectType *type);

typedef void (*CleanupFunction)(void);

/**
 * Register a function to be called before python die
 */
PYSIDE_API void registerCleanupFunction(CleanupFunction func);
PYSIDE_API void runCleanupFunctions();

/**
 * Destroy a QCoreApplication taking care of destroy all instances of QObject first.
 */
PYSIDE_API void destroyQCoreApplication();

/**
 * Check for properties and signals registered on MetaObject and return these
 * \param cppSelf Is the QObject which contains the metaobject
 * \param self Python object of cppSelf
 * \param name Name of the argument which the function will try retrieve from MetaData
 * \return The Python object which contains the Data obtained in metaObject or the Python attribute related with name
 */
PYSIDE_API PyObject *getMetaDataFromQObject(QObject *cppSelf, PyObject *self, PyObject *name);

/**
 * Check if self inherits from class_name
 * \param self Python object
 * \param class_name strict with the class name
 * \return Returns true if self object inherits from class_name, otherwise returns false
 */
PYSIDE_API bool inherits(PyTypeObject *self, const char *class_name);

PYSIDE_API void *nextQObjectMemoryAddr();
PYSIDE_API void setNextQObjectMemoryAddr(void *addr);

PYSIDE_API PyObject *getWrapperForQObject(QObject *cppSelf, SbkObjectType *sbk_type);

#ifdef PYSIDE_QML_SUPPORT
// Used by QtQuick module to notify QtQml that custom QtQuick items can be registered.
typedef bool (*QuickRegisterItemFunction)(PyObject *pyObj, const char *uri, int versionMajor,
                                 int versionMinor, const char *qmlName,
                                 QQmlPrivate::RegisterType *);
PYSIDE_API QuickRegisterItemFunction getQuickRegisterItemFunction();
PYSIDE_API void setQuickRegisterItemFunction(QuickRegisterItemFunction function);
#endif // PYSIDE_QML_SUPPORT

/**
 * Given A PyObject repesenting ASCII or Unicode data, returns an equivalent QString.
 */
PYSIDE_API QString pyStringToQString(PyObject *str);

/**
 * Registers a dynamic "qt.conf" file with the Qt resource system.
 *
 * This is used in a standalone build, to inform QLibraryInfo of the Qt prefix (where Qt libraries
 * are installed) so that plugins can be successfully loaded.
 */
PYSIDE_API bool registerInternalQtConf();


} //namespace PySide


#endif // PYSIDE_H

