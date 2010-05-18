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

#ifndef MDERIVED_H
#define MDERIVED_H

#include "libsamplemacros.h"

class LIBSAMPLE_API Base1
{
public:
    Base1() : m_value(1) {}
    virtual ~Base1() {}
    virtual int base1Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API Base2
{
public:
    Base2() : m_value(2) {}
    virtual ~Base2() {}
    virtual int base2Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API MDerived1 : public Base1, public Base2
{
public:
    MDerived1();
    virtual ~MDerived1() {}

    virtual int mderived1Method() { return m_value; }
    virtual int base1Method() { return Base1::base1Method() * 10; }
    virtual int base2Method() { return Base2::base2Method() * 10; }

    Base1* castToBase1() { return (Base1*) this; }
    Base2* castToBase2() { return (Base2*) this; }

    static MDerived1* transformFromBase1(Base1 *self);
    static MDerived1* transformFromBase2(Base2 *self);

private:
    int m_value;
};

class LIBSAMPLE_API SonOfMDerived1 : public MDerived1
{
public:
    SonOfMDerived1() : m_value(0) {}
    ~SonOfMDerived1() {}

    MDerived1* castToMDerived1() { return (MDerived1*) this; }

    int sonOfMDerived1Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API Base3
{
public:
    explicit Base3(int val = 3) : m_value(val) {}
    ~Base3() {}
    int base3Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API Base4
{
public:
    Base4() : m_value(4) {}
    ~Base4() {}
    int base4Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API Base5
{
public:
    Base5() : m_value(5) {}
    virtual ~Base5() {}
    virtual int base5Method() { return m_value; }
private:
    int m_value;
};

class LIBSAMPLE_API Base6
{
public:
    Base6() : m_value(6) {}
    virtual ~Base6() {}
    virtual int base6Method() { return m_value; }
private:
    int m_value;
};


class LIBSAMPLE_API MDerived2 : public Base3, public Base4, public Base5, public Base6
{
public:
    MDerived2();
    virtual ~MDerived2() {}

    int base4Method() { return Base3::base3Method() * 10; }
    int mderived2Method() { return m_value; }

    Base3* castToBase3() { return (Base3*) this; }
    Base4* castToBase4() { return (Base4*) this; }
    Base5* castToBase5() { return (Base5*) this; }
    Base6* castToBase6() { return (Base6*) this; }

private:
    int m_value;
};

class LIBSAMPLE_API MDerived3 : public MDerived1, public MDerived2
{
public:
    MDerived3();
    virtual ~MDerived3() {}

    virtual int mderived3Method() { return m_value; }

    MDerived1* castToMDerived1() { return (MDerived1*) this; }
    MDerived2* castToMDerived2() { return (MDerived2*) this; }

    Base3* castToBase3() { return (Base3*) this; }

private:
    int m_value;
};

class LIBSAMPLE_API MDerived4 : public Base3, public Base4
{
public:
    MDerived4();
    ~MDerived4() {}

    int mderived4Method() { return 0; }

    Base3* castToBase3() { return (Base3*) this; }
    Base4* castToBase4() { return (Base4*) this; }
private:
    int m_value;
};

class LIBSAMPLE_API MDerived5 : public Base3, public Base4
{
public:
    MDerived5();
    virtual ~MDerived5() {}

    virtual int mderived5Method() { return 0; }

    Base3* castToBase3() { return (Base3*) this; }
    Base4* castToBase4() { return (Base4*) this; }
};

#endif // MDERIVED_H

