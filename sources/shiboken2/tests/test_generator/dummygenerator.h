/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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
#ifndef DUMMYGENERATOR_H
#define DUMMYGENERATOR_H

#include "generator.h"

class GENRUNNER_API DummyGenerator : public Generator
{
public:
    DummyGenerator() {}
    ~DummyGenerator() {}
    bool doSetup(const QMap<QString, QString>& args);
    const char* name() const { return "DummyGenerator"; }

protected:
    void writeFunctionArguments(QTextStream&, const AbstractMetaFunction*, Options) const {}
    void writeArgumentNames(QTextStream&, const AbstractMetaFunction*, Options) const {}
    QString fileNameForClass(const AbstractMetaClass* metaClass) const;
    void generateClass(QTextStream& s, const AbstractMetaClass* metaClass);
    void finishGeneration() {}
};

#endif // DUMMYGENERATOR_H
