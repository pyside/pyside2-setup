/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

#ifndef INCLUDE_H
#define INCLUDE_H

#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

class Include
{
public:
    enum IncludeType {
        IncludePath,
        LocalPath,
        TargetLangImport
    };

    Include() : m_type(IncludePath) {}
    Include(IncludeType t, const QString &nam) : m_type(t), m_name(nam) {};

    bool isValid() const
    {
        return !m_name.isEmpty();
    }

    IncludeType type() const
    {
        return m_type;
    }

    QString name() const
    {
        return m_name;
    }

    QString toString() const;

    bool operator<(const Include& other) const
    {
        return m_name < other.m_name;
    }

    bool operator==(const Include& other) const
    {
        return m_type == other.m_type && m_name == other.m_name;
    }

    friend uint qHash(const Include&);
    private:
        IncludeType m_type;
        QString m_name;
};

uint qHash(const Include& inc);
QTextStream& operator<<(QTextStream& out, const Include& include);
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const Include &i);
#endif

typedef QList<Include> IncludeList;

#endif
