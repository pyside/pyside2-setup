/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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

#include "testcontainer.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestContainer::testContainerType()
{
    const char* cppCode ="\
    namespace std {\n\
    template<class T>\n\
    class list {\n\
        T get(int x) { return 0; }\n\
    };\n\
    }\n\
    class A : public std::list<int> {\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='std' generate='no' />\n\
        <container-type name='std::list' type='list' />\n\
        <object-type name='A'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 2);
    //search for class A
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QVERIFY(classA->typeEntry()->baseContainerType());
    QCOMPARE(reinterpret_cast<const ContainerTypeEntry*>(classA->typeEntry()->baseContainerType())->type(), ContainerTypeEntry::ListContainer);
}

void TestContainer::testListOfValueType()
{
    const char* cppCode ="\
    namespace std {\n\
    template<class T>\n\
    class list {\n\
        T get(int x) { return 0; }\n\
    };\n\
    }\n\
    class ValueType {};\n\
    class A : public std::list<ValueType> {\n\
    };\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <namespace-type name='std' generate='no'/>\n\
        <container-type name='std::list' type='list'/>\n\
        <value-type name='ValueType'/>\n\
        <value-type name='A'/>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, true));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 3);

    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->templateBaseClassInstantiations().count(), 1);
    const AbstractMetaType* templateInstanceType = classA->templateBaseClassInstantiations().first();
    QVERIFY(templateInstanceType);

    QCOMPARE(templateInstanceType->indirections(), 0);
    QVERIFY(!templateInstanceType->typeEntry()->isObject());
    QVERIFY(templateInstanceType->typeEntry()->isValue());
    QCOMPARE(templateInstanceType->referenceType(), NoReference);
    QVERIFY(!templateInstanceType->isObject());
    QVERIFY(!templateInstanceType->isValuePointer());
    QVERIFY(templateInstanceType->isValue());
}

QTEST_APPLESS_MAIN(TestContainer)

