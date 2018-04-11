/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#ifndef TESTABSTRACTMETACLASS_H
#define TESTABSTRACTMETACLASS_H

#include <QObject>

class AbstractMetaBuilder;

class TestAbstractMetaClass : public QObject
{
    Q_OBJECT
private slots:
    void testClassName();
    void testClassNameUnderNamespace();
    void testVirtualMethods();
    void testDefaultValues();
    void testModifiedDefaultValues();
    void testInnerClassOfAPolymorphicOne();
    void testForwardDeclaredInnerClass();
    void testSpecialFunctions();
    void testClassDefaultConstructors();
    void testClassInheritedDefaultConstructors();
    void testAbstractClassDefaultConstructors();
    void testObjectTypesMustNotHaveCopyConstructors();
    void testIsPolymorphic();
};

#endif // TESTABSTRACTMETACLASS_H
