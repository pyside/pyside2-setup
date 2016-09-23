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

#ifndef PP_STRING_H
#define PP_STRING_H

namespace rpp
{

template <typename _CharT>
class pp_string
{
    typedef std::char_traits<_CharT> traits_type;
    typedef std::size_t size_type;

    _CharT const *_M_begin;
    std::size_t _M_size;

public:
    inline pp_string():
            _M_begin(0), _M_size(0) {}

    explicit pp_string(std::string const &__s):
            _M_begin(__s.c_str()), _M_size(__s.size()) {}

    inline pp_string(_CharT const *__begin, std::size_t __size):
            _M_begin(__begin), _M_size(__size) {}

    inline _CharT const *begin() const {
        return _M_begin;
    }
    inline _CharT const *end() const {
        return _M_begin + _M_size;
    }

    inline _CharT at(std::size_t index) const {
        return _M_begin [index];
    }

    inline std::size_t size() const {
        return _M_size;
    }

    inline int compare(pp_string const &__other) const {
        size_type const __size = this->size();
        size_type const __osize = __other.size();
        size_type const __len = std::min(__size,  __osize);

        int __r = traits_type::compare(_M_begin, __other._M_begin, __len);
        if (!__r)
            __r = (int)(__size - __osize);

        return __r;
    }

    inline bool operator == (pp_string const &__other) const {
        return compare(__other) == 0;
    }

    inline bool operator != (pp_string const &__other) const {
        return compare(__other) != 0;
    }

    inline bool operator < (pp_string const &__other) const {
        return compare(__other) < 0;
    }

    inline bool operator == (char const *s) const {
        std::size_t n = strlen(s);

        if (n != _M_size)
            return false;

        return ! strncmp(_M_begin, s, n);
    }

    inline bool operator != (char const *s) const {
        return ! operator == (s);
    }
};

} // namespace rpp

#endif // PP_STRING_H

// kate: space-indent on; indent-width 2; replace-tabs on;
