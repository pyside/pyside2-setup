/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef TYPEDATABASE_TYPEDEFS_H
#define TYPEDATABASE_TYPEDEFS_H

#include <QtCore/QMultiMap>
#include <QtCore/QString>
#include <QtCore/QVector>

class ContainerTypeEntry;
class NamespaceTypeEntry;
class PrimitiveTypeEntry;
class TemplateEntry;
class TypeEntry;
class TypedefEntry;

using TypeEntryList = QVector<TypeEntry *>;
using TemplateEntryMap =QMap<QString, TemplateEntry *>;

template <class Key, class Value>
struct QMultiMapConstIteratorRange // A range of iterator for a range-based for loop
{
    using ConstIterator = typename QMultiMap<Key, Value>::const_iterator;

    ConstIterator begin() const { return m_begin; }
    ConstIterator end() const { return m_end; }

    ConstIterator m_begin;
    ConstIterator m_end;
};

using TypeEntryMultiMap = QMultiMap<QString, TypeEntry *>;
using TypeEntryMultiMapConstIteratorRange = QMultiMapConstIteratorRange<QString, TypeEntry *>;

using TypeEntryMap = QMap<QString, TypeEntry *>;
using TypedefEntryMap = QMap<QString, TypedefEntry *>;

using ContainerTypeEntryList = QVector<const ContainerTypeEntry *>;
using NamespaceTypeEntryList = QVector<NamespaceTypeEntry *>;
using PrimitiveTypeEntryList = QVector<const PrimitiveTypeEntry *>;

#endif // TYPEDATABASE_TYPEDEFS_H
