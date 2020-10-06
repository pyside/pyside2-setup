/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#pragma once

#include "pysidetest_macros.h"

#include <QtCore/QObject>

namespace FlagsNamespace
{

enum Option {
    NoOptions = 0x0,
    ShowTabs = 0x1,
    ShowAll = 0x2,
    SqueezeBlank = 0x4
};
Q_DECLARE_FLAGS(Options, Option)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

class PYSIDETEST_API ClassForEnum : public QObject
{
    Q_OBJECT
public:
    ClassForEnum(FlagsNamespace::Options opt = FlagsNamespace::Option::NoOptions);
    virtual ~ClassForEnum();
};

} // namespace FlagsNamespace
