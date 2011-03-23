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

#ifndef OVERLOAD_H
#define OVERLOAD_H

#include "str.h"
#include "size.h"
#include "point.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API Overload
{
public:
    enum FunctionEnum {
        Function0,
        Function1,
        Function2,
        Function3
    };

    enum ParamEnum {
        Param0,
        Param1
    };

    Overload() {}
    virtual ~Overload() {}

    FunctionEnum overloaded();
    FunctionEnum overloaded(Size* size);
    FunctionEnum overloaded(Point* point, ParamEnum param);
    FunctionEnum overloaded(const Point& point);

    inline void differentReturnTypes(ParamEnum param = Param0) {}
    inline int differentReturnTypes(ParamEnum param, int val) { return val; }

    inline int intOverloads(const Point& p, double d) { return 1; }
    inline int intOverloads(int i, int i2) { return 2; }
    inline int intOverloads(int i, int removedArg, double d) { return 3; }

    void singleOverload(Point* x) {}
    Point* singleOverload() {return new Point();}

    // Similar to QImage constructor
    FunctionEnum strBufferOverloads(const Str& arg0, const char* arg1 = 0, bool arg2 = true) { return Function0; }
    FunctionEnum strBufferOverloads(unsigned char* arg0, int arg1) { return Function1; }
    FunctionEnum strBufferOverloads() { return Function2; }
};

class LIBSAMPLE_API Overload2 : public Overload
{
public:
    // test bug#616, public and private method differ only by const
    void doNothingInPublic() const {}
    void doNothingInPublic(int) {}
    virtual void doNothingInPublic3() const {}
    void doNothingInPublic3(int) const {}
protected:
    void doNothingInPublic2() const {}
    void doNothingInPublic2(int) {}
private:
    void doNothingInPublic() {}
    void doNothingInPublic2() {}
    void doNothingInPublic3() {}
};

#endif // OVERLOAD_H

