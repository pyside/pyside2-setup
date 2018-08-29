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

#ifndef OTHERDERIVED_H
#define OTHERDERIVED_H

#include "libothermacros.h"
#include "abstract.h"
#include "derived.h"
#include "objecttype.h"
#include "complex.h"

class ObjectType;

class LIBOTHER_API OtherDerived : public Abstract
{
public:
    OtherDerived(int id = -1);
    ~OtherDerived() override;
    void pureVirtual() override;
    void* pureVirtualReturningVoidPtr() override;
    void unpureVirtual() override;
    PrintFormat returnAnEnum()  override { return Short; }

    inline void useObjectTypeFromOtherModule(ObjectType*) {}
    inline Event useValueTypeFromOtherModule(const Event& e) { return e; }
    inline Complex useValueTypeFromOtherModule(const Complex& c) { return c; }
    inline void useEnumTypeFromOtherModule(OverloadedFuncEnum) {}

    // factory method
    static Abstract* createObject();

    void hideFunction(HideType*) override {}

protected:
    inline const char* getClassName() { return className(); }
    virtual const char* className()  override { return "OtherDerived"; }

private:
    void pureVirtualPrivate() override;
};
#endif // OTHERDERIVED_H

