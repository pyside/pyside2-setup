/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef SMART_OBJ_H
#define SMART_OBJ_H

#include "libsmartmacros.h"
#include "smart_sharedptr.h"

#include <vector>

class Integer;
class Obj;
namespace Smart { class Integer2; }

// Couldn't name it Object because it caused some namespace clashes.
class LIB_SMART_API Obj {
public:
    Obj();
    virtual ~Obj();

    void printObj();
    Integer takeInteger(Integer val);
    SharedPtr<Obj> giveSharedPtrToObj();
    std::vector<SharedPtr<Obj> > giveSharedPtrToObjList(int size);
    virtual SharedPtr<Integer> giveSharedPtrToInteger(); // virtual for PYSIDE-1188
    SharedPtr<const Integer> giveSharedPtrToConstInteger();
    int takeSharedPtrToConstInteger(SharedPtr<const Integer> pInt);
    SharedPtr<Smart::Integer2> giveSharedPtrToInteger2();
    int takeSharedPtrToObj(SharedPtr<Obj> pObj);
    int takeSharedPtrToInteger(SharedPtr<Integer> pInt);

    int m_integer;  // public for testing member field access.
    Integer *m_internalInteger;
};

#endif // SMART_OBJ_H
