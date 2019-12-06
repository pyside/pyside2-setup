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

#ifndef SMART_REGISTRY_H
#define SMART_REGISTRY_H

#include <vector>

#include "libsmartmacros.h"

class Obj;
class Integer;

// Used to track which C++ objects are alive.
class LIB_SMART_API Registry {
public:
    static Registry *getInstance();
    ~Registry();

    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry(Registry &&) = delete;
    Registry &operator=(Registry &&) = delete;

    void add(Obj *p);
    void add(Integer *p);
    void remove(Obj *p);
    void remove(Integer *p);
    int countObjects() const;
    int countIntegers() const;
    bool shouldPrint() const;
    void setShouldPrint(bool flag);

protected:
    Registry();

private:
    std::vector<Obj *> m_objects;
    std::vector<Integer *> m_integers;
    bool m_printStuff = false;
};

#endif // SMART_REGISTRY_H
