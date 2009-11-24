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

#include <boost/graph/topological_sort.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

/**
 * Topologically sort the overloads by implicit convertion order
 *
 * This avoids using an implicit conversion if there's an explicit
 * overload for the convertible type. So, if there's an implicit convert
 * like TargetType(ConvertibleType foo) and both are in the overload list,
 * ConvertibleType is checked before TargetType.
 *
 * Side effects: Modifies m_nextOverloadData
 */
void OverloadData::sortOverloads()
{
    using namespace boost;

    OverloadDataList sorted;
    QList<int> unmappedResult;
    QSet<QPair<int, int> > deps;
    QHash<QString, int> map;
    QHash<int, OverloadData *>reverseMap;
    bool checkPyObject = false;
    int pyobjectIndex = 0;

    int i = 0;
    foreach(OverloadData *ov, m_nextOverloadData) {
        map[ov->argType()->typeEntry()->name()] = i;
        reverseMap[i] = ov;

        if (!checkPyObject && ov->argType()->typeEntry()->name().contains("PyObject")) {
            checkPyObject = true;
            pyobjectIndex = i;
        }

        i++;
    }

    foreach(OverloadData *ov, m_nextOverloadData) {
        AbstractMetaFunctionList conversions = m_generator->implicitConversions(ov->argType());
        const AbstractMetaType *targetType = ov->argType();
        foreach(AbstractMetaFunction *function, conversions) {
            AbstractMetaType *convertibleType = function->arguments().first()->type();

            if (!map.contains(convertibleType->typeEntry()->name()))
                continue;

            int target = map[targetType->typeEntry()->name()];
            int convertible = map[convertibleType->typeEntry()->name()];

            // If a reverse pair already exists, remove it. Probably due to the
            // container check (This happened to QVariant and QHash)
            QPair<int, int> reversePair = qMakePair(convertible, target);
            if (deps.contains(reversePair))
                deps.remove(reversePair);

            deps << qMakePair(target, convertible);
        }

        if (targetType->hasInstantiations()) {
            foreach(AbstractMetaType *instantiation, targetType->instantiations()) {
                if (map.contains(instantiation->typeEntry()->name())) {
                    int target = map[targetType->typeEntry()->name()];
                    int convertible = map[instantiation->typeEntry()->name()];

                    if (!deps.contains(qMakePair(convertible, target))) // Avoid cyclic dependency.
                        deps << qMakePair(target, convertible);
                }
            }
        }

        /* Add dependency on PyObject, so its check is the last one (too generic) */
        if (checkPyObject && !targetType->typeEntry()->name().contains("PyObject")) {
            deps << qMakePair(pyobjectIndex,
                              map[targetType->typeEntry()->name()]);
        }
    }

    // Special case for double(int i) (not tracked by m_generator->implicitConversions
    if (map.contains("double") && map.contains("int"))
        deps << qMakePair(map["int"], map["double"]);

    typedef adjacency_list<vecS, vecS, directedS> Graph;
    Graph g(deps.begin(), deps.end(), reverseMap.size());
    topological_sort(g, std::back_inserter(unmappedResult));

    foreach(int i, unmappedResult)
        sorted << reverseMap[i];

    m_nextOverloadData = sorted;
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
    // important cases first, based on the topological order of the implicit conversions
    if (m_nextOverloadData.size() > 1)
        sortOverloads();

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
    if (!func->isOperatorOverload()) {
        foreach (OverloadData* tmp, m_nextOverloadData) {
            // TODO: 'const char *', 'char *' and 'char' will have the same TypeEntry?

            // If an argument have a type replacement, then we should create a new overloaddata
            // for it, unless the next argument also have a identical type replacement.
            QString replacedArg = func->typeReplaced(tmp->m_argPos + 1);
            bool argsReplaced = !replacedArg.isEmpty() || !tmp->m_argTypeReplaced.isEmpty();
            if ((!argsReplaced && tmp->m_argType->typeEntry() == argType->typeEntry())
                || (argsReplaced && replacedArg == tmp->argumentTypeReplaced())) {
                tmp->addOverload(func);
                overloadData = tmp;
            }
        }
    }

    if (!overloadData) {
        overloadData = new OverloadData(m_headOverloadData, func, argType, m_argPos + 1);
        overloadData->m_generator = this->m_generator;
        QString typeReplaced = func->typeReplaced(overloadData->m_argPos + 1);

        if (!typeReplaced.isEmpty())
            overloadData->m_argTypeReplaced = typeReplaced;
        m_nextOverloadData.append(overloadData);
    }

    return overloadData;
}

QStringList OverloadData::returnTypes() const
{
    QSet<QString> retTypes;
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (!func->typeReplaced(0).isEmpty())
            retTypes << func->typeReplaced(0);
        else if (func->type() && !func->argumentRemoved(0))
            retTypes << func->type()->cppSignature();
        else
            retTypes << "void";
    }
    return QStringList(retTypes.toList());
}

bool OverloadData::hasNonVoidReturnType() const
{
    QStringList retTypes = returnTypes();
    return !retTypes.contains("void") || retTypes.size() > 1;
}

const AbstractMetaFunction* OverloadData::referenceFunction() const
{
    return m_overloads.first();
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
        if (overloadData->getFunctionWithDefaultValue())
            return true;
    }
    return false;
}

static OverloadData* _findNextArgWithDefault(OverloadData* overloadData)
{
    if (overloadData->getFunctionWithDefaultValue())
        return overloadData;

    OverloadData* result = 0;
    foreach (OverloadData* odata, overloadData->nextOverloadData()) {
        OverloadData* tmp = _findNextArgWithDefault(odata);
        if (!result || (tmp && result->argPos() > tmp->argPos()))
            result = tmp;
    }
    return result;
}

OverloadData* OverloadData::findNextArgWithDefault()
{
    return _findNextArgWithDefault(this);
}

bool OverloadData::isFinalOccurrence(const AbstractMetaFunction* func) const
{
    foreach (const OverloadData* pd, m_nextOverloadData) {
        if (pd->overloads().contains(func))
            return false;
    }
    return true;
}

const AbstractMetaFunction* OverloadData::getFunctionWithDefaultValue() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        int removedArgs = 0;
        for (int i = 0; i <= m_argPos + removedArgs; i++) {
            if (func->argumentRemoved(i + 1))
                removedArgs++;
        }
        if (!func->arguments()[m_argPos + removedArgs]->defaultValueExpression().isEmpty())
            return func;
    }
    return 0;
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
        QString type = hasArgumentTypeReplace() ? argumentTypeReplaced() : argType()->cppSignature();
        s << "<tr><td bgcolor=\"gray\" align=\"right\">type</td><td bgcolor=\"gray\" align=\"left\">";
        s << type.replace("&", "&amp;") << "</td></tr>";
        if (hasArgumentTypeReplace()) {
            s << "<tr><td bgcolor=\"gray\" align=\"right\">orig. type</td><td bgcolor=\"gray\" align=\"left\">";
            s << argType()->cppSignature().replace("&", "&amp;") << "</td></tr>";
        }

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

bool OverloadData::hasArgumentTypeReplace() const
{
    return !m_argTypeReplaced.isEmpty();
}

QString OverloadData::argumentTypeReplaced() const
{
    return m_argTypeReplaced;
}

