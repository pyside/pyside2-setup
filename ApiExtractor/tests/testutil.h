/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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

#ifndef TESTUTIL_H
#define TESTUTIL_H
#include <QtCore/QBuffer>
#include "abstractmetabuilder.h"
#include "reporthandler.h"
#include "typedatabase.h"

class TestUtil
{
public:
    TestUtil(const char* cppCode, const char* xmlCode,
             bool silent = true, const char* apiVersion = 0,
             QStringList dropTypeEntries = QStringList())
        : m_builder(0)
    {
        ReportHandler::setSilent(silent);
        m_builder = new AbstractMetaBuilder;
        TypeDatabase* td = TypeDatabase::instance(true);
        if (apiVersion)
            td->setApiVersion(QLatin1String("*"), apiVersion);
        td->setDropTypeEntries(dropTypeEntries);
        QBuffer buffer;
        // parse typesystem
        buffer.setData(xmlCode);
        td->parseFile(&buffer);
        buffer.close();
        // parse C++ code
        buffer.setData(cppCode);
        bool res = m_builder->build(&buffer);
        Q_UNUSED(res);
        Q_ASSERT(res);
    }

    ~TestUtil()
    {
        delete m_builder;
        m_builder = 0;
    }

    AbstractMetaBuilder* builder()
    {
        return m_builder;
    }

private:
    AbstractMetaBuilder* m_builder;
};

#endif
