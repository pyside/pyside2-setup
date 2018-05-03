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

#ifndef STRLIST_H
#define STRLIST_H

#include <list>
#include "str.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API StrList : public std::list<Str>
{
public:
    enum CtorEnum {
        NoParamsCtor,
        StrCtor,
        CopyCtor,
        ListOfStrCtor
    };

    inline StrList() : m_ctorUsed(NoParamsCtor) {}
    inline explicit StrList(const Str& str) : m_ctorUsed(StrCtor) { push_back(str); }
    inline StrList(const StrList& lst) : std::list<Str>(lst), m_ctorUsed(CopyCtor) {}
    inline StrList(const std::list<Str>& lst) : std::list<Str>(lst), m_ctorUsed(ListOfStrCtor) {}

    inline void append(Str str) { push_back(str); }
    Str join(const Str& sep) const;

    bool operator==(const std::list<Str>& other) const;
    inline bool operator!=(const std::list<Str>& other) const { return !(*this == other); }

    CtorEnum constructorUsed() { return m_ctorUsed; }
private:
    CtorEnum m_ctorUsed;
};

typedef StrList PStrList;

#endif // STRLIST_H
