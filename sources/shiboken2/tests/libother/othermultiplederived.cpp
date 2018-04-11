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

#include "othermultiplederived.h"

VirtualMethods OtherMultipleDerived::returnUselessClass()
{
    return VirtualMethods();
}

Base1* OtherMultipleDerived::createObject(const std::string& objName)
{
    if (objName == "Base1")
        return new Base1;
    else if (objName == "MDerived1")
        return new MDerived1;
    else if (objName == "SonOfMDerived1")
        return new SonOfMDerived1;
    else if (objName == "MDerived3")
        return new MDerived3;
    else if (objName == "OtherMultipleDerived")
        return new OtherMultipleDerived;
    return 0;
}

