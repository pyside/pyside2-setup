/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef QTXMLTOSPHINX_H
#define QTXMLTOSPHINX_H

#include "indentor.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QScopedPointer>
#include <QtCore/QStack>
#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE
class QXmlStreamReader;
QT_END_NAMESPACE
class QtDocGenerator;

class QtXmlToSphinx
{
public:
    Q_DISABLE_COPY_MOVE(QtXmlToSphinx)

    struct LinkContext;

    struct InlineImage
    {
        QString tag;
        QString href;
    };

    struct TableCell
    {
        short rowSpan = 0;
        short colSpan = 0;
        QString data;

        TableCell(const QString& text = QString()) : data(text) {}
        TableCell(const char* text) : data(QLatin1String(text)) {}
    };

    using TableRow = QList<TableCell>;

    class Table
    {
        public:
            Table() = default;

            bool isEmpty() const { return m_rows.isEmpty(); }

            void setHeaderEnabled(bool enable)
            {
                m_hasHeader = enable;
            }

            bool hasHeader() const
            {
                return m_hasHeader;
            }

            void normalize();

            bool isNormalized() const
            {
                return m_normalized;
            }

            void clear() {
                m_normalized = false;
                m_rows.clear();
            }

            void appendRow(const TableRow &row) { m_rows.append(row); }

            const TableRow &constFirst() { return m_rows.constFirst(); }
            TableRow &first() { return m_rows.first(); }
            TableRow &last() { return m_rows.last(); }

            void format(QTextStream& s, const Indentor &indent) const;

        private:
            QList<TableRow> m_rows;
            bool m_hasHeader = false;
            bool m_normalized = false;
    };

    explicit QtXmlToSphinx(const QtDocGenerator *generator,
                           Indentor &indentor,
                           const QString& doc,
                           const QString& context = QString());
    ~QtXmlToSphinx();

    static bool convertToRst(const QtDocGenerator *generator,
                             Indentor &indentor,
                             const QString &sourceFileName,
                             const QString &targetFileName,
                             const QString &context = QString(),
                             QString *errorMessage = nullptr);

    QString result() const
    {
        return m_result;
    }

    static void stripPythonQualifiers(QString *s);

private:
    QString resolveContextForMethod(const QString& methodName) const;
    QString expandFunction(const QString& function) const;
    QString transform(const QString& doc);

    void handleHeadingTag(QXmlStreamReader& reader);
    void handleParaTag(QXmlStreamReader& reader);
    void handleItalicTag(QXmlStreamReader& reader);
    void handleBoldTag(QXmlStreamReader& reader);
    void handleArgumentTag(QXmlStreamReader& reader);
    void handleSeeAlsoTag(QXmlStreamReader& reader);
    void handleSnippetTag(QXmlStreamReader& reader);
    void handleDotsTag(QXmlStreamReader& reader);
    void handleLinkTag(QXmlStreamReader& reader);
    void handleImageTag(QXmlStreamReader& reader);
    void handleInlineImageTag(QXmlStreamReader& reader);
    void handleListTag(QXmlStreamReader& reader);
    void handleTermTag(QXmlStreamReader& reader);
    void handleSuperScriptTag(QXmlStreamReader& reader);
    void handleQuoteFileTag(QXmlStreamReader& reader);

    // table tagsvoid QtXmlToSphinx::handleValueTag(QXmlStreamReader& reader)

    void handleTableTag(QXmlStreamReader& reader);
    void handleRowTag(QXmlStreamReader& reader);
    void handleItemTag(QXmlStreamReader& reader);
    void handleRawTag(QXmlStreamReader& reader);
    void handleCodeTag(QXmlStreamReader& reader);
    void handlePageTag(QXmlStreamReader&);
    void handleTargetTag(QXmlStreamReader&);

    void handleIgnoredTag(QXmlStreamReader& reader);
    void handleUnknownTag(QXmlStreamReader& reader);
    void handleUselessTag(QXmlStreamReader& reader);
    void handleAnchorTag(QXmlStreamReader& reader);
    void handleRstPassTroughTag(QXmlStreamReader& reader);

    LinkContext *handleLinkStart(const QString &type, QString ref) const;
    void handleLinkText(LinkContext *linkContext, const QString &linktext) const;
    void handleLinkEnd(LinkContext *linkContext);

    typedef void (QtXmlToSphinx::*TagHandler)(QXmlStreamReader&);
    QHash<QString, TagHandler> m_handlerMap;
    QStack<TagHandler> m_handlers;
    QTextStream m_output;
    QString m_result;

    QStack<QString*> m_buffers;


    Table m_currentTable;
    QScopedPointer<LinkContext> m_linkContext; // for <link>
    QScopedPointer<LinkContext> m_seeAlsoContext; // for <see-also>foo()</see-also>
    bool m_tableHasHeader;
    QString m_context;
    const QtDocGenerator* m_generator;
    Indentor &INDENT;
    bool m_insideBold;
    bool m_insideItalic;
    QString m_lastTagName;
    QString m_opened_anchor;
    QList<InlineImage> m_inlineImages;

    QString readFromLocations(const QStringList &locations, const QString &path,
                              const QString &identifier, QString *errorMessage);
    QString readFromLocation(const QString &location, const QString &identifier,
                             QString *errorMessage);
    void pushOutputBuffer();
    QString popOutputBuffer();
    void writeTable(Table& table);
    bool copyImage(const QString &href) const;
};

inline QTextStream& operator<<(QTextStream& s, const QtXmlToSphinx& xmlToSphinx)
{
    return s << xmlToSphinx.result();
}

#endif // QTXMLTOSPHINX_H
