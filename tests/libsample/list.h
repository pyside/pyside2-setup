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

#ifndef LIST_H
#define LIST_H

#include <list>
#include "libsamplemacros.h"
#include "point.h"

class ObjectType;

template<class T>
class List : public std::list<T>
{
};

class IntList : public List<int>
{
public:
    enum CtorEnum {
        NoParamsCtor,
        IntCtor,
        CopyCtor,
        ListOfIntCtor
    };

    inline IntList() : m_ctorUsed(NoParamsCtor) {}
    inline explicit IntList(int val) : m_ctorUsed(IntCtor) { push_back(val); }
    inline IntList(const IntList& lst) : List<int>(lst), m_ctorUsed(CopyCtor) {}
    inline IntList(const List<int>& lst) : List<int>(lst), m_ctorUsed(ListOfIntCtor) {}

    inline void append(int v) { insert(end(), v); }
    CtorEnum constructorUsed() { return m_ctorUsed; }
private:
    CtorEnum m_ctorUsed;
};

class PointValueList : public List<Point>
{
public:
    enum CtorEnum {
        NoParamsCtor,
        PointCtor,
        CopyCtor,
        ListOfPointValuesCtor
    };

    inline PointValueList() : m_ctorUsed(NoParamsCtor) {}
    inline explicit PointValueList(Point val) : m_ctorUsed(PointCtor) { push_back(val); }
    inline PointValueList(const PointValueList& lst) : List<Point>(lst), m_ctorUsed(CopyCtor) {}
    inline PointValueList(const List<Point>& lst) : List<Point>(lst), m_ctorUsed(ListOfPointValuesCtor) {}

    inline void append(Point v) { insert(end(), v); }
    CtorEnum constructorUsed() { return m_ctorUsed; }
private:
    CtorEnum m_ctorUsed;
};

class ObjectTypePtrList : public List<ObjectType*>
{
public:
    enum CtorEnum {
        NoParamsCtor,
        ObjectTypeCtor,
        CopyCtor,
        ListOfObjectTypePtrCtor
    };

    inline ObjectTypePtrList() : m_ctorUsed(NoParamsCtor) {}
    inline explicit ObjectTypePtrList(ObjectType* val) : m_ctorUsed(ObjectTypeCtor) { push_back(val); }
    inline ObjectTypePtrList(const ObjectTypePtrList& lst) : List<ObjectType*>(lst), m_ctorUsed(CopyCtor) {}
    inline ObjectTypePtrList(const List<ObjectType*>& lst) : List<ObjectType*>(lst), m_ctorUsed(ListOfObjectTypePtrCtor) {}

    inline void append(ObjectType* v) { insert(end(), v); }
    CtorEnum constructorUsed() { return m_ctorUsed; }
private:
    CtorEnum m_ctorUsed;
};

#endif // LIST_H
