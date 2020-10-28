/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "xmlutils.h"

#include "xmlutils_libxslt.h"

XQuery::XQuery() = default;

XQuery::~XQuery() = default;

QString XQuery::evaluate(QString xPathExpression, QString *errorMessage)
{
    // XQuery can't have invalid XML characters
    xPathExpression.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    xPathExpression.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    return doEvaluate(xPathExpression, errorMessage);
}

QSharedPointer<XQuery> XQuery::create(const QString &focus, QString *errorMessage)
{
#if defined(HAVE_LIBXSLT)
    return libXml_createXQuery(focus, errorMessage);
#else
    *errorMessage = QLatin1String(__FUNCTION__) + QLatin1String(" is not implemented.");
    return QSharedPointer<XQuery>();
#endif
}

QString xsl_transform(const QString &xml, const QString &xsl, QString *errorMessage)
{
#if defined(HAVE_LIBXSLT)
    return libXslt_transform(xml, xsl, errorMessage);
#else
    *errorMessage = QLatin1String(__FUNCTION__) + QLatin1String(" is not implemented.");
    return xml;
#endif
}
