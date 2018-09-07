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
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtXmlPatterns/QXmlQuery>
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

QString DocParser::getDocumentation(QXmlQuery& xquery, const QString& query,
                                    const DocModificationList& mods) const
{
    QString doc = execXQuery(xquery, query);
    return applyDocModifications(mods, doc.trimmed());
}

QString DocParser::execXQuery(QXmlQuery& xquery, const QString& query) const
{
    QString escapedQuery(query);
    // XQuery can't have invalid XML characters
    escapedQuery.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    escapedQuery.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    xquery.setQuery(escapedQuery);
    if (!xquery.isValid()) {
        qWarning() << "Bad XQuery: " << escapedQuery;
        return QString();
    }

    QString result;
    xquery.evaluateTo(&result);
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


#ifdef HAVE_LIBXSLT
namespace
{

class XslResources
{
    Q_DISABLE_COPY(XslResources)

public:
    xmlDocPtr xmlDoc = nullptr;
    xsltStylesheetPtr xslt = nullptr;
    xmlDocPtr xslResult = nullptr;

    XslResources() = default;

    ~XslResources()
    {
        if (xslt)
            xsltFreeStylesheet(xslt);

        if (xslResult)
            xmlFreeDoc(xslResult);

        if (xmlDoc)
            xmlFreeDoc(xmlDoc);

        xsltCleanupGlobals();
        xmlCleanupParser();
    }
};

} // namespace
#endif // HAVE_LIBXSLT

static inline bool isXpathDocModification(const DocModification &mod)
{
    return mod.mode() == TypeSystem::DocModificationXPathReplace;
}

QString DocParser::applyDocModifications(const DocModificationList& mods, const QString& xml) const
{
    if (mods.isEmpty() || xml.isEmpty()
        || !std::any_of(mods.cbegin(), mods.cend(), isXpathDocModification)) {
        return xml;
    }
#ifdef HAVE_LIBXSLT
    const QString result = applyDocModificationsLibXsl(mods, xml);
#else
    const QString result = applyDocModificationsQt(mods, xml);
#endif
    if (result == xml) {
        const QString message = QLatin1String("Query did not result in any modifications to \"")
            + xml + QLatin1Char('"');
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgXpathDocModificationError(mods, message)));
    }
    return result;
}

QString DocParser::applyDocModificationsLibXsl(const DocModificationList& mods, const QString& xml) const
{
#ifdef HAVE_LIBXSLT
    QString xsl = QLatin1String("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                                "<xsl:transform version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
                                "<xsl:template match=\"/\">\n"
                                "    <xsl:apply-templates />\n"
                                "</xsl:template>\n"
                                "<xsl:template match=\"*\">\n"
                                "<xsl:copy>\n"
                                "    <xsl:copy-of select=\"@*\"/>\n"
                                "    <xsl:apply-templates/>\n"
                                "</xsl:copy>\n"
                                "</xsl:template>\n"
                               );
    for (const DocModification &mod : mods) {
        if (isXpathDocModification(mod)) {
            QString xpath = mod.xpath();
            xpath.replace(QLatin1Char('"'), QLatin1String("&quot;"));
            xsl += QLatin1String("<xsl:template match=\"")
                   + xpath + QLatin1String("\">")
                   + mod.code() + QLatin1String("</xsl:template>\n");
        }
    }
    xsl += QLatin1String("</xsl:transform>");

    XslResources res;
    // Read XML data
    QByteArray xmlData = xml.toUtf8();
    res.xmlDoc = xmlParseMemory(xmlData.constData(), xmlData.size());
    if (!res.xmlDoc)
        return xml;

    // Read XSL data as a XML file
    QByteArray xslData = xsl.toUtf8();
    // xsltFreeStylesheet will delete this pointer
    xmlDocPtr xslDoc = xmlParseMemory(xslData.constData(), xslData.size());
    if (!xslDoc)
        return xml;

    // Parse XSL data
    res.xslt = xsltParseStylesheetDoc(xslDoc);
    if (!res.xslt)
        return xml;

    // Apply XSL
    res.xslResult = xsltApplyStylesheet(res.xslt, res.xmlDoc, 0);
    xmlChar* buffer = 0;
    int bufferSize;
    QString result;
    if (!xsltSaveResultToString(&buffer, &bufferSize, res.xslResult, res.xslt)) {
        result = QString::fromUtf8(reinterpret_cast<char*>(buffer), bufferSize);
        std::free(buffer);
    } else {
        result = xml;
    }
    return result.trimmed();
#else // HAVE_LIBXSLT
    Q_UNUSED(mods)
    return xml;
#endif // !HAVE_LIBXSLT
}

QString DocParser::applyDocModificationsQt(const DocModificationList& mods, const QString& xml) const
{
    const char xslPrefix[] =
R"(<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="2.0">
    <xsl:template match="/">
        <xsl:apply-templates/>\n"
    </xsl:template>
    <xsl:template match="*">
        <xsl:copy>
            <xsl:copy-of select="@*"/>
        <xsl:apply-templates/>
    </xsl:copy>
    </xsl:template>
)";

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
    xsl += QLatin1String("</xsl:stylesheet>");

    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(xml);
    query.setQuery(xsl);
    if (!query.isValid()) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgXpathDocModificationError(mods, QLatin1String("Invalid query."))));
        return xml;
    }
    QString result;
    if (!query.evaluateTo(&result)) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgXpathDocModificationError(mods, QLatin1String("evaluate() failed."))));
        return xml;
    }
    return result.trimmed();
}
