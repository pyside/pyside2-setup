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

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "simplefile.h"

class SimpleFile_p
{
public:
    SimpleFile_p(const char* filename) : m_descriptor(0), m_size(0)
    {
        m_filename = strdup(filename);
    }

    ~SimpleFile_p()
    {
        free(m_filename);
    }

    char* m_filename;
    FILE* m_descriptor;
    long m_size;
};

SimpleFile::SimpleFile(const char* filename)
{
    p = new SimpleFile_p(filename);
}

SimpleFile::~SimpleFile()
{
    close();
    delete p;
}

const char* SimpleFile::filename()
{
    return p->m_filename;
}

long SimpleFile::size()
{
    return p->m_size;
}

bool
SimpleFile::open()
{
    if ((p->m_descriptor = fopen(p->m_filename, "rb")) == 0)
        return false;

    fseek(p->m_descriptor, 0, SEEK_END);
    p->m_size = ftell(p->m_descriptor);
    rewind(p->m_descriptor);

    return true;
}

void
SimpleFile::close()
{
    if (p->m_descriptor) {
        fclose(p->m_descriptor);
        p->m_descriptor = 0;
    }
}

bool
SimpleFile::exists() const
{
    std::ifstream ifile(p->m_filename);
    return ifile;
}

bool
SimpleFile::exists(const char* filename)
{
    std::ifstream ifile(filename);
    return ifile;
}

