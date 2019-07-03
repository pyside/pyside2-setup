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

#include <abstractmetalang.h>
#include <reporthandler.h>
#include <graph.h>
#include "overloaddata.h"
#include "indentor.h"
#include "shibokengenerator.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>

static const TypeEntry *getReferencedTypeEntry(const TypeEntry *typeEntry)
{
    if (typeEntry->isPrimitive()) {
        auto pte = dynamic_cast<const PrimitiveTypeEntry *>(typeEntry);
        while (pte->referencedTypeEntry())
            pte = pte->referencedTypeEntry();
        typeEntry = pte;
    }
    return typeEntry;
}

static QString getTypeName(const AbstractMetaType *type)
{
    const TypeEntry *typeEntry = getReferencedTypeEntry(type->typeEntry());
    QString typeName = typeEntry->name();
    if (typeEntry->isContainer()) {
        QStringList types;
        const AbstractMetaTypeList &instantiations = type->instantiations();
        for (const AbstractMetaType *cType : instantiations) {
            const TypeEntry *typeEntry = getReferencedTypeEntry(cType->typeEntry());
            types << typeEntry->name();
        }
        typeName += QLatin1Char('<') + types.join(QLatin1Char(',')) + QLatin1String(" >");
    }
    return typeName;
}

static QString getTypeName(const OverloadData *ov)
{
    return ov->hasArgumentTypeReplace() ? ov->argumentTypeReplaced() : getTypeName(ov->argType());
}

static bool typesAreEqual(const AbstractMetaType *typeA, const AbstractMetaType *typeB)
{
    if (typeA->typeEntry() == typeB->typeEntry()) {
        if (typeA->isContainer() || typeA->isSmartPointer()) {
            if (typeA->instantiations().size() != typeB->instantiations().size())
                return false;

            for (int i = 0; i < typeA->instantiations().size(); ++i) {
                if (!typesAreEqual(typeA->instantiations().at(i), typeB->instantiations().at(i)))
                    return false;
            }
            return true;
        }

        return !(ShibokenGenerator::isCString(typeA) ^ ShibokenGenerator::isCString(typeB));
    }
    return false;
}


/**
 * OverloadSortData just helps writing clearer code in the
 * OverloadData::sortNextOverloads method.
 */
struct OverloadSortData
{
    /**
     * Adds a typeName into the type map without associating it with
     * a OverloadData. This is done to express type dependencies that could
     * or could not appear in overloaded signatures not processed yet.
     */
    void mapType(const QString &typeName)
    {
        if (map.contains(typeName))
            return;
        map[typeName] = counter;
        if (!reverseMap.contains(counter))
            reverseMap[counter] = 0;
        counter++;
    }

    void mapType(OverloadData *overloadData)
    {
        QString typeName = getTypeName(overloadData);
        map[typeName] = counter;
        reverseMap[counter] = overloadData;
        counter++;
    }

    int lastProcessedItemId() { return counter - 1; }

    int counter = 0;
    QHash<QString, int> map;                // typeName -> id
    QHash<int, OverloadData *> reverseMap;   // id -> OverloadData;
};

/**
 * Helper function that returns the name of a container get from containerType argument and
 * an instantiation taken either from an implicit conversion expressed by the function argument,
 * or from the string argument implicitConvTypeName.
 */
static QString getImplicitConversionTypeName(const AbstractMetaType *containerType,
                                             const AbstractMetaType *instantiation,
                                             const AbstractMetaFunction *function,
                                             const QString &implicitConvTypeName = QString())
{
    QString impConv;
    if (!implicitConvTypeName.isEmpty())
        impConv = implicitConvTypeName;
    else if (function->isConversionOperator())
        impConv = function->ownerClass()->typeEntry()->name();
    else
        impConv = getTypeName(function->arguments().constFirst()->type());

    QStringList types;
    const AbstractMetaTypeList &instantiations = containerType->instantiations();
    for (const AbstractMetaType *otherType : instantiations)
        types << (otherType == instantiation ? impConv : getTypeName(otherType));

    return containerType->typeEntry()->qualifiedCppName() + QLatin1Char('<')
           + types.join(QLatin1String(", ")) + QLatin1String(" >");
}

// overloaddata.cpp
static QString msgCyclicDependency(const QString &funcName, const QString &graphName,
                                   const OverloadData::MetaFunctionList &involvedConversions)
{
    QString result;
    QTextStream str(&result);
    str << "Cyclic dependency found on overloaddata for \"" << funcName
         << "\" method! The graph boy saved the graph at \"" << QDir::toNativeSeparators(graphName)
         << "\".";
    if (const int count = involvedConversions.size()) {
        str << " Implicit conversions (" << count << "): ";
        for (int i = 0; i < count; ++i) {
            if (i)
                str << ", \"";
            str << involvedConversions.at(i)->signature() << '"';
            if (const AbstractMetaClass *c = involvedConversions.at(i)->implementingClass())
                str << '(' << c->name() << ')';
        }
    }
    return result;
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
    bool checkPySequence = false;
    int pySeqIndex = 0;
    bool checkQString = false;
    int qstringIndex = 0;
    bool checkQVariant = false;
    int qvariantIndex = 0;
    bool checkPyBuffer = false;
    int pyBufferIndex = 0;

    // Primitive types that are not int, long, short,
    // char and their respective unsigned counterparts.
    QStringList nonIntegerPrimitives;
    nonIntegerPrimitives << QLatin1String("float") << QLatin1String("double")
        << QLatin1String("bool");

    // Signed integer primitive types.
    QStringList signedIntegerPrimitives;
    signedIntegerPrimitives << QLatin1String("int") << QLatin1String("short")
       << QLatin1String("long");

    // sort the children overloads
    for (OverloadData *ov : qAsConst(m_nextOverloadData))
        ov->sortNextOverloads();

    if (m_nextOverloadData.size() <= 1)
        return;

    // Populates the OverloadSortData object containing map and reverseMap, to map type names to ids,
    // these ids will be used by the topological sort algorithm, because is easier and faster to work
    // with graph sorting using integers.
    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        sortData.mapType(ov);

        const QString typeName(getTypeName(ov));

        if (!checkPyObject && typeName.contains(QLatin1String("PyObject"))) {
            checkPyObject = true;
            pyobjectIndex = sortData.lastProcessedItemId();
        } else if (!checkPySequence && typeName == QLatin1String("PySequence")) {
            checkPySequence = true;
            pySeqIndex = sortData.lastProcessedItemId();
        } else if (!checkPyBuffer && typeName == QLatin1String("PyBuffer")) {
            checkPyBuffer = true;
            pyBufferIndex = sortData.lastProcessedItemId();
        } else if (!checkQVariant && typeName == QLatin1String("QVariant")) {
            checkQVariant = true;
            qvariantIndex = sortData.lastProcessedItemId();
        } else if (!checkQString && typeName == QLatin1String("QString")) {
            checkQString = true;
            qstringIndex = sortData.lastProcessedItemId();
        }

        const AbstractMetaTypeList &instantiations = ov->argType()->instantiations();
        for (const AbstractMetaType *instantiation : instantiations) {
            // Add dependencies for type instantiation of container.
            QString typeName = getTypeName(instantiation);
            sortData.mapType(typeName);

            // Build dependency for implicit conversion types instantiations for base container.
            // For example, considering signatures "method(list<PointF>)" and "method(list<Point>)",
            // and being PointF implicitly convertible from Point, an list<T> instantiation with T
            // as Point must come before the PointF instantiation, or else list<Point> will never
            // be called. In the case of primitive types, list<double> must come before list<int>.
            if (instantiation->isPrimitive() && (signedIntegerPrimitives.contains(instantiation->name()))) {
                for (const QString &primitive : qAsConst(nonIntegerPrimitives))
                    sortData.mapType(getImplicitConversionTypeName(ov->argType(), instantiation, nullptr, primitive));
            } else {
                const AbstractMetaFunctionList &funcs = m_generator->implicitConversions(instantiation);
                for (const AbstractMetaFunction *function : funcs)
                    sortData.mapType(getImplicitConversionTypeName(ov->argType(), instantiation, function));
            }
        }
    }


    // Create the graph of type dependencies based on implicit conversions.
    Graph graph(sortData.reverseMap.count());
    // All C++ primitive types, add any forgotten type AT THE END OF THIS LIST!
    const char *primitiveTypes[] = {"int",
                                    "unsigned int",
                                    "long",
                                    "unsigned long",
                                    "short",
                                    "unsigned short",
                                    "bool",
                                    "unsigned char",
                                    "char",
                                    "float",
                                    "double",
                                    "const char*"
                                    };
    const int numPrimitives = sizeof(primitiveTypes)/sizeof(const char *);
    bool hasPrimitive[numPrimitives];
    for (int i = 0; i < numPrimitives; ++i)
        hasPrimitive[i] = sortData.map.contains(QLatin1String(primitiveTypes[i]));

    if (checkPySequence && checkPyObject)
        graph.addEdge(pySeqIndex, pyobjectIndex);

    QStringList classesWithIntegerImplicitConversion;

    MetaFunctionList involvedConversions;

    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        const AbstractMetaType *targetType = ov->argType();
        const QString targetTypeEntryName(getTypeName(ov));
        int targetTypeId = sortData.map[targetTypeEntryName];

        // Process implicit conversions
        const AbstractMetaFunctionList &functions = m_generator->implicitConversions(targetType);
        for (AbstractMetaFunction *function : functions) {
            QString convertibleType;
            if (function->isConversionOperator())
                convertibleType = function->ownerClass()->typeEntry()->name();
            else
                convertibleType = getTypeName(function->arguments().constFirst()->type());

            if (convertibleType == QLatin1String("int") || convertibleType == QLatin1String("unsigned int"))
                classesWithIntegerImplicitConversion << targetTypeEntryName;

            if (!sortData.map.contains(convertibleType))
                continue;

            int convertibleTypeId = sortData.map[convertibleType];

            // If a reverse pair already exists, remove it. Probably due to the
            // container check (This happened to QVariant and QHash)
            graph.removeEdge(targetTypeId, convertibleTypeId);
            graph.addEdge(convertibleTypeId, targetTypeId);
            involvedConversions.append(function);
        }

        // Process inheritance relationships
        if (targetType->isValue() || targetType->isObject()) {
            const AbstractMetaClass *metaClass = AbstractMetaClass::findClass(m_generator->classes(), targetType->typeEntry());
            const AbstractMetaClassList &ancestors = m_generator->getAllAncestors(metaClass);
            for (const AbstractMetaClass *ancestor : ancestors) {
                QString ancestorTypeName = ancestor->typeEntry()->name();
                if (!sortData.map.contains(ancestorTypeName))
                    continue;
                int ancestorTypeId = sortData.map[ancestorTypeName];
                graph.removeEdge(ancestorTypeId, targetTypeId);
                graph.addEdge(targetTypeId, ancestorTypeId);
            }
        }

        // Process template instantiations
        const AbstractMetaTypeList &instantiations = targetType->instantiations();
        for (const AbstractMetaType *instantiation : instantiations) {
            if (sortData.map.contains(getTypeName(instantiation))) {
                int convertible = sortData.map[getTypeName(instantiation)];

                if (!graph.containsEdge(targetTypeId, convertible)) // Avoid cyclic dependency.
                    graph.addEdge(convertible, targetTypeId);

                if (instantiation->isPrimitive() && (signedIntegerPrimitives.contains(instantiation->name()))) {
                    for (const QString &primitive : qAsConst(nonIntegerPrimitives)) {
                        QString convertibleTypeName = getImplicitConversionTypeName(ov->argType(), instantiation, nullptr, primitive);
                        if (!graph.containsEdge(targetTypeId, sortData.map[convertibleTypeName])) // Avoid cyclic dependency.
                            graph.addEdge(sortData.map[convertibleTypeName], targetTypeId);
                    }

                } else {
                    const AbstractMetaFunctionList &funcs = m_generator->implicitConversions(instantiation);
                    for (const AbstractMetaFunction *function : funcs) {
                        QString convertibleTypeName = getImplicitConversionTypeName(ov->argType(), instantiation, function);
                        if (!graph.containsEdge(targetTypeId, sortData.map[convertibleTypeName])) { // Avoid cyclic dependency.
                            graph.addEdge(sortData.map[convertibleTypeName], targetTypeId);
                            involvedConversions.append(function);
                        }
                    }
                }
            }
        }


        if ((checkPySequence || checkPyObject || checkPyBuffer)
            && !targetTypeEntryName.contains(QLatin1String("PyObject"))
            && !targetTypeEntryName.contains(QLatin1String("PyBuffer"))
            && !targetTypeEntryName.contains(QLatin1String("PySequence"))) {
            if (checkPySequence) {
                // PySequence will be checked after all more specific types, but before PyObject.
                graph.addEdge(targetTypeId, pySeqIndex);
            } else if (checkPyBuffer) {
                // PySequence will be checked after all more specific types, but before PyObject.
                graph.addEdge(targetTypeId, pyBufferIndex);
            } else {
                // Add dependency on PyObject, so its check is the last one (too generic).
                graph.addEdge(targetTypeId, pyobjectIndex);
            }
        } else if (checkQVariant && targetTypeEntryName != QLatin1String("QVariant")) {
            if (!graph.containsEdge(qvariantIndex, targetTypeId)) // Avoid cyclic dependency.
                graph.addEdge(targetTypeId, qvariantIndex);
        } else if (checkQString && ShibokenGenerator::isPointer(ov->argType())
            && targetTypeEntryName != QLatin1String("QString")
            && targetTypeEntryName != QLatin1String("QByteArray")
            && (!checkPyObject || targetTypeId != pyobjectIndex)) {
            if (!graph.containsEdge(qstringIndex, targetTypeId)) // Avoid cyclic dependency.
                graph.addEdge(targetTypeId, qstringIndex);
        }

        if (targetType->isEnum()) {
            // Enum values must precede primitive types.
            for (int i = 0; i < numPrimitives; ++i) {
                if (hasPrimitive[i])
                    graph.addEdge(targetTypeId, sortData.map[QLatin1String(primitiveTypes[i])]);
            }
        }
    }

    // QByteArray args need to be checked after QString args
    if (sortData.map.contains(QLatin1String("QString")) && sortData.map.contains(QLatin1String("QByteArray")))
        graph.addEdge(sortData.map[QLatin1String("QString")], sortData.map[QLatin1String("QByteArray")]);

    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        const AbstractMetaType *targetType = ov->argType();
        if (!targetType->isEnum())
            continue;

        QString targetTypeEntryName = getTypeName(targetType);
        // Enum values must precede types implicitly convertible from "int" or "unsigned int".
        for (const QString &implicitFromInt : qAsConst(classesWithIntegerImplicitConversion))
            graph.addEdge(sortData.map[targetTypeEntryName], sortData.map[implicitFromInt]);
    }


    // Special case for double(int i) (not tracked by m_generator->implicitConversions
    for (const QString &signedIntegerName : qAsConst(signedIntegerPrimitives)) {
        if (sortData.map.contains(signedIntegerName)) {
            for (const QString &nonIntegerName : qAsConst(nonIntegerPrimitives)) {
                if (sortData.map.contains(nonIntegerName))
                    graph.addEdge(sortData.map[nonIntegerName], sortData.map[signedIntegerName]);
            }
        }
    }

    // sort the overloads topologically based on the dependency graph.
    const auto unmappedResult = graph.topologicalSort();
    if (unmappedResult.isEmpty()) {
        QString funcName = referenceFunction()->name();
        if (referenceFunction()->ownerClass())
            funcName.prepend(referenceFunction()->ownerClass()->name() + QLatin1Char('.'));

        // Dump overload graph
        QString graphName = QDir::tempPath() + QLatin1Char('/') + funcName + QLatin1String(".dot");
        QHash<int, QString> nodeNames;
        for (auto it = sortData.map.cbegin(), end = sortData.map.cend(); it != end; ++it)
            nodeNames.insert(it.value(), it.key());
        graph.dumpDot(nodeNames, graphName);
        qCWarning(lcShiboken).noquote() << qPrintable(msgCyclicDependency(funcName, graphName, involvedConversions));
    }

    m_nextOverloadData.clear();
    for (int i : unmappedResult) {
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
 *   addStuff - double - PyObject *
 *                    \- int
 *
 */
OverloadData::OverloadData(const AbstractMetaFunctionList &overloads, const ShibokenGenerator *generator)
    : m_minArgs(256), m_maxArgs(0), m_argPos(-1), m_argType(nullptr),
    m_headOverloadData(this), m_previousOverloadData(nullptr), m_generator(generator)
{
    for (const AbstractMetaFunction *func : overloads) {
        m_overloads.append(func);
        int argSize = func->arguments().size() - numberOfRemovedArguments(func);
        if (m_minArgs > argSize)
            m_minArgs = argSize;
        else if (m_maxArgs < argSize)
            m_maxArgs = argSize;
        OverloadData *currentOverloadData = this;
        const AbstractMetaArgumentList &arguments = func->arguments();
        for (const AbstractMetaArgument *arg : arguments) {
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

OverloadData::OverloadData(OverloadData *headOverloadData, const AbstractMetaFunction *func,
                                 const AbstractMetaType *argType, int argPos)
    : m_minArgs(256), m_maxArgs(0), m_argPos(argPos), m_argType(argType),
      m_headOverloadData(headOverloadData), m_previousOverloadData(nullptr),
      m_generator(nullptr)
{
    if (func)
        this->addOverload(func);
}

void OverloadData::addOverload(const AbstractMetaFunction *func)
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
        if (!ShibokenGenerator::getDefaultValue(func, func->arguments().at(i)).isEmpty()) {
            int fixedArgIndex = i - removed;
            if (fixedArgIndex < m_headOverloadData->m_minArgs)
                m_headOverloadData->m_minArgs = fixedArgIndex;
        }
    }

    m_overloads.append(func);
}

OverloadData *OverloadData::addOverloadData(const AbstractMetaFunction *func,
                                            const AbstractMetaArgument *arg)
{
    const AbstractMetaType *argType = arg->type();
    OverloadData *overloadData = nullptr;
    if (!func->isOperatorOverload()) {
        for (OverloadData *tmp : qAsConst(m_nextOverloadData)) {
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
    for (const AbstractMetaFunction *func : m_overloads) {
        if (!func->typeReplaced(0).isEmpty())
            retTypes << func->typeReplaced(0);
        else if (func->type() && !func->argumentRemoved(0))
            retTypes << func->type()->cppSignature();
        else
            retTypes << QLatin1String("void");
    }
    return retTypes.values();
}

bool OverloadData::hasNonVoidReturnType() const
{
    QStringList retTypes = returnTypes();
    return !retTypes.contains(QLatin1String("void")) || retTypes.size() > 1;
}

bool OverloadData::hasVarargs() const
{
    for (const AbstractMetaFunction *func : m_overloads) {
        AbstractMetaArgumentList args = func->arguments();
        if (args.size() > 1 && args.constLast()->type()->isVarargs())
            return true;
    }
    return false;
}

bool OverloadData::hasAllowThread() const
{
    for (const AbstractMetaFunction *func : m_overloads) {
        if (func->allowThread())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction(const AbstractMetaFunctionList &overloads)
{
    for (const AbstractMetaFunction *func : qAsConst(overloads)) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction() const
{
    for (const AbstractMetaFunction *func : m_overloads) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction(const AbstractMetaFunctionList &overloads)
{
    for (const AbstractMetaFunction *func : qAsConst(overloads)) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction() const
{
    for (const AbstractMetaFunction *func : m_overloads) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticAndInstanceFunctions(const AbstractMetaFunctionList &overloads)
{
    return OverloadData::hasStaticFunction(overloads) && OverloadData::hasInstanceFunction(overloads);
}

bool OverloadData::hasStaticAndInstanceFunctions() const
{
    return OverloadData::hasStaticFunction() && OverloadData::hasInstanceFunction();
}

const AbstractMetaFunction *OverloadData::referenceFunction() const
{
    return m_overloads.constFirst();
}

const AbstractMetaArgument *OverloadData::argument(const AbstractMetaFunction *func) const
{
    if (isHeadOverloadData() || !m_overloads.contains(func))
        return nullptr;

    int argPos = 0;
    int removed = 0;
    for (int i = 0; argPos <= m_argPos; i++) {
        if (func->argumentRemoved(i + 1))
            removed++;
        else
            argPos++;
    }

    return func->arguments().at(m_argPos + removed);
}

OverloadDataList OverloadData::overloadDataOnPosition(OverloadData *overloadData, int argPos) const
{
    OverloadDataList overloadDataList;
    if (overloadData->argPos() == argPos) {
        overloadDataList.append(overloadData);
    } else if (overloadData->argPos() < argPos) {
        const OverloadDataList &data = overloadData->nextOverloadData();
        for (OverloadData *pd : data)
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
    for (OverloadData *overloadData : m_nextOverloadData) {
        if (overloadData->getFunctionWithDefaultValue())
            return true;
    }
    return false;
}

static OverloadData *_findNextArgWithDefault(OverloadData *overloadData)
{
    if (overloadData->getFunctionWithDefaultValue())
        return overloadData;

    OverloadData *result = nullptr;
    const OverloadDataList &data = overloadData->nextOverloadData();
    for (OverloadData *odata : data) {
        OverloadData *tmp = _findNextArgWithDefault(odata);
        if (!result || (tmp && result->argPos() > tmp->argPos()))
            result = tmp;
    }
    return result;
}

OverloadData *OverloadData::findNextArgWithDefault()
{
    return _findNextArgWithDefault(this);
}

bool OverloadData::isFinalOccurrence(const AbstractMetaFunction *func) const
{
    for (const OverloadData *pd : m_nextOverloadData) {
        if (pd->overloads().contains(func))
            return false;
    }
    return true;
}

OverloadData::MetaFunctionList OverloadData::overloadsWithoutRepetition() const
{
    MetaFunctionList overloads = m_overloads;
    for (const AbstractMetaFunction *func : m_overloads) {
        if (func->minimalSignature().endsWith(QLatin1String("const")))
            continue;
        for (const AbstractMetaFunction *f : qAsConst(overloads)) {
            if ((func->minimalSignature() + QLatin1String("const")) == f->minimalSignature()) {
                overloads.removeOne(f);
                break;
            }
        }
    }
    return overloads;
}

const AbstractMetaFunction *OverloadData::getFunctionWithDefaultValue() const
{
    for (const AbstractMetaFunction *func : m_overloads) {
        int removedArgs = 0;
        for (int i = 0; i <= m_argPos + removedArgs; i++) {
            if (func->argumentRemoved(i + 1))
                removedArgs++;
        }
        if (!ShibokenGenerator::getDefaultValue(func, func->arguments().at(m_argPos + removedArgs)).isEmpty())
            return func;
    }
    return nullptr;
}

QVector<int> OverloadData::invalidArgumentLengths() const
{
    QSet<int> validArgLengths;

    for (const AbstractMetaFunction *func : qAsConst(m_headOverloadData->m_overloads)) {
        const AbstractMetaArgumentList args = func->arguments();
        int offset = 0;
        for (int i = 0; i < args.size(); ++i) {
            if (func->argumentRemoved(i+1)) {
                offset++;
            } else {
                if (!ShibokenGenerator::getDefaultValue(func, args[i]).isEmpty())
                    validArgLengths << i-offset;
            }
        }
        validArgLengths << args.size() - offset;
    }

    QVector<int> invalidArgLengths;
    for (int i = minArgs() + 1; i < maxArgs(); i++) {
        if (!validArgLengths.contains(i))
            invalidArgLengths.append(i);
    }

    return invalidArgLengths;
}

int OverloadData::numberOfRemovedArguments(const AbstractMetaFunction *func, int finalArgPos)
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

QPair<int, int> OverloadData::getMinMaxArguments(const AbstractMetaFunctionList &overloads)
{
    int minArgs = 10000;
    int maxArgs = 0;
    for (const AbstractMetaFunction *func : overloads) {
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
            if (fixedArgIndex < minArgs && !ShibokenGenerator::getDefaultValue(func, func->arguments().at(j)).isEmpty())
                minArgs = fixedArgIndex;
        }
    }
    return {minArgs, maxArgs};
}

bool OverloadData::isSingleArgument(const AbstractMetaFunctionList &overloads)
{
    bool singleArgument = true;
    for (const AbstractMetaFunction *func : overloads) {
        if (func->arguments().size() - numberOfRemovedArguments(func) != 1) {
            singleArgument = false;
            break;
        }
    }
    return singleArgument;
}

void OverloadData::dumpGraph(const QString &filename) const
{
    QFile file(filename);
    if (file.open(QFile::WriteOnly)) {
        QTextStream s(&file);
        s << m_headOverloadData->dumpGraph();
    }
}

static inline QString toHtml(QString s)
{
    s.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    s.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    s.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    return s;
}

QString OverloadData::dumpGraph() const
{
    Indentor INDENT;
    Indentation indent(INDENT);
    QString result;
    QTextStream s(&result);
    if (m_argPos == -1) {
        const AbstractMetaFunction *rfunc = referenceFunction();
        s << "digraph OverloadedFunction {" << endl;
        s << INDENT << "graph [fontsize=12 fontname=freemono labelloc=t splines=true overlap=false rankdir=LR];" << endl;

        // Shows all function signatures
        s << "legend [fontsize=9 fontname=freemono shape=rect label=\"";
        for (const AbstractMetaFunction *func : m_overloads) {
            s << "f" << functionNumber(func) << " : ";
            if (func->type())
                s << toHtml(func->type()->cppSignature());
            else
                s << "void";
            s << ' ' << toHtml(func->minimalSignature()) << "\\l";
        }
        s << "\"];" << endl;

        // Function box title
        s << INDENT << '"' << rfunc->name() << "\" [shape=plaintext style=\"filled,bold\" margin=0 fontname=freemono fillcolor=white penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";
        s << "<tr><td bgcolor=\"black\" align=\"center\" cellpadding=\"6\" colspan=\"2\"><font color=\"white\">";
        if (rfunc->ownerClass())
            s << rfunc->ownerClass()->name() << "::";
        s << toHtml(rfunc->name()) << "</font>";
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
            s << toHtml(rfunc->type()->cppSignature());
        else
            s << "void";
        s << "</td></tr>";

        // Shows type changes for all function signatures
        for (const AbstractMetaFunction *func : m_overloads) {
            if (func->typeReplaced(0).isEmpty())
                continue;
            s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
            s << "-type</td><td bgcolor=\"gray\" align=\"left\">";
            s << toHtml(func->typeReplaced(0)) << "</td></tr>";
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
        for (const AbstractMetaFunction *func : m_overloads)
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        s << "</table>> ];" << endl;

        for (const OverloadData *pd : m_nextOverloadData)
            s << INDENT << '"' << rfunc->name() << "\" -> " << pd->dumpGraph();

        s << "}" << endl;
    } else {
        QString argId = QLatin1String("arg_") + QString::number(quintptr(this));
        s << argId << ';' << endl;

        s << INDENT << '"' << argId << "\" [shape=\"plaintext\" style=\"filled,bold\" margin=\"0\" fontname=\"freemono\" fillcolor=\"white\" penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";

        // Argument box title
        s << "<tr><td bgcolor=\"black\" align=\"left\" cellpadding=\"2\" colspan=\"2\">";
        s << "<font color=\"white\" point-size=\"11\">arg #" << argPos() << "</font></td></tr>";

        // Argument type information
        QString type = hasArgumentTypeReplace() ? argumentTypeReplaced() : argType()->cppSignature();
        s << "<tr><td bgcolor=\"gray\" align=\"right\">type</td><td bgcolor=\"gray\" align=\"left\">";
        s << toHtml(type) << "</td></tr>";
        if (hasArgumentTypeReplace()) {
            s << "<tr><td bgcolor=\"gray\" align=\"right\">orig. type</td><td bgcolor=\"gray\" align=\"left\">";
            s << toHtml(argType()->cppSignature()) << "</td></tr>";
        }

        // Overloads for the signature to present point
        s << "<tr><td bgcolor=\"gray\" align=\"right\">overloads</td><td bgcolor=\"gray\" align=\"left\">";
        for (const AbstractMetaFunction *func : m_overloads)
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        // Show default values (original and modified) for various functions
        for (const AbstractMetaFunction *func : m_overloads) {
            const AbstractMetaArgument *arg = argument(func);
            if (!arg)
                continue;
            QString argDefault = ShibokenGenerator::getDefaultValue(func, arg);
            if (!argDefault.isEmpty() ||
                argDefault != arg->originalDefaultValueExpression()) {
                s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
                s << "-default</td><td bgcolor=\"gray\" align=\"left\">";
                s << argDefault << "</td></tr>";
            }
            if (argDefault != arg->originalDefaultValueExpression()) {
                s << "<tr><td bgcolor=\"gray\" align=\"right\">f" << functionNumber(func);
                s << "-orig-default</td><td bgcolor=\"gray\" align=\"left\">";
                s << arg->originalDefaultValueExpression() << "</td></tr>";
            }
        }

        s << "</table>>];" << endl;

        for (const OverloadData *pd : m_nextOverloadData)
            s << INDENT << argId << " -> " << pd->dumpGraph();
    }
    return result;
}

int OverloadData::functionNumber(const AbstractMetaFunction *func) const
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

bool OverloadData::hasArgumentWithDefaultValue(const AbstractMetaFunctionList &overloads)
{
    if (OverloadData::getMinMaxArguments(overloads).second == 0)
        return false;
    for (const AbstractMetaFunction *func : overloads) {
        if (hasArgumentWithDefaultValue(func))
            return true;
    }
    return false;
}

bool OverloadData::hasArgumentWithDefaultValue() const
{
    if (maxArgs() == 0)
        return false;
    for (const AbstractMetaFunction *func : m_overloads) {
        if (hasArgumentWithDefaultValue(func))
            return true;
    }
    return false;
}

bool OverloadData::hasArgumentWithDefaultValue(const AbstractMetaFunction *func)
{
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument *arg : arguments) {
        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;
        if (!ShibokenGenerator::getDefaultValue(func, arg).isEmpty())
            return true;
    }
    return false;
}

AbstractMetaArgumentList OverloadData::getArgumentsWithDefaultValues(const AbstractMetaFunction *func)
{
    AbstractMetaArgumentList args;
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (AbstractMetaArgument *arg : arguments) {
        if (ShibokenGenerator::getDefaultValue(func, arg).isEmpty()
            || func->argumentRemoved(arg->argumentIndex() + 1))
            continue;
        args << arg;
    }
    return args;
}

#ifndef QT_NO_DEBUG_STREAM
void OverloadData::formatDebug(QDebug &d) const
{
    const int count = m_overloads.size();
    d << "argType=" << m_argType << ", minArgs=" << m_minArgs << ", maxArgs=" << m_maxArgs
        << ", argPos=" << m_argPos << ", argTypeReplaced=\"" << m_argTypeReplaced
        << "\", overloads[" << count << "]=(";
    const int oldVerbosity = d.verbosity();
    d.setVerbosity(3);
    for (int i = 0; i < count; ++i) {
        if (i)
            d << '\n';
        d << m_overloads.at(i);
    }
    d << ')';
    d.setVerbosity(oldVerbosity);
}

QDebug operator<<(QDebug d, const OverloadData *od)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "OverloadData(";
    if (od)
        od->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
