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

#ifndef STR_H
#define STR_H
#include <string>

class Str
{
public:
    Str(const Str& s);
    Str(const char* cstr = "");
    ~Str();

    Str arg(const Str& s) const;

    Str& append(const Str& s);
    Str& prepend(const Str& s);

    const char* cstring() const;
    char get_char(int pos) const;
    bool set_char(int pos, char ch);

    void show() const;

    int size() const { return m_str.size(); }

    // nonsense operator just to test reverse operators
    Str operator+(int number) const;
    bool operator==(const Str& other) const;

private:
    void init(const char* cstr);
    std::string m_str;

    friend Str operator+(int number, const Str& str);
};

Str operator+(int number, const Str& str);

#endif // STR_H

