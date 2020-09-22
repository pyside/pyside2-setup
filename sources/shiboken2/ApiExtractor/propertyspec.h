/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PROPERTYSPEC_H
#define PROPERTYSPEC_H

#include <QtCore/QStringList>

class AbstractMetaClass;
class AbstractMetaBuilderPrivate;
class TypeEntry;

QT_FORWARD_DECLARE_CLASS(QDebug)

class QPropertySpec
{
public:
    Q_DISABLE_COPY_MOVE(QPropertySpec)

    explicit QPropertySpec(const TypeEntry *type) : m_type(type) {}

    static QPropertySpec *parseQ_Property(AbstractMetaBuilderPrivate *b,
                                          AbstractMetaClass *metaClass,
                                          const QString &declarationIn,
                                          const QStringList &scopes,
                                          QString *errorMessage);

    bool isValid() const;

    const TypeEntry *type() const { return m_type; }

    int indirections() const { return m_indirections; }
    void setIndirections(int indirections) { m_indirections = indirections; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString read() const { return m_read; }
    void setRead(const QString &read) { m_read = read; }

    QString write() const { return m_write; }
    void setWrite(const QString &write) { m_write = write; }

    QString designable() const { return m_designable; }
    void setDesignable(const QString &designable) { m_designable = designable; }

    QString reset() const { return m_reset; }
    void setReset(const QString &reset) { m_reset = reset; }

    int index() const { return m_index; }
    void setIndex(int index) {m_index = index; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    QString m_name;
    QString m_read;
    QString m_write;
    QString m_designable;
    QString m_reset;
    const TypeEntry *m_type;
    int m_indirections = 0;
    int m_index = -1;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QPropertySpec &p);
#endif

#endif // PROPERTYSPEC_H
