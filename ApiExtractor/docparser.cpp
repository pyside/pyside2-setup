/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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
#include "typesystem.h"
#include <QtCore/QDebug>
#include <QtXmlPatterns/QXmlQuery>
#include <QBuffer>

#include <cstdlib>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

DocParser::DocParser()
{
    xmlSubstituteEntitiesDefault(1);
}

DocParser::~DocParser()
{
}

QString DocParser::getDocumentation(QXmlQuery& xquery, const QString& query,
                                    const DocModificationList& mods) const
{
    QString doc = execXQuery(xquery, query);
    return applyDocModifications(mods, doc);
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

namespace
{

struct XslResources
{
    xmlDocPtr xmlDoc;
    xsltStylesheetPtr xslt;
    xmlDocPtr xslResult;

    XslResources() : xmlDoc(0), xslt(0), xslResult(0) {}

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

QString DocParser::applyDocModifications(const DocModificationList& mods, const QString& xml) const
{
    if (mods.isEmpty())
        return xml;

    bool hasXPathBasedModification = false;
    foreach (DocModification mod, mods) {
        if (mod.mode() == TypeSystem::DocModificationXPathReplace) {
            hasXPathBasedModification = true;
            break;
        }
    }

    if (!hasXPathBasedModification)
        return xml;

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
    foreach (DocModification mod, mods) {
        if (mod.mode() == TypeSystem::DocModificationXPathReplace) {
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

    Q_ASSERT(result != xml);
    return result;
}

