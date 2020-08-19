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

#include "clangutils.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHashFunctions>
#include <QtCore/QProcess>

bool operator==(const CXCursor &c1, const CXCursor &c2)
{
    return c1.kind == c2.kind
        && c1.xdata == c2.xdata
        && std::equal(c1.data, c1.data + sizeof(c1.data) / sizeof(c1.data[0]), c2.data);
}

QtCompatHashFunctionType qHash(const CXCursor &c, QtCompatHashFunctionType seed)
{
    return qHash(c.kind) ^ qHash(c.xdata) ^ qHash(c.data[0])
        ^ qHash(c.data[1])  ^ qHash(c.data[2]) ^ seed;
}

bool operator==(const CXType &t1, const CXType &t2)
{
    return t1.kind == t2.kind && t1.data[0] == t2.data[0]
        && t1.data[1] == t2.data[1];
}

QtCompatHashFunctionType qHash(const CXType &ct, QtCompatHashFunctionType seed)
{
    return QtCompatHashFunctionType(ct.kind) ^ QtCompatHashFunctionType(0xFFFFFFFF & quintptr(ct.data[0]))
        ^ QtCompatHashFunctionType(0xFFFFFFFF & quintptr(ct.data[1])) ^ seed;
}

namespace clang {

bool SourceLocation::equals(const SourceLocation &rhs) const
{
    return file == rhs.file && offset == rhs.offset;
}

SourceLocation getExpansionLocation(const CXSourceLocation &location)
{
    SourceLocation result;
    clang_getExpansionLocation(location, &result.file, &result.line, &result.column, &result.offset);
    return result;
}

QString getFileName(CXFile file)
{
    QString result;
    const CXString cxFileName = clang_getFileName(file);
    // Has been observed to be 0 for invalid locations
    if (const char *cFileName = clang_getCString(cxFileName))
        result = QString::fromUtf8(cFileName);
    clang_disposeString(cxFileName);
    return result;
}

SourceLocation getCursorLocation(const CXCursor &cursor)
{
    const CXSourceRange extent = clang_getCursorExtent(cursor);
    return getExpansionLocation(clang_getRangeStart(extent));
}

CXString getFileNameFromLocation(const CXSourceLocation &location)
{
    CXFile file;
    unsigned line;
    unsigned column;
    unsigned offset;
    clang_getExpansionLocation(location, &file, &line, &column, &offset);
    return clang_getFileName(file);
}

SourceRange getCursorRange(const CXCursor &cursor)
{
    const CXSourceRange extent = clang_getCursorExtent(cursor);
    return qMakePair(getExpansionLocation(clang_getRangeStart(extent)),
                     getExpansionLocation(clang_getRangeEnd(extent)));
}

QString getCursorKindName(CXCursorKind cursorKind)
{
    CXString kindName  = clang_getCursorKindSpelling(cursorKind);
    const QString result = QString::fromUtf8(clang_getCString(kindName));
    clang_disposeString(kindName);
    return result;
}

QString getCursorSpelling(const CXCursor &cursor)
{
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    const QString result = QString::fromUtf8(clang_getCString(cursorSpelling));
    clang_disposeString(cursorSpelling);
    return result;
}

QString getCursorDisplayName(const CXCursor &cursor)
{
    CXString displayName = clang_getCursorDisplayName(cursor);
    const QString result = QString::fromUtf8(clang_getCString(displayName));
    clang_disposeString(displayName);
    return result;
}

QString getTypeName(const CXType &type)
{
    CXString typeSpelling = clang_getTypeSpelling(type);
    const QString result = QString::fromUtf8(clang_getCString(typeSpelling));
    clang_disposeString(typeSpelling);
    return result;
}

Diagnostic::Diagnostic(const QString &m, const CXCursor &c, CXDiagnosticSeverity s)
    : message(m), location(getCursorLocation(c)), source(Other), severity(s)
{
}

Diagnostic Diagnostic::fromCXDiagnostic(CXDiagnostic cd)
{
    Diagnostic result;
    result.source = Clang;
    CXString spelling = clang_getDiagnosticSpelling(cd);
    result.message = QString::fromUtf8(clang_getCString(spelling));
    clang_disposeString(spelling);
    result.severity = clang_getDiagnosticSeverity(cd);
    result.location = getExpansionLocation(clang_getDiagnosticLocation(cd));

    CXDiagnosticSet childDiagnostics = clang_getChildDiagnostics(cd);
    if (const unsigned childCount = clang_getNumDiagnosticsInSet(childDiagnostics)) {
        result.childMessages.reserve(int(childCount));
        const unsigned format = clang_defaultDiagnosticDisplayOptions();
        for (unsigned i = 0; i < childCount; ++i) {
            CXDiagnostic childDiagnostic = clang_getDiagnosticInSet(childDiagnostics, i);
            CXString cdm = clang_formatDiagnostic(childDiagnostic, format);
            result.childMessages.append(QString::fromUtf8(clang_getCString(cdm)));
            clang_disposeString(cdm);
            clang_disposeDiagnostic(childDiagnostic);
        }
    }

    return result;
}

QVector<Diagnostic> getDiagnostics(CXTranslationUnit tu)
{
    QVector<Diagnostic> result;
    const unsigned count = clang_getNumDiagnostics(tu);
    result.reserve(int(count));
    for (unsigned i = 0; i < count; ++i) {
        const CXDiagnostic d = clang_getDiagnostic(tu, i);
        result.append(Diagnostic::fromCXDiagnostic(d));
        clang_disposeDiagnostic(d);
    }
    return result;
}

QPair<int, int> parseTemplateArgumentList(const QString &l,
                                          const TemplateArgumentHandler &handler,
                                          int from)
{
    const int ltPos = l.indexOf(QLatin1Char('<'), from);
    if (ltPos == - 1)
        return qMakePair(-1, -1);
    int startPos = ltPos + 1;
    int level = 1;
    for (int p = startPos, end = l.size(); p < end; ) {
        const char c = l.at(p).toLatin1();
        switch (c) {
        case ',':
        case '>':
            handler(level, QStringView{l}.mid(startPos, p - startPos).trimmed());
            ++p;
            if (c == '>') {
                if (--level == 0)
                    return qMakePair(ltPos, p);
                // Skip over next ',': "a<b<c,d>,e>"
                for (; p < end && (l.at(p).isSpace() || l.at(p) == QLatin1Char(',')); ++p) {}
            }
            startPos = p;
            break;
        case '<':
            handler(level, QStringView{l}.mid(startPos, p - startPos).trimmed());
            ++level;
            startPos = ++p;
            break;
        default:
             ++p;
            break;
        }
    }
    return qMakePair(-1, -1);
}

CXDiagnosticSeverity maxSeverity(const QVector<Diagnostic> &ds)
{
    CXDiagnosticSeverity result = CXDiagnostic_Ignored;
    for (const Diagnostic& d : ds) {
        if (d.severity > result)
            result = d.severity;
    }
    return result;
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug s, const SourceLocation &l)
{
    QDebugStateSaver saver(s);
    s.nospace();
    s.noquote();
    s << QDir::toNativeSeparators(clang::getFileName(l.file)) << ':' << l.line;
    if (l.column)
        s << ':' << l.column;
    return s;
}

// Roughly follow g++ format:
// file.cpp:214:37: warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]
QDebug operator<<(QDebug s, const Diagnostic &d)
{
    QDebugStateSaver saver(s);
    s.nospace();
    s.noquote();
    s << d.location << ": ";
    switch (d.severity) {
    case CXDiagnostic_Ignored:
        s << "ignored";
        break;
    case CXDiagnostic_Note:
        s << "note";
        break;
    case CXDiagnostic_Warning:
        s << "warning";
        break;
    case CXDiagnostic_Error:
        s << "error";
        break;
    case CXDiagnostic_Fatal:
        s << "fatal";
        break;
    }
    s << ": " << d.message;

    if (d.source != Diagnostic::Clang)
        s << " [other]";

    if (const int childMessagesCount = d.childMessages.size()) {
        s << '\n';
        for (int i = 0; i < childMessagesCount; ++i)
            s << "   " << d.childMessages.at(i) << '\n';
    }

    return s;
}

#endif // QT_NO_DEBUG_STREAM

} // namespace clang
