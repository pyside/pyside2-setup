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

#ifndef PP_H
#define PP_H

#if defined(_WIN64) || defined(WIN64) || defined(__WIN64__) \
    || defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#  define PP_OS_WIN
#endif

#include <set>
#include <map>
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <cassert>
#include <cctype>
#include <cstdio>

#include <fcntl.h>

#ifdef HAVE_MMAP
#  include <sys/mman.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#if (_MSC_VER >= 1400)
#  define FILENO _fileno
#else
#  define FILENO fileno
#endif

#if defined (PP_OS_WIN)
#  define PATH_SEPARATOR '\\'
#else
#  define PATH_SEPARATOR '/'
#endif

#if defined (RPP_JAMBI)
#  include "parser/rxx_allocator.h"
#else
#  include "parser/rpp-allocator.h"
#endif

#if defined (_MSC_VER)
#  define pp_snprintf _snprintf
#else
#  define pp_snprintf snprintf
#endif

#include "pp-fwd.h"
#include "pp-cctype.h"
#include "pp-string.h"
#include "pp-symbol.h"
#include "pp-internal.h"
#include "pp-iterator.h"
#include "pp-macro.h"
#include "pp-environment.h"
#include "pp-scanner.h"
#include "pp-macro-expander.h"
#include "pp-engine.h"
#include "pp-engine-bits.h"

#endif // PP_H

// kate: space-indent on; indent-width 2; replace-tabs on;
