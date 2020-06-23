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

#include "sourcelocation.h"
#include <QtCore/QDir>
#include <QtCore/QDebug>

SourceLocation::SourceLocation() = default;

SourceLocation::SourceLocation(const QString &file, int l)
    : m_fileName(file), m_lineNumber(l)
{
}

bool SourceLocation::isValid() const
{
    return m_lineNumber >= 0 && !m_fileName.isEmpty();
}

QString SourceLocation::fileName() const
{
    return m_fileName;
}

void SourceLocation::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

int SourceLocation::lineNumber() const
{
    return m_lineNumber;
}

void SourceLocation::setLineNumber(int lineNumber)
{
    m_lineNumber = lineNumber;
}

QString  SourceLocation::toString() const
{
    QString result;
    QTextStream s(&result);
    format(s);
    return result;
}

template<class Stream>
void SourceLocation::format(Stream &s) const
{
    if (isValid())
        s << QDir::toNativeSeparators(m_fileName) << ':' << m_lineNumber << ':';
    else
        s << "<unknown>";
}

QTextStream &operator<<(QTextStream &s, const SourceLocation &l)
{
    if (l.isValid()) {
        l.format(s);
        s << '\t'; // ":\t" is used by ReportHandler for filtering suppressions
    }
    return s;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const SourceLocation &l)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    l.format(d);
    return d;
}
#endif
