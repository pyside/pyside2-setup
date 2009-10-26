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

#include "testconversionruletag.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <QFile>
#include <QTemporaryFile>

void TestConversionRuleTag::testConversionRuleTag()
{
    // temp file used later
    const char conversionData[] = "Hi! I'm a conversion rule.";
    QTemporaryFile file;
    file.open();
    QCOMPARE(file.write(conversionData), qint64(sizeof(conversionData)-1));
    file.close();

    const char cppCode[] = "struct A {};";
    QString xmlCode = "\
    <typesystem package=\"Foo\">\
        <value-type name='A'>\
            <conversion-rule file='"+ file.fileName() +"' />\
        </value-type>\
    </typesystem>";
    TestUtil t(cppCode, xmlCode.toLocal8Bit().data());
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* classA = classes.findClass("A");
    QVERIFY(classA);
    const ComplexTypeEntry* typeEntry = classA->typeEntry();
    QVERIFY(typeEntry->hasConversionRule());
    QCOMPARE(typeEntry->conversionRule(), QString(conversionData));
}

QTEST_APPLESS_MAIN(TestConversionRuleTag)

#include "testconversionruletag.moc"
