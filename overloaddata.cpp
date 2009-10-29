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

#include <QtCore/QFile>
#include "overloaddata.h"
#include "shibokengenerator.h"

static bool overloadDataLessThan(const OverloadData* o1, const OverloadData* o2)
{
    return o1->argTypeWeight() < o2->argTypeWeight();
}

// Prepare the information about overloaded methods signatures
OverloadData::OverloadData(const AbstractMetaFunctionList overloads, const ShibokenGenerator* generator)
    : m_minArgs(256), m_maxArgs(0), m_argPos(-1), m_argType(0),
      m_headOverloadData(this), m_generator(generator)
{
    foreach (const AbstractMetaFunction* func, overloads) {
        m_overloads.append(func);
        int argSize = func->arguments().size();
        if (m_minArgs > argSize)
            m_minArgs = argSize;
        else if (m_maxArgs < argSize)
            m_maxArgs = argSize;
        OverloadData* currentOverloadData = this;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (func->argumentRemoved(arg->argumentIndex() + 1))
                continue;
            currentOverloadData = currentOverloadData->addOverloadData(func, arg->type());
        }
    }

    // Sort the overload possibilities so that the overload decisor code goes for the most
    // important cases first, based on the weight system defined in OverloadData::addOverloadData
    if (m_nextOverloadData.size() > 1)
        qSort(m_nextOverloadData.begin(), m_nextOverloadData.end(), overloadDataLessThan);

    // Fix minArgs
    if (minArgs() > maxArgs())
        m_headOverloadData->m_minArgs = maxArgs();
}

OverloadData::OverloadData(OverloadData* headOverloadData, const AbstractMetaFunction* func,
                                 const AbstractMetaType* argType, int argPos)
    : m_minArgs(256), m_maxArgs(0), m_argPos(argPos), m_argType(argType),
      m_headOverloadData(headOverloadData)
{
    if (func)
        this->addOverload(func);
}

void OverloadData::addOverload(const AbstractMetaFunction* func)
{
    int origNumArgs = func->arguments().size();
    int removed = numberOfRemovedArguments(func);
    int numArgs = origNumArgs - removed;

    if (numArgs > m_headOverloadData->m_maxArgs)
        m_headOverloadData->m_maxArgs = numArgs;

    if (numArgs < m_headOverloadData->m_minArgs)
        m_headOverloadData->m_minArgs = numArgs;

    for (int i = 0; m_headOverloadData->m_minArgs > 0 && i < origNumArgs; i++) {
        if (func->argumentRemoved(i + 1))
            continue;
        if (!func->arguments()[i]->defaultValueExpression().isEmpty()) {
            int fixedArgIndex = i - removed;
            if (fixedArgIndex < m_headOverloadData->m_minArgs)
                m_headOverloadData->m_minArgs = fixedArgIndex;
        }
    }

    m_overloads.append(func);
}

OverloadData* OverloadData::addOverloadData(const AbstractMetaFunction* func,
                                            const AbstractMetaType* argType)
{
    OverloadData* overloadData = 0;
    foreach (OverloadData* tmp, m_nextOverloadData) {
        // TODO: 'const char *', 'char *' and 'char' will have the same TypeEntry?
        if (tmp->m_argType->typeEntry() == argType->typeEntry()) {
            tmp->addOverload(func);
            overloadData = tmp;
            continue;
        }
    }

    if (!overloadData) {
        overloadData = new OverloadData(m_headOverloadData, func, argType, m_argPos + 1);
        overloadData->m_generator = this->m_generator;

        // The following code sets weights to the types of the possible arguments
        // following the current one.
        // The rule is: native strings goes first, followed by the primitive types
        // (among those the most precise have more priority), and finally the wrapped C++
        // types are ordered based on how many implicit conversions they have (the ones who
        // have more go to the end).
        if (ShibokenGenerator::isPyInt(argType)) {
            overloadData->m_argTypeWeight = -1;
        } else if (argType->isPrimitive()) {
            if (argType->typeEntry()->name() == "double" || argType->typeEntry()->name() == "float")
                overloadData->m_argTypeWeight = -3;
            else
                overloadData->m_argTypeWeight = -2;
        } else if (argType->name() == "char" && argType->isNativePointer()) {
            overloadData->m_argTypeWeight = -4;
        } else if (argType->typeEntry()->isValue() || argType->typeEntry()->isObject()) {
            overloadData->m_argTypeWeight = m_generator->implicitConversions(argType).size();
        } else {
            overloadData->m_argTypeWeight = 0;
        }

        m_nextOverloadData.append(overloadData);
    }

    return overloadData;
}

const AbstractMetaFunction* OverloadData::referenceFunction() const
{
    return m_overloads.at(0);
}

const AbstractMetaArgument* OverloadData::argument(const AbstractMetaFunction* func) const
{
    if (isHeadOverloadData() || !m_overloads.contains(func))
        return 0;

    int argPos = 0;
    int removed = 0;
    for (int i = 0; argPos <= m_argPos; i++) {
        if (func->argumentRemoved(i + 1))
            removed++;
        else
            argPos++;
    }

    return func->arguments()[m_argPos + removed];
}

OverloadDataList OverloadData::overloadDataOnPosition(OverloadData* overloadData, int argPos) const
{
    OverloadDataList overloadDataList;
    if (overloadData->argPos() == argPos) {
        overloadDataList.append(overloadData);
    } else if (overloadData->argPos() < argPos) {
        foreach (OverloadData* pd, overloadData->nextOverloadData())
            overloadDataList += overloadDataOnPosition(pd, argPos);
    }
    return overloadDataList;
}

OverloadDataList OverloadData::overloadDataOnPosition(int argPos) const
{
    OverloadDataList overloadDataList;
    overloadDataList += overloadDataOnPosition(m_headOverloadData, argPos);
    return overloadDataList;
}

bool OverloadData::nextArgumentHasDefaultValue() const
{
    foreach (OverloadData* overloadData, m_nextOverloadData) {
        if (overloadData->hasDefaultValue())
            return true;
    }
    return false;
}

bool OverloadData::isFinalOccurrence(const AbstractMetaFunction* func) const
{
    foreach (const OverloadData* pd, m_nextOverloadData) {
        if (pd->overloads().contains(func))
            return false;
    }
    return true;
}

bool OverloadData::hasDefaultValue() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (!func->arguments()[m_argPos]->defaultValueExpression().isEmpty())
            return true;
    }
    return false;
}

QList<int> OverloadData::invalidArgumentLengths() const
{
    QSet<int> validArgLengths;
    foreach (const AbstractMetaFunction* func, m_headOverloadData->m_overloads) {
        validArgLengths << func->arguments().size();
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (!arg->defaultValueExpression().isEmpty())
                validArgLengths << arg->argumentIndex();
        }
    }

    QList<int> invalidArgLengths;
    for (int i = minArgs() + 1; i < maxArgs(); i++) {
        if (!validArgLengths.contains(i))
            invalidArgLengths.append(i);
    }

    return invalidArgLengths;
}

int OverloadData::numberOfRemovedArguments(const AbstractMetaFunction* func, int finalArgPos)
{
    int removed = 0;
    if (finalArgPos < 0)
        finalArgPos = func->arguments().size();
    for (int i = 0; i < finalArgPos; i++) {
        if (func->argumentRemoved(i + 1))
            removed++;
    }
    return removed;
}

QPair<int, int> OverloadData::getMinMaxArguments(const AbstractMetaFunctionList overloads)
{
    int minArgs = 10000;
    int maxArgs = 0;
    for (int i = 0; i < overloads.size(); i++) {
        const AbstractMetaFunction* func = overloads[i];
        int origNumArgs = func->arguments().size();
        int removed = numberOfRemovedArguments(func);
        int numArgs = origNumArgs - removed;
        if (maxArgs < numArgs)
            maxArgs = numArgs;
        if (minArgs > numArgs)
            minArgs = numArgs;
        for (int j = 0; j < origNumArgs; j++) {
            if (func->argumentRemoved(j + 1))
                continue;
            int fixedArgIndex = j - removed;
            if (fixedArgIndex < minArgs && !func->arguments()[j]->defaultValueExpression().isEmpty())
                minArgs = fixedArgIndex;
        }
    }
    return QPair<int, int>(minArgs, maxArgs);
}

void OverloadData::dumpGraph(QString filename) const
{
    QFile file(filename);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);
        s << m_headOverloadData->dumpGraph();
    }
}

QString OverloadData::dumpGraph() const
{
    QString indent(4, ' ');
    QString result;
    QTextStream s(&result);
    if (m_argPos == -1) {
        const AbstractMetaFunction* rfunc = referenceFunction();
        s << "digraph OverloadedFunction {" << endl;
        s << indent << "graph [fontsize=12 fontname=freemono labelloc=t splines=true overlap=false rankdir=LR];" << endl;

        // Shows all function signatures
        s << "legend [fontsize=9 fontname=freemono shape=rect label=\"";
        foreach (const AbstractMetaFunction* func, overloads()) {
            s << "f" << functionNumber(func) << " : ";
            if (func->type())
                s << func->type()->cppSignature().replace('<', "&lt;").replace('>', "&gt;");
            else
                s << "void";
            s << ' ' << func->minimalSignature().replace('<', "&lt;").replace('>', "&gt;") << "\\l";
        }
        s << "\"];" << endl;

        // Function box title
        s << indent << '"' << rfunc->name() << "\" [shape=plaintext style=\"filled,bold\" margin=0 fontname=freemono fillcolor=white penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";
        s << "<tr><td bgcolor=\"black\" align=\"center\" cellpadding=\"6\" colspan=\"2\"><font color=\"white\">";
        if (rfunc->ownerClass())
            s << rfunc->ownerClass()->name() << "::";
        s << rfunc->name().replace('<', "&lt;").replace('>', "&gt;") << "</font>";
        if (rfunc->isVirtual()) {
            s << "<br/><font color=\"white\" point-size=\"10\">&lt;&lt;";
            if (rfunc->isAbstract())
                s << "pure ";
            s << "virtual&gt;&gt;</font>";
        }
        s << "</td></tr>";

        // Function return type
        s << "<tr><td bgcolor=\"gray\" align=\"right\">original type</td><td bgcolor=\"gray\" align=\"left\">";
        if (rfunc->type())
            s << rfunc->type()->cppSignature().replace('<', "&lt;").replace('>', "&gt;");
        else
            s << "void";
        s << "</td></tr>";

        // Shows type changes for all function signatures
        foreach (const AbstractMetaFunction* func, overloads()) {
            if (func->typeReplaced(0).isEmpty())
                continue;
            s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
            s << "-type</td><td bgcolor=\"gray\" align=\"left\">";
            s << func->typeReplaced(0).replace('<', "&lt;").replace('>', "&gt;") << "</td></tr>";
        }

        // Minimum and maximum number of arguments
        s << "<tr><td bgcolor=\"gray\" align=\"right\">minArgs</td><td bgcolor=\"gray\" align=\"left\">";
        s << minArgs() << "</td></tr>";
        s << "<tr><td bgcolor=\"gray\" align=\"right\">maxArgs</td><td bgcolor=\"gray\" align=\"left\">";
        s << maxArgs() << "</td></tr>";

        if (rfunc->ownerClass()) {
            if (rfunc->implementingClass() != rfunc->ownerClass())
                s << "<tr><td align=\"right\">implementor</td><td align=\"left\">" << rfunc->implementingClass()->name() << "</td></tr>";
            if (rfunc->declaringClass() != rfunc->ownerClass() && rfunc->declaringClass() != rfunc->implementingClass())
                s << "<tr><td align=\"right\">declarator</td><td align=\"left\">" << rfunc->declaringClass()->name() << "</td></tr>";
        }

        // Overloads for the signature to present point
        s << "<tr><td bgcolor=\"gray\" align=\"right\">overloads</td><td bgcolor=\"gray\" align=\"left\">";
        foreach (const AbstractMetaFunction* func, overloads())
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        s << "</table>> ];" << endl;

        foreach (const OverloadData* pd, nextOverloadData())
            s << indent << '"' << rfunc->name() << "\" -> " << pd->dumpGraph();

        s << "}" << endl;
    } else {
        QString argId = QString("arg_%1").arg((ulong)this);
        s << argId << ';' << endl;

        s << indent << '"' << argId << "\" [shape=\"plaintext\" style=\"filled,bold\" margin=\"0\" fontname=\"freemono\" fillcolor=\"white\" penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";

        // Argument box title
        s << "<tr><td bgcolor=\"black\" align=\"left\" cellpadding=\"2\" colspan=\"2\">";
        s << "<font color=\"white\" point-size=\"11\">arg #" << argPos() << "</font></td></tr>";

        // Argument type information
        s << "<tr><td bgcolor=\"gray\" align=\"right\">type</td><td bgcolor=\"gray\" align=\"left\">";
        s << argType()->cppSignature().replace("&", "&amp;") << "</td></tr>";

        // Overloads for the signature to present point
        s << "<tr><td bgcolor=\"gray\" align=\"right\">overloads</td><td bgcolor=\"gray\" align=\"left\">";
        foreach (const AbstractMetaFunction* func, overloads())
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        // Show default values (original and modified) for various functions
        foreach (const AbstractMetaFunction* func, overloads()) {
            const AbstractMetaArgument* arg = argument(func);
            if (!arg)
                continue;
            if (!arg->defaultValueExpression().isEmpty() ||
                arg->defaultValueExpression() != arg->originalDefaultValueExpression()) {
                s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
                s << "-default</td><td bgcolor=\"gray\" align=\"left\">";
                s << arg->defaultValueExpression() << "</td></tr>";
            }
            if (arg->defaultValueExpression() != arg->originalDefaultValueExpression()) {
                s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
                s << "-orig-default</td><td bgcolor=\"gray\" align=\"left\">";
                s << arg->originalDefaultValueExpression() << "</td></tr>";
            }
        }

        s << "</table>>];" << endl;

        foreach (const OverloadData* pd, nextOverloadData())
            s << indent << argId << " -> " << pd->dumpGraph();
    }
    return result;
}

int OverloadData::functionNumber(const AbstractMetaFunction* func) const
{
    return m_headOverloadData->m_overloads.indexOf(func);
}

OverloadData::~OverloadData()
{
    while (!m_nextOverloadData.isEmpty())
        delete m_nextOverloadData.takeLast();
}
