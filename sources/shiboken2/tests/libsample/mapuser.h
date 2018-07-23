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

#ifndef MAPUSER_H
#define MAPUSER_H

#include <map>
#include <list>
#include <utility>
#include <string>
#include "complex.h"
#include "bytearray.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API MapUser
{
public:
    MapUser() {}
    virtual ~MapUser() {}

    virtual std::map<std::string, std::pair<Complex, int> > createMap();
    std::map<std::string, std::pair<Complex, int> > callCreateMap();

    void showMap(std::map<std::string, int> mapping);

    inline void setMap(std::map<std::string, std::list<int> > map) { m_map = map; }
    inline std::map<std::string, std::list<int> > getMap() { return m_map; }

    // Compile test
    static void pointerToMap(std::map<std::string, std::string>* arg) {}
    static void referenceToMap(std::map<std::string, std::string>& arg) {}

    inline const std::map<int, ByteArray>& passMapIntValueType(const std::map<int, ByteArray>& arg) { return arg; }

    std::map<int, std::list<std::list<double> > > foo() const;

private:
    std::map<std::string, std::list<int> > m_map;
};

#endif // MAPUSER_H
