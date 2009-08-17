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

#ifndef REPORTHANDLER_H
#define REPORTHANDLER_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <cstring>

class ProgressAnimation
{
public:
    ProgressAnimation()
    {
        anim_data = "|/-\\";
        anim_frame = anim_data;
        std::strcpy(anim_string, "[ ]");
        m_current = m_max = 0;
    }
    const char* toString()
    {
        step();
        return anim_string;
    }
    template<typename T>
    void setCollection(T collection)
    {
        m_current = 1;
        m_max = collection.count();
    }

    int current() const
    {
        return m_current;
    }
    int max() const
    {
        return m_max;
    }

private:
    const char* anim_data;
    char anim_string[4];
    const char* anim_frame;
    int m_max;
    int m_current;

    void step()
    {
        if (!*(++anim_frame))
            anim_frame = anim_data;
        anim_string[1] = *anim_frame;
        m_current++;
    }
};

class ReportHandler
{
public:
    enum DebugLevel { NoDebug, SparseDebug, MediumDebug, FullDebug };

    static void setContext(const QString &context)
    {
        m_context = context;
    }

    static DebugLevel debugLevel()
    {
        return m_debugLevel;
    }
    static void setDebugLevel(DebugLevel level)
    {
        m_debugLevel = level;
    }

    static int warningCount()
    {
        return m_warningCount;
    }

    static int suppressedCount()
    {
        return m_suppressedCount;
    }

    static void warning(const QString &str);

    template <typename T>
    static void setProgressReference(T collection)
    {
        m_anim.setCollection(collection);
    }

    static void progress(const QString &str, ...);

    static void debugSparse(const QString &str)
    {
        debug(SparseDebug, str);
    }
    static void debugMedium(const QString &str)
    {
        debug(MediumDebug, str);
    }
    static void debugFull(const QString &str)
    {
        debug(FullDebug, str);
    }
    static void debug(DebugLevel level, const QString &str);

    static bool isSilent()
    {
        return m_silent;
    }
    static void setSilent(bool silent)
    {
        m_silent = silent;
    }

private:
    static bool m_silent;
    static int m_warningCount;
    static int m_suppressedCount;
    static DebugLevel m_debugLevel;
    static QString m_context;
    static QSet<QString> m_reportedWarnings;

    static ProgressAnimation m_anim;
    static char m_progressBuffer[1024];

    static void printProgress();
};

#endif // REPORTHANDLER_H
