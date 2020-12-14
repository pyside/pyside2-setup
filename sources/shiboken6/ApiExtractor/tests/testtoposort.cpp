/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
#include "graph.h"

#include <QtTest/QTest>
#include <QtCore/QDebug>

using IntGraph = Graph<int>;

Q_DECLARE_METATYPE(IntGraph)

using IntList = QList<int>;

void TestTopoSort::testTopoSort_data()
{
    QTest::addColumn<IntGraph>("graph");
    QTest::addColumn<bool>("expectedValid");
    QTest::addColumn<IntList>("expectedOrder");

    const int nodes1[] = {0, 1, 2};
    IntGraph g(std::begin(nodes1), std::end(nodes1));
    g.addEdge(1, 2);
    g.addEdge(0, 1);
    IntList expected = {0, 1, 2};
    QTest::newRow("DAG") << g << true << expected;

    const int nodes2[] = {0, 1};
    g.clear();
    g.setNodes(std::begin(nodes2), std::end(nodes2));
    expected = {1, 0};
    QTest::newRow("No edges") << g << true << expected;

    g.clear();
    g.setNodes(std::begin(nodes1), std::end(nodes1));
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    expected.clear();
    QTest::newRow("Cyclic") << g << false << expected;
}

void TestTopoSort::testTopoSort()
{
    QFETCH(IntGraph, graph);
    QFETCH(bool, expectedValid);
    QFETCH(IntList, expectedOrder);

    const auto result = graph.topologicalSort();
    QCOMPARE(result.isValid(), expectedValid);
    if (expectedValid) {
        QCOMPARE(result.result, expectedOrder);
        QVERIFY(result.cyclic.isEmpty());
    } else {
        QVERIFY(!result.cyclic.isEmpty());
    }
}

QTEST_APPLESS_MAIN(TestTopoSort)

