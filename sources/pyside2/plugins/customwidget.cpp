/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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


#include "customwidget.h"


struct PyCustomWidgetPrivate
{
    PyObject* pyObject;
    bool initialized;
};

PyCustomWidget::PyCustomWidget(PyObject* objectType)
    : m_data(new PyCustomWidgetPrivate())
{
    m_data->pyObject = objectType;
    m_name = QString(reinterpret_cast<PyTypeObject*>(objectType)->tp_name);
}

PyCustomWidget::~PyCustomWidget()
{
    delete m_data;
}

bool PyCustomWidget::isContainer() const
{
    return false;
}

bool PyCustomWidget::isInitialized() const
{
    return m_data->initialized;
}

QIcon PyCustomWidget::icon() const
{
    return QIcon();
}

QString PyCustomWidget::domXml() const
{
    return QString();
}

QString PyCustomWidget::group() const
{
    return QString();
}

QString PyCustomWidget::includeFile() const
{
    return QString();
}

QString PyCustomWidget::name() const
{
    return m_name;
}

QString PyCustomWidget::toolTip() const
{
    return QString();
}

QString PyCustomWidget::whatsThis() const
{
    return QString();
}

QWidget* PyCustomWidget::createWidget(QWidget* parent)
{
    //Create a python instance and return cpp object
    PyObject* pyParent;
    bool unkowParent = false;
    if (parent) {
        pyParent = reinterpret_cast<PyObject*>(Shiboken::BindingManager::instance().retrieveWrapper(parent));
        if (pyParent) {
            Py_INCREF(pyParent);
        } else {
            static Shiboken::Conversions::SpecificConverter converter("QWidget*");
            pyParent = converter.toPython(&parent);
            unkowParent = true;
        }
    } else {
        Py_INCREF(Py_None);
        pyParent = Py_None;
    }

    Shiboken::AutoDecRef pyArgs(PyTuple_New(1));
    PyTuple_SET_ITEM(pyArgs, 0, pyParent); //tuple will keep pyParent reference

    //Call python constructor
    SbkObject* result = reinterpret_cast<SbkObject*>(PyObject_CallObject(m_data->pyObject, pyArgs));

    QWidget* widget = 0;
    if (result) {
        if (unkowParent) //if parent does not exists in python, transfer the ownership to cpp
            Shiboken::Object::releaseOwnership(result);
        else
            Shiboken::Object::setParent(pyParent, reinterpret_cast<PyObject*>(result));

        widget = reinterpret_cast<QWidget*>(Shiboken::Object::cppPointer(result, Py_TYPE(result)));
    }

    return widget;
}

void PyCustomWidget::initialize(QDesignerFormEditorInterface* core)
{
    m_data->initialized = true;
}
