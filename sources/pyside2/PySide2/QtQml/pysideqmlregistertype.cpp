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

#include "pysideqmlregistertype.h"

// shiboken
#include <shiboken.h>

// pyside
#include <pyside.h>
#include <pyside_p.h>
#include <pysideproperty.h>

// auto generated headers
#include "pyside2_qtcore_python.h"
#include "pyside2_qtqml_python.h"

#ifndef PYSIDE_MAX_QML_TYPES
// Maximum number of different Qt QML types the user can export to QML using
// qmlRegisterType. This limit exists because the QML engine instantiates objects
// by calling a function with one argument (a void *pointer where the object should
// be created), and thus does not allow us to choose which object to create. Thus
// we create a C++ factory function for each new registered type at compile time.
#define PYSIDE_MAX_QML_TYPES 50
#endif

// Forward declarations.
static void propListMetaCall(PySideProperty *pp, PyObject *self, QMetaObject::Call call,
                             void **args);

// All registered python types and their creation functions.
static PyObject *pyTypes[PYSIDE_MAX_QML_TYPES];
static void (*createFuncs[PYSIDE_MAX_QML_TYPES])(void *);

// Mutex used to avoid race condition on PySide::nextQObjectMemoryAddr.
static QMutex nextQmlElementMutex;

template<int N>
struct ElementFactoryBase
{
    static void createInto(void *memory)
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
        createFuncs[N] = &ElementFactoryBase<N>::createInto;
        ElementFactory<N-1>::init();
    }
};

template<>
struct  ElementFactory<0> : ElementFactoryBase<0>
{
    static void init()
    {
        createFuncs[0] = &ElementFactoryBase<0>::createInto;
    }
};

int PySide::qmlRegisterType(PyObject *pyObj, const char *uri, int versionMajor,
                            int versionMinor, const char *qmlName)
{
    using namespace Shiboken;

    static PyTypeObject *qobjectType = Shiboken::Conversions::getPythonTypeObject("QObject*");
    assert(qobjectType);
    static int nextType = 0;

    if (nextType >= PYSIDE_MAX_QML_TYPES) {
        PyErr_Format(PyExc_TypeError, "You can only export %d custom QML types to QML.",
                     PYSIDE_MAX_QML_TYPES);
        return -1;
    }

    PyTypeObject *pyObjType = reinterpret_cast<PyTypeObject *>(pyObj);
    if (!PySequence_Contains(pyObjType->tp_mro, reinterpret_cast<PyObject *>(qobjectType))) {
        PyErr_Format(PyExc_TypeError, "A type inherited from %s expected, got %s.",
                     qobjectType->tp_name, pyObjType->tp_name);
        return -1;
    }

    const QMetaObject *metaObject = PySide::retrieveMetaObject(pyObjType);
    Q_ASSERT(metaObject);

    QQmlPrivate::RegisterType type;
    type.version = 0;

    // Allow registering Qt Quick items.
    bool registered = false;
#ifdef PYSIDE_QML_SUPPORT
    QuickRegisterItemFunction quickRegisterItemFunction = getQuickRegisterItemFunction();
    if (quickRegisterItemFunction) {
        registered = quickRegisterItemFunction(pyObj, uri, versionMajor, versionMinor,
                                               qmlName, &type);
    }
#endif

    // Register as simple QObject rather than Qt Quick item.
    if (!registered) {
        // Incref the type object, don't worry about decref'ing it because
        // there's no way to unregister a QML type.
        Py_INCREF(pyObj);

        pyTypes[nextType] = pyObj;

        // FIXME: Fix this to assign new type ids each time.
        type.typeId = qMetaTypeId<QObject *>();
        type.listId = qMetaTypeId<QQmlListProperty<QObject> >();
        type.attachedPropertiesFunction = QQmlPrivate::attachedPropertiesFunc<QObject>();
        type.attachedPropertiesMetaObject = QQmlPrivate::attachedPropertiesMetaObject<QObject>();

        type.parserStatusCast =
                QQmlPrivate::StaticCastSelector<QObject, QQmlParserStatus>::cast();
        type.valueSourceCast =
                QQmlPrivate::StaticCastSelector<QObject, QQmlPropertyValueSource>::cast();
        type.valueInterceptorCast =
                QQmlPrivate::StaticCastSelector<QObject, QQmlPropertyValueInterceptor>::cast();

        int objectSize = static_cast<int>(PySide::getSizeOfQObject(
                                              reinterpret_cast<SbkObjectType *>(pyObj)));
        type.objectSize = objectSize;
        type.create = createFuncs[nextType];
        type.uri = uri;
        type.versionMajor = versionMajor;
        type.versionMinor = versionMinor;
        type.elementName = qmlName;

        type.extensionObjectCreate = 0;
        type.extensionMetaObject = 0;
        type.customParser = 0;
        ++nextType;
    }
    type.metaObject = metaObject; // Snapshot may have changed.

    int qmlTypeId = QQmlPrivate::qmlregister(QQmlPrivate::TypeRegistration, &type);
    if (qmlTypeId == -1) {
        PyErr_Format(PyExc_TypeError, "QML meta type registration of \"%s\" failed.",
                     qmlName);
    }
    return qmlTypeId;
}

extern "C"
{

// This is the user data we store in the property.
struct QmlListProperty
{
    PyTypeObject *type;
    PyObject *append;
    PyObject *at;
    PyObject *clear;
    PyObject *count;
};

static int propListTpInit(PyObject *self, PyObject *args, PyObject *kwds)
{
    static const char *kwlist[] = {"type", "append", "at", "clear", "count", 0};
    PySideProperty *pySelf = reinterpret_cast<PySideProperty *>(self);
    QmlListProperty *data = new QmlListProperty;
    memset(data, 0, sizeof(QmlListProperty));

    if (!PyArg_ParseTupleAndKeywords(args, kwds,
                                     "OO|OOO:QtQml.ListProperty", (char **) kwlist,
                                     &data->type,
                                     &data->append,
                                     &data->at,
                                     &data->clear,
                                     &data->count)) {
        return -1;
    }
    PySide::Property::setMetaCallHandler(pySelf, &propListMetaCall);
    PySide::Property::setTypeName(pySelf, "QQmlListProperty<QObject>");
    PySide::Property::setUserData(pySelf, data);

    return 0;
}

void propListTpFree(void *self)
{
    auto pySelf = reinterpret_cast<PySideProperty *>(self);
    delete reinterpret_cast<QmlListProperty *>(PySide::Property::userData(pySelf));
    // calls base type constructor
    Py_TYPE(pySelf)->tp_base->tp_free(self);
}

static PyType_Slot PropertyListType_slots[] = {
    {Py_tp_init, (void *)propListTpInit},
    {Py_tp_free, (void *)propListTpFree},
    {Py_tp_dealloc, (void *)object_dealloc},
    {0, 0}
};
static PyType_Spec PropertyListType_spec = {
    "PySide2.QtQml.ListProperty",
    sizeof(PySideProperty),
    0,
    Py_TPFLAGS_DEFAULT,
    PropertyListType_slots,
};


PyTypeObject *PropertyListTypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        PyObject *bases = Py_BuildValue("(O)", PySidePropertyTypeF());
        type = (PyTypeObject *)PyType_FromSpecWithBases(&PropertyListType_spec, bases);
        Py_XDECREF(bases);
    }
    return type;
}

} // extern "C"

// Implementation of QQmlListProperty<T>::AppendFunction callback
void propListAppender(QQmlListProperty<QObject> *propList, QObject *item)
{
    Shiboken::GilState state;

    Shiboken::AutoDecRef args(PyTuple_New(2));
    PyTuple_SET_ITEM(args, 0, Shiboken::Conversions::pointerToPython((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], propList->object));
    PyTuple_SET_ITEM(args, 1, Shiboken::Conversions::pointerToPython((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], item));

    auto data = reinterpret_cast<QmlListProperty *>(propList->data);
    Shiboken::AutoDecRef retVal(PyObject_CallObject(data->append, args));

    if (PyErr_Occurred())
        PyErr_Print();
}

// Implementation of QQmlListProperty<T>::CountFunction callback
int propListCount(QQmlListProperty<QObject> *propList)
{
    Shiboken::GilState state;

    Shiboken::AutoDecRef args(PyTuple_New(1));
    PyTuple_SET_ITEM(args, 0, Shiboken::Conversions::pointerToPython((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], propList->object));

    auto data = reinterpret_cast<QmlListProperty *>(propList->data);
    Shiboken::AutoDecRef retVal(PyObject_CallObject(data->count, args));

    // Check return type
    int cppResult = 0;
    PythonToCppFunc pythonToCpp = 0;
    if (PyErr_Occurred())
        PyErr_Print();
    else if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), retVal)))
        pythonToCpp(retVal, &cppResult);
    return cppResult;
}

// Implementation of QQmlListProperty<T>::AtFunction callback
QObject *propListAt(QQmlListProperty<QObject> *propList, int index)
{
    Shiboken::GilState state;

    Shiboken::AutoDecRef args(PyTuple_New(2));
    PyTuple_SET_ITEM(args, 0, Shiboken::Conversions::pointerToPython((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], propList->object));
    PyTuple_SET_ITEM(args, 1, Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<int>(), &index));

    auto data = reinterpret_cast<QmlListProperty *>(propList->data);
    Shiboken::AutoDecRef retVal(PyObject_CallObject(data->at, args));

    QObject *result = 0;
    if (PyErr_Occurred())
        PyErr_Print();
    else if (PyType_IsSubtype(Py_TYPE(retVal), data->type))
        Shiboken::Conversions::pythonToCppPointer((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], retVal, &result);
    return result;
}

// Implementation of QQmlListProperty<T>::ClearFunction callback
void propListClear(QQmlListProperty<QObject> * propList)
{
    Shiboken::GilState state;

    Shiboken::AutoDecRef args(PyTuple_New(1));
    PyTuple_SET_ITEM(args, 0, Shiboken::Conversions::pointerToPython((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], propList->object));

    auto data = reinterpret_cast<QmlListProperty *>(propList->data);
    Shiboken::AutoDecRef retVal(PyObject_CallObject(data->clear, args));

    if (PyErr_Occurred())
        PyErr_Print();
}

// qt_metacall specialization for ListProperties
static void propListMetaCall(PySideProperty *pp, PyObject *self, QMetaObject::Call call, void **args)
{
    if (call != QMetaObject::ReadProperty)
        return;

    auto data = reinterpret_cast<QmlListProperty *>(PySide::Property::userData(pp));
    QObject *qobj;
    Shiboken::Conversions::pythonToCppPointer((SbkObjectType *)SbkPySide2_QtCoreTypes[SBK_QOBJECT_IDX], self, &qobj);
    QQmlListProperty<QObject> declProp(qobj, data, &propListAppender, &propListCount, &propListAt, &propListClear);

    // Copy the data to the memory location requested by the meta call
    void *v = args[0];
    *reinterpret_cast<QQmlListProperty<QObject> *>(v) = declProp;
}

// VolatileBool (volatile bool) type definition.

static PyObject *
QtQml_VolatileBoolObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    static const char *kwlist[] = {"x", 0};
    PyObject *x = Py_False;
    long ok;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O:bool", const_cast<char **>(kwlist), &x))
        return Q_NULLPTR;
    ok = PyObject_IsTrue(x);
    if (ok < 0)
        return Q_NULLPTR;

    QtQml_VolatileBoolObject *self
            = reinterpret_cast<QtQml_VolatileBoolObject *>(type->tp_alloc(type, 0));

    if (self != Q_NULLPTR)
        self->flag = ok;

    return reinterpret_cast<PyObject *>(self);
}

static PyObject *
QtQml_VolatileBoolObject_get(QtQml_VolatileBoolObject *self)
{
    if (self->flag)
        return Py_True;
    return Py_False;
}

static PyObject *
QtQml_VolatileBoolObject_set(QtQml_VolatileBoolObject *self, PyObject *args)
{
    PyObject *value = Py_False;
    long ok;

    if (!PyArg_ParseTuple(args, "O:bool", &value)) {
        return Q_NULLPTR;
    }

    ok = PyObject_IsTrue(value);
    if (ok < 0) {
        PyErr_SetString(PyExc_TypeError, "Not a boolean value.");
        return Q_NULLPTR;
    }

    if (ok > 0)
        self->flag = true;
    else
        self->flag = false;

    Py_RETURN_NONE;
}

static PyMethodDef QtQml_VolatileBoolObject_methods[] = {
    {"get", reinterpret_cast<PyCFunction>(QtQml_VolatileBoolObject_get), METH_NOARGS,
     "B.get() -> Bool. Returns the value of the volatile boolean"
    },
    {"set", reinterpret_cast<PyCFunction>(QtQml_VolatileBoolObject_set), METH_VARARGS,
     "B.set(a) -> None. Sets the value of the volatile boolean"
    },
    {Q_NULLPTR}  /* Sentinel */
};

static PyObject *
QtQml_VolatileBoolObject_repr(QtQml_VolatileBoolObject *self)
{
    PyObject *s;

    if (self->flag)
        s = PyBytes_FromFormat("%s(True)",
                                Py_TYPE(self)->tp_name);
    else
        s = PyBytes_FromFormat("%s(False)",
                                Py_TYPE(self)->tp_name);
    Py_XINCREF(s);
    return s;
}

static PyObject *
QtQml_VolatileBoolObject_str(QtQml_VolatileBoolObject *self)
{
    PyObject *s;

    if (self->flag)
        s = PyBytes_FromFormat("%s(True) -> %p",
                                Py_TYPE(self)->tp_name, &(self->flag));
    else
        s = PyBytes_FromFormat("%s(False) -> %p",
                                Py_TYPE(self)->tp_name, &(self->flag));
    Py_XINCREF(s);
    return s;
}

static PyType_Slot QtQml_VolatileBoolType_slots[] = {
    {Py_tp_repr, (void *)reinterpret_cast<reprfunc>(QtQml_VolatileBoolObject_repr)},
    {Py_tp_str, (void *)reinterpret_cast<reprfunc>(QtQml_VolatileBoolObject_str)},
    {Py_tp_methods, (void *)QtQml_VolatileBoolObject_methods},
    {Py_tp_new, (void *)QtQml_VolatileBoolObject_new},
    {Py_tp_dealloc, (void *)object_dealloc},
    {0, 0}
};
static PyType_Spec QtQml_VolatileBoolType_spec = {
    "PySide2.QtQml.VolatileBool",
    sizeof(QtQml_VolatileBoolObject),
    0,
    Py_TPFLAGS_DEFAULT,
    QtQml_VolatileBoolType_slots,
};


PyTypeObject *QtQml_VolatileBoolTypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type)
        type = (PyTypeObject *)PyType_FromSpec(&QtQml_VolatileBoolType_spec);
    return type;
}

void PySide::initQmlSupport(PyObject *module)
{
    ElementFactory<PYSIDE_MAX_QML_TYPES - 1>::init();

    // Export QmlListProperty type
    if (PyType_Ready(PropertyListTypeF()) < 0) {
        PyErr_Print();
        qWarning() << "Error initializing PropertyList type.";
        return;
    }

    Py_INCREF(reinterpret_cast<PyObject *>(PropertyListTypeF()));
    PyModule_AddObject(module, PepType_GetNameStr(PropertyListTypeF()),
                       reinterpret_cast<PyObject *>(PropertyListTypeF()));

    if (PyType_Ready(QtQml_VolatileBoolTypeF()) < 0) {
        PyErr_Print();
        qWarning() << "Error initializing VolatileBool type.";
        return;
    }

    Py_INCREF(QtQml_VolatileBoolTypeF());
    PyModule_AddObject(module, PepType_GetNameStr(QtQml_VolatileBoolTypeF()),
                       reinterpret_cast<PyObject *>(QtQml_VolatileBoolTypeF()));
}
