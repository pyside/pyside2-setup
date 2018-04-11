/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt for Python project.
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

#include "graph.h"
#include <QVector>
#include <QDebug>
#include <QLinkedList>
#include <QSet>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <QFile>

struct Graph::GraphPrivate
{
    enum Color { WHITE, GRAY, BLACK };
    typedef QVector<QSet<int> > Edges;
    typedef QSet<int>::const_iterator EdgeIterator;

    Edges edges;

    GraphPrivate(int numNodes) : edges(numNodes)
    {
    }

    void dfsVisit(int node, QLinkedList<int>& result, QVector<Color>& colors) const
    {
        colors[node] = GRAY;
        EdgeIterator it = edges[node].begin();
        for (; it != edges[node].end(); ++it) {
            if (colors[*it] == WHITE)
                dfsVisit(*it, result, colors);
            else if (colors[*it] == GRAY) // This is not a DAG!
                return;
        }
        colors[node] = BLACK;
        result.push_front(node);
    }
};

Graph::Graph(int numNodes) : m_d(new GraphPrivate(numNodes))
{
}

Graph::~Graph()
{
    delete m_d;
}

int Graph::nodeCount() const
{
    return m_d->edges.size();
}

QLinkedList<int> Graph::topologicalSort() const
{
    int nodeCount = Graph::nodeCount();
    QLinkedList<int> result;
    QVector<GraphPrivate::Color> colors(nodeCount, GraphPrivate::WHITE);

    for (int i = 0; i < nodeCount; ++i) {
        if (colors[i] == GraphPrivate::WHITE)
            m_d->dfsVisit(i, result, colors);
    }

    // Not a DAG!
    if (result.size() != nodeCount)
        return QLinkedList<int>();
    return result;
}

bool Graph::containsEdge(int from, int to)
{
    return m_d->edges[from].contains(to);
}

void Graph::addEdge(int from, int to)
{
    Q_ASSERT(to < (int)m_d->edges.size());
    m_d->edges[from].insert(to);
}

void Graph::removeEdge(int from, int to)
{
    m_d->edges[from].remove(to);
}

void Graph::dump() const
{
    for (int i = 0; i < m_d->edges.size(); ++i) {
        std::cout << i << " -> ";
        std::copy(m_d->edges[i].begin(), m_d->edges[i].end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }
}

void Graph::dumpDot(const QHash< int, QString >& nodeNames, const QString& fileName) const
{
    QFile output(fileName);
    if (!output.open(QIODevice::WriteOnly))
        return;
    QTextStream s(&output);
    s << "digraph D {\n";
    for (int i = 0; i < m_d->edges.size(); ++i) {
        for (auto it = m_d->edges[i].cbegin(), end = m_d->edges[i].cend(); it != end; ++it)
            s << '"' << nodeNames[i] << "\" -> \"" << nodeNames[*it] << "\"\n";
    }
    s << "}\n";
}
