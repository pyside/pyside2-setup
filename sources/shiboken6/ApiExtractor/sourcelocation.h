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

#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#include <QString>

QT_FORWARD_DECLARE_CLASS(QDebug)
QT_FORWARD_DECLARE_CLASS(QTextStream)

class SourceLocation
{
public:
    explicit SourceLocation(const QString &file, int l);
    SourceLocation();

    bool isValid() const;

    QString fileName() const;
    void setFileName(const QString &fileName);

    int lineNumber() const;
    void setLineNumber(int lineNumber);

    QString toString() const;

    template<class Stream>
    void format(Stream &s) const;

private:
    QString m_fileName;
    int m_lineNumber = 0;
};

QTextStream &operator<<(QTextStream &s, const SourceLocation &l);

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const SourceLocation &l);
#endif

#endif // SOURCE_LOCATION_H
