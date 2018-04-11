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

#ifndef SAMPLE_H
#define SAMPLE_H

#include "libsamplemacros.h"

// namespace with the same name of the current package to try to mess up with the generator
namespace sample
{
    // to increase the mess we add a class with the same name of the package/namespace
    class LIBSAMPLE_API sample
    {
    public:
        sample(int value = 0);
        int value() const;
    private:
        int m_value;
    };

    // shiboken must not generate richcompare for namespace sample
    LIBSAMPLE_API bool operator==(const sample&s1, const sample&s2);
}

#endif
