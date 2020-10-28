/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef HEADER_PATHS_H
#define HEADER_PATHS_H

#include <QByteArray>
#include <QVector>
#include <QString>

enum class HeaderType
{
    Standard,
    System,         // -isystem
    Framework,      // macOS framework path
    FrameworkSystem // macOS framework system path
};

class HeaderPath {
public:
    QByteArray path;
    HeaderType type;

    static QByteArray includeOption(const HeaderPath &p)
    {
        QByteArray option;
        switch (p.type) {
        case HeaderType::Standard:
            option = QByteArrayLiteral("-I");
            break;
        case HeaderType::System:
            option = QByteArrayLiteral("-isystem");
            break;
        case HeaderType::Framework:
            option = QByteArrayLiteral("-F");
            break;
        case HeaderType::FrameworkSystem:
            option = QByteArrayLiteral("-iframework");
            break;
        }
        return option + p.path;
    }
};

using HeaderPaths = QVector<HeaderPath>;

#endif // HEADER_PATHS_H
