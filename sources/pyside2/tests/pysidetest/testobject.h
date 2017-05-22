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

#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>
#include <QApplication>
#include <QMetaType>
#include <QVariant>
#ifdef pysidetest_EXPORTS
#define PYSIDE_EXPORTS 1
#endif
#include "pysidemacros.h"

class IntValue
{
public:

    IntValue(int val): value(val){};
    IntValue() : value(0) {};
    int value;
};

typedef IntValue TypedefValue;

class PYSIDE_API TestObject : public QObject
{
    Q_OBJECT
public:
    static void createApp() { int argc=0; new QApplication(argc, 0); };
    static int checkType(const QVariant& var) { return (int)var.type(); }

    TestObject(int idValue, QObject* parent = 0) : QObject(parent), m_idValue(idValue) {}
    int idValue() const { return m_idValue; }
    static int staticMethodDouble(int value) { return value * 2; }
    void addChild(QObject* c) { m_children.append(c); emit childrenChanged(m_children); }

    void emitIdValueSignal();
    void emitStaticMethodDoubleSignal();

    void emitSignalWithDefaultValue_void();
    void emitSignalWithDefaultValue_bool();

    void emitSignalWithTypedefValue(int value);

signals:
    void idValue(int newValue);
    void justASignal();
    void staticMethodDouble();
    void childrenChanged(const QList<QObject*>&);
    void signalWithDefaultValue(bool value = false);
    void signalWithTypedefValue(TypedefValue value);

private:
    int m_idValue;
    QList<QObject*> m_children;
};


typedef int PySideInt;


namespace PySideCPP {

class PYSIDE_API TestObjectWithNamespace :  public QObject
{
    Q_OBJECT
public:
    TestObjectWithNamespace(QObject* parent) : QObject(parent) {}
    QString name() { return "TestObjectWithNamespace"; }

    void callSignal(TestObjectWithNamespace* obj) { emitSignal(obj); }
    void callSignalWithNamespace(TestObjectWithNamespace* obj) { emitSignalWithNamespace(obj); }
    void callSignalWithTypedef(int val) { emitSignalWithTypedef(val); }

signals:
    void emitSignal(TestObjectWithNamespace* obj);
    void emitSignalWithNamespace(PySideCPP::TestObjectWithNamespace* obj);
    void emitSignalWithTypedef(PySideInt val);
};


} // Namespace PySideCPP

namespace PySideCPP2 {

enum Enum1 { Option1 = 1, Option2 = 2 };


typedef long PySideLong;

class PYSIDE_API TestObjectWithoutNamespace :  public QObject
{
    Q_OBJECT
public:
    enum Enum2 { Option3 = 3, Option4 =  4};
    TestObjectWithoutNamespace(QObject* parent) : QObject(parent) {}
    QString name() { return "TestObjectWithoutNamespace"; }

    void callSignal(TestObjectWithoutNamespace* obj) { emitSignal(obj); }
    void callSignalWithNamespace(TestObjectWithoutNamespace* obj) { emitSignalWithNamespace(obj); }
    void callSignalWithTypedef(long val) { emitSignalWithTypedef(val); }

signals:
    void emitSignal(TestObjectWithoutNamespace* obj);
    void emitSignalWithNamespace(PySideCPP2::TestObjectWithoutNamespace* obj);
    void emitSignalWithTypedef(PySideLong val);
};


} // Namespace PySideCPP2

#endif // TESTOBJECT_H

