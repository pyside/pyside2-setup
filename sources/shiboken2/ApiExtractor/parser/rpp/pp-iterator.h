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

#ifndef PP_ITERATOR_H
#define PP_ITERATOR_H

#include <iterator>

namespace rpp
{

class pp_null_output_iterator
        : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
    pp_null_output_iterator() {}

    template <typename _Tp>
    pp_null_output_iterator &operator=(_Tp const &) {
        return *this;
    }

    inline pp_null_output_iterator &operator *() {
        return *this;
    }
    inline pp_null_output_iterator &operator ++ () {
        return *this;
    }
    inline pp_null_output_iterator operator ++ (int) {
        return *this;
    }
};

template <typename _Container>
class pp_output_iterator
        : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
    std::string &_M_result;

public:
    explicit pp_output_iterator(std::string &__result):
            _M_result(__result) {}

    inline pp_output_iterator<_Container>& operator=(const pp_output_iterator<_Container>& other)
    {
        _M_result = other._M_result;
        return *this;
    }

    inline pp_output_iterator &operator=(typename _Container::const_reference __v) {
        if (_M_result.capacity() == _M_result.size())
            _M_result.reserve(_M_result.capacity() << 2);

        _M_result.push_back(__v);
        return *this;
    }

    inline pp_output_iterator &operator *() {
        return *this;
    }
    inline pp_output_iterator &operator ++ () {
        return *this;
    }
    inline pp_output_iterator operator ++ (int) {
        return *this;
    }
};

} // namespace rpp

#endif // PP_ITERATOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
