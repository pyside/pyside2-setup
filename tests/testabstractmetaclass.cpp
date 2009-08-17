/*
* This file is part of the API Extractor project.
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

#include "testabstractmetaclass.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include <QBuffer>


TestAbstractMetaClass::TestAbstractMetaClass() : m_builder(0)
{
}

void TestAbstractMetaClass::init()
{
    m_builder = new AbstractMetaBuilder;
}

void TestAbstractMetaClass::cleanup()
{
    delete m_builder;
    m_builder = 0;
}

void TestAbstractMetaClass::processCode (const char* cppCode, const char* xmlCode )
{
    QBuffer buffer;
    // parse typesystem
    buffer.setData(xmlCode);
    TypeDatabase::instance()->parseFile(&buffer);
    buffer.close();
    // parse C++ code
    buffer.setData(cppCode);
    m_builder->build(&buffer);
}

void TestAbstractMetaClass::testClassName()
{
    const char* cppCode ="class ClassName {};";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"ClassName\"/></typesystem>";
    processCode(cppCode, xmlCode);
    AbstractMetaClassList classes = m_builder->classes();
    QCOMPARE(classes.count(), 1);
    QCOMPARE(classes[0]->name(), QString("ClassName"));
}

void TestAbstractMetaClass::testClassNameUnderNamespace()
{
    const char* cppCode ="namespace Namespace { class ClassName {}; }";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name=\"Namespace\"/> \
        <value-type name=\"Namespace::ClassName\"/> \
    </typesystem>";
    processCode(cppCode, xmlCode);
    AbstractMetaClassList classes = m_builder->classes();
    QCOMPARE(classes.count(), 2); // 1 namespace + 1 class
    QCOMPARE(classes[0]->name(), QString("ClassName"));
    QCOMPARE(classes[0]->qualifiedCppName(), QString("Namespace::ClassName"));
    QCOMPARE(classes[1]->name(), QString("Namespace"));
}


QTEST_APPLESS_MAIN(TestAbstractMetaClass)

#include "testabstractmetaclass.moc"
