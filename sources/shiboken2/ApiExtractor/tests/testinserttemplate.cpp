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

#include "testinserttemplate.h"
#include <QtTest/QTest>
#include "testutil.h"
#include <abstractmetalang.h>
#include <typesystem.h>

void TestInsertTemplate::testInsertTemplateOnClassInjectCode()
{
    const char* cppCode ="struct A{};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <template name='code_template'>\n\
        code template content\n\
        </template>\n\
        <value-type name='A'>\n\
            <inject-code class='native'>\n\
                <insert-template name='code_template'/>\n\
            </inject-code>\n\
        </value-type>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.count(), 1);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, QLatin1String("A"));
    QVERIFY(classA);
    QCOMPARE(classA->typeEntry()->codeSnips().count(), 1);
    QString code = classA->typeEntry()->codeSnips().first().code();
    QVERIFY(code.contains(QLatin1String("code template content")));
}

void TestInsertTemplate::testInsertTemplateOnModuleInjectCode()
{
    const char* cppCode ="";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <template name='code_template'>\n\
        code template content\n\
        </template>\n\
        <inject-code class='native'>\n\
            <insert-template name='code_template'/>\n\
        </inject-code>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode, false));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    QVERIFY(classes.isEmpty());

    const TypeSystemTypeEntry *module = TypeDatabase::instance()->defaultTypeSystemType();
    QVERIFY(module);
    QCOMPARE(module->name(), QLatin1String("Foo"));
    QVERIFY(module);
    QCOMPARE(module->codeSnips().count(), 1);
    QString code = module->codeSnips().first().code().trimmed();
    QVERIFY(code.contains(QLatin1String("code template content")));
}

QTEST_APPLESS_MAIN(TestInsertTemplate)
