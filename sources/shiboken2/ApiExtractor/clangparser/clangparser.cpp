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

#include "clangparser.h"
#include "clangutils.h"
#include "clangdebugutils.h"
#include "compilersupport.h"

#include <QtCore/QByteArrayList>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QString>

namespace clang {

QString SourceFileCache::getFileName(CXFile file)
{
    auto it = m_fileNameCache.find(file);
    if (it == m_fileNameCache.end())
        it = m_fileNameCache.insert(file, clang::getFileName(file));
    return it.value();
}

std::string_view SourceFileCache::getCodeSnippet(const CXCursor &cursor,
                                                 QString *errorMessage)
{
    static const char empty[] = "";

    if (errorMessage)
        errorMessage->clear();

    const SourceRange range = getCursorRange(cursor);
    // Quick check for equal locations: Frequently happens if the code is
    // the result of a macro expansion
    if (range.first == range.second)
         return std::string_view(empty, 0);

    if (range.first.file != range.second.file) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Range spans several files");
        return std::string_view(empty, 0);
    }

    auto it = m_fileBufferCache.find(range.first.file);
    if (it == m_fileBufferCache.end()) {
        const QString fileName = getFileName(range.first.file);
        if (fileName.isEmpty()) {
            if (errorMessage)
                 *errorMessage = QStringLiteral("Range has no file");
            return std::string_view(empty, 0);
        }
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            if (errorMessage) {
                QTextStream str(errorMessage);
                str << "Cannot open \"" << QDir::toNativeSeparators(fileName)
                    << "\": " << file.errorString();
            }
            return std::string_view(empty, 0);
        }
        it = m_fileBufferCache.insert(range.first.file, file.readAll());
    }

    const unsigned pos = range.first.offset;
    const unsigned end = range.second.offset;
    Q_ASSERT(end > pos);
    const QByteArray &contents = it.value();
    if (end >= unsigned(contents.size())) {
        if (errorMessage) {
            QTextStream str(errorMessage);
            str << "Range end " << end << " is above size of \""
                << QDir::toNativeSeparators(getFileName(range.first.file))
                << "\" (" << contents.size() << ')';
        }
        return std::string_view(empty, 0);
    }

    return std::string_view(contents.constData() + pos, end - pos);
}

BaseVisitor::BaseVisitor() = default;
BaseVisitor::~BaseVisitor() = default;

bool BaseVisitor::visitLocation(const CXSourceLocation &location) const
{
    return clang_Location_isFromMainFile(location) != 0;
}

BaseVisitor::StartTokenResult BaseVisitor::cbHandleStartToken(const CXCursor &cursor)
{
    switch (cursor.kind) {
    default:
        break;
    }

    return startToken(cursor);
}

bool BaseVisitor::cbHandleEndToken(const CXCursor &cursor, StartTokenResult startResult)
{
    const bool result = startResult != Recurse || endToken(cursor);
    switch (cursor.kind) {
    default:
        break;
    }

    return result;
}

std::string_view BaseVisitor::getCodeSnippet(const CXCursor &cursor)
{
    QString errorMessage;
    const std::string_view result = m_fileCache.getCodeSnippet(cursor, &errorMessage);
    if (result.empty() && !errorMessage.isEmpty()) {
        QString message;
        QTextStream str(&message);
        str << "Unable to retrieve code snippet \"" << getCursorSpelling(cursor)
            << "\": " << errorMessage;
        appendDiagnostic(Diagnostic(message, cursor, CXDiagnostic_Error));
    }
    return result;
}

QString BaseVisitor::getCodeSnippetString(const CXCursor &cursor)
{
    const std::string_view result = getCodeSnippet(cursor);
    return result.empty()
        ? QString()
        : QString::fromUtf8(result.data(), qsizetype(result.size()));
}

static CXChildVisitResult
    visitorCallback(CXCursor cursor, CXCursor /* parent */, CXClientData clientData)
{
    auto *bv = reinterpret_cast<BaseVisitor *>(clientData);

    const CXSourceLocation location = clang_getCursorLocation(cursor);
    if (!bv->visitLocation(location))
        return CXChildVisit_Continue;

    const BaseVisitor::StartTokenResult startResult = bv->cbHandleStartToken(cursor);
    switch (startResult) {
    case clang::BaseVisitor::Error:
        return CXChildVisit_Break;
    case clang::BaseVisitor::Skip:
        break;
    case clang::BaseVisitor::Recurse:
        clang_visitChildren(cursor, visitorCallback, clientData);
        break;
    }

    if (!bv->cbHandleEndToken(cursor, startResult))
        return CXChildVisit_Break;

    return CXChildVisit_Continue;
}

BaseVisitor::Diagnostics BaseVisitor::diagnostics() const
{
    return m_diagnostics;
}

void BaseVisitor::setDiagnostics(const Diagnostics &d)
{
    m_diagnostics = d;
}

void BaseVisitor::appendDiagnostic(const Diagnostic &d)
{
    m_diagnostics.append(d);
}

static inline const char **byteArrayListToFlatArgV(const QByteArrayList &bl)
{
    const char **result = new const char *[bl.size() + 1];
    result[bl.size()] = nullptr;
    std::transform(bl.cbegin(), bl.cend(), result,
                   [] (const QByteArray &a) { return a.constData(); });
    return result;
}

static QByteArray msgCreateTranslationUnit(const QByteArrayList &clangArgs, unsigned flags)
{
    QByteArray result = "clang_parseTranslationUnit2(0x";
    result += QByteArray::number(flags, 16);
    const int count = clangArgs.size();
    result += ", cmd[" + QByteArray::number(count) + "]=";
    for (int i = 0; i < count; ++i) {
        const QByteArray &arg = clangArgs.at(i);
        if (i)
            result += ' ';
        const bool quote = arg.contains(' ') || arg.contains('(');
        if (quote)
            result += '"';
        result += arg;
        if (quote)
            result += '"';
    }
    result += ')';
    return result;
}

static CXTranslationUnit createTranslationUnit(CXIndex index,
                                               const QByteArrayList &args,
                                               unsigned flags = 0)
{
    // courtesy qdoc
    const unsigned defaultFlags = CXTranslationUnit_SkipFunctionBodies
        | CXTranslationUnit_Incomplete;

    static const QByteArrayList defaultArgs = {
#ifndef Q_OS_WIN
        "-fPIC",
#endif
#ifdef Q_OS_MACOS
        "-Wno-expansion-to-defined", // Workaround for warnings in Darwin stdlib, see
                                     // https://github.com/darlinghq/darling/issues/204
#endif
        "-Wno-constant-logical-operand"
    };

    const QByteArrayList clangArgs = emulatedCompilerOptions() + defaultArgs + args;
    QScopedArrayPointer<const char *> argv(byteArrayListToFlatArgV(clangArgs));
    qDebug().noquote().nospace() << msgCreateTranslationUnit(clangArgs, flags);

    CXTranslationUnit tu;
    CXErrorCode err = clang_parseTranslationUnit2(index, nullptr, argv.data(),
                                                  clangArgs.size(), nullptr, 0,
                                                  defaultFlags | flags, &tu);
    if (err || !tu) {
        qWarning().noquote().nospace() << "Could not parse "
            << clangArgs.constLast().constData() << ", error code: " << err;
        return nullptr;
    }
    return tu;
}

/* clangFlags are flags to clang_parseTranslationUnit2() such as
 * CXTranslationUnit_KeepGoing (from CINDEX_VERSION_MAJOR/CINDEX_VERSION_MINOR 0.35)
 */

bool parse(const QByteArrayList  &clangArgs, unsigned clangFlags, BaseVisitor &bv)
{
    CXIndex index = clang_createIndex(0 /* excludeDeclarationsFromPCH */,
                                      1 /* displayDiagnostics */);
    if (!index) {
        qWarning() << "clang_createIndex() failed!";
        return false;
    }

    CXTranslationUnit translationUnit = createTranslationUnit(index, clangArgs, clangFlags);
    if (!translationUnit)
        return false;

    CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);

    clang_visitChildren(rootCursor, visitorCallback, reinterpret_cast<CXClientData>(&bv));

    QVector<Diagnostic> diagnostics = getDiagnostics(translationUnit);
    diagnostics.append(bv.diagnostics());
    bv.setDiagnostics(diagnostics);

    const bool ok = maxSeverity(diagnostics) < CXDiagnostic_Error;
    if (!ok) {
        QDebug debug = qWarning();
        debug.noquote();
        debug.nospace();
        debug << "Errors in "
            << QDir::toNativeSeparators(QFile::decodeName(clangArgs.constLast())) << ":\n";
        for (const Diagnostic &diagnostic : qAsConst(diagnostics))
            debug << diagnostic << '\n';
    }

    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);
    return ok;
}

} // namespace clang
