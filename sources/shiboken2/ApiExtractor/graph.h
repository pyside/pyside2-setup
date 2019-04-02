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

#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
#include <QHash>
#include <QString>

/// A graph that can have their nodes topologically sorted.
class Graph
{
public:
    using Indexes = QVector<int>;

    /// Create a new graph with \p numNodes nodes.
    Graph(int numNodes);
    ~Graph();

    /// Returns the numbed of nodes in this graph.
    int nodeCount() const;
    /// Returns true if the graph contains the edge from -> to
    bool containsEdge(int from, int to);
    /// Adds an edge to this graph.
    void addEdge(int from, int to);
    /// Removes an edge out of this graph.
    void removeEdge(int from, int to);
    /// Print this graph to stdout.
    void dump() const;
    /**
    *   Dumps a dot graph to a file named \p filename.
    *   \param nodeNames map used to translate node ids to human readable text.
    *   \param fileName file name where the output should be written.
    */
    void dumpDot(const QHash<int, QString>& nodeNames, const QString& fileName) const;

    /**
    *   Topologically sort this graph.
    *   \return A collection with all nodes topologically sorted or an empty collection if a cyclic
    *   dependency was found.
    */
    Indexes topologicalSort() const;
private:

    struct GraphPrivate;
    GraphPrivate* m_d;
};

#endif
