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

#ifndef PP_CCTYPE_H
#define PP_CCTYPE_H

#include <cctype>

namespace rpp
{

inline bool pp_isalpha(int __ch)
{
    return std::isalpha((unsigned char) __ch) != 0;
}

inline bool pp_isalnum(int __ch)
{
    return std::isalnum((unsigned char) __ch) != 0;
}

inline bool pp_isdigit(int __ch)
{
    return std::isdigit((unsigned char) __ch) != 0;
}

inline bool pp_isspace(int __ch)
{
    return std::isspace((unsigned char) __ch) != 0;
}

} // namespace rpp

#endif // PP_CCTYPE_H

// kate: space-indent on; indent-width 2; replace-tabs on;
