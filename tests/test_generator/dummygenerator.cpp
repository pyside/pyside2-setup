/*
 * This file is part of the PySide project.
 *
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

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
        foreach (const QString& key, args.keys()) {
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

