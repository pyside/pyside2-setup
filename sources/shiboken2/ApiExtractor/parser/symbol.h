/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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


#ifndef SYMBOL_H
#define SYMBOL_H

#include <QtCore/QString>
#include <cstring>

#include <QtCore/QHash>
#include <QtCore/QPair>

struct NameSymbol
{
    const char *data;
    std::size_t count;

    inline QString as_string() const
    {
        return QString::fromUtf8(data, (int) count);
    }

    inline bool operator == (const NameSymbol &other) const
    {
        return count == other.count
               && !std::strncmp(data, other.data, count);
    }

protected:
    inline NameSymbol() {}
    inline NameSymbol(const char *d, std::size_t c)
            : data(d), count(c) {}

private:
    void operator = (const NameSymbol &);

    friend class NameTable;
};

inline uint qHash(const NameSymbol &r)
{
    uint hash_value = 0;

    for (std::size_t i = 0; i < r.count; ++i)
        hash_value = (hash_value << 5) - hash_value + r.data[i];

    return hash_value;
}

inline uint qHash(const QPair<const char*, std::size_t> &r)
{
    uint hash_value = 0;

    for (std::size_t i = 0; i < r.second; ++i)
        hash_value = (hash_value << 5) - hash_value + r.first[i];

    return hash_value;
}

class NameTable
{
public:
    typedef QPair<const char *, std::size_t> KeyType;
    typedef QHash<KeyType, NameSymbol*> ContainerType;

public:
    NameTable() {}

    ~NameTable()
    {
        qDeleteAll(_M_storage);
    }

    inline const NameSymbol *findOrInsert(const char *str, std::size_t len)
    {
        KeyType key(str, len);

        NameSymbol *name = _M_storage.value(key);
        if (!name) {
            name = new NameSymbol(str, len);
            _M_storage.insert(key, name);
        }

        return name;
    }

    inline std::size_t count() const { return _M_storage.size(); }

private:
    ContainerType _M_storage;

private:
    NameTable(const NameTable &other);
    void operator=(const NameTable &other);
};

#endif // SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;

