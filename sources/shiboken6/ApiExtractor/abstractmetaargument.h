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

#ifndef ABSTRACTMETAARGUMENT_H
#define ABSTRACTMETAARGUMENT_H

#include "abstractmetalang_typedefs.h"
#include "abstractmetatype.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include <QtCore/QSharedDataPointer>

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMetaArgumentData;
class Documentation;

class AbstractMetaArgument
{
public:
    AbstractMetaArgument();
    ~AbstractMetaArgument();
    AbstractMetaArgument(const AbstractMetaArgument &);
    AbstractMetaArgument &operator=(const AbstractMetaArgument &);
    AbstractMetaArgument(AbstractMetaArgument &&);
    AbstractMetaArgument &operator=(AbstractMetaArgument &&);


    const AbstractMetaType &type() const;
    void setType(const AbstractMetaType &type);

    QString name() const;
    void setName(const QString &name, bool realName = true);
    bool hasName() const;

    void setDocumentation(const Documentation& doc);
    Documentation documentation() const;

    QString defaultValueExpression() const;
    void setDefaultValueExpression(const QString &expr);

    QString originalDefaultValueExpression() const;
    void setOriginalDefaultValueExpression(const QString &expr);

    bool hasDefaultValueExpression() const;
    bool hasOriginalDefaultValueExpression() const;
    bool hasUnmodifiedDefaultValueExpression() const;
    bool hasModifiedDefaultValueExpression() const;

    QString toString() const;

    int argumentIndex() const;
    void setArgumentIndex(int argIndex);

private:
    QSharedDataPointer<AbstractMetaArgumentData> d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaArgument &aa);
QDebug operator<<(QDebug d, const AbstractMetaArgument *aa);
#endif

#endif // ABSTRACTMETAARGUMENT_H
