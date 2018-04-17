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

#ifndef FILEOUT_H
#define FILEOUT_H

#include <QtCore/QObject>
#include <QtCore/QTextStream>

QT_FORWARD_DECLARE_CLASS(QFile)

class FileOut : public QObject
{
private:
    QByteArray tmp;
    QString name;

public:
    enum State { Failure, Unchanged, Success };

    FileOut(QString name);
    ~FileOut()
    {
        if (!isDone)
            done();
    }

    State done();
    State done(QString *errorMessage);

    static QString msgCannotOpenForReading(const QFile &f);
    static QString msgCannotOpenForWriting(const QFile &f);

    QTextStream stream;

    static bool dummy;
    static bool diff;

private:
    bool isDone;
};

#endif // FILEOUT_H
