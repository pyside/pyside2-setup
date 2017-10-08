/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "compilersupport.h"
#include "header_paths.h"

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QStringList>

#include <string.h>
#include <algorithm>
#include <iterator>

namespace clang {

static bool runProcess(const QString &program, const QStringList &arguments,
                       QByteArray *stdOutIn = nullptr, QByteArray *stdErrIn = nullptr)
{
    QProcess process;
    process.start(program, arguments, QProcess::ReadWrite);
    if (!process.waitForStarted()) {
        qWarning().noquote().nospace() << "Unable to start "
            << process.program() << ": " << process.errorString();
        return false;
    }
    process.closeWriteChannel();
    const bool finished = process.waitForFinished();
    const QByteArray stdErr = process.readAllStandardError();
    if (stdErrIn)
        *stdErrIn = stdErr;
    if (stdOutIn)
        *stdOutIn = process.readAllStandardOutput();

    if (!finished) {
        qWarning().noquote().nospace() << process.program() << " timed out: " << stdErr;
        process.kill();
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit) {
        qWarning().noquote().nospace() << process.program() << " crashed: " << stdErr;
        return false;
    }

    if (process.exitCode() != 0) {
        qWarning().noquote().nospace() <<  process.program() << " exited "
            << process.exitCode() << ": " << stdErr;
        return false;
    }

    return true;
}

#if defined(Q_CC_GNU)

static QByteArray frameworkPath() { return QByteArrayLiteral(" (framework directory)"); }

// Determine g++'s internal include paths from the output of
// g++ -E -x c++ - -v </dev/null
// Output looks like:
// #include <...> search starts here:
// /usr/local/include
// /System/Library/Frameworks (framework directory)
// End of search list.
static HeaderPaths gppInternalIncludePaths(const QString &compiler)
{
    HeaderPaths result;
    QStringList arguments;
    arguments << QStringLiteral("-E") << QStringLiteral("-x") << QStringLiteral("c++")
         << QStringLiteral("-") << QStringLiteral("-v");
    QByteArray stdOut;
    QByteArray stdErr;
    if (!runProcess(compiler, arguments, &stdOut, &stdErr))
        return result;
    const QByteArrayList stdErrLines = stdErr.split('\n');
    bool isIncludeDir = false;
    for (const QByteArray &line : stdErrLines) {
        if (isIncludeDir) {
            if (line.startsWith(QByteArrayLiteral("End of search list"))) {
                isIncludeDir = false;
            } else {
                HeaderPath headerPath(line.trimmed());
                if (headerPath.path.endsWith(frameworkPath())) {
                    headerPath.m_isFramework = true;
                    headerPath.path.truncate(headerPath.path.size() - frameworkPath().size());
                }
                result.append(headerPath);
            }
        } else if (line.startsWith(QByteArrayLiteral("#include <...> search starts here"))) {
            isIncludeDir = true;
        }
    }
    return result;
}
#endif // Q_CC_MSVC

// Detect Vulkan as supported from Qt 5.10 by checking the environment variables.
static void detectVulkan(HeaderPaths *headerPaths)
{
    static const char *vulkanVariables[] = {"VULKAN_SDK", "VK_SDK_PATH"};
    for (const char *vulkanVariable : vulkanVariables) {
        if (qEnvironmentVariableIsSet(vulkanVariable)) {
            headerPaths->append(HeaderPath(qgetenv(vulkanVariable) + QByteArrayLiteral("/include")));
            break;
        }
    }
}

// For MSVC, we set the MS compatibility version and let Clang figure out its own
// options and include paths.
// For the others, we pass "-nostdinc" since libclang tries to add it's own system
// include paths, which together with the clang compiler paths causes some clash
// which causes std types not being found and construct -I/-F options from the
// include paths of the host compiler.

#ifdef Q_CC_CLANG
static QByteArray noStandardIncludeOption() { return QByteArrayLiteral("-nostdinc"); }
#endif

// Returns clang options needed for emulating the host compiler
QByteArrayList emulatedCompilerOptions()
{
    QByteArrayList result;
#if defined(Q_CC_MSVC)
    HeaderPaths headerPaths;
    result.append(QByteArrayLiteral("-fms-compatibility-version=19"));
    result.append(QByteArrayLiteral("-Wno-microsoft-enum-value"));
#elif defined(Q_CC_CLANG)
    HeaderPaths headerPaths = gppInternalIncludePaths(QStringLiteral("clang++"));
    result.append(noStandardIncludeOption());
#elif defined(Q_CC_GNU)
    HeaderPaths headerPaths;

    // The clang builtin includes directory is used to find the definitions for intrinsic functions
    // and builtin types. It is necessary to use the clang includes to prevent redefinition errors.
    // The default toolchain includes should be picked up automatically by clang without specifying
    // them implicitly.
    QByteArray clangBuiltinIncludesDir(CLANG_BUILTIN_INCLUDES_DIR);

    if (!clangBuiltinIncludesDir.isEmpty()) {
        result.append(QByteArrayLiteral("-isystem"));
        result.append(clangBuiltinIncludesDir);
    }
#else
    HeaderPaths headerPaths;
#endif
    detectVulkan(&headerPaths);
    std::transform(headerPaths.cbegin(), headerPaths.cend(),
                   std::back_inserter(result), [](const HeaderPath &p) {
                       return HeaderPath::includeOption(p, true);
    });
    return result;
}

} // namespace clang
