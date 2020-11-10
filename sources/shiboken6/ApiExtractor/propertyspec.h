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

class AbstractMetaType;

#include <QtCore/QStringList>
#include <QtCore/QSharedDataPointer>

#include <optional>

class AbstractMetaClass;
class AbstractMetaBuilderPrivate;
class AbstractMetaType;
class TypeEntry;

struct TypeSystemProperty;

class QPropertySpecData;

QT_FORWARD_DECLARE_CLASS(QDebug)

class QPropertySpec
{
public:
    explicit QPropertySpec(const TypeSystemProperty &ts,
                           const AbstractMetaType &type);
    QPropertySpec(const QPropertySpec &);
    QPropertySpec &operator=(const QPropertySpec &);
    QPropertySpec(QPropertySpec &&);
    QPropertySpec &operator=(QPropertySpec &&);
    ~QPropertySpec();

    static TypeSystemProperty typeSystemPropertyFromQ_Property(const QString &declarationIn,
                                                               QString *errorMessage);


    static std::optional<QPropertySpec>
        fromTypeSystemProperty(AbstractMetaBuilderPrivate *b,
                               AbstractMetaClass *metaClass,
                               const TypeSystemProperty &ts,
                               const QStringList &scopes,
                               QString *errorMessage);

    static std::optional<QPropertySpec>
        parseQ_Property(AbstractMetaBuilderPrivate *b,
                        AbstractMetaClass *metaClass,
                        const QString &declarationIn,
                        const QStringList &scopes,
                        QString *errorMessage);

    const AbstractMetaType &type() const;
    void setType(const AbstractMetaType &t);

    const TypeEntry *typeEntry() const;

    QString name() const;
    void setName(const QString &name);

    QString read() const;
    void setRead(const QString &read);

    QString write() const;
    void setWrite(const QString &write);
    bool hasWrite() const;

    QString designable() const;
    void setDesignable(const QString &designable);

    QString reset() const;
    void setReset(const QString &reset);

    int index() const;
    void setIndex(int index);

    // Indicates whether actual code is generated instead of relying on libpyside.
    bool generateGetSetDef() const;
    void setGenerateGetSetDef(bool generateGetSetDef);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    QSharedDataPointer<QPropertySpecData> d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QPropertySpec &p);
#endif

#endif // PROPERTYSPEC_H
