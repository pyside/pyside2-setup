/*
 * This file is part of the API Extractor project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <cstring>

#ifndef NOCOLOR
#define COLOR_END "\033[0m"
#define COLOR_WHITE "\033[1;37m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_GREEN "\033[0;32m"
#else
#define COLOR_END ""
#define COLOR_WHITE ""
#define COLOR_YELLOW ""
#define COLOR_GREEN ""
#endif


bool ReportHandler::m_silent = false;
int ReportHandler::m_warningCount = 0;
int ReportHandler::m_suppressedCount = 0;
QString ReportHandler::m_context;
ReportHandler::DebugLevel ReportHandler::m_debugLevel = NoDebug;
QSet<QString> ReportHandler::m_reportedWarnings;
char ReportHandler::m_progressBuffer[1024] = {0};
ProgressAnimation ReportHandler::m_anim;


void ReportHandler::warning(const QString &text)
{
    if (m_silent)
        return;

// Context is useless!
//     QString warningText = QString("\r" COLOR_YELLOW "WARNING(%1)" COLOR_END " :: %2").arg(m_context).arg(text);
    QString warningText = QString("\033[1K\r" COLOR_YELLOW "WARNING" COLOR_END " :: %2").arg(text);

    TypeDatabase *db = TypeDatabase::instance();
    if (db && db->isSuppressedWarning(text)) {
        ++m_suppressedCount;
    } else if (!m_reportedWarnings.contains(text)) {
        puts(qPrintable(warningText));
        printProgress();
        ++m_warningCount;

        m_reportedWarnings.insert(text);
    }
}

void ReportHandler::progress(const QString& str, ...)
{
    if (m_silent)
        return;
    QString msg = QString("\033[1K\r" COLOR_WHITE "%1 (%2/%3) " COLOR_END).arg(m_anim.toString()).arg(m_anim.current()).arg(m_anim.max()) + str;
    va_list argp;
    va_start(argp, str);
    vsnprintf(m_progressBuffer, sizeof(m_progressBuffer), msg.toLocal8Bit().constData(), argp);
    va_end(argp);
    printProgress();
}

void ReportHandler::printProgress()
{
    printf(m_progressBuffer);
    fflush(stdout);
}


void ReportHandler::debug(DebugLevel level, const QString &text)
{
    if (m_debugLevel == NoDebug)
        return;

    if (level <= m_debugLevel) {
        printf("\r" COLOR_GREEN "DEBUG" COLOR_END " :: %-70s\n", qPrintable(text));
        printProgress();
    }
}
