/*
* This file is part of the Shiboken Python Binding Generator project.
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: PySide team <contact@pyside.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "bucket.h"
#include <iostream>

using namespace std;

Bucket::Bucket() : m_locked(false)
{
}

void Bucket::push(int x)
{
    m_data.push_back(x);
}

int Bucket::pop(void)
{
    int x = 0;

    if (m_data.size() > 0) {
        x = m_data.front();
        m_data.pop_front();
    }

    return x;
}

bool Bucket::empty()
{
    return m_data.empty();
}

void Bucket::lock()
{
    m_locked = true;
    while (m_locked);
}

void Bucket::unlock()
{
    m_locked = false;
}

bool Bucket::virtualBlockerMethod()
{
    lock();
    // The return value was added just for diversity sake.
    return true;
}

