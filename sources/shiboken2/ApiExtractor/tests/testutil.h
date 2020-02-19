/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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

#ifndef TESTUTIL_H
#define TESTUTIL_H
#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include "abstractmetabuilder.h"
#include "reporthandler.h"
#include "typedatabase.h"

namespace TestUtil
{
    static AbstractMetaBuilder *parse(const char *cppCode, const char *xmlCode,
                                      bool silent = true,
                                      const QString &apiVersion = QString(),
                                      const QStringList &dropTypeEntries = QStringList())
    {
        ReportHandler::setSilent(silent);
        ReportHandler::startTimer();
        TypeDatabase* td = TypeDatabase::instance(true);
        if (apiVersion.isEmpty())
            TypeDatabase::clearApiVersions();
        else if (!TypeDatabase::setApiVersion(QLatin1String("*"), apiVersion))
            return nullptr;
        td->setDropTypeEntries(dropTypeEntries);
        QBuffer buffer;
        // parse typesystem
        buffer.setData(xmlCode);
        if (!buffer.open(QIODevice::ReadOnly))
            return Q_NULLPTR;
        if (!td->parseFile(&buffer))
            return nullptr;
        buffer.close();
        // parse C++ code
        QTemporaryFile tempSource(QDir::tempPath() + QLatin1String("/st_XXXXXX_main.cpp"));
        if (!tempSource.open()) {
            qWarning().noquote().nospace() << "Creation of temporary file failed: "
                << tempSource.errorString();
            return nullptr;
        }
        QByteArrayList arguments;
        arguments.append(QFile::encodeName(tempSource.fileName()));
        tempSource.write(cppCode, qint64(strlen(cppCode)));
        tempSource.close();
        auto *builder = new AbstractMetaBuilder;
        if (!builder->build(arguments)) {
            delete builder;
            return Q_NULLPTR;
        }
        return builder;
    }
} // namespace TestUtil

#endif
