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
class AbstractMetaType;
class TypeEntry;

struct TypeSystemProperty;

QT_FORWARD_DECLARE_CLASS(QDebug)

class QPropertySpec
{
public:
    Q_DISABLE_COPY_MOVE(QPropertySpec)

    explicit QPropertySpec(const TypeSystemProperty &ts,
                           const AbstractMetaType *type);
    ~QPropertySpec();

    static TypeSystemProperty typeSystemPropertyFromQ_Property(const QString &declarationIn,
                                                               QString *errorMessage);

    static QPropertySpec *fromTypeSystemProperty(AbstractMetaBuilderPrivate *b,
                                                 AbstractMetaClass *metaClass,
                                                 const TypeSystemProperty &ts,
                                                 const QStringList &scopes,
                                                 QString *errorMessage);

    static QPropertySpec *parseQ_Property(AbstractMetaBuilderPrivate *b,
                                          AbstractMetaClass *metaClass,
                                          const QString &declarationIn,
                                          const QStringList &scopes,
                                          QString *errorMessage);

    bool isValid() const;

    const AbstractMetaType *type() const { return m_type; }
    void setType(AbstractMetaType *t) { m_type = t; }

    const TypeEntry *typeEntry() const;

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString read() const { return m_read; }
    void setRead(const QString &read) { m_read = read; }

    QString write() const { return m_write; }
    void setWrite(const QString &write) { m_write = write; }
    bool hasWrite() const { return !m_write.isEmpty(); }

    QString designable() const { return m_designable; }
    void setDesignable(const QString &designable) { m_designable = designable; }

    QString reset() const { return m_reset; }
    void setReset(const QString &reset) { m_reset = reset; }

    int index() const { return m_index; }
    void setIndex(int index) {m_index = index; }

    bool generateGetSetDef() const { return m_generateGetSetDef; }
    void setGenerateGetSetDef(bool generateGetSetDef) { m_generateGetSetDef = generateGetSetDef; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    QString m_name;
    QString m_read;
    QString m_write;
    QString m_designable;
    QString m_reset;
    const AbstractMetaType *m_type = nullptr;
    int m_index = -1;
    // Indicates whether actual code is generated instead of relying on libpyside.
    bool m_generateGetSetDef = false;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QPropertySpec &p);
#endif

#endif // PROPERTYSPEC_H
