/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef RECT_H
#define RECT_H

#include "libsamplemacros.h"

class LIBSAMPLE_API Rect
{
public:
    Rect()
    {
        m_left = m_top = 0;
        m_right = m_bottom = -1;
    }
    Rect(int left, int top, int right, int bottom)
        : m_left(left), m_top(top), m_right(right), m_bottom(bottom) { }
    ~Rect() {}
    inline int left() const { return m_left; }
    inline int top() const { return m_top; }
    inline int right() const { return m_right; }
    inline int bottom() const { return m_bottom; }
private:
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;
};

class LIBSAMPLE_API RectF
{
public:
    RectF()
    {
        m_left = m_top = 0;
        m_right = m_bottom = -1;
    }
    RectF(int left, int top, int right, int bottom)
        : m_left(left), m_top(top), m_right(right), m_bottom(bottom) { }
    RectF(const Rect& other)
    {
        m_left = other.left();
        m_top = other.top();
        m_right = other.right();
        m_bottom = other.bottom();
    }
    ~RectF() {}
    inline double left() const { return m_left; }
    inline double top() const { return m_top; }
    inline double right() const { return m_right; }
    inline double bottom() const { return m_bottom; }
private:
    double m_left;
    double m_top;
    double m_right;
    double m_bottom;
};

#endif // RECT_H

