/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
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

#include "apiextractorresult.h"
#include "abstractmetalang.h"
#include "abstractmetaenum.h"

#include "typesystem.h"

std::optional<AbstractMetaEnum> ApiExtractorResult::findAbstractMetaEnum(const TypeEntry *typeEntry) const
{
    if (typeEntry && typeEntry->isFlags())
        typeEntry = static_cast<const FlagsTypeEntry *>(typeEntry)->originator();
    const auto it = m_enums.constFind(typeEntry);
    if (it == m_enums.constEnd())
        return {};
    return it.value();
}

AbstractMetaFunctionCList ApiExtractorResult::implicitConversions(const TypeEntry *type) const
{
    if (type->isValue()) {
        if (auto metaClass = AbstractMetaClass::findClass(m_metaClasses, type))
            return metaClass->implicitConversions();
    }
    return {};
}

AbstractMetaFunctionCList ApiExtractorResult::implicitConversions(const AbstractMetaType &metaType) const
{
    return implicitConversions(metaType.typeEntry());
}
