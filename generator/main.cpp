/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "shibokenconfig.h"
#include <iostream>
#include <QtCore>

int main(int argc, char* argv[])
{
    QStringList args;
    args.append("--generator-set=shiboken");
    for (int i = 1; i < argc; i++) {
        if (QString("--version") == argv[i]) {
            std::cout << "shiboken v" SHIBOKEN_VERSION << std::endl;
            std::cout << "Copyright (C) 2009-2011 Nokia Corporation and/or its subsidiary(-ies)" << std::endl;
            return EXIT_SUCCESS;
        }
        args.append(argv[i]);
    }

    /* This will be necessary when the Generatorrunner calls the printUsage()
     * function and the message that'll printed on the screen shows "shiboken"
     * instead of "generator".
     *
     * Note: this argument doesn't do anything else other than just to set up
     * the message that will be printed on the screen when calling printUsage()
     * from Generatorrunner.
     */
    args.append("--alias-name=shiboken");

    return QProcess::execute(GENERATOR_BINARY, args);
}

