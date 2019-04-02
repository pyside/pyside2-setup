/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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

#include "testtoposort.h"
#include <QtTest/QTest>
#include "graph.h"
#include <QDebug>

void TestTopoSort::testTopoSort()
{
    {
        Graph g(3);
        g.addEdge(1, 2);
        g.addEdge(0, 1);
        const auto result = g.topologicalSort();
        QCOMPARE(result.size(), 3);
        auto it = result.begin();
        QCOMPARE(*it, 0);
        QCOMPARE(*(++it), 1);
        QCOMPARE(*(++it), 2);
    }
    {
        Graph g(2);
        const auto result = g.topologicalSort();
        QCOMPARE(result.size(), 2);
        auto it = result.begin();
        QCOMPARE(*it, 1);
        QCOMPARE(*(++it), 0);
    }
}

void TestTopoSort::testCiclicGraph()
{
    Graph g(3);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    const auto result = g.topologicalSort();
    QVERIFY(result.isEmpty());
}

QTEST_APPLESS_MAIN(TestTopoSort)

