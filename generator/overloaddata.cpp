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
#include <reporthandler.h>
#include <graph.h>
#include "overloaddata.h"
#include "shibokengenerator.h"

static const TypeEntry* getAliasedTypeEntry(const TypeEntry* typeEntry)
{
    if (typeEntry->isPrimitive()) {
        const PrimitiveTypeEntry* pte = reinterpret_cast<const PrimitiveTypeEntry*>(typeEntry);
        while (pte->aliasedTypeEntry())
            pte = pte->aliasedTypeEntry();
        typeEntry = pte;
    }
    return typeEntry;
}

static QString getTypeName(const AbstractMetaType* type)
{
    const TypeEntry* typeEntry = getAliasedTypeEntry(type->typeEntry());
    QString typeName = typeEntry->name();
    if (typeEntry->isContainer()) {
        QStringList types;
        foreach (const AbstractMetaType* cType, type->instantiations()) {
            const TypeEntry* typeEntry = getAliasedTypeEntry(cType->typeEntry());
            types << typeEntry->name();
        }
        typeName += QString("<%1 >").arg(types.join(","));
    }
    return typeName;
}

static bool typesAreEqual(const AbstractMetaType* typeA, const AbstractMetaType* typeB)
{

    bool equal = typeA->typeEntry() == typeB->typeEntry();
    if (equal && typeA->isContainer()) {
        if (typeA->instantiations().size() != typeB->instantiations().size())
            return false;
        for (int i = 0; i < typeA->instantiations().size(); ++i) {
            if (!typesAreEqual(typeA->instantiations().at(i), typeB->instantiations().at(i)))
                return false;
        }
    }
    return equal;
}

/**
 * OverloadSortData just helps writing clearer code in the
 * OverloadData::sortNextOverloads method.
 */
struct OverloadSortData
{
    OverloadSortData() : counter(0) {};

    /**
     * Adds a typeName into the type map without associating it with
     * a OverloadData. This is done to express type dependencies that could
     * or could not appear in overloaded signatures not processed yet.
     */
    void mapType(const QString& typeName)
    {
        if (map.contains(typeName))
            return;
        map[typeName] = counter;
        if (!reverseMap.contains(counter))
            reverseMap[counter] = 0;
        counter++;
    }

    void mapType(OverloadData* overloadData)
    {
        QString typeName = getTypeName(overloadData->argType());
        map[typeName] = counter;
        reverseMap[counter] = overloadData;
        counter++;
    }

    int lastProcessedItemId() { return counter - 1; }

    int counter;
    QHash<QString, int> map;                // typeName -> id
    QHash<int, OverloadData*> reverseMap;   // id -> OverloadData;
};

/**
 * Helper function that returns the name of a container get from containerType argument and
 * an instantiation taken either from an implicit conversion expressed by the function argument,
 * or from the string argument implicitConvTypeName.
 */
static QString getImplicitConversionTypeName(const AbstractMetaType* containerType,
                                             const AbstractMetaType* instantiation,
                                             const AbstractMetaFunction* function,
                                             const QString& implicitConvTypeName = QString())
{
    QString impConv;
    if (!implicitConvTypeName.isEmpty())
        impConv = implicitConvTypeName;
    else if (function->isConversionOperator())
        impConv = function->ownerClass()->typeEntry()->name();
    else
        impConv = getTypeName(function->arguments().first()->type());

    QStringList types;
    foreach (const AbstractMetaType* otherType, containerType->instantiations())
        types << (otherType == instantiation ? impConv : getTypeName(otherType));

    const ContainerTypeEntry* containerTypeEntry = reinterpret_cast<const ContainerTypeEntry*>(containerType->typeEntry());
    return containerTypeEntry->qualifiedCppName() + '<' + types.join(", ") + " >";
}

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
void OverloadData::sortNextOverloads()
{
    OverloadSortData sortData;
    bool checkPyObject = false;
    int pyobjectIndex = 0;

    // Primitive types that are not int, long, short,
    // char and their respective unsigned counterparts.
    QStringList nonIntegerPrimitives;
    nonIntegerPrimitives << "float" << "double" << "bool";

    // Signed integer primitive types.
    QStringList signedIntegerPrimitives;
    signedIntegerPrimitives << "int" << "short" << "long";

    // sort the children overloads
    foreach(OverloadData *ov, m_nextOverloadData)
        ov->sortNextOverloads();

    if (m_nextOverloadData.size() <= 1)
        return;

    // Populates the OverloadSortData object containing map and reverseMap, to map type names to ids,
    // these ids will be used by the topological sort algorithm, because is easier and faster to work
    // with graph sorting using integers.
    foreach(OverloadData* ov, m_nextOverloadData) {
        sortData.mapType(ov);
        if (!checkPyObject && getTypeName(ov->argType()).contains("PyObject")) {
            checkPyObject = true;
            pyobjectIndex = sortData.lastProcessedItemId();
        }

        foreach (const AbstractMetaType* instantiation, ov->argType()->instantiations()) {
            // Add dependencies for type instantiation of container.
            QString typeName = getTypeName(instantiation);
            sortData.mapType(typeName);

            // Build dependency for implicit conversion types instantiations for base container.
            // For example, considering signatures "method(list<PointF>)" and "method(list<Point>)",
            // and being PointF implicitly convertible from Point, an list<T> instantiation with T
            // as Point must come before the PointF instantiation, or else list<Point> will never
            // be called. In the case of primitive types, list<double> must come before list<int>.
            if (instantiation->isPrimitive() && (signedIntegerPrimitives.contains(instantiation->name()))) {
                foreach (const QString& primitive, nonIntegerPrimitives)
                    sortData.mapType(getImplicitConversionTypeName(ov->argType(), instantiation, 0, primitive));
            } else {
                foreach (const AbstractMetaFunction* function, m_generator->implicitConversions(instantiation))
                    sortData.mapType(getImplicitConversionTypeName(ov->argType(), instantiation, function));
            }
        }
    }


    // Create the graph of type dependencies based on implicit conversions.
    Graph graph(sortData.reverseMap.count());
    // All C++ primitive types, add any forgotten type AT THE END OF THIS LIST!
    const char* primitiveTypes[] = {"int",
                                    "unsigned int",
                                    "long",
                                    "unsigned long",
                                    "short",
                                    "unsigned short",
                                    "bool",
                                    "unsigned char",
                                    "char",
                                    "float",
                                    "double"
                                    };
    const int numPrimitives = sizeof(primitiveTypes)/sizeof(const char*);
    bool hasPrimitive[numPrimitives];
    for (int i = 0; i < numPrimitives; ++i)
        hasPrimitive[i] = sortData.map.contains(primitiveTypes[i]);

    foreach(OverloadData* ov, m_nextOverloadData) {
        const AbstractMetaType* targetType = ov->argType();
        const TypeEntry* targetTypeEntry = getAliasedTypeEntry(targetType->typeEntry());
        QString targetTypeEntryName = getTypeName(targetType);

        foreach(AbstractMetaFunction* function, m_generator->implicitConversions(targetType)) {
            QString convertibleType;
            if (function->isConversionOperator())
                convertibleType = function->ownerClass()->typeEntry()->name();
            else
                convertibleType = getTypeName(function->arguments().first()->type());

            if (!sortData.map.contains(convertibleType))
                continue;

            int targetTypeId = sortData.map[targetTypeEntryName];
            int convertibleTypeId = sortData.map[convertibleType];

            // If a reverse pair already exists, remove it. Probably due to the
            // container check (This happened to QVariant and QHash)
            graph.removeEdge(targetTypeId, convertibleTypeId);
            graph.addEdge(convertibleTypeId, targetTypeId);
        }

        foreach (const AbstractMetaType* instantiation, targetType->instantiations()) {
            if (sortData.map.contains(getTypeName(instantiation))) {
                int target = sortData.map[targetTypeEntryName];
                int convertible = sortData.map[getTypeName(instantiation)];

                if (!graph.containsEdge(target, convertible)) // Avoid cyclic dependency.
                    graph.addEdge(convertible, target);

                if (instantiation->isPrimitive() && (signedIntegerPrimitives.contains(instantiation->name()))) {
                    foreach (const QString& primitive, nonIntegerPrimitives) {
                        QString convertibleTypeName = getImplicitConversionTypeName(ov->argType(), instantiation, 0, primitive);
                        if (!graph.containsEdge(target, sortData.map[convertibleTypeName])) // Avoid cyclic dependency.
                            graph.addEdge(sortData.map[convertibleTypeName], target);
                    }

                } else {
                    foreach (const AbstractMetaFunction* function, m_generator->implicitConversions(instantiation)) {
                        QString convertibleTypeName = getImplicitConversionTypeName(ov->argType(), instantiation, function);
                        if (!graph.containsEdge(target, sortData.map[convertibleTypeName])) // Avoid cyclic dependency.
                            graph.addEdge(sortData.map[convertibleTypeName], target);
                    }
                }
            }
        }

        /* Add dependency on PyObject, so its check is the last one (too generic) */
        if (checkPyObject && !targetTypeEntryName.contains("PyObject"))
            graph.addEdge(sortData.map[targetTypeEntryName], pyobjectIndex);

        if (targetTypeEntry->isEnum()) {
            for (int i = 0; i < numPrimitives; ++i) {
                if (hasPrimitive[i])
                    graph.addEdge(sortData.map[targetTypeEntryName], sortData.map[primitiveTypes[i]]);
            }
        }
    }

    // Special case for double(int i) (not tracked by m_generator->implicitConversions
    foreach (const QString& signedIntegerName, signedIntegerPrimitives) {
        if (sortData.map.contains(signedIntegerName)) {
            foreach (const QString& nonIntegerName, nonIntegerPrimitives) {
                if (sortData.map.contains(nonIntegerName))
                    graph.addEdge(sortData.map[nonIntegerName], sortData.map[signedIntegerName]);
            }
        }
    }

    // sort the overloads topologically based on the dependency graph.
    QLinkedList<int> unmappedResult = graph.topologicalSort();
    if (unmappedResult.isEmpty()) {
        QString funcName = referenceFunction()->name();
        if (referenceFunction()->ownerClass())
            funcName.prepend(referenceFunction()->ownerClass()->name() + '.');
        ReportHandler::warning(QString("Cyclic dependency found on overloaddata for '%1' method!").arg(qPrintable(funcName)));
    }

    m_nextOverloadData.clear();
    foreach(int i, unmappedResult) {
        if (!sortData.reverseMap[i])
            continue;
        m_nextOverloadData << sortData.reverseMap[i];
    }
}

/**
 * Root constructor for OverloadData
 *
 * This constructor receives the list of overloads for a given function and iterates generating
 * the graph of OverloadData instances. Each OverloadData instance references an argument/type
 * combination.
 *
 * Example:
 *      addStuff(double, PyObject *)
 *      addStuff(double, int)
 *
 * Given these two overloads, there will be the following graph:
 *
 *   addStuff - double - PyObject*
 *                    \- int
 *
 */
OverloadData::OverloadData(const AbstractMetaFunctionList& overloads, const ShibokenGenerator* generator)
    : m_minArgs(256), m_maxArgs(0), m_argPos(-1), m_argType(0),
    m_headOverloadData(this), m_previousOverloadData(0), m_generator(generator)
{
    foreach (const AbstractMetaFunction* func, overloads) {
        m_overloads.append(func);
        int argSize = func->arguments().size() - numberOfRemovedArguments(func);
        if (m_minArgs > argSize)
            m_minArgs = argSize;
        else if (m_maxArgs < argSize)
            m_maxArgs = argSize;
        OverloadData* currentOverloadData = this;
        foreach (const AbstractMetaArgument* arg, func->arguments()) {
            if (func->argumentRemoved(arg->argumentIndex() + 1))
                continue;
            currentOverloadData = currentOverloadData->addOverloadData(func, arg);
        }
    }

    // Sort the overload possibilities so that the overload decisor code goes for the most
    // important cases first, based on the topological order of the implicit conversions
    sortNextOverloads();

    // Fix minArgs
    if (minArgs() > maxArgs())
        m_headOverloadData->m_minArgs = maxArgs();
}

OverloadData::OverloadData(OverloadData* headOverloadData, const AbstractMetaFunction* func,
                                 const AbstractMetaType* argType, int argPos)
    : m_minArgs(256), m_maxArgs(0), m_argPos(argPos), m_argType(argType),
      m_headOverloadData(headOverloadData), m_previousOverloadData(0)
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
                                            const AbstractMetaArgument* arg)
{
    const AbstractMetaType* argType = arg->type();
    OverloadData* overloadData = 0;
    if (!func->isOperatorOverload()) {
        foreach (OverloadData* tmp, m_nextOverloadData) {
            // TODO: 'const char *', 'char *' and 'char' will have the same TypeEntry?

            // If an argument have a type replacement, then we should create a new overloaddata
            // for it, unless the next argument also have a identical type replacement.
            QString replacedArg = func->typeReplaced(tmp->m_argPos + 1);
            bool argsReplaced = !replacedArg.isEmpty() || !tmp->m_argTypeReplaced.isEmpty();
            if ((!argsReplaced && typesAreEqual(tmp->m_argType, argType))
                || (argsReplaced && replacedArg == tmp->argumentTypeReplaced())) {
                tmp->addOverload(func);
                overloadData = tmp;
            }
        }
    }

    if (!overloadData) {
        overloadData = new OverloadData(m_headOverloadData, func, argType, m_argPos + 1);
        overloadData->m_previousOverloadData = this;
        overloadData->m_generator = this->m_generator;
        QString typeReplaced = func->typeReplaced(arg->argumentIndex() + 1);

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

bool OverloadData::hasVarargs() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        AbstractMetaArgumentList args = func->arguments();
        if (args.size() > 1 && args.last()->type()->isVarargs())
            return true;
    }
    return false;
}

bool OverloadData::hasAllowThread() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (func->allowThread())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction(const AbstractMetaFunctionList& overloads)
{
    foreach (const AbstractMetaFunction* func, overloads) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction(const AbstractMetaFunctionList& overloads)
{
    foreach (const AbstractMetaFunction* func, overloads) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction() const
{
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticAndInstanceFunctions(const AbstractMetaFunctionList& overloads)
{
    return OverloadData::hasStaticFunction(overloads) && OverloadData::hasInstanceFunction(overloads);
}

bool OverloadData::hasStaticAndInstanceFunctions() const
{
    return OverloadData::hasStaticFunction() && OverloadData::hasInstanceFunction();
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

QList<const AbstractMetaFunction*> OverloadData::overloadsWithoutRepetition() const
{
    QList<const AbstractMetaFunction*> overloads = m_overloads;
    foreach (const AbstractMetaFunction* func, m_overloads) {
        if (func->minimalSignature().endsWith("const"))
            continue;
        foreach (const AbstractMetaFunction* f, overloads) {
            if ((func->minimalSignature() + "const") == f->minimalSignature()) {
                overloads.removeOne(f);
                break;
            }
        }
    }
    return overloads;
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
        const AbstractMetaArgumentList args = func->arguments();
        int offset = 0;
        for (int i = 0; i < args.size(); ++i) {
            if (func->argumentRemoved(i+1)) {
                offset++;
            } else {
                if (!args[i]->defaultValueExpression().isEmpty())
                    validArgLengths << i-offset;
            }
        }
        validArgLengths << args.size() - offset;
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
    if (finalArgPos < 0) {
        for (int i = 0; i < func->arguments().size(); i++) {
            if (func->argumentRemoved(i + 1))
                removed++;
        }
    } else {
        for (int i = 0; i < finalArgPos + removed; i++) {
            if (func->argumentRemoved(i + 1))
                removed++;
        }
    }
    return removed;
}

QPair<int, int> OverloadData::getMinMaxArguments(const AbstractMetaFunctionList& overloads)
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

bool OverloadData::isSingleArgument(const AbstractMetaFunctionList& overloads)
{
    bool singleArgument = true;
    foreach (const AbstractMetaFunction* func, overloads) {
        if (func->arguments().size() - numberOfRemovedArguments(func) != 1) {
            singleArgument = false;
            break;
        }
    }
    return singleArgument;
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

bool OverloadData::hasArgumentWithDefaultValue(const AbstractMetaFunctionList& overloads)
{
    if (OverloadData::getMinMaxArguments(overloads).second == 0)
        return false;
    foreach (const AbstractMetaFunction* func, overloads) {
        if (hasArgumentWithDefaultValue(func))
            return true;
    }
    return false;
}

bool OverloadData::hasArgumentWithDefaultValue() const
{
    if (maxArgs() == 0)
        return false;
    foreach (const AbstractMetaFunction* func, overloads()) {
        if (hasArgumentWithDefaultValue(func))
            return true;
    }
    return false;
}

bool OverloadData::hasArgumentWithDefaultValue(const AbstractMetaFunction* func)
{
    foreach (const AbstractMetaArgument* arg, func->arguments()) {
        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;
        if (!arg->defaultValueExpression().isEmpty())
            return true;
    }
    return false;
}

AbstractMetaArgumentList OverloadData::getArgumentsWithDefaultValues(const AbstractMetaFunction* func)
{
    AbstractMetaArgumentList args;
    foreach (AbstractMetaArgument* arg, func->arguments()) {
        if (arg->defaultValueExpression().isEmpty()
            || func->argumentRemoved(arg->argumentIndex() + 1))
            continue;
        args << arg;
    }
    return args;
}

