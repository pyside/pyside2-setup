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

#ifndef OVERLOADDATA_H
#define OVERLOADDATA_H

#include <apiextractor/abstractmetalang.h>
#include <QtCore/QList>
#include <QtCore/QBitArray>

#include "shibokengenerator.h"

class OverloadData;
typedef QList<OverloadData*> OverloadDataList;

class OverloadData
{
public:
    OverloadData(const AbstractMetaFunctionList overloads, const ShibokenGenerator* generator);

    int minArgs() const { return m_headOverloadData->m_minArgs; }
    int maxArgs() const { return m_headOverloadData->m_maxArgs; }
    int argPos() const { return m_argPos; }

    const AbstractMetaType* argType() const { return m_argType; }
    const AbstractMetaFunction* referenceFunction() const;
    const AbstractMetaArgument* argument(const AbstractMetaFunction* func) const;
    OverloadDataList overloadDataOnPosition(int argPos) const;

    bool isHeadOverloadData() const { return this == m_headOverloadData; }
    bool hasDefaultValue() const;
    bool nextArgumentHasDefaultValue() const;
    /// Returns the nearest occurrence, including this instance, of an argument with a default value.
    OverloadData* findNextArgWithDefault();
    bool isFinalOccurrence(const AbstractMetaFunction* func) const;

    QList<const AbstractMetaFunction*> overloads() const { return m_overloads; }
    OverloadDataList nextOverloadData() const { return m_nextOverloadData; }

    QList<int> invalidArgumentLengths() const;

    static int numberOfRemovedArguments(const AbstractMetaFunction* func, int finalArgPos = -1);
    static QPair<int, int> getMinMaxArguments(const AbstractMetaFunctionList overloads);

    void dumpGraph(QString filename) const;
    QString dumpGraph() const;

    ~OverloadData();

private:
    OverloadData(OverloadData* headOverloadData, const AbstractMetaFunction* func,
                 const AbstractMetaType* argType, int argPos);

    void addOverload(const AbstractMetaFunction* func);
    OverloadData* addOverloadData(const AbstractMetaFunction* func, const AbstractMetaType* argType);

    void sortOverloads();

    int functionNumber(const AbstractMetaFunction* func) const;
    OverloadDataList overloadDataOnPosition(OverloadData* overloadData, int argPos) const;

    int m_minArgs;
    int m_maxArgs;
    int m_argPos;
    const AbstractMetaType* m_argType;
    QList<const AbstractMetaFunction*> m_overloads;

    OverloadData* m_headOverloadData;
    OverloadDataList m_nextOverloadData;
    const ShibokenGenerator* m_generator;
};


#endif // OVERLOADDATA_H
