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

static QObject* _findChildHelper(const QObject* parent, const QString& name, PyTypeObject* desiredType)
{
    foreach(QObject* child, parent->children()) {
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject*](child));
        if (PyType_IsSubtype(pyChild->ob_type, desiredType)
            && (name.isNull() || name == child->objectName())) {
            return child;
        }
    }

    QObject* obj;
    foreach(QObject* child, parent->children()) {
        obj = _findChildHelper(child, name, desiredType);
        if (obj)
            return obj;
    }
    return 0;
}

static inline bool _findChildrenComparator(const QObject*& child, const QRegExp& name)
{
    return name.indexIn(child->objectName()) != -1;
}

static inline bool _findChildrenComparator(const QObject*& child, const QString& name)
{
    return name.isNull() || name == child->objectName();
}

template<typename T>
static void _findChildrenHelper(const QObject* parent, const T& name, PyTypeObject* desiredType, PyObject* result)
{
    foreach(const QObject* child, parent->children()) {
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject*](child));
        if (PyType_IsSubtype(pyChild->ob_type, desiredType) && _findChildrenComparator(child, name))
            PyList_Append(result, pyChild);
        _findChildrenHelper(child, name, desiredType, result);
    }
}
