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

#include "testaddfunction.h"
#include <QtTest/QTest>
#include "testutil.h"


void TestAddFunction::testParsingFuncNameAndConstness()
{
    const char sig1[] = "func(type1, const type2, const type3* const)";
    AddedFunction f1(sig1, "void");
    QCOMPARE(f1.name(), QString("func"));
    AddedFunction::TypeInfo retval = f1.returnType();
    QCOMPARE(retval.name, QString("void"));
    QCOMPARE(retval.indirections, 0);
    QCOMPARE(retval.isConst, false);
    QCOMPARE(retval.isRef, false);

    const char sig2[] = "    _fu__nc_       (  type1, const type2, const Abc<int& , C<char*> *   >  * *, const type3* const    )   const ";
    AddedFunction f2(sig2, "const Abc<int& , C<char*> *   >  * *");
    QCOMPARE(f2.name(), QString("_fu__nc_"));
    retval = f2.returnType();
    QCOMPARE(retval.name, QString("Abc<int& , C<char*> *   >"));
    QCOMPARE(retval.indirections, 2);
    QCOMPARE(retval.isConst, true);
    QCOMPARE(retval.isRef, false);
    QList< AddedFunction::TypeInfo > args = f2.arguments();
    QCOMPARE(args.count(), 4);
    retval = args[2];
    QCOMPARE(retval.name, QString("Abc<int& , C<char*> *   >"));
    QCOMPARE(retval.indirections, 2);
    QCOMPARE(retval.isConst, true);
    QCOMPARE(retval.isRef, false);


}

void TestAddFunction::testAddFunction()
{
#if 0
    const char cppCode[] = "struct A { void a(); };";
    const char xmlCode[] = "\
    <typesystem package=\"Foo\"> \
        <value-type name='A'> \
            <add-function signature='b(int, float)' return-type='int' acess='protected'>\
            </add-function>\
        </value-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass("A");
    QVERIFY(classA);
    QCOMPARE(classA->functions().count(), 2); // default ctor and the added function
#endif
}

QTEST_APPLESS_MAIN(TestAddFunction)

#include "testaddfunction.moc"
