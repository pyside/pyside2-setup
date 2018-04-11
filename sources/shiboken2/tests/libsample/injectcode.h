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

#ifndef INJECTCODE_H
#define INJECTCODE_H

#include "libsamplemacros.h"
#include <utility>
#include <string>

class LIBSAMPLE_API InjectCode
{
public:
    InjectCode();
    virtual ~InjectCode();

    const char* simpleMethod1(int arg0, int arg1);
    const char* simpleMethod2();
    const char* simpleMethod3(int argc, char** argv);

    const char* overloadedMethod(int argc, char** argv);
    const char* overloadedMethod(int arg0, double arg1);
    const char* overloadedMethod(int arg0, bool arg1);

    virtual int arrayMethod(int count, int* values) const;
    inline int callArrayMethod(int count, int *values) const { return arrayMethod(count, values); }
    virtual const char* virtualMethod(int arg);
    int sumArrayAndLength(int* values) const;
private:
    // This attr is just to retain the memory pointed by all return values,
    // So, the memory returned by all methods will be valid until someone call
    // another method of this class.
    std::string m_valueHolder;

    template<typename T>
    const char* toStr(const T& value);
};

#endif // INJECTCODE_H

