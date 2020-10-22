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

#include "abstractmetaargument.h"
#include "documentation.h"

#include <QtCore/QDebug>
#include <QtCore/QSharedData>

class AbstractMetaArgumentData : public QSharedData
{
public:
    QString toString() const;

    QString m_name;
    AbstractMetaType m_type;
    bool m_hasName = false;
    Documentation m_doc;
    QString m_expression;
    QString m_originalExpression;
    int m_argumentIndex = 0;
};

AbstractMetaArgument::AbstractMetaArgument() : d(new AbstractMetaArgumentData)
{
}

AbstractMetaArgument::~AbstractMetaArgument() = default;

AbstractMetaArgument::AbstractMetaArgument(const AbstractMetaArgument &) = default;

AbstractMetaArgument &AbstractMetaArgument::operator=(const AbstractMetaArgument &) = default;

AbstractMetaArgument::AbstractMetaArgument(AbstractMetaArgument &&) = default;

AbstractMetaArgument &AbstractMetaArgument::operator=(AbstractMetaArgument &&) = default;

const AbstractMetaType &AbstractMetaArgument::type() const
{
    return d->m_type;
}

void AbstractMetaArgument::setType(const AbstractMetaType &type)
{
    if (d->m_type != type)
        d->m_type = type;
}

QString AbstractMetaArgument::name() const
{
    return d->m_name;
}

void AbstractMetaArgument::setName(const QString &name, bool realName)
{
    if (d->m_name != name || d->m_hasName != realName) {
        d->m_name = name;
        d->m_hasName = realName;
    }
}

bool AbstractMetaArgument::hasName() const
{
    return d->m_hasName;
}

void AbstractMetaArgument::setDocumentation(const Documentation &doc)
{
    if (d->m_doc != doc)
        d->m_doc = doc;
}

Documentation AbstractMetaArgument::documentation() const
{
    return d->m_doc;
}

QString AbstractMetaArgument::defaultValueExpression() const
{
    return d->m_expression;
}

void AbstractMetaArgument::setDefaultValueExpression(const QString &expr)
{
    if (d->m_expression != expr)
        d->m_expression = expr;
}

QString AbstractMetaArgument::originalDefaultValueExpression() const
{
    return d->m_originalExpression;
}

void AbstractMetaArgument::setOriginalDefaultValueExpression(const QString &expr)
{
    if (d->m_originalExpression != expr)
        d->m_originalExpression = expr;
}

bool AbstractMetaArgument::hasOriginalDefaultValueExpression() const
{
    return !d->m_originalExpression.isEmpty();
}

bool AbstractMetaArgument::hasDefaultValueExpression() const
{
    return !d->m_expression.isEmpty();
}

bool AbstractMetaArgument::hasUnmodifiedDefaultValueExpression() const
{
    return !d->m_originalExpression.isEmpty() && d->m_originalExpression == d->m_expression;
}

bool AbstractMetaArgument::hasModifiedDefaultValueExpression() const
{
    return !d->m_expression.isEmpty() && d->m_originalExpression != d->m_expression;
}

QString AbstractMetaArgumentData::toString() const
{
    QString result = m_type.name() + QLatin1Char(' ') + m_name;
    if (!m_expression.isEmpty())
        result += QLatin1String(" = ") + m_expression;
    return result;
}

QString AbstractMetaArgument::toString() const
{
    return d->toString();
}

int AbstractMetaArgument::argumentIndex() const
{
    return d->m_argumentIndex;
}

void AbstractMetaArgument::setArgumentIndex(int argIndex)
{
    if (d->m_argumentIndex != argIndex)
        d->m_argumentIndex = argIndex;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaArgument &aa)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaArgument(" << aa.toString() << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaArgument *aa)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaArgument(";
    if (aa)
        d << aa->toString();
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
