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

#ifndef REFERENCE_H
#define REFERENCE_H

#include "libsamplemacros.h"

class LIBSAMPLE_API Reference
{
public:
    explicit Reference(int objId = -1)
            : m_objId(objId) {}
    virtual ~Reference() {}

    inline int objId() { return m_objId; }
    inline void setObjId(int objId) { m_objId = objId; }

    inline static int usesReference(Reference& r) { return r.m_objId; }
    inline static int usesConstReference(const Reference& r) { return r.m_objId; }

    virtual int usesReferenceVirtual(Reference& r, int inc);
    virtual int usesConstReferenceVirtual(const Reference& r, int inc);

    int callUsesReferenceVirtual(Reference& r, int inc);
    int callUsesConstReferenceVirtual(const Reference& r, int inc);

    virtual void alterReferenceIdVirtual(Reference& r);
    void callAlterReferenceIdVirtual(Reference& r);

    void show() const;

    inline static int multiplier() { return 10; }

    virtual Reference& returnMyFirstArg(Reference& ref) { return ref; }
    virtual Reference& returnMySecondArg(int a, Reference& ref) { return ref; }

    // nonsense operator to test if Shiboken is ignoring dereference operators.
    int operator*() { return m_objId; }
private:
    int m_objId;
};

class LIBSAMPLE_API ObjTypeReference
{
public:
    ObjTypeReference() {}
    ObjTypeReference(const ObjTypeReference&) {}
    virtual ~ObjTypeReference();
    virtual ObjTypeReference& returnMyFirstArg(ObjTypeReference& ref) { return ref; }
    virtual ObjTypeReference& returnMySecondArg(int a, ObjTypeReference& ref) { return ref; }
    virtual ObjTypeReference& justAPureVirtualFunc(ObjTypeReference& ref) = 0;
};

#endif // REFERENCE_H

