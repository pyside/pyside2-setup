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

#ifndef PEN_H
#define PEN_H

#include "libsamplemacros.h"
#include "samplenamespace.h"

class LIBSAMPLE_API Color
{
public:
    Color();
    Color(SampleNamespace::InValue arg);
    Color(unsigned int arg);

    bool isNull() const;
private:
    bool m_null;
};

class LIBSAMPLE_API Pen
{
public:
    enum { EmptyCtor, EnumCtor, ColorCtor, CopyCtor };

    enum  RenderHints { None = 0, Antialiasing = 0x1, TextAntialiasing = 0x2 };

    Pen();
    Pen(SampleNamespace::Option option);
    Pen(const Color& color);
    Pen(const Pen& pen);

    // PYSIDE-1325, default initializer
    void drawLine(int x1, int y1, int x2, int y2, RenderHints renderHints = {});

    int ctorType();
private:
    int m_ctor;
};

#endif
