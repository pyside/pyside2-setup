/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef CLANGUTILS_H
#define CLANGUTILS_H

#include <clang-c/Index.h>
#include <qtcompat.h>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QDebug)

bool operator==(const CXCursor &c1, const CXCursor &c2);
QtCompatHashFunctionType qHash(const CXCursor &c, QtCompatHashFunctionType seed = 0);

bool operator==(const CXType &t1, const CXType &t2);
QtCompatHashFunctionType qHash(const CXType &ct, QtCompatHashFunctionType seed);

namespace clang {

QString getCursorKindName(CXCursorKind cursorKind);
QString getCursorSpelling(const CXCursor &cursor);
QString getCursorDisplayName(const CXCursor &cursor);
QString getTypeName(const CXType &type);
inline QString getCursorTypeName(const CXCursor &cursor)
    { return getTypeName(clang_getCursorType(cursor)); }
inline QString getCursorResultTypeName(const CXCursor &cursor)
    { return getTypeName(clang_getCursorResultType(cursor)); }

inline bool isCursorValid(const CXCursor &c)
{
    return c.kind < CXCursor_FirstInvalid || c.kind >  CXCursor_LastInvalid;
}

QString getFileName(CXFile file); // Uncached,see BaseVisitor for a cached version

struct SourceLocation
{
    bool equals(const SourceLocation &rhs) const;

    CXFile file;
    unsigned line = 0;
    unsigned column = 0;
    unsigned offset = 0;
};

inline bool operator==(const SourceLocation &l1, const SourceLocation &l2)
{ return l1.equals(l2); }

inline bool operator!=(const SourceLocation &l1, const SourceLocation &l2)
{ return !l1.equals(l2); }

SourceLocation getExpansionLocation(const CXSourceLocation &location);

using SourceRange =QPair<SourceLocation, SourceLocation>;

SourceLocation getCursorLocation(const CXCursor &cursor);
CXString getFileNameFromLocation(const CXSourceLocation &location);
SourceRange getCursorRange(const CXCursor &cursor);

struct Diagnostic {
    enum  Source { Clang, Other };

    Diagnostic() = default;
    // Clang
    static Diagnostic fromCXDiagnostic(CXDiagnostic cd);
    // Other
    explicit Diagnostic(const QString &m, const CXCursor &c, CXDiagnosticSeverity s = CXDiagnostic_Warning);

    QString message;
    QStringList childMessages;
    SourceLocation location;
    Source source = Clang;
    CXDiagnosticSeverity severity = CXDiagnostic_Warning;
};

QVector<Diagnostic> getDiagnostics(CXTranslationUnit tu);
CXDiagnosticSeverity maxSeverity(const QVector<Diagnostic> &ds);

// Parse a template argument list "a<b<c,d>,e>" and invoke a handler
// with each match (level and string). Return begin and end of the list.
using TemplateArgumentHandler = std::function<void (int, const QStringRef &)>;

QPair<int, int> parseTemplateArgumentList(const QString &l,
                                          const TemplateArgumentHandler &handler,
                                          int from = 0);

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const SourceLocation &);
QDebug operator<<(QDebug, const Diagnostic &);
#endif // QT_NO_DEBUG_STREAM
} // namespace clang

#endif // CLANGUTILS_H
