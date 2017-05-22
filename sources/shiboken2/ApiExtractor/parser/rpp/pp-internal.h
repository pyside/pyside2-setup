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

#ifndef PP_INTERNAL_H
#define PP_INTERNAL_H

#include <algorithm>
#include <string>
#include "pp.h"

namespace rpp
{

namespace _PP_internal
{

inline void extract_file_path(const std::string &__filename, std::string *__filepath)
{
    std::size_t __index = __filename.rfind(PATH_SEPARATOR);

    if (__index == std::string::npos)
        *__filepath = "/";

    else
        __filepath->assign(__filename, 0, __index + 1);
}

template <typename _OutputIterator>
void output_line(const std::string &__filename, int __line, _OutputIterator __result)
{
    std::string __msg;

    __msg += "# ";

    char __line_descr[16];
    pp_snprintf(__line_descr, 16, "%d", __line);
    __msg += __line_descr;

    __msg += " \"";

    if (__filename.empty())
        __msg += "<internal>";
    else
        __msg += __filename;

    __msg += "\"\n";
    std::copy(__msg.begin(), __msg.end(), __result);
}

template <typename _InputIterator>
inline bool comment_p(_InputIterator __first, _InputIterator __last)  /*const*/
{
    if (__first == __last)
        return false;

    if (*__first != '/')
        return false;

    if (++__first == __last)
        return false;

    return (*__first == '/' || *__first == '*');
}

struct _Compare_string: public std::binary_function<bool, pp_fast_string const *, pp_fast_string const *> {
    inline bool operator()(pp_fast_string const *__lhs, pp_fast_string const *__rhs) const {
        return *__lhs < *__rhs;
    }
};

struct _Equal_to_string: public std::binary_function<bool, pp_fast_string const *, pp_fast_string const *> {
    inline bool operator()(pp_fast_string const *__lhs, pp_fast_string const *__rhs) const {
        return *__lhs == *__rhs;
    }
};

struct _Hash_string: public std::unary_function<std::size_t, pp_fast_string const *> {
    inline std::size_t operator()(pp_fast_string const *__s) const {
        char const *__ptr = __s->begin();
        std::size_t __size = __s->size();
        std::size_t __h = 0;

        for (std::size_t i = 0; i < __size; ++i)
            __h = (__h << 5) - __h + __ptr [i];

        return __h;
    }
};

} // _PP_internal

} // namespace rpp

#endif // PP_INTERNAL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
