/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef HEADER_PATHS_H
#define HEADER_PATHS_H

#include <QByteArray>
#include <QList>
#include <QString>

class HeaderPath {
public:
    explicit HeaderPath(const QByteArray &p = QByteArray()) : path(p), m_isFramework(false) {}
    explicit HeaderPath(const QString &s = QString(), bool isFramework = false) :
        path(s.toLatin1()), m_isFramework(isFramework) {}

    QByteArray path;
    bool m_isFramework; // macOS framework path

    static QByteArray includeOption(const HeaderPath &p, bool systemInclude = false)
    {
        QByteArray option;

        if (p.m_isFramework) {
            if (systemInclude)
                option = QByteArrayLiteral("-iframework");
            else
                option = QByteArrayLiteral("-F");
        } else if (systemInclude) {
            option = QByteArrayLiteral("-isystem");
        } else {
            option = QByteArrayLiteral("-I");
        }

        return option + p.path;
    }
};

typedef QList<HeaderPath> HeaderPaths;

#endif // HEADER_PATHS_H
