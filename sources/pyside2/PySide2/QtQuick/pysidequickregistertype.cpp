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

#include "pysidequickregistertype.h"

#include <pyside.h>
#include <pyside_p.h>
#include <shiboken.h>

// Auto generated headers.
#include "qquickitem_wrapper.h"
#include "qquickpainteditem_wrapper.h"
#include "qquickframebufferobject_wrapper.h"
#include "pyside2_qtcore_python.h"
#include "pyside2_qtquick_python.h"
#include "pyside2_qtqml_python.h"

#ifndef PYSIDE_MAX_QUICK_TYPES
// Maximum number of different Qt Quick types the user can export to QML using
// qmlRegisterType. This limit exists because the QML engine instantiates objects
// by calling a function with one argument (a void* pointer where the object should
// be created), and thus does not allow us to choose which object to create. Thus
// we create a C++ factory function for each new registered type at compile time.
#  define PYSIDE_MAX_QUICK_TYPES 50
#endif // !PYSIDE_MAX_QUICK_TYPES

// All registered python types and their creation functions.
static PyObject *pyTypes[PYSIDE_MAX_QUICK_TYPES];
static void (*createFuncs[PYSIDE_MAX_QUICK_TYPES])(void*);

// Mutex used to avoid race condition on PySide::nextQObjectMemoryAddr.
static QMutex nextQmlElementMutex;

// Python object factory functions.
template<int N>
struct ElementFactoryBase
{
    static void createQuickItem(void *memory)
    {
        QMutexLocker locker(&nextQmlElementMutex);
        PySide::setNextQObjectMemoryAddr(memory);
        Shiboken::GilState state;
        PyObject *obj = PyObject_CallObject(pyTypes[N], 0);
        if (!obj || PyErr_Occurred())
            PyErr_Print();
        PySide::setNextQObjectMemoryAddr(0);
    }
};

template<int N>
struct ElementFactory : ElementFactoryBase<N>
{
    static void init()
    {
        createFuncs[N] = &ElementFactoryBase<N>::createQuickItem;
        ElementFactory<N-1>::init();
    }
};

template<>
struct  ElementFactory<0> : ElementFactoryBase<0>
{
    static void init()
    {
        createFuncs[0] = &ElementFactoryBase<0>::createQuickItem;
    }
};

#define PY_REGISTER_IF_INHERITS_FROM(className, typeToRegister,typePointerName, \
                                     typeListName, typeMetaObject, type, registered) \
        registerTypeIfInheritsFromClass<className##Wrapper>(#className, typeToRegister, \
                                                            typePointerName, typeListName, \
                                                            typeMetaObject, type, registered)

bool pyTypeObjectInheritsFromClass(PyTypeObject *pyObjType, QByteArray className)
{
    className.append('*');
    PyTypeObject *classPyType = Shiboken::Conversions::getPythonTypeObject(className.constData());
    bool isDerived = PySequence_Contains(pyObjType->tp_mro,
                                         reinterpret_cast<PyObject *>(classPyType));
    return isDerived;
}

template <class WrapperClass>
void registerTypeIfInheritsFromClass(
        QByteArray className,
        PyTypeObject *typeToRegister,
        const QByteArray &typePointerName,
        const QByteArray &typeListName,
        const QMetaObject *typeMetaObject,
        QQmlPrivate::RegisterType *type,
        bool &registered)
{
    bool shouldRegister = !registered && pyTypeObjectInheritsFromClass(typeToRegister, className);
    if (shouldRegister) {
        int ptrType =
                QMetaType::registerNormalizedType(
                    typePointerName.constData(),
                    QtMetaTypePrivate::QMetaTypeFunctionHelper<WrapperClass *>::Destruct,
                    QtMetaTypePrivate::QMetaTypeFunctionHelper<WrapperClass *>::Construct,
                    sizeof(WrapperClass *),
                    static_cast< ::QFlags<QMetaType::TypeFlag> >(QtPrivate::QMetaTypeTypeFlags<
                                                                 WrapperClass *>::Flags),
                    typeMetaObject);
        if (ptrType == -1) {
            PyErr_Format(PyExc_TypeError, "Meta type registration of \"%s\" for QML usage failed.",
                         typePointerName.constData());
            return;
        }

        int lstType =
                QMetaType::registerNormalizedType(
                    typeListName.constData(),
                    QtMetaTypePrivate::QMetaTypeFunctionHelper<QQmlListProperty<WrapperClass> >
                      ::Destruct,
                    QtMetaTypePrivate::QMetaTypeFunctionHelper<QQmlListProperty<WrapperClass> >
                      ::Construct,
                    sizeof(QQmlListProperty<WrapperClass>),
                    static_cast< ::QFlags<QMetaType::TypeFlag> >(
                        QtPrivate::QMetaTypeTypeFlags<QQmlListProperty<WrapperClass> >::Flags),
                    static_cast<QMetaObject*>(0));
        if (lstType == -1) {
            PyErr_Format(PyExc_TypeError, "Meta type registration of \"%s\" for QML usage failed.",
                         typeListName.constData());
            return;
        }

        type->typeId = ptrType;
        type->listId = lstType;
        type->attachedPropertiesFunction = QQmlPrivate::attachedPropertiesFunc<WrapperClass>();
        type->attachedPropertiesMetaObject =
                QQmlPrivate::attachedPropertiesMetaObject<WrapperClass>();
        type->parserStatusCast =
                QQmlPrivate::StaticCastSelector<WrapperClass, QQmlParserStatus>::cast();
        type->valueSourceCast =
                QQmlPrivate::StaticCastSelector<WrapperClass, QQmlPropertyValueSource>::cast();
        type->valueInterceptorCast =
                QQmlPrivate::StaticCastSelector<WrapperClass, QQmlPropertyValueInterceptor>::cast();
        type->objectSize = sizeof(WrapperClass);
        registered = true;
    }
}

bool quickRegisterType(PyObject *pyObj, const char *uri, int versionMajor, int versionMinor,
                               const char *qmlName, QQmlPrivate::RegisterType *type)
{
    using namespace Shiboken;
    static int nextType = 0;

    if (nextType >= PYSIDE_MAX_QUICK_TYPES) {
        PyErr_Format(PyExc_TypeError,
                     "You can only export %d Qt Quick types to QML.", PYSIDE_MAX_QUICK_TYPES);
        return false;
    }

    PyTypeObject *pyObjType = reinterpret_cast<PyTypeObject *>(pyObj);
    PyTypeObject *qQuickItemPyType =
            Shiboken::Conversions::getPythonTypeObject("QQuickItem*");
    bool isQuickItem = PySequence_Contains(pyObjType->tp_mro,
                                           reinterpret_cast<PyObject *>(qQuickItemPyType));

    // Register only classes that inherit QQuickItem or its children.
    if (!isQuickItem)
        return false;

    // Used inside macros to register the type.
    const QMetaObject *metaObject = PySide::retrieveMetaObject(pyObj);
    Q_ASSERT(metaObject);


    // Incref the type object, don't worry about decref'ing it because
    // there's no way to unregister a QML type.
    Py_INCREF(pyObj);

    pyTypes[nextType] = pyObj;

    // Used in macro registration.
    QByteArray pointerName(qmlName);
    pointerName.append('*');
    QByteArray listName(qmlName);
    listName.prepend("QQmlListProperty<");
    listName.append('>');

    bool registered = false;
    PY_REGISTER_IF_INHERITS_FROM(QQuickPaintedItem, pyObjType, pointerName, listName, metaObject,
                                 type, registered);
    PY_REGISTER_IF_INHERITS_FROM(QQuickFramebufferObject, pyObjType, pointerName, listName,
                                 metaObject, type, registered);
    PY_REGISTER_IF_INHERITS_FROM(QQuickItem, pyObjType, pointerName, listName, metaObject,
                                 type, registered);
    if (!registered)
        return false;

    type->create = createFuncs[nextType];
    type->version = 0;
    type->uri = uri;
    type->versionMajor = versionMajor;
    type->versionMinor = versionMinor;
    type->elementName = qmlName;
    type->metaObject = metaObject;

    type->extensionObjectCreate = 0;
    type->extensionMetaObject = 0;
    type->customParser = 0;

    ++nextType;
    return true;
}

void PySide::initQuickSupport(PyObject *module)
{
    Q_UNUSED(module);
    ElementFactory<PYSIDE_MAX_QUICK_TYPES - 1>::init();
#ifdef PYSIDE_QML_SUPPORT
    setQuickRegisterItemFunction(quickRegisterType);
#endif
}
