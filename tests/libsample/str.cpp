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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Str::Str(const Str& s)
{
    init(s.cstring());
}

Str::Str(const char* cstr)
{
    init(cstr);
}

void
Str::init(const char* cstr)
{
    m_size = strlen(cstr);
    m_str = (char *) malloc(m_size + 1);
    if (m_size == 0)
        m_str[0] = '\0';
    else
        strncpy(m_str, cstr, m_size + 1);
}

Str::~Str()
{
    free(m_str);
}

Str
Str::arg(const Str& s) const
{
    char* var = strstr(m_str, "%VAR");
    if (!var)
        return Str(m_str);

    char* newstr = (char*) malloc (m_size + s.size() - 3);

    int var_pos = 0;
    for (const char* ptr = m_str; ptr != var; ptr++)
        var_pos++;

    strncpy(newstr, m_str, var_pos);
    char* ptr = newstr + var_pos;
    strncpy(ptr, s.cstring(), s.size());
    ptr = ptr + s.size();
    strcpy(ptr, m_str + var_pos + 4);

    Str result(newstr);
    free(newstr);

    return result;
}

const char*
Str::cstring() const
{
    return m_str;
}

void
Str::show() const
{
    printf("%s", m_str);
}

char
Str::get_char(int pos) const
{
    if (pos < 0)
        pos = m_size - pos;
    if (pos < 0 || pos >= m_size)
        return -1;
    return m_str[pos];
}

bool
Str::set_char(int pos, char ch)
{
    if (pos < 0)
        pos = m_size - pos;
    if (pos < 0 || pos >= m_size)
        return false;
    m_str[pos] = ch;
    return true;
}

