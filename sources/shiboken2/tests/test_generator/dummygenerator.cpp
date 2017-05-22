/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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

#include <iostream>
#include "dummygenerator.h"

EXPORT_GENERATOR_PLUGIN(new DummyGenerator)

using namespace std;

QString
DummyGenerator::fileNameForClass(const AbstractMetaClass* metaClass) const
{
    return QString("%1_generated.txt").arg(metaClass->name().toLower());
}

void
DummyGenerator::generateClass(QTextStream& s, const AbstractMetaClass* metaClass)
{
    s << "// Generated code for class: " << qPrintable(metaClass->name()) << endl;
}

bool
DummyGenerator::doSetup(const QMap<QString, QString>& args)
{
    if (args.contains("dump-arguments") && !args["dump-arguments"].isEmpty()) {
        QFile logFile(args["dump-arguments"]);
        logFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&logFile);
        for (QMap<QString, QString>::const_iterator it = args.cbegin(), end = args.cend(); it != end; ++it) {
            const QString& key = it.key();
            if (key == "arg-1")
                out << "header-file";
            else if (key == "arg-2")
                out << "typesystem-file";
            else
                out << key;
            if (!args[key].isEmpty())
                out << " = " << args[key];
            out << endl;
        }
    }
    return true;
}

