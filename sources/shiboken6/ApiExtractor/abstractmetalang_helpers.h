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

#ifndef ABSTRACTMETALANG_HELPERS_H
#define ABSTRACTMETALANG_HELPERS_H

template <class MetaClass>
MetaClass *findByName(QVector<MetaClass *> haystack, QStringView needle)
{
    for (MetaClass *c : haystack) {
        if (c->name() == needle)
            return c;
    }
    return nullptr;
}

// Helper for recursing the base classes of an AbstractMetaClass.
// Returns the class for which the predicate is true.
template <class Predicate>
const AbstractMetaClass *recurseClassHierarchy(const AbstractMetaClass *klass,
                                               Predicate pred)
{
    if (pred(klass))
        return klass;
    for (auto base : klass->baseClasses()) {
        if (auto r = recurseClassHierarchy(base, pred))
            return r;
    }
    return nullptr;
}

#endif // ABSTRACTMETALANG_HELPERS_H
