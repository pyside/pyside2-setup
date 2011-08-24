/*
* This file is part of the API Extractor project.
*
* Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "testtypeindex.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <QDir>

void TestTypeIndex::initTestCase()
{
    m_td = TypeDatabase::instance(true);
    m_td->addTypesystemPath(QDir::currentPath());
    QVERIFY(m_td->parseFile("typeindex_1.xml"));
    QFile cppCode("typeindex.h");
    bool res = m_builder.build(&cppCode);
    QVERIFY(res);
}

void TestTypeIndex::testTypeIndex_data()
{
    QTest::addColumn<QString>("typeName");
    QTest::addColumn<int>("typeIndex");

    QTest::newRow("unsigned int") << "unsigned int" << 0;
    QTest::newRow("bool") << "bool" << 0;
    QTest::newRow("P1") << "P1" << 0;
    QTest::newRow("P2") << "P2" << 0;
    QTest::newRow("P3") << "P3" << 1;
    QTest::newRow("Pa4") << "Pa4" << 2;
    QTest::newRow("Value1") << "Value1" << 0;
    QTest::newRow("Value2") << "Value2" << 1;
    QTest::newRow("P4") << "P4" << 0;
    QTest::newRow("P5") << "P5" << 1;
    QTest::newRow("Value3") << "Value3" << 0;
    QTest::newRow("Value4") << "Value4" << 1;
}

void TestTypeIndex::testTypeIndex()
{
    QFETCH(QString, typeName);
    QFETCH(int, typeIndex);

    TypeEntry* type = m_td->findType(typeName);
    QVERIFY(type);

    QCOMPARE(getTypeIndex(type), typeIndex);
}

void TestTypeIndex::testMaxTypeIndex()
{
    QCOMPARE(getMaxPrimitiveTypeIndex("Foo"), 2);
    QCOMPARE(getMaxTypeIndex("Foo"), 1);
    QCOMPARE(getMaxPrimitiveTypeIndex("Bar"), 1);
    QCOMPARE(getMaxTypeIndex("Bar"), 1);
}

void TestTypeIndex::testDeprecatedFunction()
{
    // This returns max type index + 1, the new versions return the right value
    QCOMPARE(getMaxTypeIndex(), 2);
}

QTEST_APPLESS_MAIN(TestTypeIndex)

#include "testtypeindex.moc"

