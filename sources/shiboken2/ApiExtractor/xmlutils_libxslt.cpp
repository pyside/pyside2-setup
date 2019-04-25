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

#include "xmlutils_libxslt.h"
#include "xmlutils.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>

#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

#include <libxml/xmlsave.h>
#include <libxml/xpath.h>

#include <cstdlib>
#include <memory>

static void cleanup()
{
    xsltCleanupGlobals();
    xmlCleanupParser();
}

static void ensureInitialized()
{
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        xmlInitParser();
        xsltInit();
        qAddPostRoutine(cleanup);
    }
}

namespace {

// RAI Helpers for cleaning up libxml2/libxslt data

struct XmlDocDeleter // for std::unique_ptr<xmlDoc>
{
    void operator()(xmlDocPtr xmlDoc) { xmlFreeDoc(xmlDoc); }
};

struct XmlXPathObjectDeleter
{
    void operator()(xmlXPathObjectPtr xPathObject) { xmlXPathFreeObject(xPathObject); }
};

struct XmlStyleSheetDeleter // for std::unique_ptr<xsltStylesheet>
{
    void operator()(xsltStylesheetPtr xslt) { xsltFreeStylesheet(xslt); }
};

struct XmlXPathContextDeleter
{
    void operator()(xmlXPathContextPtr xPathContext) { xmlXPathFreeContext(xPathContext); }
};

} // namespace

using XmlDocUniquePtr = std::unique_ptr<xmlDoc, XmlDocDeleter>;
using XmlPathObjectUniquePtr = std::unique_ptr<xmlXPathObject, XmlXPathObjectDeleter>;
using XmlStyleSheetUniquePtr = std::unique_ptr<xsltStylesheet, XmlStyleSheetDeleter>;
using XmlXPathContextUniquePtr = std::unique_ptr<xmlXPathContext, XmlXPathContextDeleter>;

// Helpers for formatting nodes obtained from XPATH queries

static int qbXmlOutputWriteCallback(void *context, const char *buffer, int len)
{
    static_cast<QByteArray *>(context)->append(buffer, len);
    return len;
}

static int qbXmlOutputCloseCallback(void * /* context */) { return 0; }

static QByteArray formatNode(xmlNodePtr node, QString *errorMessage)
{
    QByteArray result;
    xmlSaveCtxtPtr saveContext =
        xmlSaveToIO(qbXmlOutputWriteCallback, qbXmlOutputCloseCallback,
                    &result, "UTF-8", 0);
    if (!saveContext) {
        *errorMessage = QLatin1String("xmlSaveToIO() failed.");
        return result;
    }
    const long saveResult = xmlSaveTree(saveContext, node);
    xmlSaveClose(saveContext);
    if (saveResult < 0)
        *errorMessage = QLatin1String("xmlSaveTree() failed.");
    return result;
}

// XPath expressions
class LibXmlXQuery : public XQuery
{
public:
    explicit LibXmlXQuery(XmlDocUniquePtr &doc, XmlXPathContextUniquePtr &xpathContext) :
       m_doc(std::move(doc)), m_xpathContext(std::move(xpathContext))
    {
        ensureInitialized();
    }

protected:
    QString doEvaluate(const QString &xPathExpression, QString *errorMessage) override;

private:
    XmlDocUniquePtr m_doc;
    XmlXPathContextUniquePtr m_xpathContext;
};

QString LibXmlXQuery::doEvaluate(const QString &xPathExpression, QString *errorMessage)
{
    const QByteArray xPathExpressionB = xPathExpression.toUtf8();
    auto xPathExpressionX = reinterpret_cast<const xmlChar *>(xPathExpressionB.constData());

    XmlPathObjectUniquePtr xPathObject(xmlXPathEvalExpression(xPathExpressionX, m_xpathContext.get()));
    if (!xPathObject) {
         *errorMessage = QLatin1String("xmlXPathEvalExpression() failed for \"") + xPathExpression
                         + QLatin1Char('"');
        return QString();
    }
    QString result;
    if (xmlNodeSetPtr nodeSet = xPathObject->nodesetval) {
        for (int n = 0, count = nodeSet->nodeNr; n < count; ++n) {
            auto node = nodeSet->nodeTab[n];
            if (node->type == XML_ELEMENT_NODE) {
                result += QString::fromLocal8Bit(formatNode(node, errorMessage));
                if (!errorMessage->isEmpty())
                    return QString();
            }
        }
    }
    return result;
}

QSharedPointer<XQuery> libXml_createXQuery(const QString &focus, QString *errorMessage)
{
    XmlDocUniquePtr doc(xmlParseFile(QFile::encodeName(focus).constData()));
    if (!doc) {
        *errorMessage = QLatin1String("libxml2: Cannot set focus to ") + QDir::toNativeSeparators(focus);
        return {};
    }
    XmlXPathContextUniquePtr xpathContext(xmlXPathNewContext(doc.get()));
    if (!xpathContext) {
        *errorMessage = QLatin1String("libxml2: xmlXPathNewContext() failed");
        return {};
    }
    return QSharedPointer<XQuery>(new LibXmlXQuery(doc, xpathContext));
}

// XSLT transformation

static const char xsltPrefix[] = R"(<?xml version="1.0" encoding="UTF-8" ?>
    <xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
)";

QString libXslt_transform(const QString &xml, QString xsl, QString *errorMessage)
{
    ensureInitialized();
    // Read XML data
    if (!xsl.startsWith(QLatin1String("<?xml"))) {
        xsl.prepend(QLatin1String(xsltPrefix));
        xsl.append(QLatin1String("</xsl:transform>"));
    }
    const QByteArray xmlData = xml.toUtf8();
    XmlDocUniquePtr xmlDoc(xmlParseMemory(xmlData.constData(), xmlData.size()));
    if (!xmlDoc) {
        *errorMessage = QLatin1String("xmlParseMemory() failed for XML.");
        return xml;
    }

    // Read XSL data as a XML file
    const QByteArray xslData = xsl.toUtf8();
    // xsltFreeStylesheet will delete this pointer
    xmlDocPtr xslDoc = xmlParseMemory(xslData.constData(), xslData.size());
    if (!xslDoc) {
        *errorMessage = QLatin1String("xmlParseMemory() failed for XSL \"")
            + xsl + QLatin1String("\".");
        return xml;
    };

    // Parse XSL data
    XmlStyleSheetUniquePtr xslt(xsltParseStylesheetDoc(xslDoc));
    if (!xslt) {
        *errorMessage = QLatin1String("xsltParseStylesheetDoc() failed.");
        return xml;
    }

    // Apply XSL
    XmlDocUniquePtr xslResult(xsltApplyStylesheet(xslt.get(), xmlDoc.get(), nullptr));
    xmlChar *buffer = nullptr;
    int bufferSize;
    QString result;
    if (xsltSaveResultToString(&buffer, &bufferSize, xslResult.get(), xslt.get()) == 0) {
        result = QString::fromUtf8(reinterpret_cast<char*>(buffer), bufferSize);
        std::free(buffer);
    } else {
        *errorMessage = QLatin1String("xsltSaveResultToString() failed.");
        result = xml;
    }
    return result.trimmed();
}
