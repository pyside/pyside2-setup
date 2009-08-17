/*
 * This file is part of the Shiboken Python Binding Generator project.
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
#include "polymorphicdata.h"
#include "shibokengenerator.h"

// Prepare the information about polymorphic methods signatures
PolymorphicData::PolymorphicData(const AbstractMetaFunctionList overloads)
    : m_minArgs(256), m_maxArgs(0), m_argType(0),
      m_argPos(-1), m_headPolymorphicData(this)
{
    foreach (const AbstractMetaFunction* func, overloads) {
        m_overloads.append(func);
        int argSize = func->arguments().size();
        if (m_minArgs > argSize)
            m_minArgs = argSize;
        else if (m_maxArgs < argSize)
            m_maxArgs = argSize;
        PolymorphicData* currentPolymorphicData = this;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (func->argumentRemoved(arg->argumentIndex() + 1))
                continue;
            currentPolymorphicData = currentPolymorphicData->addPolymorphicData(func, arg->type());
        }
    }

    // Fix minArgs
    if (minArgs() > maxArgs())
        m_headPolymorphicData->m_minArgs = maxArgs();
}

PolymorphicData::PolymorphicData(PolymorphicData* headPolymorphicData, const AbstractMetaFunction* func,
                                 const AbstractMetaType* argType, int argPos)
    : m_minArgs(256), m_maxArgs(0), m_argType(argType), m_argPos(argPos),
      m_headPolymorphicData(headPolymorphicData)
{
    if (func)
        this->addPolymorphic(func);
}

void PolymorphicData::addPolymorphic(const AbstractMetaFunction* func)
{
    int origNumArgs = func->arguments().size();
    int removed = numberOfRemovedArguments(func);
    int numArgs = origNumArgs - removed;

    if (numArgs > m_headPolymorphicData->m_maxArgs)
        m_headPolymorphicData->m_maxArgs = numArgs;

    if (numArgs < m_headPolymorphicData->m_minArgs)
        m_headPolymorphicData->m_minArgs = numArgs;

    for (int i = 0; m_headPolymorphicData->m_minArgs > 0 && i < origNumArgs; i++) {
        if (func->argumentRemoved(i + 1))
            continue;
        if (!func->arguments()[i]->defaultValueExpression().isEmpty()) {
            int fixedArgIndex = i - removed;
            if (fixedArgIndex < m_headPolymorphicData->m_minArgs)
                m_headPolymorphicData->m_minArgs = fixedArgIndex;
        }
    }

    m_overloads.append(func);
}

PolymorphicData* PolymorphicData::addPolymorphicData(const AbstractMetaFunction* func,
                                                     const AbstractMetaType* argType)
{
    PolymorphicData* polymorphicData = 0;
    foreach (PolymorphicData* tmp, m_nextPolymorphicData) {
        // TODO: 'const char *', 'char *' and 'char' will have the same TypeEntry?
        if (tmp->m_argType->typeEntry() == argType->typeEntry()) {
            tmp->addPolymorphic(func);
            polymorphicData = tmp;
            continue;
        }
    }

    if (!polymorphicData) {
        polymorphicData = new PolymorphicData(m_headPolymorphicData, func, argType, m_argPos + 1);
        // The following code always put PyInt as the last element to be checked.
        // This is useful to check the python argument as PyNumber instead of
        // PyInt, but not getting in the way of other tipes of higher precision
        // (e.g. PyFloat)
        if (ShibokenGenerator::isPyInt(argType))
            m_nextPolymorphicData.append(polymorphicData);
        else
            m_nextPolymorphicData.prepend(polymorphicData);
    }

    return polymorphicData;
}

const AbstractMetaFunction* PolymorphicData::referenceFunction() const
{
    return m_overloads.at(0);
}

const AbstractMetaArgument* PolymorphicData::argument(const AbstractMetaFunction* func) const
{
    if (isHeadPolymorphicData() || !m_overloads.contains(func))
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

PolymorphicDataList PolymorphicData::polymorphicDataOnPosition(PolymorphicData* polyData, int argPos) const
{
    PolymorphicDataList polyDataList;
    if (polyData->argPos() == argPos) {
        polyDataList.append(polyData);
    } else if (polyData->argPos() < argPos) {
        foreach (PolymorphicData* pd, polyData->nextPolymorphicData())
            polyDataList += polymorphicDataOnPosition(pd, argPos);
    }
    return polyDataList;
}

PolymorphicDataList PolymorphicData::polymorphicDataOnPosition(int argPos) const
{
    PolymorphicDataList polyDataList;
    polyDataList += polymorphicDataOnPosition(m_headPolymorphicData, argPos);
    return polyDataList;
}

bool PolymorphicData::nextArgumentHasDefaultValue() const
{
    foreach (PolymorphicData* polymorphicData, m_nextPolymorphicData) {
        if (polymorphicData->hasDefaultValue())
            return true;
    }
    return false;
}

bool PolymorphicData::isFinalOccurrence(const AbstractMetaFunction* func) const
{
    foreach (const PolymorphicData* pd, m_nextPolymorphicData) {
        if (pd->overloads().contains(func))
            return false;
    }
    return true;
}

bool PolymorphicData::hasDefaultValue() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (!func->arguments()[m_argPos]->defaultValueExpression().isEmpty())
            return true;
    }
    return false;
}

QList<int> PolymorphicData::invalidArgumentLengths() const
{
    QSet<int> validArgLengths;
    foreach (const AbstractMetaFunction* func, m_headPolymorphicData->m_overloads) {
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

int PolymorphicData::numberOfRemovedArguments(const AbstractMetaFunction* func, int finalArgPos)
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

QPair<int, int> PolymorphicData::getMinMaxArguments(const AbstractMetaFunctionList overloads)
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

void PolymorphicData::dumpGraph(QString filename) const
{
    QFile file(filename);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);
        s << dumpGraph(m_headPolymorphicData);
    }
}

QString PolymorphicData::dumpGraph(const PolymorphicData* polyData) const
{
    if (!polyData)
        return QString();

    QString indent(4, ' ');
    QString result;
    QTextStream s(&result);
    if (polyData->m_argPos == -1) {
        const AbstractMetaFunction* rfunc = polyData->referenceFunction();
        s << "digraph PolymorphicFunction {" << endl;
        s << indent << "graph [fontsize=12 fontname=freemono labelloc=t splines=true overlap=false rankdir=LR];" << endl;

        // Shows all function signatures
        s << "legend [fontsize=9 fontname=freemono shape=rect label=\"";
        foreach (const AbstractMetaFunction* func, polyData->overloads()) {
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
        foreach (const AbstractMetaFunction* func, polyData->overloads()) {
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
        foreach (const AbstractMetaFunction* func, polyData->overloads())
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        s << "</table>> ];" << endl;

        foreach (const PolymorphicData* pd, polyData->nextPolymorphicData())
            s << indent << '"' << rfunc->name() << "\" -> " << dumpGraph(pd);

        s << "}" << endl;
    } else {
        QString argId = QString("arg_%1").arg((long)polyData);
        s << argId << ';' << endl;

        s << indent << '"' << argId << "\" [shape=\"plaintext\" style=\"filled,bold\" margin=\"0\" fontname=\"freemono\" fillcolor=\"white\" penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";

        // Argument box title
        s << "<tr><td bgcolor=\"black\" align=\"left\" cellpadding=\"2\" colspan=\"2\">";
        s << "<font color=\"white\" point-size=\"11\">arg #" << polyData->argPos() << "</font></td></tr>";

        // Argument type information
        s << "<tr><td bgcolor=\"gray\" align=\"right\">type</td><td bgcolor=\"gray\" align=\"left\">";
        s << polyData->argType()->cppSignature().replace("&", "&amp;") << "</td></tr>";

        // Overloads for the signature to present point
        s << "<tr><td bgcolor=\"gray\" align=\"right\">overloads</td><td bgcolor=\"gray\" align=\"left\">";
        foreach (const AbstractMetaFunction* func, polyData->overloads())
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        // Show default values (original and modified) for various functions
        foreach (const AbstractMetaFunction* func, polyData->overloads()) {
            const AbstractMetaArgument* arg = polyData->argument(func);
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

        foreach (const PolymorphicData* pd, polyData->nextPolymorphicData())
            s << indent << argId << " -> " << dumpGraph(pd);
    }
    return result;
}

int PolymorphicData::functionNumber(const AbstractMetaFunction* func) const
{
    m_headPolymorphicData->m_overloads.indexOf(func);
}

PolymorphicData::~PolymorphicData()
{
    while (!m_nextPolymorphicData.isEmpty())
        delete m_nextPolymorphicData.takeLast();
}
