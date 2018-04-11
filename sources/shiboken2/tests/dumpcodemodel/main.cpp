/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#include <abstractmetabuilder_p.h>
#include <parser/codemodel.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include <iostream>
#include <algorithm>
#include <iterator>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.setApplicationDescription(QStringLiteral("Code model tester"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption verboseOption(QStringLiteral("d"),
                                     QStringLiteral("Display verbose output about types"));
    parser.addOption(verboseOption);
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("C++ source file"));

    parser.process(app);
    const QStringList &positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty())
        parser.showHelp(1);

    QByteArrayList arguments;
    std::transform(positionalArguments.cbegin(), positionalArguments.cend(),
                   std::back_inserter(arguments), QFile::encodeName);
    const FileModelItem dom = AbstractMetaBuilderPrivate::buildDom(arguments, 0);
    if (dom.isNull()) {
        QString message = QLatin1String("Unable to parse ") + positionalArguments.join(QLatin1Char(' '));
        std::cerr << qPrintable(message) << '\n';
        return -2;
    }

    QString output;
    {
        QDebug debug(&output);
        if (parser.isSet(verboseOption))
            debug.setVerbosity(3);
        debug << dom.data();
    }
    std::cout << qPrintable(output) << '\n';

    return 0;
}
