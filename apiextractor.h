/*
 * This file is part of the API Extractor project.
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

#ifndef APIEXTRACTOR_H
#define APIEXTRACTOR_H

#include <QLinkedList>
#include <QMap>
#include <QString>

class Generator;

class ApiExtractor
{
public:
    ApiExtractor(int argc, char** argv);
    ~ApiExtractor();

    void addGenerator(Generator* generator);
    void setVersionHandler(void (*versionHandler)(const char*))
    {
        m_versionHandler = versionHandler;
    }

    int exec();

private:
    QLinkedList<Generator*> m_generators;
    QMap<QString, QString> m_args;
    QString m_typeSystemFileName;
    QString m_globalHeaderFileName;
    const char* m_programName;
    void (*m_versionHandler)(const char*);

    bool parseGeneralArgs();
    void printUsage();

    // disable copy
    ApiExtractor(const ApiExtractor&);
    ApiExtractor& operator=(const ApiExtractor&);
};

#endif // APIEXTRACTOR_H
