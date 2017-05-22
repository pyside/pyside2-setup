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

#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <list>

#include "libsamplemacros.h"

class Intersection;

class LIBSAMPLE_API Filter
{
};

class LIBSAMPLE_API Data : public Filter
{

public:
    enum Field {
        Name,
        Album,
        Year
    };

    Data(Field field, std::string value);

    Field field() const { return m_field; }
    std::string value() const { return m_value; }

private:
    Field m_field;
    std::string m_value;
};

class LIBSAMPLE_API Union : public Filter
{
public:

    Union(const Data&);
    Union(const Intersection&);
    Union() {};
    Union(const Union&);

    std::list<Filter> filters() const { return m_filters; }
    void addFilter(const Filter& data) { m_filters.push_back(data); }

private:
    std::list<Filter> m_filters;
};

class LIBSAMPLE_API Intersection : public Filter
{
public:

    Intersection(const Data&);
    Intersection(const Union&);
    Intersection() {};
    Intersection(const Intersection&);

    std::list<Filter> filters() const { return m_filters; }
    void addFilter(const Filter& data) { m_filters.push_back(data); }

private:
    std::list<Filter> m_filters;
};

LIBSAMPLE_API Intersection operator&(const Intersection& a, const Intersection& b);

#endif // FILTER_H


