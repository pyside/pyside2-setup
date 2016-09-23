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

#ifndef OBJECTTYPEOPERATORS_H
#define OBJECTTYPEOPERATORS_H

#include "libsamplemacros.h"
#include <string>

class LIBSAMPLE_API ObjectTypeOperators
{
public:
    explicit ObjectTypeOperators(const std::string key);
    virtual ~ObjectTypeOperators() {}

    bool operator==(const ObjectTypeOperators& other) const;
    const ObjectTypeOperators& operator<(const ObjectTypeOperators& other) const;

    // chaos!
    virtual void operator>(const ObjectTypeOperators&) { m_key.append("operator>"); }

    std::string key() const { return m_key; }

private:
    std::string m_key;

    ObjectTypeOperators(ObjectTypeOperators&);
    ObjectTypeOperators& operator=(ObjectTypeOperators&);
};

LIBSAMPLE_API bool operator==(const ObjectTypeOperators* obj, const std::string& str);
LIBSAMPLE_API bool operator==(const std::string& str, const ObjectTypeOperators* obj);
LIBSAMPLE_API std::string operator+(const ObjectTypeOperators* obj, const std::string& str);
LIBSAMPLE_API std::string operator+(const std::string& str, const ObjectTypeOperators* obj);

#endif // OBJECTTYPEOPERATORS_H
