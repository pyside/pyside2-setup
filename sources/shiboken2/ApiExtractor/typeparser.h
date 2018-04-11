/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt for Python project.
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

#ifndef TYPEPARSER_H
#define TYPEPARSER_H

#include "parser/codemodel_enums.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

QT_FORWARD_DECLARE_CLASS(QDebug)

class TypeParser
{
public:
    struct Info
    {
        Info() : referenceType(NoReference), is_constant(false), is_busted(false), indirections(0) { }
        QStringList qualified_name;
        QStringList arrays;
        QVector<Info> template_instantiations;
        ReferenceType referenceType;
        uint is_constant : 1;
        uint is_busted : 1;
        uint indirections : 6;

        QString toString() const;
        QString instantiationName() const;
    };

    static Info parse(const QString &str, QString *errorMessage = Q_NULLPTR);
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeParser::Info &);
#endif

#endif // TYPEPARSER_H
