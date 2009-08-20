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

#include "reporthandler.h"
#include <QStringList>

class AbstractMetaBuilder;
class QIODevice;

class ApiExtractor
{
public:
    ApiExtractor();
    ~ApiExtractor();

    void setTypeSystem(const QString& typeSystemFileName);
    void setCppFileName(const QString& cppFileName);
    void setDebugLevel(ReportHandler::DebugLevel debugLevel);
    void setSupressWarnings(bool value);
    void addTypesystemSearchPath(const QString& path);
    void addIncludePath(const QString& path);

    bool run();
private:
    QString m_typeSystemFileName;
    QString m_cppFileName;
    QStringList m_includePaths;
    AbstractMetaBuilder* m_builder;

    // disable copy
    ApiExtractor(const ApiExtractor&);
    ApiExtractor& operator=(const ApiExtractor&);
};

#endif // APIEXTRACTOR_H
