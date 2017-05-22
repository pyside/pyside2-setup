/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class PreprocessorPrivate;

class Preprocessor
{
public:
    Preprocessor();
    ~Preprocessor();

    void processFile(const QString &fileName);
    void processString(const QByteArray &str);

    void addIncludePaths(const QStringList &includePaths);

    QByteArray result() const;

    QStringList macroNames() const;

    struct MacroItem {
        QString name;
        QStringList parameters;
        QString definition;
        bool isFunctionLike;
#ifdef PP_WITH_MACRO_POSITION
        QString fileName;
#endif
    };
    QList<MacroItem> macros() const;

private:
    Q_DISABLE_COPY(Preprocessor)
    PreprocessorPrivate *d;
};

#endif
