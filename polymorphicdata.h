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

#ifndef POLYMORPHICDATA_H
#define POLYMORPHICDATA_H

#include <apiextractor/abstractmetalang.h>
#include <QtCore/QList>
#include <QtCore/QBitArray>

class PolymorphicData;
typedef QList<PolymorphicData*> PolymorphicDataList;

class PolymorphicData
{
public:
    PolymorphicData(const AbstractMetaFunctionList overloads);

    int minArgs() const { return m_headPolymorphicData->m_minArgs; }
    int maxArgs() const { return m_headPolymorphicData->m_maxArgs; }
    int argPos() const { return m_argPos; }

    const AbstractMetaType* argType() const { return m_argType; }
    const AbstractMetaFunction* referenceFunction() const;
    const AbstractMetaArgument* argument(const AbstractMetaFunction* func) const;
    PolymorphicDataList polymorphicDataOnPosition(int argPos) const;

    bool isHeadPolymorphicData() const { return this == m_headPolymorphicData; }
    bool hasDefaultValue() const;
    bool nextArgumentHasDefaultValue() const;
    bool isFinalOccurrence(const AbstractMetaFunction* func) const;

    QList<const AbstractMetaFunction*> overloads() const { return m_overloads; }
    PolymorphicDataList nextPolymorphicData() const { return m_nextPolymorphicData; }

    QList<int> invalidArgumentLengths() const;

    static int numberOfRemovedArguments(const AbstractMetaFunction* func, int finalArgPos = -1);
    static QPair<int, int> getMinMaxArguments(const AbstractMetaFunctionList overloads);

    void dumpGraph(QString filename) const;
    QString dumpGraph() const;

    ~PolymorphicData();

private:
    PolymorphicData(PolymorphicData* headPolymorphicData, const AbstractMetaFunction* func,
                    const AbstractMetaType* argType, int argPos);

    void addPolymorphic(const AbstractMetaFunction* func);
    PolymorphicData* addPolymorphicData(const AbstractMetaFunction* func, const AbstractMetaType* argType);

    int functionNumber(const AbstractMetaFunction* func) const;
    PolymorphicDataList polymorphicDataOnPosition(PolymorphicData* polyData, int argPos) const;

    int m_minArgs;
    int m_maxArgs;
    int m_argPos;
    const AbstractMetaType* m_argType;
    QList<const AbstractMetaFunction*> m_overloads;

    PolymorphicData* m_headPolymorphicData;
    PolymorphicDataList m_nextPolymorphicData;
};


#endif // POLYMORPHICDATA_H
