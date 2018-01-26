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

#ifndef DERIVED_H
#define DERIVED_H

#include "libsamplemacros.h"
#include "abstract.h"

enum OverloadedFuncEnum {
    OverloadedFunc_ii,
    OverloadedFunc_d
};

class LIBSAMPLE_API Derived : public Abstract
{
public:
    enum OtherOverloadedFuncEnum {
        OtherOverloadedFunc_iibd,
        OtherOverloadedFunc_id
    };

    class SomeInnerClass {
    public:
        void uselessMethod() {}
        SomeInnerClass operator+(const SomeInnerClass& other) { return other; }
        bool operator==(const SomeInnerClass& other) { return true; }
    };

    Derived(int id = -1);
    ~Derived() override;
    void pureVirtual() override;
    void* pureVirtualReturningVoidPtr() override;
    void unpureVirtual() override;

    PrintFormat returnAnEnum()  override { return Short; }
    Type type() const override { return TpDerived; }

    // factory method
    static Abstract* createObject();

    // single argument
    bool singleArgument(bool b);

    // method with default value
    double defaultValue(int n = 0);

    // overloads
    OverloadedFuncEnum overloaded(int i = 0, int d = 0);
    OverloadedFuncEnum overloaded(double n);

    // more overloads
    OtherOverloadedFuncEnum otherOverloaded(int a, int b, bool c, double d);
    OtherOverloadedFuncEnum otherOverloaded(int a, double b);

    inline SomeInnerClass returnMyParameter(const SomeInnerClass& s) { return s; }

    static Abstract* triggerImpossibleTypeDiscovery();
    static Abstract* triggerAnotherImpossibleTypeDiscovery();

    void hideFunction(HideType*) override {}
protected:
    const char* getClassName() { return className(); }
    virtual const char* className() override { return "Derived"; }

private:
    void pureVirtualPrivate() override;
};
#endif // DERIVED_H

