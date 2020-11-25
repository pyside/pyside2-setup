/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "dynamicqmetaobject.h"
#include "dynamicqmetaobject_p.h"
#include "pysidesignal.h"
#include "pysidesignal_p.h"
#include "pysideproperty.h"
#include "pysideproperty_p.h"
#include "pysideslot_p.h"
#include "pysideqenum.h"

#include <shiboken.h>

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <private/qmetaobjectbuilder_p.h>

#include <cstring>
#include <vector>

using namespace PySide;

// MetaObjectBuilder: Provides the QMetaObject's returned by
// QObject::metaObject() for PySide6 objects. There are several
// scenarios to consider:
// 1) A plain Qt class (say QTimer) is instantiated. In that case,
// return the base meta object until a modification is made by
// adding methods, properties or class info (cf qmetaobject_test.py).
// In that case, instantiate a QMetaObjectBuilder inheriting the
// base meta meta object, add the method and return the result
// of QMetaObjectBuilder::toMetaObject() (with dirty handling should
// further modifications be made).
// 2) A Python class inheriting a Qt class is instantiated. For this,
// instantiate a QMetaObjectBuilder and add the methods/properties
// found by inspecting the Python class.

class MetaObjectBuilderPrivate
{
public:
    using MetaObjects = std::vector<const QMetaObject *>;

    QMetaObjectBuilder *ensureBuilder();
    void parsePythonType(PyTypeObject *type);
    int indexOfMethod(QMetaMethod::MethodType mtype,
                      const QByteArray &signature) const;
    int indexOfProperty(const QByteArray &name) const;
    int addSlot(const QByteArray &signature);
    int addSlot(const QByteArray &signature, const QByteArray &type);
    int addSignal(const QByteArray &signature);
    void removeMethod(QMetaMethod::MethodType mtype, int index);
    int getPropertyNotifyId(PySideProperty *property) const;
    int addProperty(const QByteArray &property, PyObject *data);
    void addInfo(const QByteArray &key, const  QByteArray &value);
    void addInfo(const QMap<QByteArray, QByteArray> &info);
    void addEnumerator(const char *name, bool flag, bool scoped,
                       const MetaObjectBuilder::EnumValues &entries);
    void removeProperty(int index);
    const QMetaObject *update();

    QMetaObjectBuilder *m_builder = nullptr;

    const QMetaObject *m_baseObject = nullptr;
    MetaObjects m_cachedMetaObjects;
    bool m_dirty = true;
};

QMetaObjectBuilder *MetaObjectBuilderPrivate::ensureBuilder()
{
    if (!m_builder) {
        m_builder = new QMetaObjectBuilder();
        m_builder->setClassName(m_baseObject->className());
        m_builder->setSuperClass(m_baseObject);
    }
    return m_builder;
}

MetaObjectBuilder::MetaObjectBuilder(const char *className, const QMetaObject *metaObject) :
    m_d(new MetaObjectBuilderPrivate)
{
    m_d->m_baseObject = metaObject;
    m_d->m_builder = new QMetaObjectBuilder();
    m_d->m_builder->setClassName(className);
    m_d->m_builder->setSuperClass(metaObject);
    m_d->m_builder->setClassName(className);
}

MetaObjectBuilder::MetaObjectBuilder(PyTypeObject *type, const QMetaObject *metaObject)
    : m_d(new MetaObjectBuilderPrivate)
{
    m_d->m_baseObject = metaObject;
    const char *className = type->tp_name;
    if (const char *lastDot = strrchr(type->tp_name, '.'))
        className = lastDot + 1;
    // Different names indicate a Python class inheriting a Qt class.
    // Parse the type.
    if (strcmp(className, metaObject->className()) != 0) {
        m_d->m_builder = new QMetaObjectBuilder();
        m_d->m_builder->setClassName(className);
        m_d->m_builder->setSuperClass(metaObject);
        m_d->parsePythonType(type);
    }
}

MetaObjectBuilder::~MetaObjectBuilder()
{
    for (auto *metaObject : m_d->m_cachedMetaObjects)
        free(const_cast<QMetaObject*>(metaObject));
    delete m_d->m_builder;
    delete m_d;
}

int MetaObjectBuilderPrivate::indexOfMethod(QMetaMethod::MethodType mtype,
                                            const QByteArray &signature) const
{
    int result = -1;
    if (m_builder) {
        switch (mtype) {
        case QMetaMethod::Signal:
            result = m_builder->indexOfSignal(signature);
            break;
        case QMetaMethod::Slot:
            result = m_builder->indexOfSlot(signature);
            break;
        case QMetaMethod::Constructor:
            result = m_builder->indexOfConstructor(signature);
            break;
        case QMetaMethod::Method:
            result = m_builder->indexOfMethod(signature);
            break;
        }
        if (result >= 0)
            return result + m_baseObject->methodCount();
    }
    switch (mtype) {
    case QMetaMethod::Signal:
        result = m_baseObject->indexOfSignal(signature);
        break;
    case QMetaMethod::Slot:
        result = m_baseObject->indexOfSlot(signature);
        break;
    case QMetaMethod::Constructor:
        result = m_baseObject->indexOfConstructor(signature);
        break;
    case QMetaMethod::Method:
        result = m_baseObject->indexOfMethod(signature);
        break;
    }
    return result;
}

int MetaObjectBuilder::indexOfMethod(QMetaMethod::MethodType mtype,
                                     const QByteArray &signature) const
{
    return m_d->indexOfMethod(mtype, signature);
}

int MetaObjectBuilderPrivate::indexOfProperty(const QByteArray &name) const
{
    if (m_builder) {
        const int result = m_builder->indexOfProperty(name);
        if (result >= 0)
            return m_baseObject->propertyCount() + result;
    }
    return m_baseObject->indexOfProperty(name);
}

int MetaObjectBuilder::indexOfProperty(const QByteArray &name) const
{
    return m_d->indexOfProperty(name);
}

static bool checkMethodSignature(const QByteArray &signature)
{
    // Common mistake not to add parentheses to the signature.
    const int openParen = signature.indexOf('(');
    const int closingParen = signature.lastIndexOf(')');
    const bool ok = openParen != -1 && closingParen != -1 && openParen < closingParen;
    if (!ok) {
         const QByteArray message =
             "MetaObjectBuilder::addMethod: Invalid method signature provided for \""
                 + signature + '"';
         PyErr_WarnEx(PyExc_RuntimeWarning, message.constData(), 0);
    }
    return ok;
}

int MetaObjectBuilderPrivate::addSlot(const QByteArray &signature)
{
    if (!checkMethodSignature(signature))
        return -1;
    m_dirty = true;
    return m_baseObject->methodCount()
        + ensureBuilder()->addSlot(signature).index();
}

int MetaObjectBuilder::addSlot(const char *signature)
{
    return m_d->addSlot(signature);
}

int MetaObjectBuilderPrivate::addSlot(const QByteArray &signature,
                                      const QByteArray &type)
{
    if (!checkMethodSignature(signature))
        return -1;
    m_dirty = true;
    QMetaMethodBuilder methodBuilder = ensureBuilder()->addSlot(signature);
    methodBuilder.setReturnType(type);
    return m_baseObject->methodCount() + methodBuilder.index();
}

int MetaObjectBuilder::addSlot(const char *signature, const char *type)
{
    return m_d->addSlot(signature, type);
}

int MetaObjectBuilderPrivate::addSignal(const QByteArray &signature)
{
    if (!checkMethodSignature(signature))
        return -1;
    m_dirty = true;
    return m_baseObject->methodCount()
        + ensureBuilder()->addSignal(signature).index();
}

int MetaObjectBuilder::addSignal(const char *signature)
{
    return m_d->addSignal(signature);
}

void MetaObjectBuilderPrivate::removeMethod(QMetaMethod::MethodType mtype,
                                            int index)
{
    index -= m_baseObject->methodCount();
    auto builder = ensureBuilder();
    Q_ASSERT(index >= 0 && index < builder->methodCount());
    switch (mtype) {
    case QMetaMethod::Constructor:
        builder->removeConstructor(index);
        break;
    default:
        builder->removeMethod(index);
        break;
    }
    m_dirty = true;
}

void MetaObjectBuilder::removeMethod(QMetaMethod::MethodType mtype, int index)
{
    m_d->removeMethod(mtype, index);
}

int MetaObjectBuilderPrivate::getPropertyNotifyId(PySideProperty *property) const
{
    int notifyId = -1;
    if (property->d->notify) {
        if (const char *signalNotify = PySide::Property::getNotifyName(property))
            notifyId = indexOfMethod(QMetaMethod::Signal, signalNotify);
    }
    return notifyId;
}

int MetaObjectBuilderPrivate::addProperty(const QByteArray &propertyName,
                                          PyObject *data)
{
    int index = indexOfProperty(propertyName);
    if (index != -1)
        return index;

    PySideProperty *property = reinterpret_cast<PySideProperty *>(data);
    int propertyNotifyId = getPropertyNotifyId(property);
    if (propertyNotifyId >= 0)
        propertyNotifyId -= m_baseObject->methodCount();
    auto newProperty =
        ensureBuilder()->addProperty(propertyName, property->d->typeName,
                                     propertyNotifyId);
    // Adding property attributes
    newProperty.setReadable(PySide::Property::isReadable(property));
    newProperty.setWritable(PySide::Property::isWritable(property));
    newProperty.setResettable(PySide::Property::hasReset(property));
    newProperty.setDesignable(PySide::Property::isDesignable(property));
    newProperty.setScriptable(PySide::Property::isScriptable(property));
    newProperty.setStored(PySide::Property::isStored(property));
    newProperty.setUser(PySide::Property::isUser(property));
    newProperty.setConstant(PySide::Property::isConstant(property));
    newProperty.setFinal(PySide::Property::isFinal(property));

    index = newProperty.index() + m_baseObject->propertyCount();
    m_dirty = true;
    return index;
}

int MetaObjectBuilder::addProperty(const char *property, PyObject *data)
{
    return m_d->addProperty(property, data);
}

void MetaObjectBuilderPrivate::addInfo(const QByteArray &key,
                                       const QByteArray &value)
{
    ensureBuilder()->addClassInfo(key, value);
    m_dirty = true;
}

void MetaObjectBuilder::addInfo(const char *key, const char *value)
{
    m_d->addInfo(key, value);
}

void MetaObjectBuilderPrivate::addInfo(const QMap<QByteArray, QByteArray> &info)
{
    auto builder = ensureBuilder();
    for (auto i = info.constBegin(), end = info.constEnd(); i != end; ++i)
        builder->addClassInfo(i.key(), i.value());
    m_dirty = true;
}

void MetaObjectBuilder::addInfo(const QMap<QByteArray, QByteArray> &info)
{
    m_d->addInfo(info);
}

void MetaObjectBuilder::addEnumerator(const char *name, bool flag, bool scoped,
                                      const EnumValues &entries)
{
    m_d->addEnumerator(name, flag, scoped, entries);
}

void MetaObjectBuilderPrivate::addEnumerator(const char *name, bool flag, bool scoped,
                                             const MetaObjectBuilder::EnumValues &entries)
{
    auto builder = ensureBuilder();
    int have_already = builder->indexOfEnumerator(name);
    if (have_already >= 0)
        builder->removeEnumerator(have_already);
    auto enumbuilder = builder->addEnumerator(name);
    enumbuilder.setIsFlag(flag);
    enumbuilder.setIsScoped(scoped);

    for (const auto &item : entries)
        enumbuilder.addKey(item.first, item.second);
    m_dirty = true;
}

void MetaObjectBuilderPrivate::removeProperty(int index)
{
    index -= m_baseObject->propertyCount();
    auto builder = ensureBuilder();
    Q_ASSERT(index >= 0 && index < builder->propertyCount());
    builder->removeProperty(index);
    m_dirty = true;
}

void MetaObjectBuilder::removeProperty(int index)
{
    m_d->removeProperty(index);
}

// PYSIDE-315: Instead of sorting the items and maybe breaking indices, we
// ensure that the signals and slots are sorted by the improved
// parsePythonType() (signals must go before slots). The order can only
// become distorted if the class is modified after creation. In that
// case, we give a warning.

static QString msgMethodSortOrder(const QMetaObject *mo, int offendingIndex)
{
    QString result;
    QTextStream str(&result);
    str << "\n\n*** Sort Warning ***\nSignals and slots in QMetaObject '"
        << mo->className()
        << "' are not ordered correctly, this may lead to issues.\n";
    const int methodOffset = mo->methodOffset();
    for (int m = methodOffset, methodCount = mo->methodCount(); m < methodCount; ++m) {
        const auto method = mo->method(m);
        str << (m - methodOffset + 1) << (m > offendingIndex ? '!' : ' ')
            << (method.methodType() == QMetaMethod::Signal ? " Signal " : " Slot   ")
            << method.methodSignature() << '\n';
    }
    return result;
}

static void checkMethodOrder(const QMetaObject *metaObject)
{
    const int lastMethod = metaObject->methodCount() - 1;
    for (int m = metaObject->methodOffset(); m < lastMethod; ++m)  {
        if (metaObject->method(m).methodType() == QMetaMethod::Slot
            && metaObject->method(m + 1).methodType() == QMetaMethod::Signal) {
            const auto message = msgMethodSortOrder(metaObject, m);
            PyErr_WarnEx(PyExc_RuntimeWarning, qPrintable(message), 0);
            // Prevent a warning from being turned into an error. We cannot easily unwind.
            PyErr_Clear();
            break;
        }
    }
}

const QMetaObject *MetaObjectBuilderPrivate::update()
{
    if (!m_builder)
        return m_baseObject;
    if (m_cachedMetaObjects.empty() || m_dirty) {
        // PYSIDE-803: The dirty branch needs to be protected by the GIL.
        // This was moved from SignalManager::retrieveMetaObject to here,
        // which is only the update in "return builder->update()".
        Shiboken::GilState gil;
        m_cachedMetaObjects.push_back(m_builder->toMetaObject());
        checkMethodOrder(m_cachedMetaObjects.back());
        m_dirty = false;
    }
    return m_cachedMetaObjects.back();
}

const QMetaObject *MetaObjectBuilder::update()
{
    return m_d->update();
}

using namespace Shiboken;

void MetaObjectBuilderPrivate::parsePythonType(PyTypeObject *type)
{
    // Get all non-QObject-derived base types in method resolution order, filtering out the types
    // that can't have signals, slots or properties.
    // This enforces registering of all signals and slots at type parsing time, and not later at
    // signal connection time, thus making sure no method indices change which would break
    // existing connections.
    const PyObject *mro = type->tp_mro;
    const Py_ssize_t basesCount = PyTuple_GET_SIZE(mro);
    PyTypeObject *qObjectType = Conversions::getPythonTypeObject("QObject*");

    std::vector<PyTypeObject *> basesToCheck;
    // Prepend the actual type that we are parsing.
    basesToCheck.reserve(1u + basesCount);
    basesToCheck.push_back(type);

    auto sbkObjTypeF = reinterpret_cast<PyTypeObject *>(SbkObject_TypeF());
    auto baseObjType = reinterpret_cast<PyTypeObject *>(&PyBaseObject_Type);
    for (Py_ssize_t i = 0; i < basesCount; ++i) {
        auto baseType = reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(mro, i));
        if (baseType != sbkObjTypeF && baseType != baseObjType
            && PyType_IsSubtype(baseType, qObjectType) == 0) {
            basesToCheck.push_back(baseType);
        }
    }

    // PYSIDE-315: Handle all signals first, in all involved types.
    // Leave the properties to be registered after signals because they may depend on
    // notify signals.
    for (PyTypeObject *baseType : basesToCheck) {
        PyObject *attrs = baseType->tp_dict;
        PyObject *key = nullptr;
        PyObject *value = nullptr;
        Py_ssize_t pos = 0;

        while (PyDict_Next(attrs, &pos, &key, &value)) {
            if (Signal::checkType(value)) {
                // Register signals.
                auto data = reinterpret_cast<PySideSignal *>(value);
                if (data->data->signalName.isEmpty())
                    data->data->signalName = String::toCString(key);
                for (const auto &s : data->data->signatures) {
                    const auto sig = data->data->signalName + '(' + s.signature + ')';
                    if (m_baseObject->indexOfSignal(sig) == -1) {
                        // Registering the parameterNames to the QMetaObject (PYSIDE-634)
                        // from:
                        //     Signal(..., arguments=['...', ...]
                        // the arguments are now on data-data->signalArguments
                        if (!data->data->signalArguments->isEmpty()) {
                            m_builder->addSignal(sig).setParameterNames(*data->data->signalArguments);
                        } else {
                            m_builder->addSignal(sig);
                        }
                    }
                }
            }
        }
    }

    AutoDecRef slotAttrName(String::fromCString(PYSIDE_SLOT_LIST_ATTR));
    // PYSIDE-315: Now take care of the rest.
    // Signals and slots should be separated, unless the types are modified, later.
    // We check for this using "is_sorted()". Sorting no longer happens at all.
    for (PyTypeObject *baseType : basesToCheck) {
        PyObject *attrs = baseType->tp_dict;
        PyObject *key = nullptr;
        PyObject *value = nullptr;
        Py_ssize_t pos = 0;

        while (PyDict_Next(attrs, &pos, &key, &value)) {
            if (Property::checkType(value)) {
                const int index = m_baseObject->indexOfProperty(String::toCString(key));
                if (index == -1)
                    addProperty(String::toCString(key), value);
            } else if (Py_TYPE(value)->tp_call != nullptr) {
                // PYSIDE-198: PyFunction_Check does not work with Nuitka.
                // Register slots.
                if (PyObject_HasAttr(value, slotAttrName)) {
                    PyObject *signatureList = PyObject_GetAttr(value, slotAttrName);
                    for (Py_ssize_t i = 0, i_max = PyList_Size(signatureList); i < i_max; ++i) {
                        PyObject *pySignature = PyList_GET_ITEM(signatureList, i);
                        QByteArray signature(String::toCString(pySignature));
                        // Split the slot type and its signature.
                        QByteArray type;
                        const int spacePos = signature.indexOf(' ');
                        if (spacePos != -1) {
                            type = signature.left(spacePos);
                            signature.remove(0, spacePos + 1);
                        }
                        const int index = m_baseObject->indexOfSlot(signature);
                        if (index == -1) {
                            if (type.isEmpty() || type == "void")
                                addSlot(signature);
                            else
                                addSlot(signature, type);
                        }
                    }
                }
            }
        }
    }
    // PYSIDE-957: Collect the delayed QEnums
    auto collectedEnums = PySide::QEnum::resolveDelayedQEnums(type);
    for (PyObject *obEnumType : collectedEnums) {
        bool isFlag = PySide::QEnum::isFlag(obEnumType);
        AutoDecRef obName(PyObject_GetAttr(obEnumType, PyMagicName::name()));
        // Everything has been checked already in resolveDelayedQEnums.
        // Therefore, we don't need to error-check here again.
        auto name = String::toCString(obName);
        AutoDecRef members(PyObject_GetAttr(obEnumType, PyMagicName::members()));
        AutoDecRef items(PyMapping_Items(members));
        Py_ssize_t nr_items = PySequence_Length(items);

        QList<QPair<QByteArray, int> > entries;
        for (Py_ssize_t idx = 0; idx < nr_items; ++idx) {
            AutoDecRef item(PySequence_GetItem(items, idx));
            AutoDecRef key(PySequence_GetItem(item, 0));
            AutoDecRef member(PySequence_GetItem(item, 1));
            AutoDecRef value(PyObject_GetAttr(member, Shiboken::PyName::value()));
            auto ckey = String::toCString(key);
            auto ivalue = PyInt_AsSsize_t(value);   // int/long cheating
            auto thing = QPair<QByteArray, int>(ckey, int(ivalue));
            entries.push_back(thing);
        }
        addEnumerator(name, isFlag, true, entries);
    }
}
