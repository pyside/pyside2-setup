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

#ifndef SBKDATE_H
#define SBKDATE_H

#include "libsamplemacros.h"

class LIBSAMPLE_API SbkDate
{
public:
    SbkDate(int d, int m, int y) : m_d(d), m_m(m), m_y(y) {}

    inline int day() const { return m_d; }
    inline int month() const { return m_m; }
    inline int year() const { return m_y; }

private:
    int m_d;
    int m_m;
    int m_y;
};

#endif // SBKDATE_H

