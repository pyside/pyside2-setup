/*
 * This file is part of the Boost Python Generator project.
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

#include <QtCore/QCoreApplication>
#include <apiextractor/apiextractor.h>
#include "hppgenerator.h"
#include "cppgenerator.h"
#include "hppgenerator.h"
#include "convertergenerator.h"
#include "docgenerator.h"
#include "boostpythongeneratorversion.h"
#include <iostream>

void showVersion(const char* apiextractor_version) {
    using namespace std;

    cout << "BoostPythonGenerator v" BOOSTPYTHONGENERATOR_VERSION << " using " << apiextractor_version << endl;
    cout << "Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)" << endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv); // needed by qxmlpatterns

    ApiExtractor extractor(argc, argv);
    extractor.addGenerator(new HppGenerator);
    extractor.addGenerator(new CppGenerator);
    extractor.addGenerator(new ConverterGenerator);
    extractor.addGenerator(new DocGenerator);
    extractor.setVersionHandler(&showVersion);
    return extractor.exec();
}
