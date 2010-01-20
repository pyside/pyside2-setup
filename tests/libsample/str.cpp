/*
 * This file is part of the Shiboken Python Binding Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "str.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace std;

Str::Str(const Str& s)
{
    init(s.cstring());
}

Str::Str(char c)
{
    char str[2] = { c, 0 };
    init(str);
}

Str::Str(const char* cstr)
{
    init(cstr);
}

void
Str::init(const char* cstr)
{
    m_str = cstr;
}

Str::~Str()
{
}

Str
Str::arg(const Str& s) const
{
    size_t idx = m_str.find_first_of("%VAR");
    if (idx == std::string::npos) {
        return *this;
    } else {
        std::string result = m_str;
        result.replace(idx, 4, s.m_str);
        return result.c_str();
    }
}

Str&
Str::append(const Str& s)
{
    m_str += s.m_str;
    return *this;
}

Str&
Str::prepend(const Str& s)
{
    m_str = s.m_str + m_str;
    return *this;
}

const char*
Str::cstring() const
{
    return m_str.c_str();
}

int
Str::toInt(bool* ok, int base) const
{
    bool my_ok;
    int result = 0;
    istringstream conv(m_str);
    switch (base) {
        case 8:
            conv >> std::oct >> result;
            break;
        case 10:
            conv >> std::dec >> result;
            break;
        case 16:
            conv >> std::hex >> result;
            break;
    }
    my_ok = istringstream::eofbit & conv.rdstate();
    if (!my_ok)
        result = 0;
    if (ok)
        *ok = my_ok;
    return result;
}

void
Str::show() const
{
    printf("%s", cstring());
}

char
Str::get_char(int pos) const
{
    return m_str[pos];
}

bool
Str::set_char(int pos, char ch)
{
    m_str[pos] = ch;
    return true;
}

Str Str::operator+(int number) const
{
    ostringstream in;
    in << m_str << number;
    return in.str().c_str();
}

bool Str::operator==(const Str& other) const
{
    return m_str == other.m_str;
}

Str operator+(int number, const Str& str)
{
    ostringstream in;
    in << number << str.m_str;
    return in.str().c_str();
}

