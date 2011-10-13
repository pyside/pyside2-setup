/*
 * This file is part of the Shiboken Python Binding Generator project.
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef HANDLE_H
#define HANDLE_H

#include "libsamplemacros.h"

class OBJ
{
};

typedef OBJ* HANDLE;

class LIBSAMPLE_API HandleHolder
{
public:
    explicit HandleHolder(HANDLE ptr = 0) : m_handle(ptr) {}
    void set(HANDLE ptr) { m_handle = m_handle; }
    HANDLE get() { return m_handle; }

    static HANDLE createHandle()
    {
        return (HANDLE) new OBJ;
    }

    bool compare(HandleHolder* other)
    {
        return other->m_handle == m_handle;
    }
private:
    HANDLE m_handle;
};

struct PrimitiveStruct {};
typedef struct PrimitiveStruct* PrimitiveStructPtr;
struct LIBSAMPLE_API PrimitiveStructPointerHolder
{
    PrimitiveStructPtr primitiveStructPtr;
};

#endif // HANDLE_H
