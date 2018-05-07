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

#ifndef OVERLOAD_H
#define OVERLOAD_H

#include "echo.h"
#include "str.h"
#include "size.h"
#include "point.h"
#include "pointf.h"
#include "polygon.h"
#include "rect.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API Overload
{
public:
    enum FunctionEnum {
        Function0,
        Function1,
        Function2,
        Function3,
        Function4,
        Function5,
        Function6
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

    inline FunctionEnum intDoubleOverloads(int a0, int a1) const { return Function0; }
    inline FunctionEnum intDoubleOverloads(double a0, double a1) const { return Function1; }

    void singleOverload(Point* x) {}
    Point* singleOverload() {return new Point();}

    // Similar to QImage::trueMatrix(QMatrix,int,int) and QImage::trueMatrix(QTransform,int,int)
    FunctionEnum wrapperIntIntOverloads(const Point& arg0, int arg1, int arg2) { return Function0; }
    FunctionEnum wrapperIntIntOverloads(const Polygon& arg0, int arg1, int arg2) { return Function1; }

    // Similar to QImage constructor
    FunctionEnum strBufferOverloads(const Str& arg0, const char* arg1 = 0, bool arg2 = true) { return Function0; }
    FunctionEnum strBufferOverloads(unsigned char* arg0, int arg1) { return Function1; }
    FunctionEnum strBufferOverloads() { return Function2; }

    // Similar to QPainter::drawText(...)
    FunctionEnum drawText(const Point& a0, const Str& a1) { return Function0; }
    FunctionEnum drawText(const PointF& a0, const Str& a1) { return Function1; }
    FunctionEnum drawText(const Rect& a0, int a1, const Str& a2) { return Function2; }
    FunctionEnum drawText(const RectF& a0, int a1, const Str& a2) { return Function3; }
    FunctionEnum drawText(const RectF& a0, const Str& a1, const Echo& a2 = Echo()) { return Function4; }
    FunctionEnum drawText(int a0, int a1, const Str& a2) { return Function5; }
    FunctionEnum drawText(int a0, int a1, int a2, int a3, int a4, const Str& a5) { return Function6; }

    // A variant of the one similar to QPainter::drawText(...)
    FunctionEnum drawText2(const Point& a0, const Str& a1) { return Function0; }
    FunctionEnum drawText2(const PointF& a0, const Str& a1) { return Function1; }
    FunctionEnum drawText2(const Rect& a0, int a1, const Str& a2) { return Function2; }
    FunctionEnum drawText2(const RectF& a0, int a1, const Str& a2) { return Function3; }
    FunctionEnum drawText2(const RectF& a0, const Str& a1, const Echo& a2 = Echo()) { return Function4; }
    FunctionEnum drawText2(int a0, int a1, const Str& a2) { return Function5; }
    FunctionEnum drawText2(int a0, int a1, int a2, int a3 = 0, int a4 = 0, const Str& a5 = Str()) { return Function6; }

    // A simpler variant of the one similar to QPainter::drawText(...)
    FunctionEnum drawText3(const Str& a0, const Str& a1, const Str& a2) { return Function0; }
    FunctionEnum drawText3(int a0, int a1, int a2, int a3, int a4) { return Function1; }

    // Another simpler variant of the one similar to QPainter::drawText(...)
    FunctionEnum drawText4(int a0, int a1, int a2) { return Function0; }
    FunctionEnum drawText4(int a0, int a1, int a2, int a3, int a4) { return Function1; }

    FunctionEnum acceptSequence() { return Function0; }
    FunctionEnum acceptSequence(int a0, int a1) { return Function1; }
    FunctionEnum acceptSequence(const Str& a0, ParamEnum a1 = Param0) { return Function2; }
    FunctionEnum acceptSequence(const Size& a0) { return Function3; }
    // The type must be changed to PySequence.
    FunctionEnum acceptSequence(const char* const a0[]) { return Function4; }
    FunctionEnum acceptSequence(void* a0) { return Function5; }
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

