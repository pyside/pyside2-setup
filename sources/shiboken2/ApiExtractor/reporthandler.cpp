/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "reporthandler.h"
#include "typesystem.h"
#include "typedatabase.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>
#include <cstring>
#include <cstdarg>
#include <cstdio>

#if defined(_WINDOWS) || defined(NOCOLOR)
    #define COLOR_END ""
    #define COLOR_WHITE ""
    #define COLOR_YELLOW ""
    #define COLOR_GREEN ""
#else
    #define COLOR_END "\033[0m"
    #define COLOR_WHITE "\033[1;37m"
    #define COLOR_YELLOW "\033[1;33m"
    #define COLOR_GREEN "\033[0;32m"
#endif

static bool m_silent = false;
static int m_warningCount = 0;
static int m_suppressedCount = 0;
static ReportHandler::DebugLevel m_debugLevel = ReportHandler::NoDebug;
static QSet<QString> m_reportedWarnings;
static QString m_prefix;
static bool m_withinProgress = false;
static int m_step_warning = 0;
static QElapsedTimer m_timer;

Q_LOGGING_CATEGORY(lcShiboken, "qt.shiboken")

void ReportHandler::install()
{
    qInstallMessageHandler(ReportHandler::messageOutput);
    startTimer();
}

void ReportHandler::startTimer()
{
    m_timer.start();
}

ReportHandler::DebugLevel ReportHandler::debugLevel()
{
    return m_debugLevel;
}

void ReportHandler::setDebugLevel(ReportHandler::DebugLevel level)
{
    m_debugLevel = level;
}

int ReportHandler::suppressedCount()
{
    return m_suppressedCount;
}

int ReportHandler::warningCount()
{
    return m_warningCount;
}

bool ReportHandler::isSilent()
{
    return m_silent;
}

void ReportHandler::setSilent(bool silent)
{
    m_silent = silent;
}

void ReportHandler::setPrefix(const QString &p)
{
    m_prefix = p;
}

void ReportHandler::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &text)
{
    if (type == QtWarningMsg) {
        if (m_silent || m_reportedWarnings.contains(text))
            return;
        const TypeDatabase *db = TypeDatabase::instance();
        if (db && db->isSuppressedWarning(text)) {
            ++m_suppressedCount;
            return;
        }
        ++m_warningCount;
        ++m_step_warning;
        m_reportedWarnings.insert(text);
    }
    QString message = m_prefix;
    if (!message.isEmpty())
        message.append(QLatin1Char(' '));
    message.append(text);
    fprintf(stderr, "%s\n", qPrintable(qFormatLogMessage(type, context, message)));
}

static QByteArray timeStamp()
{
    const qint64 elapsed = m_timer.elapsed();
    return elapsed > 5000
        ? QByteArray::number(elapsed / 1000) + 's'
        : QByteArray::number(elapsed) + "ms";
}

void ReportHandler::startProgress(const QByteArray& str)
{
    if (m_silent)
        return;

    if (m_withinProgress)
        endProgress();

    m_withinProgress = true;
    const auto ts = '[' + timeStamp() + ']';
    std::printf("%s %8s %-60s", qPrintable(m_prefix), ts.constData(), str.constData());
    std::fflush(stdout);
}

void ReportHandler::endProgress()
{
    if (m_silent)
        return;

    m_withinProgress = false;
    const char *endMessage = m_step_warning == 0
        ?  "[" COLOR_GREEN "OK" COLOR_END "]\n"
        : "[" COLOR_YELLOW "WARNING" COLOR_END "]\n";
    std::fputs(endMessage, stdout);
    std::fflush(stdout);
    m_step_warning = 0;
}

QByteArray ReportHandler::doneMessage()
{
    QByteArray result = "Done, " + m_prefix.toUtf8() + ' ' + timeStamp();
    if (m_warningCount)
        result += ", " + QByteArray::number(m_warningCount) + " warnings";
    if (m_suppressedCount)
        result += " (" + QByteArray::number(m_suppressedCount) + " known issues)";
    return  result;
}
