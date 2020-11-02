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

#ifndef DYNAMICMETAPROPERTY_P_H
#define DYNAMICMETAPROPERTY_P_H

#include <sbkpython.h>

#include <QtCore/QByteArray>
#include <QtCore/QMetaMethod>

struct PySideProperty;
namespace PySide
{
    class MethodData
    {
    public:
        MethodData();
        /**
         * \param signature method signature
         * \param type method return type
         */
        MethodData(QMetaMethod::MethodType mtype,
                   const QByteArray &signature,
                   const QByteArray &rtype = QByteArray("void"));
        void clear();
        bool isValid() const;
        const QByteArray &signature() const { return m_signature; }
        const QByteArray &returnType() const { return m_rtype; }
        QMetaMethod::MethodType methodType() const { return m_mtype; }
        //Qt5 moc: now we have to store method parameter names, count, type
        QList<QByteArray> parameterTypes() const;
        int parameterCount() const;
        QByteArray name() const;
        bool operator==(const MethodData &other) const;

    private:
        QByteArray m_signature;
        QByteArray m_rtype;
        QMetaMethod::MethodType m_mtype;
        static const QByteArray m_emptySig;
    };

    class PropertyData
    {
    public:
        PropertyData();
        PropertyData(const char *name, int cachedNotifyId = 0, PySideProperty *data = 0);
        const QByteArray &name() const { return m_name; }
        PySideProperty *data() const { return m_data; }
        QByteArray type() const;
        uint flags() const;
        bool isValid() const;
        int cachedNotifyId() const;
        bool operator==(const PropertyData &other) const;
        bool operator==(const char *name) const;

    private:
        QByteArray m_name;
        int m_cachedNotifyId;
        PySideProperty *m_data;
    };

inline bool MethodData::operator==(const MethodData &other) const
{
    return m_mtype == other.methodType() && m_signature == other.signature();
}

}

#endif
