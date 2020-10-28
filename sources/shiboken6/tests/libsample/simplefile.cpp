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

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "simplefile.h"

class SimpleFile_p
{
public:
    SimpleFile_p(const char* filename) : m_descriptor(nullptr), m_size(0)
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
    if ((p->m_descriptor = fopen(p->m_filename, "rb")) == nullptr)
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
        p->m_descriptor = nullptr;
    }
}

bool
SimpleFile::exists() const
{
    std::ifstream ifile(p->m_filename);
    return !ifile.fail();
}

bool
SimpleFile::exists(const char* filename)
{
    std::ifstream ifile(filename);
    return !ifile.fail();
}

