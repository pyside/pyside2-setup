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

#ifndef POINTF_H
#define POINTF_H

#include "point.h"
#include <utility>

#include "libsamplemacros.h"

class LIBSAMPLE_API PointF
{
public:
    PointF(const Point& point);
    PointF(double x = 0.0, double y = 0.0);
    ~PointF() {}

    inline double x() const { return m_x; }
    inline double y() const { return m_y; }

    inline void setX(double x) { m_x = x; }
    inline void setY(double y) { m_y = y; }

    // This method could simply return the midpoint,
    // but the interesting part of the test is to set the
    // result in the pointer argument.
    void midpoint(const PointF& other, PointF* midpoint) const;

    // The != operator is not implemented for the purpose of testing
    // for the absence of the __ne__ method in the Python binding.
    bool operator==(const PointF& other);

    PointF operator+(const PointF& other);
    PointF operator-(const PointF& other);

    friend LIBSAMPLE_API PointF operator*(const PointF& pt, double mult);
    friend LIBSAMPLE_API PointF operator*(const PointF& pt, int mult);
    friend LIBSAMPLE_API PointF operator*(double mult, const PointF& pt);
    friend LIBSAMPLE_API PointF operator*(int mult, const PointF& pt);
    friend LIBSAMPLE_API PointF operator-(const PointF& pt);
    friend LIBSAMPLE_API bool operator!(const PointF& pt);

    PointF& operator+=(PointF &other);
    PointF& operator-=(PointF &other);

    void show();

private:
    double m_x;
    double m_y;
};

LIBSAMPLE_API PointF operator*(const PointF& pt, double mult);
LIBSAMPLE_API PointF operator*(const PointF& pt, int mult);
LIBSAMPLE_API PointF operator*(double mult, const PointF& pt);
LIBSAMPLE_API PointF operator*(int mult, const PointF& pt);
LIBSAMPLE_API PointF operator-(const PointF& pt);
LIBSAMPLE_API bool operator!(const PointF& pt);

LIBSAMPLE_API PointF operator*(const PointF& pt, double multiplier);

#endif // POINTF_H
