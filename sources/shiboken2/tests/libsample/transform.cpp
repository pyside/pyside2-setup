/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2013 Kitware, Inc.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#include "transform.h"

#ifdef _WIN32
#include <math.h>
#include <float.h>
static inline bool isfinite(double a) { return _finite(a); }
#else
#include <cmath>
#endif

using namespace std;

Point applyHomogeneousTransform(
    const Point& in,
    double m11, double m12, double m13,
    double m21, double m22, double m23,
    double m31, double m32, double m33,
    bool* okay)
{
    double x = m11 * in.x() + m12 * in.y() + m13;
    double y = m21 * in.x() + m22 * in.y() + m23;
    double w = m31 * in.x() + m32 * in.y() + m33;

    if (isfinite(w) && fabs(w) > 1e-10)
    {
        if (okay)
            *okay = true;
        return Point(x / w, y / w);
    }
    else
    {
        if (okay)
            *okay = false;
        return Point();
    }
}
