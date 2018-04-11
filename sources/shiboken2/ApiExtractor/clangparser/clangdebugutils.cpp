/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt for Python project.
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

#include "clangdebugutils.h"
#include "clangutils.h"

#include <QtCore/QDebug>
#include <QtCore/QString>

#include <string.h>

#ifndef QT_NO_DEBUG_STREAM

#ifdef Q_OS_WIN
const char pathSep = '\\';
#else
const char pathSep = '/';
#endif

static const char *baseName(const char *fileName)
{
    const char *b = strrchr(fileName, pathSep);
    return b ? b + 1 : fileName;
}

QDebug operator<<(QDebug s, const CXString &cs)
{
    s << clang_getCString(cs);
    return s;
}

QDebug operator<<(QDebug s, CXCursorKind cursorKind) // Enum
{
    const CXString kindName = clang_getCursorKindSpelling(cursorKind);
    s << kindName;
    clang_disposeString(kindName);
    return s;
}

static const char *accessSpecsStrings[]
{
    // CX_CXXInvalidAccessSpecifier, CX_CXXPublic, CX_CXXProtected, CX_CXXPrivate
    "invalid",  "public", "protected", "private"
};

QDebug operator<<(QDebug s, CX_CXXAccessSpecifier ac)
{
    s << accessSpecsStrings[ac];
    return s;
}

QDebug operator<<(QDebug s, const CXType &t)
{
    CXString typeSpelling = clang_getTypeSpelling(t);
    s << typeSpelling;
    clang_disposeString(typeSpelling);
    return s;
}

QDebug operator<<(QDebug s, const CXCursor &cursor)
{
    QDebugStateSaver saver(s);
    s.nospace();
    s.noquote();
    const CXCursorKind kind = clang_getCursorKind(cursor);
    s << kind;
    if (kind >= CXCursor_FirstInvalid && kind <= CXCursor_LastInvalid)
        return s;
    const CXType type = clang_getCursorType(cursor);
    switch (kind) {
        case CXCursor_CXXAccessSpecifier:
        s << ' ' << clang_getCXXAccessSpecifier(cursor);
        break;
    case CXCursor_CXXBaseSpecifier:
         s << ", inherits=\"" << clang::getCursorSpelling(clang_getTypeDeclaration(type)) << '"';
         break;
    case CXCursor_CXXMethod:
    case CXCursor_FunctionDecl:
    case CXCursor_ConversionFunction:
        s << ", result type=\"" << clang_getCursorResultType(cursor) << '"';
        break;
    case CXCursor_TypedefDecl:
        s << ", underlyingType=\"" << clang_getTypedefDeclUnderlyingType(cursor) << '"';
        break;
    default:
        break;
    }

    if (type.kind != CXType_Invalid)
        s  << ", type=\"" << type << '"';
    if (clang_Cursor_hasAttrs(cursor))
        s  << ", [attrs]";

    const QString cursorSpelling = clang::getCursorSpelling(cursor);
    if (!cursorSpelling.isEmpty())
        s << ", spelling=\"" << cursorSpelling << '"';
    CXString cursorDisplay = clang_getCursorDisplayName(cursor);
    if (const char *dpy = clang_getCString(cursorDisplay)) {
        const QString display = QString::fromUtf8(dpy);
        if (display != cursorSpelling)
            s << ", display=\"" << dpy << '"';
    }
    clang_disposeString(cursorDisplay);
    return s;
}

QDebug operator<<(QDebug s, const CXSourceLocation &location)
{
    QDebugStateSaver saver(s);
    s.nospace();
    CXFile file; // void *
    unsigned line;
    unsigned column;
    unsigned offset;
    clang_getExpansionLocation(location, &file, &line, &column, &offset);
    const CXString cxFileName = clang_getFileName(file);
    // Has been observed to be 0 for invalid locations
    if (const char *cFileName = clang_getCString(cxFileName))
        s << baseName(cFileName) << ':';
    s << line << ':' << column;
    clang_disposeString(cxFileName);
    return s;
}

#endif // !QT_NO_DEBUG_STREAM
