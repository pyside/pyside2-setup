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

#ifndef MODIFICATIONS_P_H
#define MODIFICATIONS_P_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringView>

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE

// Helpers to split a parameter list of <add-function>, <declare-function>
// in a separate header for testing purposes

namespace AddedFunctionParser {

struct Argument
{
    bool equals(const Argument &rhs) const;

    QString type;
    QString name;
    QString defaultValue;
};

using Arguments = QList<Argument>;

inline bool operator==(const Argument &a1, const Argument &a2) { return a1.equals(a2); }
inline bool operator!=(const Argument &a1, const Argument &a2) { return !a1.equals(a2); }

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const Argument &a);
#endif

Arguments splitParameters(QStringView paramString, QString *errorMessage = nullptr);

} // namespace AddedFunctionParser

#endif // MODIFICATIONS_P_H
