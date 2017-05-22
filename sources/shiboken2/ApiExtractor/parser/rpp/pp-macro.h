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

#ifndef PP_MACRO_H
#define PP_MACRO_H

#include <vector>
#include "pp-fwd.h"

namespace rpp
{

struct pp_macro {
#if defined (PP_WITH_MACRO_POSITION)
    pp_fast_string const *file;
#endif
    pp_fast_string const *name;
    pp_fast_string const *definition;
    std::vector<pp_fast_string const *> formals;

    union {
        int unsigned state;

        struct {
            int unsigned hidden: 1;
            int unsigned function_like: 1;
            int unsigned variadics: 1;
        };
    };

    int lines;
    pp_macro *next;
    std::size_t hash_code;

    inline pp_macro():
#if defined (PP_WITH_MACRO_POSITION)
            file(0),
#endif
            name(0),
            definition(0),
            state(0),
            lines(0),
            next(0),
            hash_code(0) {}
};

} // namespace rpp

#endif // PP_MACRO_H

// kate: space-indent on; indent-width 2; replace-tabs on;
