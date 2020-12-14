/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef GRAPH_H
#define GRAPH_H

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <algorithm>

/// Result of topologically sorting of a graph (list of nodes in order
/// or list of nodes that have cyclic dependencies).
template <class Node>
struct GraphSortResult
{
    using NodeList = QList<Node>;

    bool isValid() const { return !result.isEmpty() && cyclic.isEmpty(); }
    void format(QDebug &debug) const;

    NodeList result;
    NodeList cyclic;
};

/// A graph that can have its nodes topologically sorted. The nodes need to
/// have operator==().
template <class Node>
class Graph
{
public:
    using NodeList = QList<Node>;

    Graph() = default;

    // Construct from a sequence of nodes
    template<class It>
    explicit Graph(It i1, It i2) { setNodes(i1, i2); }

    template<class It>
    void setNodes(It i1, It i2)
    {
        for (; i1 != i2; ++i1)
            addNode(*i1);
    }

    bool addNode(Node n);

    /// Returns whether node was registered
    bool hasNode(Node node) { return indexOfNode(node) != -1; }

    /// Returns the numbed of nodes in this graph.
    qsizetype nodeCount() const { return m_nodeEntries.size(); }

    /// Returns true if the graph contains the edge from -> to
    bool containsEdge(Node from, Node to) const;
    /// Returns true if the graph has any edges
    bool hasEdges() const;

    /// Adds an edge to this graph.
    bool addEdge(Node from, Node to);
    /// Removes an edge from this graph.
    bool removeEdge(Node from, Node to);
    /// Clears the graph
    void clear() { m_nodeEntries.clear(); }

    /// Dumps a dot graph to a file named \p filename.
    /// \param fileName file name where the output should be written.
    /// \param f function returning the name of a node
    template <class NameFunction>
    bool dumpDot(const QString& fileName, NameFunction f) const;

    void format(QDebug &debug) const;

    /**
    *   Topologically sort this graph.
    *   \return A collection with all nodes topologically sorted or an empty collection if a cyclic
    *   dependency was found.
    */
    GraphSortResult<Node> topologicalSort() const;

private:
    enum Color { WHITE, GRAY, BLACK };

    struct NodeEntry
    {
        Node node;
        NodeList targets;
        mutable Color color;
    };

    Color colorAt(qsizetype i) const { return m_nodeEntries.at(i).color; }
    qsizetype indexOfNode(Node n) const;
    void depthFirstVisit(qsizetype i, NodeList &result) const;

    QList<NodeEntry> m_nodeEntries;
};

template <class Node>
qsizetype Graph<Node>::indexOfNode(Node n) const
{
    for (qsizetype i = 0, size = m_nodeEntries.size(); i < size; ++i) {
        if (m_nodeEntries.at(i).node == n)
            return i;
    }
    return -1;
}

template <class Node>
bool Graph<Node>::addNode(Node n)
{
    if (hasNode(n))
        return false;
    m_nodeEntries.append({n, {}, WHITE});
    return true;
}

template <class Node>
void Graph<Node>::depthFirstVisit(qsizetype i, NodeList &result) const
{
    m_nodeEntries[i].color = GRAY;

    for (auto to : m_nodeEntries[i].targets) {
        const qsizetype toIndex = indexOfNode(to);
        Q_ASSERT(toIndex != -1);
        switch (colorAt(toIndex)) {
        case WHITE:
            depthFirstVisit(toIndex, result);
            break;
        case GRAY:
            return; // This is not a DAG!
        case BLACK:
            break;
        }
    }

    m_nodeEntries[i].color = BLACK;

    result.append(m_nodeEntries.at(i).node);
}

template <class Node>
GraphSortResult<Node> Graph<Node>::topologicalSort() const
{
    const qsizetype size = m_nodeEntries.size();

    GraphSortResult<Node> result;
    result.result.reserve(size);

    if (hasEdges()) {
        for (qsizetype i = 0; i < size; ++i)
            m_nodeEntries[i].color = WHITE;
        for (qsizetype i = 0; i < size; ++i)  {
            if (colorAt(i) == WHITE) // recursive calls may have set it to black
                depthFirstVisit(i, result.result);
        }
    } else { // no edges, shortcut
        std::transform(m_nodeEntries.cbegin(), m_nodeEntries.cend(),
                       std::back_inserter(result.result),
                       [](const NodeEntry &ne) { return ne.node; });
    }

    if (result.result.size() == size) {
        std::reverse(result.result.begin(), result.result.end());
    } else {
        for (const auto &nodeEntry : m_nodeEntries) {
            if (!result.result.contains(nodeEntry.node))
                result.cyclic.append(nodeEntry.node);
        }
        result.result.clear(); // Not a DAG!
    }
    return result;
}

template <class Node>
bool Graph<Node>::containsEdge(Node from, Node to) const
{
    const qsizetype i = indexOfNode(from);
    return i != -1 && m_nodeEntries.at(i).targets.contains(to);
}

template <class Node>
bool Graph<Node>::hasEdges() const
{
    for (const auto &nodeEntry : m_nodeEntries) {
        if (!nodeEntry.targets.isEmpty())
            return true;
    }
    return false;
}

template <class Node>
bool Graph<Node>::addEdge(Node from, Node to)
{
    const qsizetype i = indexOfNode(from);
    if (i == -1 || !hasNode(to) || m_nodeEntries.at(i).targets.contains(to))
        return false;
    m_nodeEntries[i].targets.append(to);
    return true;
}

template <class Node>
bool Graph<Node>::removeEdge(Node from, Node to)
{
    const qsizetype i = indexOfNode(from);
    if (i == -1)
        return false;
    auto &targets = m_nodeEntries[i].targets;
    const qsizetype toIndex = targets.indexOf(to);
    if (toIndex == -1)
        return false;
    targets.removeAt(toIndex);
    return true;
}

template <class Node>
template <class NameFunction>
bool Graph<Node>::dumpDot(const QString& fileName,
                          NameFunction nameFunction) const
{
    QFile output(fileName);
    if (!output.open(QIODevice::WriteOnly))
        return false;
    QTextStream s(&output);
    s << "digraph D {\n";
    for (const auto &nodeEntry : m_nodeEntries) {
        if (!nodeEntry.targets.isEmpty()) {
            const QString fromName = nameFunction(nodeEntry.node);
            for (const Node &to : nodeEntry.targets)
                s << '"' << fromName << "\" -> \"" << nameFunction(to) << "\"\n";
        }
    }
    s << "}\n";
    return true;
}

template <class Node>
void Graph<Node>::format(QDebug &debug) const
{
    const qsizetype size = m_nodeEntries.size();
    debug << "nodes[" << size << "] = (";
    for (qsizetype i = 0; i < size; ++i) {
        const auto &nodeEntry = m_nodeEntries.at(i);
        if (i)
            debug << ", ";
        debug << nodeEntry.node;
        if (!nodeEntry.targets.isEmpty()) {
            debug << " -> [";
            for (qsizetype t = 0, tsize = nodeEntry.targets.size(); t < tsize; ++t) {
                if (t)
                    debug << ", ";
                debug << nodeEntry.targets.at(t);
            }
            debug << ']';
        }
    }
    debug << ')';
}

template <class Node>
QDebug operator<<(QDebug debug, const Graph<Node> &g)
{
    QDebugStateSaver saver(debug);
    debug.noquote();
    debug.nospace();
    debug << "Graph(";
    g.format(debug);
    debug << ')';
    return debug;
}

template <class Node>
void GraphSortResult<Node>::format(QDebug &debug) const
{
    if (isValid())
        debug << "Valid, " << result;
    else
        debug << "Invalid, cyclic dependencies: " << cyclic;
}

template <class Node>
QDebug operator<<(QDebug debug, const GraphSortResult<Node> &r)
{
    QDebugStateSaver saver(debug);
    debug.noquote();
    debug.nospace();
    debug << "Graph::SortResult(";
    r.format(debug);
    debug << ')';
    return debug;
}

#endif // GRAPH_H
