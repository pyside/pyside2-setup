/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
// @snippet uitools-loadui
/*
 * Based on code provided by:
 *          Antonio Valentino <antonio.valentino at tiscali.it>
 *          Frédéric <frederic.mantegazza at gbiloba.org>
 */

#include <shiboken.h>
#include <QUiLoader>
#include <QFile>
#include <QWidget>

static void createChildrenNameAttributes(PyObject *root, QObject *object)
{
    for (auto *child : object->children()) {
        const QByteArray name = child->objectName().toLocal8Bit();

        if (!name.isEmpty() && !name.startsWith("_") && !name.startsWith("qt_")) {
            Shiboken::AutoDecRef attrName(Py_BuildValue("s", name.constData()));
            if (!PyObject_HasAttr(root, attrName)) {
                Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject *](child));
                PyObject_SetAttr(root, attrName, pyChild);
            }
            createChildrenNameAttributes(root, child);
        }
        createChildrenNameAttributes(root, child);
    }
}

static PyObject *QUiLoadedLoadUiFromDevice(QUiLoader *self, QIODevice *dev, QWidget *parent)
{
    QWidget *wdg = self->load(dev, parent);

    if (wdg) {
        PyObject *pyWdg = %CONVERTTOPYTHON[QWidget *](wdg);
        createChildrenNameAttributes(pyWdg, wdg);
        if (parent) {
            Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](parent));
            Shiboken::Object::setParent(pyParent, pyWdg);
        }
        return pyWdg;
    }

    if (!PyErr_Occurred())
        PyErr_SetString(PyExc_RuntimeError, "Unable to open/read ui device");
    return nullptr;
}

static PyObject *QUiLoaderLoadUiFromFileName(QUiLoader *self, const QString &uiFile, QWidget *parent)
{
    QFile fd(uiFile);
    return QUiLoadedLoadUiFromDevice(self, &fd, parent);
}
// @snippet uitools-loadui

// @snippet quiloader
Q_IMPORT_PLUGIN(PyCustomWidgets);
// @snippet quiloader

// @snippet quiloader-registercustomwidget
registerCustomWidget(%PYARG_1);
%CPPSELF.addPluginPath(""); // force reload widgets
// @snippet quiloader-registercustomwidget

// @snippet quiloader-load-1
// Avoid calling the original function: %CPPSELF.%FUNCTION_NAME()
%PYARG_0 = QUiLoadedLoadUiFromDevice(%CPPSELF, %1, %2);
// @snippet quiloader-load-1

// @snippet quiloader-load-2
// Avoid calling the original function: %CPPSELF.%FUNCTION_NAME()
%PYARG_0 = QUiLoaderLoadUiFromFileName(%CPPSELF, %1, %2);
// @snippet quiloader-load-2
