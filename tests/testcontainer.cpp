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

#include "testcontainer.h"
#include <QtTest/QTest>
#include "testutil.h"

void TestContainer::testContainerType()
{
    const char* cppCode ="\
    namespace std {\
    template<class T>\
    class list { \
        T get(int x) { return 0; }\
    };\
    }\
    class A : public std::list<int> {\
    };\
    ";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name='std' generate='no' /> \
        <container-type name='std::list' type='list' /> \
        <object-type name='A'/> \
    </typesystem>";

    TestUtil t(cppCode, xmlCode, true);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes.count(), 2);
    //search for class A
    AbstractMetaClass* classA = classes.findClass("A");
    QVERIFY(classA);
    QVERIFY(classA->typeEntry()->baseContainerType());
    QCOMPARE(reinterpret_cast<const ContainerTypeEntry*>(classA->typeEntry()->baseContainerType())->type(), ContainerTypeEntry::ListContainer);
}

QTEST_APPLESS_MAIN(TestContainer)

#include "testcontainer.moc"
