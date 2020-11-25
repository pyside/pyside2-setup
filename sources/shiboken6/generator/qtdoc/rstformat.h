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

#ifndef RSTFORMAT_H
#define RSTFORMAT_H

#include <textstream.h>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QVersionNumber>

struct rstVersionAdded
{
    explicit rstVersionAdded(const QVersionNumber &v) : m_version(v) {}

    const QVersionNumber m_version;
};

inline TextStream &operator<<(TextStream &s, const rstVersionAdded &v)
{
    s << ".. versionadded:: "<< v.m_version.toString() << "\n\n";
    return s;
}

inline QByteArray rstDeprecationNote(const char *what)
{
    return QByteArrayLiteral(".. note:: This ")
        + what + QByteArrayLiteral(" is deprecated.\n\n");
}

class Pad
{
public:
    explicit Pad(char c, int count) : m_char(c), m_count(count) {}

    void write(TextStream &str) const
    {
        for (int i = 0; i < m_count; ++i)
            str << m_char;
    }

private:
    const char m_char;
    const int m_count;
};

inline TextStream &operator<<(TextStream &str, const Pad &pad)
{
    pad.write(str);
    return str;
}

template <class String>
inline int writeEscapedRstText(TextStream &str, const String &s)
{
    int escaped = 0;
    for (const QChar &c : s) {
        switch (c.unicode()) {
        case '*':
        case '`':
        case '_':
        case '\\':
            str << '\\';
            ++escaped;
            break;
        }
        str << c;
    }
    return s.size() + escaped;
}

class escape
{
public:
    explicit escape(QStringView s) : m_string(s) {}

    void write(TextStream &str) const { writeEscapedRstText(str, m_string); }

private:
    const QStringView m_string;
};

inline TextStream &operator<<(TextStream &str, const escape &e)
{
    e.write(str);
    return str;
}

// RST anchor string: Anything else but letters, numbers, '_' or '.' replaced by '-'
inline bool isValidRstLabelChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.');
}

inline QString toRstLabel(QString s)
{
    for (int i = 0, size = s.size(); i < size; ++i) {
        if (!isValidRstLabelChar(s.at(i)))
            s[i] = QLatin1Char('-');
    }
    return s;
}

class rstLabel
{
public:
    explicit rstLabel(const QString &l) : m_label(l) {}

    friend TextStream &operator<<(TextStream &str, const rstLabel &a)
    {
        str << ".. _" << toRstLabel(a.m_label) << ":\n\n";
        return str;
    }

private:
    const QString &m_label;
};

#endif // RSTFORMAT_H
