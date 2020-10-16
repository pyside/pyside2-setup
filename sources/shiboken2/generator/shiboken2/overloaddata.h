/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
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

#ifndef OVERLOADDATA_H
#define OVERLOADDATA_H

#include <abstractmetalang_typedefs.h>
#include <QtCore/QBitArray>
#include <QtCore/QVector>

QT_FORWARD_DECLARE_CLASS(QDebug)

class ShibokenGenerator;

class OverloadData;
using OverloadDataList = QVector<OverloadData *>;

class OverloadData
{
public:
    using MetaFunctionList = QVector<const AbstractMetaFunction *>;

    OverloadData(const AbstractMetaFunctionList &overloads, const ShibokenGenerator *generator);
    ~OverloadData();

    int minArgs() const { return m_headOverloadData->m_minArgs; }
    int maxArgs() const { return m_headOverloadData->m_maxArgs; }
    int argPos() const { return m_argPos; }

    const AbstractMetaType &argType() const { return m_argType; }

    /// Returns a string list containing all the possible return types (including void) for the current OverloadData.
    QStringList returnTypes() const;

    /// Returns true if any of the overloads for the current OverloadData has a return type different from void.
    bool hasNonVoidReturnType() const;

    /// Returns true if any of the overloads for the current OverloadData has a varargs argument.
    bool hasVarargs() const;

    /// Returns true if any of the overloads for the current OverloadData allows threads when called.
    bool hasAllowThread() const;

    /// Returns true if any of the overloads for the current OverloadData is static.
    bool hasStaticFunction() const;

    /// Returns true if any of the overloads passed as argument is static.
    static bool hasStaticFunction(const AbstractMetaFunctionList &overloads);

    /// Returns true if any of the overloads for the current OverloadData is not static.
    bool hasInstanceFunction() const;

    /// Returns true if any of the overloads passed as argument is not static.
    static bool hasInstanceFunction(const AbstractMetaFunctionList &overloads);

    /// Returns true if among the overloads for the current OverloadData there are static and non-static methods altogether.
    bool hasStaticAndInstanceFunctions() const;

    /// Returns true if among the overloads passed as argument there are static and non-static methods altogether.
    static bool hasStaticAndInstanceFunctions(const AbstractMetaFunctionList &overloads);

    const AbstractMetaFunction *referenceFunction() const;
    const AbstractMetaArgument *argument(const AbstractMetaFunction *func) const;
    OverloadDataList overloadDataOnPosition(int argPos) const;

    bool isHeadOverloadData() const { return this == m_headOverloadData; }

    /// Returns the root OverloadData object that represents all the overloads.
    OverloadData *headOverloadData() const { return m_headOverloadData; }

    /// Returns the function that has a default value at the current OverloadData argument position, otherwise returns null.
    const AbstractMetaFunction *getFunctionWithDefaultValue() const;

    bool nextArgumentHasDefaultValue() const;
    /// Returns the nearest occurrence, including this instance, of an argument with a default value.
    OverloadData *findNextArgWithDefault();
    bool isFinalOccurrence(const AbstractMetaFunction *func) const;

    /// Returns the list of overloads removing repeated constant functions (ex.: "foo()" and "foo()const", the second is removed).
    MetaFunctionList overloadsWithoutRepetition() const;
    const MetaFunctionList &overloads() const { return m_overloads; }
    OverloadDataList nextOverloadData() const { return m_nextOverloadData; }
    OverloadData *previousOverloadData() const { return m_previousOverloadData; }

    QVector<int> invalidArgumentLengths() const;

    static int numberOfRemovedArguments(const AbstractMetaFunction *func, int finalArgPos = -1);
    static QPair<int, int> getMinMaxArguments(const AbstractMetaFunctionList &overloads);
    /// Returns true if all overloads have no more than one argument.
    static bool isSingleArgument(const AbstractMetaFunctionList &overloads);

    void dumpGraph(const QString &filename) const;
    QString dumpGraph() const;

    bool hasArgumentTypeReplace() const;
    QString argumentTypeReplaced() const;

    bool hasArgumentWithDefaultValue() const;
    static bool hasArgumentWithDefaultValue(const AbstractMetaFunctionList &overloads);
    static bool hasArgumentWithDefaultValue(const AbstractMetaFunction *func);

    /// Returns a list of function arguments which have default values and were not removed.
    static AbstractMetaArgumentList getArgumentsWithDefaultValues(const AbstractMetaFunction *func);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &) const;
#endif

private:
    OverloadData(OverloadData *headOverloadData, const AbstractMetaFunction *func,
                 const AbstractMetaType &argType, int argPos);

    void addOverload(const AbstractMetaFunction *func);
    OverloadData *addOverloadData(const AbstractMetaFunction *func, const AbstractMetaArgument *arg);

    void sortNextOverloads();
    bool sortByOverloadNumberModification();

    int functionNumber(const AbstractMetaFunction *func) const;
    OverloadDataList overloadDataOnPosition(OverloadData *overloadData, int argPos) const;

    int m_minArgs;
    int m_maxArgs;
    int m_argPos;
    AbstractMetaType m_argType;
    QString m_argTypeReplaced;
    MetaFunctionList m_overloads;

    OverloadData *m_headOverloadData;
    OverloadDataList m_nextOverloadData;
    OverloadData *m_previousOverloadData;
    const ShibokenGenerator *m_generator;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const OverloadData *);
#endif

#endif // OVERLOADDATA_H
