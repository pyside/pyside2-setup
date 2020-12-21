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

#include <abstractmetafunction.h>
#include <apiextractorresult.h>
#include <abstractmetalang.h>
#include <reporthandler.h>
#include <typesystem.h>
#include <graph.h>
#include "overloaddata.h"
#include "ctypenames.h"
#include "textstream.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>

#include <algorithm>

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

static QString getTypeName(const AbstractMetaType &type)
{
    const TypeEntry *typeEntry = getReferencedTypeEntry(type.typeEntry());
    QString typeName = typeEntry->name();
    if (typeEntry->isContainer()) {
        QStringList types;
        for (const auto &cType : type.instantiations()) {
            const TypeEntry *typeEntry = getReferencedTypeEntry(cType.typeEntry());
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

static bool typesAreEqual(const AbstractMetaType &typeA, const AbstractMetaType &typeB)
{
    if (typeA.typeEntry() == typeB.typeEntry()) {
        if (typeA.isContainer() || typeA.isSmartPointer()) {
            if (typeA.instantiations().size() != typeB.instantiations().size())
                return false;

            for (int i = 0; i < typeA.instantiations().size(); ++i) {
                if (!typesAreEqual(typeA.instantiations().at(i), typeB.instantiations().at(i)))
                    return false;
            }
            return true;
        }

        return !(typeA.isCString() ^ typeB.isCString());
    }
    return false;
}

/**
 * Helper function that returns the name of a container get from containerType argument and
 * an instantiation taken either from an implicit conversion expressed by the function argument,
 * or from the string argument implicitConvTypeName.
 */
static QString getImplicitConversionTypeName(const AbstractMetaType &containerType,
                                             const AbstractMetaType &instantiation,
                                             const AbstractMetaFunctionCPtr &function,
                                             const QString &implicitConvTypeName = QString())
{
    QString impConv;
    if (!implicitConvTypeName.isEmpty())
        impConv = implicitConvTypeName;
    else if (function->isConversionOperator())
        impConv = function->ownerClass()->typeEntry()->name();
    else
        impConv = getTypeName(function->arguments().constFirst().type());

    QStringList types;
    for (const auto &otherType : containerType.instantiations())
        types << (otherType == instantiation ? impConv : getTypeName(otherType));

    return containerType.typeEntry()->qualifiedCppName() + QLatin1Char('<')
           + types.join(QLatin1String(", ")) + QLatin1String(" >");
}

// overloaddata.cpp
static QString msgCyclicDependency(const QString &funcName, const QString &graphName,
                                   const AbstractMetaFunctionCList &cyclic,
                                   const AbstractMetaFunctionCList &involvedConversions)
{
    QString result;
    QTextStream str(&result);
    str << "Cyclic dependency found on overloaddata for \"" << funcName
         << "\" method! The graph boy saved the graph at \"" << QDir::toNativeSeparators(graphName)
         << "\". Cyclic functions:";
    for (auto c : cyclic)
        str << ' ' << c->signature();
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

static inline int overloadNumber(const OverloadData *o)
{
    return o->referenceFunction()->overloadNumber();
}

bool OverloadData::sortByOverloadNumberModification()
{
    if (std::all_of(m_nextOverloadData.cbegin(), m_nextOverloadData.cend(),
                    [](const OverloadData *o) { return overloadNumber(o) == TypeSystem::OverloadNumberDefault; })) {
        return false;
    }
    std::stable_sort(m_nextOverloadData.begin(), m_nextOverloadData.end(),
                     [] (const OverloadData *o1, const OverloadData *o2) {
                         return overloadNumber(o1) < overloadNumber(o2);
                     });
    return true;
}

static inline QString pyObjectT() { return QStringLiteral("PyObject"); }
static inline QString pySequenceT() { return QStringLiteral("PySequence"); }
static inline QString pyBufferT() { return QStringLiteral("PyBuffer"); }

using OverloadGraph = Graph<QString>;

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
    QHash<QString, OverloadDataList> typeToOverloads;

    bool checkPyObject = false;
    bool checkPySequence = false;
    bool checkQString = false;
    bool checkQVariant = false;
    bool checkPyBuffer = false;

    // Primitive types that are not int, long, short,
    // char and their respective unsigned counterparts.
    static const QStringList nonIntegerPrimitives{floatT(), doubleT(), boolT()};

    // Signed integer primitive types.
    static const QStringList signedIntegerPrimitives{intT(), shortT(), longT(), longLongT()};

    // sort the children overloads
    for (OverloadData *ov : qAsConst(m_nextOverloadData))
        ov->sortNextOverloads();

    if (m_nextOverloadData.size() <= 1 || sortByOverloadNumberModification())
        return;

    // Populates the OverloadSortData object containing map and reverseMap, to map type names to ids,
    // these ids will be used by the topological sort algorithm, because is easier and faster to work
    // with graph sorting using integers.

    OverloadGraph graph;
    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        const QString typeName = getTypeName(ov);
        auto it = typeToOverloads.find(typeName);
        if (it == typeToOverloads.end()) {
            typeToOverloads.insert(typeName, {ov});
            graph.addNode(typeName);
        } else {
            it.value().append(ov);
        }

        if (!checkPyObject && typeName == pyObjectT())
            checkPyObject = true;
        else if (!checkPySequence && typeName == pySequenceT())
            checkPySequence = true;
        else if (!checkPyBuffer && typeName == pyBufferT())
            checkPyBuffer = true;
        else if (!checkQVariant && typeName == qVariantT())
            checkQVariant = true;
        else if (!checkQString && typeName == qStringT())
            checkQString = true;

        for (const auto &instantiation : ov->argType().instantiations()) {
            // Add dependencies for type instantiation of container.
            graph.addNode(getTypeName(instantiation));

            // Build dependency for implicit conversion types instantiations for base container.
            // For example, considering signatures "method(list<PointF>)" and "method(list<Point>)",
            // and being PointF implicitly convertible from Point, an list<T> instantiation with T
            // as Point must come before the PointF instantiation, or else list<Point> will never
            // be called. In the case of primitive types, list<double> must come before list<int>.
            if (instantiation.isPrimitive() && (signedIntegerPrimitives.contains(instantiation.name()))) {
                for (const QString &primitive : qAsConst(nonIntegerPrimitives))
                    graph.addNode(getImplicitConversionTypeName(ov->argType(), instantiation, nullptr, primitive));
            } else {
                const auto &funcs = m_api.implicitConversions(instantiation);
                for (const auto &function : funcs)
                    graph.addNode(getImplicitConversionTypeName(ov->argType(), instantiation, function));
            }
        }
    }


    // Create the graph of type dependencies based on implicit conversions.
    // All C++ primitive types, add any forgotten type AT THE END OF THIS LIST!
    static const QStringList primitiveTypes{intT(), unsignedIntT(), longT(), unsignedLongT(),
        shortT(), unsignedShortT(), boolT(), unsignedCharT(), charT(), floatT(),
        doubleT(), constCharPtrT()};

    QStringList foundPrimitiveTypeIds;
    for (const auto &p : primitiveTypes) {
        if (graph.hasNode(p))
            foundPrimitiveTypeIds.append(p);
    }

    if (checkPySequence && checkPyObject)
        graph.addEdge(pySequenceT(), pyObjectT());

    QStringList classesWithIntegerImplicitConversion;

    AbstractMetaFunctionCList involvedConversions;

    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        const AbstractMetaType &targetType = ov->argType();
        const QString targetTypeEntryName = getTypeName(ov);

        // Process implicit conversions
        const auto &functions = m_api.implicitConversions(targetType);
        for (const auto &function : functions) {
            QString convertibleType;
            if (function->isConversionOperator())
                convertibleType = function->ownerClass()->typeEntry()->name();
            else
                convertibleType = getTypeName(function->arguments().constFirst().type());

            if (convertibleType == intT() || convertibleType == unsignedIntT())
                classesWithIntegerImplicitConversion << targetTypeEntryName;

            if (!graph.hasNode(convertibleType))
                continue;

            // If a reverse pair already exists, remove it. Probably due to the
            // container check (This happened to QVariant and QHash)
            graph.removeEdge(targetTypeEntryName, convertibleType);
            graph.addEdge(convertibleType, targetTypeEntryName);
            involvedConversions.append(function);
        }

        // Process inheritance relationships
        if (targetType.isValue() || targetType.isObject()) {
            auto metaClass = AbstractMetaClass::findClass(m_api.classes(),
                                                          targetType.typeEntry());
            const AbstractMetaClassList &ancestors = metaClass->allTypeSystemAncestors();
            for (const AbstractMetaClass *ancestor : ancestors) {
                QString ancestorTypeName = ancestor->typeEntry()->name();
                if (!graph.hasNode(ancestorTypeName))
                    continue;
                graph.removeEdge(ancestorTypeName, targetTypeEntryName);
                graph.addEdge(targetTypeEntryName, ancestorTypeName);
            }
        }

        // Process template instantiations
        for (const auto &instantiation : targetType.instantiations()) {
            const QString convertible = getTypeName(instantiation);
            if (graph.hasNode(convertible)) {
                if (!graph.containsEdge(targetTypeEntryName, convertible)) // Avoid cyclic dependency.
                    graph.addEdge(convertible, targetTypeEntryName);

                if (instantiation.isPrimitive() && (signedIntegerPrimitives.contains(instantiation.name()))) {
                    for (const QString &primitive : qAsConst(nonIntegerPrimitives)) {
                        QString convertibleTypeName =
                            getImplicitConversionTypeName(ov->argType(), instantiation, nullptr, primitive);
                        // Avoid cyclic dependency.
                        if (!graph.containsEdge(targetTypeEntryName, convertibleTypeName))
                            graph.addEdge(convertibleTypeName, targetTypeEntryName);
                    }

                } else {
                    const auto &funcs = m_api.implicitConversions(instantiation);
                    for (const auto &function : funcs) {
                        QString convertibleTypeName =
                            getImplicitConversionTypeName(ov->argType(), instantiation, function);
                        // Avoid cyclic dependency.
                        if (!graph.containsEdge(targetTypeEntryName, convertibleTypeName)) {
                            graph.addEdge(convertibleTypeName, targetTypeEntryName);
                            involvedConversions.append(function);
                        }
                    }
                }
            }
        }


        if ((checkPySequence || checkPyObject || checkPyBuffer)
            && !targetTypeEntryName.contains(pyObjectT())
            && !targetTypeEntryName.contains(pyBufferT())
            && !targetTypeEntryName.contains(pySequenceT())) {
            if (checkPySequence) {
                // PySequence will be checked after all more specific types, but before PyObject.
                graph.addEdge(targetTypeEntryName, pySequenceT());
            } else if (checkPyBuffer) {
                // PySequence will be checked after all more specific types, but before PyObject.
                graph.addEdge(targetTypeEntryName, pyBufferT());
            } else {
                // Add dependency on PyObject, so its check is the last one (too generic).
                graph.addEdge(targetTypeEntryName, pyObjectT());
            }
        } else if (checkQVariant && targetTypeEntryName != qVariantT()) {
            if (!graph.containsEdge(qVariantT(), targetTypeEntryName)) // Avoid cyclic dependency.
                graph.addEdge(targetTypeEntryName, qVariantT());
        } else if (checkQString && ov->argType().isPointer()
            && targetTypeEntryName != qStringT()
            && targetTypeEntryName != qByteArrayT()
            && (!checkPyObject || targetTypeEntryName != pyObjectT())) {
            if (!graph.containsEdge(qStringT(), targetTypeEntryName)) // Avoid cyclic dependency.
                graph.addEdge(targetTypeEntryName, qStringT());
        }

        if (targetType.isEnum()) {
            // Enum values must precede primitive types.
            for (const auto &id : foundPrimitiveTypeIds)
                graph.addEdge(targetTypeEntryName, id);
        }
    }

    // QByteArray args need to be checked after QString args
    if (graph.hasNode(qStringT()) && graph.hasNode(qByteArrayT()))
        graph.addEdge(qStringT(), qByteArrayT());

    for (OverloadData *ov : qAsConst(m_nextOverloadData)) {
        const AbstractMetaType &targetType = ov->argType();
        if (!targetType.isEnum())
            continue;

        QString targetTypeEntryName = getTypeName(targetType);
        // Enum values must precede types implicitly convertible from "int" or "unsigned int".
        for (const QString &implicitFromInt : qAsConst(classesWithIntegerImplicitConversion))
            graph.addEdge(targetTypeEntryName, implicitFromInt);
    }


    // Special case for double(int i) (not tracked by m_generator->implicitConversions
    for (const QString &signedIntegerName : qAsConst(signedIntegerPrimitives)) {
        if (graph.hasNode(signedIntegerName)) {
            for (const QString &nonIntegerName : qAsConst(nonIntegerPrimitives)) {
                if (graph.hasNode(nonIntegerName))
                    graph.addEdge(nonIntegerName, signedIntegerName);
            }
        }
    }

    // sort the overloads topologically based on the dependency graph.
    const auto unmappedResult = graph.topologicalSort();
    if (!unmappedResult.isValid()) {
        QString funcName = referenceFunction()->name();
        if (referenceFunction()->ownerClass())
            funcName.prepend(referenceFunction()->ownerClass()->name() + QLatin1Char('.'));

        // Dump overload graph
        QString graphName = QDir::tempPath() + QLatin1Char('/') + funcName + QLatin1String(".dot");
        graph.dumpDot(graphName, [] (const QString &n) { return n; });
        AbstractMetaFunctionCList cyclic;
        for (const auto &typeName : unmappedResult.cyclic) {
            const auto oit = typeToOverloads.constFind(typeName);
            if (oit != typeToOverloads.cend())
                cyclic.append(oit.value().constFirst()->referenceFunction());
        }
        qCWarning(lcShiboken, "%s", qPrintable(msgCyclicDependency(funcName, graphName, cyclic, involvedConversions)));
    }

    m_nextOverloadData.clear();
    for (const auto &typeName : unmappedResult.result) {
        const auto oit = typeToOverloads.constFind(typeName);
        if (oit != typeToOverloads.cend()) {
            std::copy(oit.value().crbegin(), oit.value().crend(),
                      std::back_inserter(m_nextOverloadData));
        }
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
OverloadData::OverloadData(const AbstractMetaFunctionCList &overloads,
                           const ApiExtractorResult &api)
    : m_minArgs(256), m_maxArgs(0), m_argPos(-1), m_argType(nullptr),
    m_headOverloadData(this), m_previousOverloadData(nullptr),
    m_api(api)
{
    for (const auto &func : overloads) {
        m_overloads.append(func);
        int argSize = func->arguments().size() - numberOfRemovedArguments(func);
        if (m_minArgs > argSize)
            m_minArgs = argSize;
        else if (m_maxArgs < argSize)
            m_maxArgs = argSize;
        OverloadData *currentOverloadData = this;
        const AbstractMetaArgumentList &arguments = func->arguments();
        for (const AbstractMetaArgument &arg : arguments) {
            if (func->argumentRemoved(arg.argumentIndex() + 1))
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

OverloadData::OverloadData(OverloadData *headOverloadData, const AbstractMetaFunctionCPtr &func,
                           const AbstractMetaType &argType, int argPos,
                           const ApiExtractorResult &api) :
      m_minArgs(256), m_maxArgs(0), m_argPos(argPos), m_argType(argType),
      m_headOverloadData(headOverloadData), m_previousOverloadData(nullptr), m_api(api)
{
    if (func)
        this->addOverload(func);
}

void OverloadData::addOverload(const AbstractMetaFunctionCPtr &func)
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
        if (func->arguments().at(i).hasDefaultValueExpression()) {
            int fixedArgIndex = i - removed;
            if (fixedArgIndex < m_headOverloadData->m_minArgs)
                m_headOverloadData->m_minArgs = fixedArgIndex;
        }
    }

    m_overloads.append(func);
}

OverloadData *OverloadData::addOverloadData(const AbstractMetaFunctionCPtr &func,
                                            const AbstractMetaArgument &arg)
{
    const AbstractMetaType &argType = arg.type();
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
        overloadData = new OverloadData(m_headOverloadData, func, argType, m_argPos + 1, m_api);
        overloadData->m_previousOverloadData = this;
        QString typeReplaced = func->typeReplaced(arg.argumentIndex() + 1);

        if (!typeReplaced.isEmpty())
            overloadData->m_argTypeReplaced = typeReplaced;
        m_nextOverloadData.append(overloadData);
    }

    return overloadData;
}

QStringList OverloadData::returnTypes() const
{
    QSet<QString> retTypes;
    for (const auto &func : m_overloads) {
        if (!func->typeReplaced(0).isEmpty())
            retTypes << func->typeReplaced(0);
        else if (!func->argumentRemoved(0))
            retTypes << func->type().cppSignature();
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
    for (const auto &func : m_overloads) {
        AbstractMetaArgumentList args = func->arguments();
        if (args.size() > 1 && args.constLast().type().isVarargs())
            return true;
    }
    return false;
}

bool OverloadData::hasAllowThread() const
{
    for (const auto &func : m_overloads) {
        if (func->allowThread())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction(const AbstractMetaFunctionCList &overloads)
{
    for (const auto &func : overloads) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticFunction() const
{
    for (const auto &func : m_overloads) {
        if (func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction(const AbstractMetaFunctionCList &overloads)
{
    for (const auto &func : overloads) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasInstanceFunction() const
{
    for (const auto &func : m_overloads) {
        if (!func->isStatic())
            return true;
    }
    return false;
}

bool OverloadData::hasStaticAndInstanceFunctions(const AbstractMetaFunctionCList &overloads)
{
    return OverloadData::hasStaticFunction(overloads) && OverloadData::hasInstanceFunction(overloads);
}

bool OverloadData::hasStaticAndInstanceFunctions() const
{
    return OverloadData::hasStaticFunction() && OverloadData::hasInstanceFunction();
}

AbstractMetaFunctionCPtr OverloadData::referenceFunction() const
{
    return m_overloads.constFirst();
}

const AbstractMetaArgument *OverloadData::argument(const AbstractMetaFunctionCPtr &func) const
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

    return &func->arguments().at(m_argPos + removed);
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
        if (!overloadData->getFunctionWithDefaultValue().isNull())
            return true;
    }
    return false;
}

static OverloadData *_findNextArgWithDefault(OverloadData *overloadData)
{
    if (!overloadData->getFunctionWithDefaultValue().isNull())
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

bool OverloadData::isFinalOccurrence(const AbstractMetaFunctionCPtr &func) const
{
    for (const OverloadData *pd : m_nextOverloadData) {
        if (pd->overloads().contains(func))
            return false;
    }
    return true;
}

AbstractMetaFunctionCList OverloadData::overloadsWithoutRepetition() const
{
    AbstractMetaFunctionCList overloads = m_overloads;
    for (const auto &func : m_overloads) {
        if (func->minimalSignature().endsWith(QLatin1String("const")))
            continue;
        for (const auto &f : qAsConst(overloads)) {
            if ((func->minimalSignature() + QLatin1String("const")) == f->minimalSignature()) {
                overloads.removeOne(f);
                break;
            }
        }
    }
    return overloads;
}

AbstractMetaFunctionCPtr OverloadData::getFunctionWithDefaultValue() const
{
    for (const auto &func : m_overloads) {
        int removedArgs = 0;
        for (int i = 0; i <= m_argPos + removedArgs; i++) {
            if (func->argumentRemoved(i + 1))
                removedArgs++;
        }
        if (func->arguments().at(m_argPos + removedArgs).hasDefaultValueExpression())
            return func;
    }
    return {};
}

QList<int> OverloadData::invalidArgumentLengths() const
{
    QSet<int> validArgLengths;

    for (const auto &func : qAsConst(m_headOverloadData->m_overloads)) {
        const AbstractMetaArgumentList args = func->arguments();
        int offset = 0;
        for (int i = 0; i < args.size(); ++i) {
            if (func->argumentRemoved(i+1)) {
                offset++;
            } else {
                if (args.at(i).hasDefaultValueExpression())
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

int OverloadData::numberOfRemovedArguments(const AbstractMetaFunctionCPtr &func, int finalArgPos)
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

bool OverloadData::isSingleArgument(const AbstractMetaFunctionCList &overloads)
{
    bool singleArgument = true;
    for (const auto &func : overloads) {
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
        TextStream s(&file);
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
    QString result;
    QTextStream s(&result);
    if (m_argPos == -1) {
        const auto rfunc = referenceFunction();
        s << "digraph OverloadedFunction {\n";
        s << "    graph [fontsize=12 fontname=freemono labelloc=t splines=true overlap=false rankdir=LR];\n";

        // Shows all function signatures
        s << "legend [fontsize=9 fontname=freemono shape=rect label=\"";
        for (const auto &func : m_overloads) {
            s << "f" << functionNumber(func) << " : "
                << toHtml(func->type().cppSignature())
                << ' ' << toHtml(func->minimalSignature()) << "\\l";
        }
        s << "\"];\n";

        // Function box title
        s << "    \"" << rfunc->name() << "\" [shape=plaintext style=\"filled,bold\" margin=0 fontname=freemono fillcolor=white penwidth=1 ";
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
        s << "<tr><td bgcolor=\"gray\" align=\"right\">original type</td><td bgcolor=\"gray\" align=\"left\">"
            << toHtml(rfunc->type().cppSignature())
            << "</td></tr>";

        // Shows type changes for all function signatures
        for (const auto &func : m_overloads) {
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
        for (const auto &func : m_overloads)
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        s << "</table>> ];\n";

        for (const OverloadData *pd : m_nextOverloadData)
            s << "    \""  << rfunc->name() << "\" -> " << pd->dumpGraph();

        s << "}\n";
    } else {
        QString argId = QLatin1String("arg_") + QString::number(quintptr(this));
        s << argId << ";\n";

        s << "    \"" << argId << "\" [shape=\"plaintext\" style=\"filled,bold\" margin=\"0\" fontname=\"freemono\" fillcolor=\"white\" penwidth=1 ";
        s << "label=<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">";

        // Argument box title
        s << "<tr><td bgcolor=\"black\" align=\"left\" cellpadding=\"2\" colspan=\"2\">";
        s << "<font color=\"white\" point-size=\"11\">arg #" << argPos() << "</font></td></tr>";

        // Argument type information
        QString type = hasArgumentTypeReplace() ? argumentTypeReplaced() : argType().cppSignature();
        s << "<tr><td bgcolor=\"gray\" align=\"right\">type</td><td bgcolor=\"gray\" align=\"left\">";
        s << toHtml(type) << "</td></tr>";
        if (hasArgumentTypeReplace()) {
            s << "<tr><td bgcolor=\"gray\" align=\"right\">orig. type</td><td bgcolor=\"gray\" align=\"left\">";
            s << toHtml(argType().cppSignature()) << "</td></tr>";
        }

        // Overloads for the signature to present point
        s << "<tr><td bgcolor=\"gray\" align=\"right\">overloads</td><td bgcolor=\"gray\" align=\"left\">";
        for (const auto &func : m_overloads)
            s << 'f' << functionNumber(func) << ' ';
        s << "</td></tr>";

        // Show default values (original and modified) for various functions
        for (const auto &func : m_overloads) {
            const AbstractMetaArgument *arg = argument(func);
            if (!arg)
                continue;
            QString argDefault = arg->defaultValueExpression();
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

        s << "</table>>];\n";

        for (const OverloadData *pd : m_nextOverloadData)
            s << "    " << argId << " -> " << pd->dumpGraph();
    }
    return result;
}

int OverloadData::functionNumber(const AbstractMetaFunctionCPtr &func) const
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

bool OverloadData::hasArgumentWithDefaultValue() const
{
    if (maxArgs() == 0)
        return false;
    for (const auto &func : m_overloads) {
        if (hasArgumentWithDefaultValue(func))
            return true;
    }
    return false;
}

bool OverloadData::hasArgumentWithDefaultValue(const AbstractMetaFunctionCPtr &func)
{
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument &arg : arguments) {
        if (func->argumentRemoved(arg.argumentIndex() + 1))
            continue;
        if (arg.hasDefaultValueExpression())
            return true;
    }
    return false;
}

AbstractMetaArgumentList OverloadData::getArgumentsWithDefaultValues(const AbstractMetaFunctionCPtr &func)
{
    AbstractMetaArgumentList args;
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument &arg : arguments) {
        if (!arg.hasDefaultValueExpression()
            || func->argumentRemoved(arg.argumentIndex() + 1))
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
