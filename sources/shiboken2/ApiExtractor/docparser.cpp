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
#include "docparser.h"
#include "abstractmetalang.h"
#include "messages.h"
#include "reporthandler.h"
#include "typesystem.h"
#include "xmlutils.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QBuffer>

#include <cstdlib>
#ifdef HAVE_LIBXSLT
#  include <libxslt/xsltutils.h>
#  include <libxslt/transform.h>
#endif

#include <algorithm>

DocParser::DocParser()
{
#ifdef HAVE_LIBXSLT
    xmlSubstituteEntitiesDefault(1);
#endif
}

DocParser::~DocParser() = default;

QString DocParser::getDocumentation(const XQueryPtr &xquery, const QString& query,
                                    const DocModificationList& mods) const
{
    QString doc = execXQuery(xquery, query);
    return applyDocModifications(mods, doc.trimmed());
}

QString DocParser::execXQuery(const XQueryPtr &xquery, const QString& query) const
{
    QString errorMessage;
    const QString result = xquery->evaluate(query, &errorMessage);
    if (!errorMessage.isEmpty())
        qCWarning(lcShiboken, "%s", qPrintable(errorMessage));
    return result;
}

bool DocParser::skipForQuery(const AbstractMetaFunction *func)
{
    // Skip private functions and copies created by AbstractMetaClass::fixFunctions()
    if (!func || func->isPrivate()
        || (func->attributes() & AbstractMetaAttributes::AddedMethod) != 0
        || func->isModifiedRemoved()
        || func->declaringClass() != func->ownerClass()
        || func->isCastOperator()) {
        return true;
    }
    switch (func->functionType()) {
    case AbstractMetaFunction::MoveConstructorFunction:
    case AbstractMetaFunction::AssignmentOperatorFunction:
    case AbstractMetaFunction::MoveAssignmentOperatorFunction:
        return true;
    default:
        break;
    }
    return false;
}

AbstractMetaFunctionList DocParser::documentableFunctions(const AbstractMetaClass *metaClass)
{
    AbstractMetaFunctionList result = metaClass->functionsInTargetLang();
    for (int i = result.size() - 1; i >= 0; --i)  {
        if (DocParser::skipForQuery(result.at(i)) || result.at(i)->isUserAdded())
            result.removeAt(i);
    }
    return result;
}

static inline bool isXpathDocModification(const DocModification &mod)
{
    return mod.mode() == TypeSystem::DocModificationXPathReplace;
}

QString DocParser::applyDocModifications(const DocModificationList& mods, const QString& xml) const
{
    const char xslPrefix[] =
R"(<xsl:template match="/">
    <xsl:apply-templates />
</xsl:template>
<xsl:template match="*">
<xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
</xsl:copy>
</xsl:template>
)";

    if (mods.isEmpty() || xml.isEmpty()
        || !std::any_of(mods.cbegin(), mods.cend(), isXpathDocModification)) {
        return xml;
    }

    QString xsl = QLatin1String(xslPrefix);
    for (const DocModification &mod : mods) {
        if (isXpathDocModification(mod)) {
            QString xpath = mod.xpath();
            xpath.replace(QLatin1Char('"'), QLatin1String("&quot;"));
            xsl += QLatin1String("<xsl:template match=\"")
                   + xpath + QLatin1String("\">")
                   + mod.code() + QLatin1String("</xsl:template>\n");
        }
    }

    QString errorMessage;
    const QString result = xsl_transform(xml, xsl, &errorMessage);
    if (!errorMessage.isEmpty())
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgXpathDocModificationError(mods, errorMessage)));
    if (result == xml) {
        const QString message = QLatin1String("Query did not result in any modifications to \"")
            + xml + QLatin1Char('"');
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgXpathDocModificationError(mods, message)));
    }
    return result;
}
