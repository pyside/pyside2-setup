/*
 * This file is part of the API Extractor project.
 *
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "reporthandler.h"
#include "typesystem.h"
#include "typedatabase.h"
#include <QtCore/QSet>
#include <cstring>
#include <cstdarg>
#include <cstdio>

#if _WINDOWS || NOCOLOR
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
static QString m_progressBuffer;
static int m_step_size = 0;
static int m_step = -1;
static int m_step_warning = 0;

Q_LOGGING_CATEGORY(lcShiboken, "qt.shiboken")

static void printProgress()
{
    std::printf("%s", m_progressBuffer.toUtf8().data());
    std::fflush(stdout);
    m_progressBuffer.clear();
}

void ReportHandler::install()
{
    qInstallMessageHandler(ReportHandler::messageOutput);
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

void ReportHandler::setProgressReference(int max)
{
    m_step_size = max;
    m_step = -1;
}

bool ReportHandler::isSilent()
{
    return m_silent;
}

void ReportHandler::setSilent(bool silent)
{
    m_silent = silent;
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
    fprintf(stderr, "%s\n", qPrintable(qFormatLogMessage(type, context, text)));
}

void ReportHandler::progress(const QString& str, ...)
{
    if (m_silent)
        return;

    if (m_step == -1) {
        QTextStream buf(&m_progressBuffer);
        buf.setFieldWidth(45);
        buf.setFieldAlignment(QTextStream::AlignLeft);
        buf << str;
        printProgress();
        m_step = 0;
    }
    m_step++;
    if (m_step >= m_step_size) {
        if (m_step_warning == 0) {
            m_progressBuffer = "[" COLOR_GREEN "OK" COLOR_END "]\n";
        } else {
            m_progressBuffer = "[" COLOR_YELLOW "WARNING" COLOR_END "]\n";
        }
        printProgress();
        m_step_warning = 0;
    }
}
