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

#ifndef PP_SYMBOL_H
#define PP_SYMBOL_H

#include <cassert>
#include <iterator>
#include "pp-fwd.h"
#include "parser/rxx_allocator.h"

namespace rpp
{

class pp_symbol
{
    static rxx_allocator<char> &allocator_instance() {
        static rxx_allocator<char>__allocator;
        return __allocator;
    }
    static rxx_allocator<pp_fast_string> &ppfs_allocator_instance ()
    {
        static rxx_allocator<pp_fast_string>__ppfs_allocator;
        return __ppfs_allocator;
    }

public:
    static int &N() {
        static int __N;
        return __N;
    }

    static pp_fast_string const *get(char const *__data, std::size_t __size) {
        ++N();
        char *data = allocator_instance().allocate(__size + 1);
        memcpy(data, __data, __size);
        data[__size] = '\0';

        pp_fast_string *where = ppfs_allocator_instance ().allocate (sizeof (pp_fast_string));
        return new(where) pp_fast_string(data, __size);
    }

    template <typename _InputIterator>
    static pp_fast_string const *get(_InputIterator __first, _InputIterator __last) {
        ++N();
        std::ptrdiff_t __size;
#if defined(__SUNPRO_CC)
        std::distance(__first, __last, __size);
#else
        __size = std::distance(__first, __last);
#endif
        assert(__size >= 0 && __size < 512);

        char *data = allocator_instance().allocate(__size + 1);
        std::copy(__first, __last, data);
        data[__size] = '\0';

        pp_fast_string *where = ppfs_allocator_instance ().allocate (sizeof (pp_fast_string));
        return new(where) pp_fast_string(data, __size);
    }

    static pp_fast_string const *get(std::string const &__s) {
        return get(__s.c_str(), __s.size());
    }
};

} // namespace rpp

#endif // PP_SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
