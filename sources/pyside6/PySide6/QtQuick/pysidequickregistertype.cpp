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
#include "pyside6_qtcore_python.h"
#include "pyside6_qtquick_python.h"
#include "pyside6_qtqml_python.h"

// Mutex used to avoid race condition on PySide::nextQObjectMemoryAddr.
static QMutex nextQmlElementMutex;

static void createQuickItem(void *memory, void *type)
{
    QMutexLocker locker(&nextQmlElementMutex);
    PySide::setNextQObjectMemoryAddr(memory);
    Shiboken::GilState state;
    PyObject *obj = PyObject_CallObject(reinterpret_cast<PyObject *>(type), 0);
    if (!obj || PyErr_Occurred())
        PyErr_Print();
    PySide::setNextQObjectMemoryAddr(0);
}

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

template <typename T>
struct QPysideQmlMetaTypeInterface : QtPrivate::QMetaTypeInterface
{
    const QByteArray name;

    QPysideQmlMetaTypeInterface(const QByteArray &name, const QMetaObject *metaObject = nullptr)
        : QMetaTypeInterface {
            /*.revision=*/ 0,
            /*.alignment=*/ alignof(T),
            /*.size=*/ sizeof(T),
            /*.flags=*/ QtPrivate::QMetaTypeTypeFlags<T>::Flags,
            /*.typeId=*/ 0,
            /*.metaObject=*/ metaObject,
            /*.name=*/ name.constData(),
            /*.defaultCtr=*/ [](const QMetaTypeInterface *, void *addr) { new (addr) T(); },
            /*.copyCtr=*/ [](const QMetaTypeInterface *, void *addr, const void *other) {
                new (addr) T(*reinterpret_cast<const T *>(other));
            },
            /*.moveCtr=*/ [](const QMetaTypeInterface *, void *addr, void *other) {
                new (addr) T(std::move(*reinterpret_cast<T *>(other)));
            },
            /*.dtor=*/ [](const QMetaTypeInterface *, void *addr) {
                reinterpret_cast<T *>(addr)->~T();
            },
            /*.equals=*/ nullptr,
            /*.lessThan=*/ nullptr,
            /*.debugStream=*/ nullptr,
            /*.dataStreamOut=*/ nullptr,
            /*.dataStreamIn=*/ nullptr,
            /*.legacyRegisterOp=*/ nullptr
        }
        , name(name) {}
};

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
        QMetaType ptrType(new QPysideQmlMetaTypeInterface<WrapperClass *>(typePointerName, typeMetaObject));
        QMetaType lstType(new QPysideQmlMetaTypeInterface<QQmlListProperty<WrapperClass>>(typeListName));

        type->typeId = std::move(ptrType);
        type->listId = std::move(lstType);
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
                       const char *qmlName, bool creatable, const char *noCreationReason, QQmlPrivate::RegisterType *type)
{
    using namespace Shiboken;

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

    type->structVersion = 0;
    type->create = creatable ? createQuickItem : nullptr;
    type->noCreationReason = noCreationReason;
    type->userdata = pyObj;
    type->uri = uri;
    type->version = QTypeRevision::fromVersion(versionMajor, versionMinor);
    type->elementName = qmlName;
    type->metaObject = metaObject;

    type->extensionObjectCreate = 0;
    type->extensionMetaObject = 0;
    type->customParser = 0;

    return true;
}

void PySide::initQuickSupport(PyObject *module)
{
    Q_UNUSED(module);
#ifdef PYSIDE_QML_SUPPORT
    setQuickRegisterItemFunction(quickRegisterType);
#endif
}
