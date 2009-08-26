/*
 * This file is part of the Boost Python Generator project.
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

#include "qtdocgenerator.h"
#include <reporthandler.h>
#include <qtdocparser.h>
#include <algorithm>
#include <QtCore/QStack>
#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QFile>
#include <QtCore/QDir>

EXPORT_GENERATOR_PLUGIN(new QtDocGenerator)

static Indentor INDENT;

namespace
{

static bool functionSort(const AbstractMetaFunction *func1, const AbstractMetaFunction *func2)
{
    return func1->name() < func2->name();
}

QString createRepeatedChar(int i, char c)
{
    QString out;
    for (int j = 0; j < i; ++j)
        out += c;

    return out;
}

QString escape(QString& str)
{
    return str
            .replace("*", "\\*")
            .replace("_", "\\_");
}

QString escape(const QStringRef& strref)
{
    QString str = strref.toString();
    return escape(str);
}

}

QtXmlToSphinx::QtXmlToSphinx(QtDocGenerator* generator, const QString& doc, const QString& context)
        : m_context(context), m_generator(generator), m_insideBold(false), m_insideItalic(false)
{
    m_handlerMap.insert("heading", &QtXmlToSphinx::handleHeadingTag);
    m_handlerMap.insert("brief", &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert("para", &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert("italic", &QtXmlToSphinx::handleItalicTag);
    m_handlerMap.insert("bold", &QtXmlToSphinx::handleBoldTag);
    m_handlerMap.insert("see-also", &QtXmlToSphinx::handleSeeAlsoTag);
    m_handlerMap.insert("snippet", &QtXmlToSphinx::handleSnippetTag);
    m_handlerMap.insert("dots", &QtXmlToSphinx::handleDotsTag);
    m_handlerMap.insert("codeline", &QtXmlToSphinx::handleDotsTag);
    m_handlerMap.insert("table", &QtXmlToSphinx::handleTableTag);
    m_handlerMap.insert("header", &QtXmlToSphinx::handleRowTag);
    m_handlerMap.insert("row", &QtXmlToSphinx::handleRowTag);
    m_handlerMap.insert("item", &QtXmlToSphinx::handleItemTag);
    m_handlerMap.insert("argument", &QtXmlToSphinx::handleArgumentTag);
    m_handlerMap.insert("teletype", &QtXmlToSphinx::handleArgumentTag);
    m_handlerMap.insert("link", &QtXmlToSphinx::handleLinkTag);
    m_handlerMap.insert("inlineimage", &QtXmlToSphinx::handleImageTag);
    m_handlerMap.insert("image", &QtXmlToSphinx::handleImageTag);
    m_handlerMap.insert("list", &QtXmlToSphinx::handleListTag);
    m_handlerMap.insert("term", &QtXmlToSphinx::handleTermTag);
    m_handlerMap.insert("raw", &QtXmlToSphinx::handleRawTag);
    m_handlerMap.insert("underline", &QtXmlToSphinx::handleItalicTag);
    m_handlerMap.insert("superscript", &QtXmlToSphinx::handleSuperScriptTag);
    m_handlerMap.insert("code", &QtXmlToSphinx::handleCodeTag);
    m_handlerMap.insert("legalese", &QtXmlToSphinx::handleCodeTag);
    m_handlerMap.insert("section", &QtXmlToSphinx::handleAnchorTag);
    m_handlerMap.insert("quotefile", &QtXmlToSphinx::handleQuoteFileTag);

    // ignored tags
    m_handlerMap.insert("generatedlist", &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert("tableofcontents", &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert("quotefromfile", &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert("skipto", &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert("target", &QtXmlToSphinx::handleIgnoredTag);

    // useless tags
    m_handlerMap.insert("description", &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert("definition", &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert("printuntil", &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert("relation", &QtXmlToSphinx::handleUselessTag);

    m_result = transform(doc);
}

void QtXmlToSphinx::pushOutputBuffer()
{
    QString* buffer = new QString();
    m_buffers << buffer;
    m_output.setString(buffer);
}

QString QtXmlToSphinx::popOutputBuffer()
{
    Q_ASSERT(!m_buffers.isEmpty());
    QString* str = m_buffers.pop();
    QString strcpy(*str);
    delete str;
    m_output.setString(m_buffers.isEmpty() ? 0 : m_buffers.top());
    return strcpy;
}


QString QtXmlToSphinx::transform(const QString& doc)
{
    Q_ASSERT(m_buffers.isEmpty());
    Indentation indentation(INDENT);
    if (doc.trimmed().isEmpty())
        return doc;

    pushOutputBuffer();

    QXmlStreamReader reader(doc);

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (reader.hasError()) {
            m_output << INDENT << "XML Error: " + reader.errorString() + "\n" + doc;
            ReportHandler::warning("XML Error: " + reader.errorString() + "\n" + doc);
            break;
        }

        if (token == QXmlStreamReader::StartElement) {
            QStringRef tagName = reader.name();
            TagHandler handler = m_handlerMap.value(tagName.toString(), &QtXmlToSphinx::handleUnknownTag);
            if (!m_handlers.isEmpty() && ( (m_handlers.top() == &QtXmlToSphinx::handleIgnoredTag) ||
                                           (m_handlers.top() == &QtXmlToSphinx::handleRawTag)) )
                handler = &QtXmlToSphinx::handleIgnoredTag;

            m_handlers.push(handler);
        }
        if (!m_handlers.isEmpty())
            (this->*(m_handlers.top()))(reader);

        if (token == QXmlStreamReader::EndElement) {
            m_handlers.pop();
            m_lastTagName = reader.name().toString();
        }
    }
    m_output.flush();
    QString retval = popOutputBuffer();
    Q_ASSERT(m_buffers.isEmpty());
    return retval;
}

QString QtXmlToSphinx::readFromLocation(QString& location, QString& identifier)
{
    QFile inputFile;
    inputFile.setFileName(location);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        ReportHandler::warning("Couldn't read code snippet file: "+inputFile.fileName());
        return QString();
    }

    QRegExp searchString("//!\\s*\\[" + identifier + "\\]");
    QRegExp codeSnippetCode("//!\\s*\\[[\\w\\d\\s]+\\]");
    QString code;
    QString line;
    bool identifierIsEmpty = identifier.isEmpty();
    bool getCode = false;

    while (!inputFile.atEnd()) {
        line = inputFile.readLine();
        if (identifierIsEmpty)
            code += line;
        else if (getCode && !line.contains(searchString))
            code += line.replace(codeSnippetCode, "");
        else if (line.contains(searchString))
            if (getCode)
                break;
            else
                getCode = true;
    }

    if (code.isEmpty())
        ReportHandler::warning("Code snippet file found ("+location+"), but snippet "+ identifier +" not found.");

    return code;
}

void QtXmlToSphinx::handleHeadingTag(QXmlStreamReader& reader)
{
    static QString heading;
    static char type;
    static char types[] = { '-', '^' };
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        int typeIdx = reader.attributes().value("level").toString().toInt();
        if (typeIdx >= sizeof(types))
            type = types[sizeof(types)-1];
        else
            type = types[typeIdx];
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << createRepeatedChar(heading.length(), type) << endl << endl;
    } else if (token == QXmlStreamReader::Characters) {
        heading = escape(reader.text()).trimmed();
        m_output << endl << endl << heading << endl;
    }
}

void QtXmlToSphinx::handleParaTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        pushOutputBuffer();
    } else if (token == QXmlStreamReader::EndElement) {
        QString result = popOutputBuffer().simplified();
        if (result.startsWith("**Warning:**"))
            result.replace(0, 12, ".. warning:: ");
        else if (result.startsWith("**Note:**"))
            result.replace(0, 9, ".. note:: ");

        m_output << INDENT << result << endl << endl;
    } else if (token == QXmlStreamReader::Characters) {
        QString text = escape(reader.text());
        if (!m_output.string()->isEmpty()) {
            QChar start = text[0];
            QChar end = m_output.string()->at(m_output.string()->length() - 1);
            if ((end == '*' || end == '`') && start != ' ' && !start.isPunct())
                m_output << '\\';
        }
        m_output << INDENT << text;
    }
}

void QtXmlToSphinx::handleItalicTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement) {
        m_insideItalic = !m_insideItalic;
        m_output << '*';
    } else if (token == QXmlStreamReader::Characters) {
        m_output << escape(reader.text()).trimmed();
    }
}

void QtXmlToSphinx::handleBoldTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement) {
        m_insideBold = !m_insideBold;
        m_output << "**";
    } else if (token == QXmlStreamReader::Characters) {
        m_output << escape(reader.text()).trimmed();
    }
}

void QtXmlToSphinx::handleArgumentTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement)
        m_output << "``";
    else if (token == QXmlStreamReader::Characters)
        m_output << reader.text().toString().trimmed();
}

void QtXmlToSphinx::handleSeeAlsoTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement)
        m_output << INDENT << ".. seealso:: ";
    else if (token == QXmlStreamReader::EndElement)
        m_output << endl;
}

void QtXmlToSphinx::handleSnippetTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        bool consecutiveSnippet = m_lastTagName == "snippet" || m_lastTagName == "dots" || m_lastTagName == "codeline";
        if (consecutiveSnippet) {
            m_output.flush();
            m_output.string()->chop(2);
        }
        QString location = reader.attributes().value("location").toString();
        QString identifier = reader.attributes().value("identifier").toString();
        location.prepend(m_generator->codeSnippetDir() + '/');
        QString code = readFromLocation(location, identifier);
        if (!consecutiveSnippet)
            m_output << INDENT << "::\n\n";

        Indentation indentation(INDENT);
        if (code.isEmpty()) {
            m_output << INDENT << "<Code snippet \"" << location << ':' << identifier << "\" not found>" << endl;
        } else {
            foreach (QString line, code.split("\n")) {
                if (!QString(line).trimmed().isEmpty())
                    m_output << INDENT << line;

                m_output << endl;
            }
        }
        m_output << endl;
    }
}
void QtXmlToSphinx::handleDotsTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        bool consecutiveSnippet = m_lastTagName == "snippet" || m_lastTagName == "dots" || m_lastTagName == "codeline";
        if (consecutiveSnippet) {
            m_output.flush();
            m_output.string()->chop(2);
        }
        Indentation indentation(INDENT);
        pushOutputBuffer();
        m_output << INDENT;
        int indent = reader.attributes().value("indent").toString().toInt();
        for (int i = 0; i < indent; ++i)
            m_output << ' ';
    } else if (token == QXmlStreamReader::Characters) {
        m_output << reader.text().toString();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << popOutputBuffer() << "\n\n\n";
    }
}

void QtXmlToSphinx::handleTableTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        m_currentTable.clear();
        m_tableHasHeader = false;
    } else if (token == QXmlStreamReader::EndElement) {
        // write the table on m_output
        m_currentTable.enableHeader(m_tableHasHeader);
        m_currentTable.normalize();
        m_output << m_currentTable;
        m_currentTable.clear();
    }
}

void QtXmlToSphinx::handleTermTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        pushOutputBuffer();
    } else if (token == QXmlStreamReader::Characters) {
        m_output << reader.text().toString().replace("::", ".");
    } else if (token == QXmlStreamReader::EndElement) {
        TableCell cell;
        cell.data = popOutputBuffer().trimmed();
        m_currentTable << (TableRow() << cell);
    }
}


void QtXmlToSphinx::handleItemTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        if (m_currentTable.isEmpty())
            m_currentTable << TableRow();
        TableRow& row = m_currentTable.last();
        TableCell cell;
        cell.colSpan = reader.attributes().value("colspan").toString().toShort();
        cell.rowSpan = reader.attributes().value("rowspan").toString().toShort();
        row << cell;
        pushOutputBuffer();
    } else if (token == QXmlStreamReader::EndElement) {
        QString data = popOutputBuffer().trimmed();
        if (!m_currentTable.isEmpty()) {
            TableRow& row = m_currentTable.last();
            if (!row.isEmpty())
                row.last().data = data;
        }
    }
}

void QtXmlToSphinx::handleRowTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        m_tableHasHeader = reader.name() == "header";
        m_currentTable << TableRow();
    }
}

void QtXmlToSphinx::handleListTag(QXmlStreamReader& reader)
{
    // BUG We do not support a list inside a table cell
    static QString listType;
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        listType = reader.attributes().value("type").toString();
        if (listType == "enum") {
            m_currentTable << (TableRow() << "Constant" << "Description");
            m_tableHasHeader = true;
        }
        INDENT.indent--;
    } else if (token == QXmlStreamReader::EndElement) {
        INDENT.indent++;
        if (!m_currentTable.isEmpty()) {
            if (listType == "bullet") {
                m_output << endl;
                foreach (TableCell cell, m_currentTable.first()) {
                    QStringList itemLines = cell.data.split('\n');
                    m_output << INDENT << "* " << itemLines.first() << endl;
                    for (int i = 1, max = itemLines.count(); i < max; ++i)
                        m_output << INDENT << "  " << itemLines[i] << endl;
                }
                m_output << endl;
            } else if (listType == "enum") {
                m_currentTable.enableHeader(m_tableHasHeader);
                m_currentTable.normalize();
                m_output << m_currentTable;
            }
        }
        m_currentTable.clear();
    }
}

void QtXmlToSphinx::handleLinkTag(QXmlStreamReader& reader)
{
    static QString l_linktag;
    static QString l_linkref;
    static QString l_linktext;
    static QString l_linktagending;
    static QString l_type;
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        l_linktagending = "` ";
        if (m_insideBold) {
            l_linktag.prepend("**");
            l_linktagending.append("**");
        } else if (m_insideItalic) {
            l_linktag.prepend('*');
            l_linktagending.append('*');
        }
        l_type = reader.attributes().value("type").toString();

        // TODO: create a flag PROPERTY-AS-FUNCTION to ask if the properties
        // are recognized as such or not in the binding
        if (l_type == "property")
            l_type = "function";

        if (l_type == "typedef")
            l_type = "class";

        QString linkSource;
        if (l_type == "function" || l_type == "class") {
            linkSource  = "raw";
        } else if (l_type == "enum") {
            linkSource  = "enum";
        } else if (l_type == "page") {
            linkSource  = "page";
        } else {
            linkSource = "href";
        }

        l_linkref = reader.attributes().value(linkSource).toString();
        l_linkref.replace("::", ".");
        l_linkref.remove("()");

        if (l_type == "function" && !m_context.isEmpty()) {
            l_linktag = " :meth:`";
            QStringList rawlinklist = l_linkref.split(".");
            if (rawlinklist.size() == 1 || rawlinklist[0] == m_context)
                l_linkref.prepend("~" + m_context + '.');
        } else if (l_type == "function" && m_context.isEmpty()) {
            l_linktag = " :func:`";
        } else if (l_type == "class") {
            l_linktag = " :class:`";
        } else if (l_type == "enum") {
            l_linktag = " :attr:`";
        } else if (l_type == "page" && l_linkref == m_generator->moduleName()) {
            l_linktag = " :mod:`";
        } else {
            l_linktag = " :ref:`";
        }

    } else if (token == QXmlStreamReader::Characters) {
        QString linktext = reader.text().toString();
        linktext.replace("::", ".");
        QString item = l_linkref.split(".").last();
        if (l_linkref == linktext
            || (l_linkref + "()") == linktext
            || item == linktext
            || (item + "()") == linktext)
            l_linktext.clear();
        else
            l_linktext = linktext + QLatin1String("<");
    } else if (token == QXmlStreamReader::EndElement) {
        if (!l_linktext.isEmpty())
            l_linktagending.prepend('>');
        m_output << l_linktag << l_linktext << escape(l_linkref) << l_linktagending;
    }
}

void QtXmlToSphinx::handleImageTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString href = reader.attributes().value("href").toString();
        QDir dir(m_generator->outputDirectory() + '/' + m_generator->packageName().replace(".", "/"));
        QString imgPath = dir.relativeFilePath(m_generator->libSourceDir() + "/doc/src/") + '/' + href;

        if (reader.name() == "image")
            m_output << INDENT << ".. image:: " <<  imgPath << endl << endl;
        else
            m_output << ".. image:: " << imgPath << ' ';
    }
}

void QtXmlToSphinx::handleRawTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString format = reader.attributes().value("format").toString();
        m_output << INDENT << ".. raw:: " << format.toLower() << endl << endl;
    } else if (token == QXmlStreamReader::Characters) {
        QStringList lst(reader.text().toString().split("\n"));
        foreach(QString row, lst)
            m_output << INDENT << INDENT << row << endl;
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << endl << endl;
    }
}

void QtXmlToSphinx::handleCodeTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString format = reader.attributes().value("format").toString();
        m_output << INDENT << "::" << endl << endl;
        INDENT.indent++;
    } else if (token == QXmlStreamReader::Characters) {
        QStringList lst(reader.text().toString().split("\n"));
        foreach(QString row, lst)
            m_output << INDENT << INDENT << row << endl;
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << endl << endl;
        INDENT.indent--;
    }
}

void QtXmlToSphinx::handleUnknownTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement)
        ReportHandler::warning("Unknow QtDoc tag: \"" + reader.name().toString() + "\".");
}

void QtXmlToSphinx::handleSuperScriptTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        m_output << " :sup:`";
        pushOutputBuffer();
    } else if (token == QXmlStreamReader::Characters) {
        m_output << reader.text().toString();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << popOutputBuffer();
        m_output << '`';
    }
}

void QtXmlToSphinx::handleIgnoredTag(QXmlStreamReader&)
{
}

void QtXmlToSphinx::handleUselessTag(QXmlStreamReader&)
{
    // Tag "description" just marks the init of "Detailed description" title.
    // Tag "definition" just marks enums. We have a different way to process them.
}

void QtXmlToSphinx::handleAnchorTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString anchor;
        if (reader.attributes().hasAttribute("id"))
            anchor = reader.attributes().value("id").toString();
        else if (reader.attributes().hasAttribute("name"))
            anchor = reader.attributes().value("name").toString();
        if (!anchor.isEmpty() && m_opened_anchor != anchor) {
            m_opened_anchor = anchor;
            m_output << INDENT << ".. _" << m_context << "_" << anchor.toLower() << ":" << endl << endl;
        }
   } else if (token == QXmlStreamReader::EndElement) {
       m_opened_anchor = "";
   }
}

void QtXmlToSphinx::handleQuoteFileTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::Characters) {
        QString location = reader.text().toString();
        QString identifier = "";
        location.prepend(m_generator->libSourceDir() + '/');
        QString code = readFromLocation(location, identifier);

        m_output << INDENT << "::\n\n";
        Indentation indentation(INDENT);
        if (code.isEmpty()) {
            m_output << INDENT << "<Code snippet \"" << location << "\" not found>" << endl;
        } else {
            foreach (QString line, code.split("\n")) {
                if (!QString(line).trimmed().isEmpty())
                    m_output << INDENT << line;

                m_output << endl;
            }
        }
        m_output << endl;
    }
}

void QtXmlToSphinx::Table::normalize()
{
    if (m_normalized || isEmpty())
        return;

    int row;
    int col;
    QtXmlToSphinx::Table& self = *this;

    // add col spans
    for (row = 0; row < count(); ++row) {
        for (col = 0; col < at(row).count(); ++col) {
            QtXmlToSphinx::TableCell& cell = self[row][col];
            if (cell.colSpan > 0) {
                QtXmlToSphinx::TableCell newCell;
                newCell.colSpan = -1;
                for (int i = 0, max = cell.colSpan-1; i < max; ++i) {
                    self[row].insert(col+1, newCell);
                }
                cell.colSpan = 0;
                col++;
            }
        }
    }

    // row spans
    const int numCols = first().count();
    for (col = 0; col < numCols; ++col) {
        for (row = 0; row < count(); ++row) {
            if (col < self[row].count()) {
                QtXmlToSphinx::TableCell& cell = self[row][col];
                if (cell.rowSpan > 0) {
                    QtXmlToSphinx::TableCell newCell;
                    newCell.rowSpan = -1;
                    int max = std::min(cell.rowSpan - 1, count());
                    cell.rowSpan = 0;
                    for (int i = 0; i < max; ++i) {
                        self[row+i+1].insert(col, newCell);
                    }
                    row++;
                }
            }
        }
    }
    m_normalized = true;
}

QTextStream& operator<<(QTextStream& s, const QtXmlToSphinx::Table &table)
{
    if (table.isEmpty())
        return s;

    if (!table.isNormalized()) {
        ReportHandler::warning("Attempt to print an unnormalized table!");
        return s;
    }

    // calc width and height of each column and row
    QVector<int> colWidths(table.first().count());
    QVector<int> rowHeights(table.count());
    for (int i = 0, maxI = table.count(); i < maxI; ++i) {
        const QtXmlToSphinx::TableRow& row = table[i];
        for (int j = 0, maxJ = row.count(); j < maxJ; ++j) {
            QStringList rowLines = row[j].data.split('\n'); // cache this would be a good idea
            foreach (QString str, rowLines)
                colWidths[j] = std::max(colWidths[j], str.count());
            rowHeights[i] = std::max(rowHeights[i], row[j].data.count('\n') + 1);
        }
    }

    if (!*std::max_element(colWidths.begin(), colWidths.end()))
        return s; // empty table (table with empty cells)

    // create a horizontal line to be used later.
    QString horizontalLine("+");
    for (int i = 0, max = colWidths.count(); i < max; ++i) {
        horizontalLine += createRepeatedChar(colWidths[i], '-');
        horizontalLine += '+';
    }

    // write table rows
    for (int i = 0, maxI = table.count(); i < maxI; ++i) { // for each row
        const QtXmlToSphinx::TableRow& row = table[i];

        // print line
        s << INDENT << '+';
        char c = (!i && table.hasHeader()) ? '=' : '-';
        for (int col = 0, max = colWidths.count(); col < max; ++col) {
            char c;
            if (row[col].rowSpan == -1)
                c = ' ';
            else if (i == 1 && table.hasHeader())
                c = '=';
            else
                c = '-';
            s << createRepeatedChar(colWidths[col], c) << '+';
        }
        s << endl;


        // Print the table cells
        for (int rowLine = 0; rowLine < rowHeights[i]; ++rowLine) { // for each line in a row
            for (int j = 0, maxJ = row.count(); j < maxJ; ++j) { // for each column
                const QtXmlToSphinx::TableCell& cell = row[j];
                QStringList rowLines = cell.data.split('\n'); // FIXME: Cache this!!!
                if (!j) // First column, so we need print the identation
                    s << INDENT;

                if (!j || !cell.colSpan)
                    s << '|';
                else
                    s << ' ';
                s << qSetFieldWidth(colWidths[j]) << left;
                s << (rowLine < rowLines.count() ? rowLines[rowLine] : "");
                s << qSetFieldWidth(0);
            }
            s << '|' << endl;
        }
    }
    s << INDENT << horizontalLine << endl;
    s << endl;
    return s;
}

static QString getClassName(const AbstractMetaClass *cppClass) {
    return cppClass->name().replace("::", ".");
}

static QString getFuncName(const AbstractMetaFunction *cppFunc) {
    static bool hashInitialized = false;
    static QHash<QString, QString> operatorsHash;
    if (!hashInitialized) {
        operatorsHash.insert("operator+", "__add__");
        operatorsHash.insert("operator+=", "__iadd__");
        operatorsHash.insert("operator-", "__sub__");
        operatorsHash.insert("operator-=", "__isub__");
        operatorsHash.insert("operator*", "__mul__");
        operatorsHash.insert("operator*=", "__imul__");
        operatorsHash.insert("operator/", "__div__");
        operatorsHash.insert("operator/=", "__idiv__");
        operatorsHash.insert("operator%", "__mod__");
        operatorsHash.insert("operator%=", "__imod__");
        operatorsHash.insert("operator<<", "__lshift__");
        operatorsHash.insert("operator<<=", "__ilshift__");
        operatorsHash.insert("operator>>", "__rshift__");
        operatorsHash.insert("operator>>=", "__irshift__");
        operatorsHash.insert("operator&", "__and__");
        operatorsHash.insert("operator&=", "__iand__");
        operatorsHash.insert("operator|", "__or__");
        operatorsHash.insert("operator|=", "__ior__");
        operatorsHash.insert("operator^", "__xor__");
        operatorsHash.insert("operator^=", "__ixor__");
        operatorsHash.insert("operator==", "__eq__");
        operatorsHash.insert("operator!=", "__ne__");
        operatorsHash.insert("operator<", "__lt__");
        operatorsHash.insert("operator<=", "__le__");
        operatorsHash.insert("operator>", "__gt__");
        operatorsHash.insert("operator>=", "__ge__");
        hashInitialized = true;
    }

    QHash<QString, QString>::const_iterator it = operatorsHash.find(cppFunc->name());
    QString result = it != operatorsHash.end() ? it.value() : cppFunc->name();
    return result.replace("::", ".");
}

QString QtDocGenerator::fileNameForClass(const AbstractMetaClass *cppClass) const
{
    return QString("%1.rst").arg(getClassName(cppClass));
}

void QtDocGenerator::writeFormatedText(QTextStream& s, const Documentation& doc, const AbstractMetaClass* metaClass)
{
    QString metaClassName;

    if (metaClass)
        metaClassName = getClassName(metaClass);

    if (doc.format() == Documentation::Native) {
        QtXmlToSphinx x(this, doc.value(), metaClassName);
        s << x;
    } else {
        s << doc.value();
    }

    s << endl;
}

void QtDocGenerator::writeFunctionBrief(QTextStream &s,
                                      const AbstractMetaClass *cppClass,
                                      const AbstractMetaFunction *cppFunction)
{
    s << INDENT << "def :meth:`"
      << cppFunction->name() << "<";
    if (cppClass && cppClass->name() != cppFunction->name())
        s << getClassName(cppClass) << '.';

    s << cppFunction->name() << ">`"
      << " (" << parseArgDocStyle(cppClass, cppFunction) << "):";
}

void QtDocGenerator::generateClass(QTextStream &s, const AbstractMetaClass *cppClass)
{
    QString doc;
    QTextStream  doc_s(&doc);

    ReportHandler::debugSparse("Generating Documentation for " + cppClass->fullName());
    s << ".. module:: " << packageName() << endl;
    QString className = getClassName(cppClass);
    s << ".. _" << className << ":" << endl << endl;

    s << className << endl;
    s << createRepeatedChar(className.count(), '*') << endl << endl;

    s << ".. inheritance-diagram:: " << className << endl
      << "    :parts: 2" << endl << endl; // TODO: This would be a parameter in the future...

    //Function list
    AbstractMetaFunctionList functionList = filterFunctions(cppClass);
    qSort(functionList.begin(), functionList.end(), functionSort);

    doc_s << "Detailed Description\n"
             "--------------------\n\n";

    writeInjectDocumentation(doc_s, DocModification::Prepend, cppClass, 0);
    writeFormatedText(doc_s, cppClass->documentation(), cppClass);


    if (!cppClass->isNamespace()) {

        writeConstructors(doc_s, cppClass);
        writeEnums(doc_s, cppClass);
        writeFields(doc_s, cppClass);

        foreach (AbstractMetaFunction *func, functionList) {
            if ((func->isConstructor() || func->isModifiedRemoved()) ||
                (func->declaringClass() != cppClass))
                continue;

            if (func->isStatic())
                doc_s <<  ".. staticmethod:: ";
            else
                doc_s <<  ".. method:: ";

            writeFunction(doc_s, true, cppClass, func);
        }
    }

    writeInjectDocumentation(doc_s, DocModification::Append, cppClass, 0);

    writeFunctionList(s, doc, cppClass);

    s << doc;
}

QString QtDocGenerator::parseFunctionDeclaration(const QString &doc, const AbstractMetaClass *cppClass)
{
    //.. method:: QObject.childEvent(arg__1)
    //def :meth:`removeEventFilter<QObject.removeEventFilter>` (arg__1):

    QString data = doc;
    QString markup;

    if (data.startsWith(".. method::"))
        markup = ".. method::";
    else if (data.startsWith(".. staticmethod::"))
        markup = ".. staticmethod::";
    else
        return QString();

    data = data.mid(markup.size()); //remove .. method::
    data = data.mid(data.indexOf(".") + 1); //remove class name

    QString methName = data.mid(0, data.indexOf("("));
    QString methArgs = data.mid(data.indexOf("("));

    data = QString("def :meth:`%1<%2.%3>` %4")
        .arg(methName)
        .arg(cppClass->name())
        .arg(methName)
        .arg(methArgs);

    return data;
}


void QtDocGenerator::writeFunctionList(QTextStream &s, const QString &content, const AbstractMetaClass *cppClass)
{
    QStringList functionList;
    QStringList staticFunctionList;

    QStringList lst = content.split("\n");
    foreach(QString row, lst) {
        QString data = row.trimmed();
        if (data.startsWith(".. method::")) {
            functionList << parseFunctionDeclaration(data, cppClass);
        }
        else if (data.startsWith(".. staticmethod::")) {
            staticFunctionList << parseFunctionDeclaration(data, cppClass);
        }
    }

    if ((functionList.size() > 0) || (staticFunctionList.size() > 0))
    {
        QtXmlToSphinx::Table functionTable;
        QtXmlToSphinx::TableRow row;

        s << "Synopsis" << endl
          << "--------" << endl << endl;

        if (functionList.size() > 0) {
            s << "Functions" << endl
              << "^^^^^^^^^" << endl << endl;

            qSort(functionList);
            foreach (QString func, functionList) {
                row << func;
                functionTable << row;
                row.clear();
            }

            functionTable.normalize();
            s << functionTable << endl;
            functionTable.clear();
        }

        if (staticFunctionList.size() > 0) {
            s << "Static functions" << endl
              << "^^^^^^^^^^^^^^^^" << endl;

            qSort(staticFunctionList);
            foreach (QString func, staticFunctionList) {
                row << func;
                functionTable << row;
                row.clear();
            }

            functionTable.normalize();
            s << functionTable << endl;
        }
    }
}

void QtDocGenerator::writeEnums(QTextStream& s, const AbstractMetaClass* cppClass)
{
    static const QString section_title(".. attribute:: ");

    foreach (AbstractMetaEnum *en, cppClass->enums()) {
        s << section_title << getClassName(cppClass) << "." << en->name() << endl << endl;
        writeFormatedText(s, en->documentation(), cppClass);
    }
}

void QtDocGenerator::writeFields(QTextStream &s, const AbstractMetaClass *cppClass)
{
    static const QString section_title(".. attribute:: ");

    foreach (AbstractMetaField *field, cppClass->fields()) {
        s << section_title << getClassName(cppClass) << "." << field->name() << endl << endl;
        //TODO: request for member ‘documentation’ is ambiguous
        writeFormatedText(s, field->AbstractMetaAttributes::documentation(), cppClass);
    }
}

void QtDocGenerator::writeConstructors(QTextStream &s, const AbstractMetaClass *cppClass)
{
    static const QString sectionTitle = ".. class:: ";
    static const QString sectionTitleSpace = QString(sectionTitle.size(), ' ');

    AbstractMetaFunctionList lst = cppClass->queryFunctions(AbstractMetaClass::Constructors | AbstractMetaClass::Visible);

    bool first = true;
    QHash<QString, AbstractMetaArgument *> arg_map;

    foreach(AbstractMetaFunction *func, lst) {
        if (func->isModifiedRemoved())
            continue;

        if (first) {
            first = false;
            s << sectionTitle;
        } else {
            s << sectionTitleSpace;
        }
        writeFunction(s, false, cppClass, func);
        foreach(AbstractMetaArgument *arg, func->arguments())
        {
            if (!arg_map.contains(arg->argumentName())) {
                arg_map.insert(arg->argumentName(), arg);
            }
        }
    }

    s << endl;

    foreach (AbstractMetaArgument *arg, arg_map.values()) {
        Indentation indentation(INDENT);
        writeParamerteType(s, cppClass, arg);
    }

    s << endl;

    foreach (AbstractMetaFunction *func, lst) {
        writeFormatedText(s, func->documentation(), cppClass);
    }
}

QString QtDocGenerator::parseArgDocStyle(const AbstractMetaClass *cppClass, const AbstractMetaFunction *func)
{
    QString ret;
    bool optional = false;

    foreach (AbstractMetaArgument *arg, func->arguments()) {

        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        if (arg->argumentIndex() > 0)
            ret += ",";

        if (!arg->defaultValueExpression().isEmpty() && (!optional)) {
            ret += "[";
            optional = true;
        }

        ret += arg->argumentName();

        if (optional)
            ret += "=" + arg->defaultValueExpression();
    }

    if (optional)
        ret += "]";

    return ret;
}

void QtDocGenerator::writeDocSnips(QTextStream &s,
                                 const CodeSnipList &codeSnips,
                                 CodeSnip::Position position,
                                 TypeSystem::Language language)
{
    Indentation indentation(INDENT);
    QStringList invalidStrings;
    const static QString startMarkup("[sphinx-begin]");
    const static QString endMarkup("[sphinx-end]");

    invalidStrings << "*" << "//" << "/*" << "*/";

    foreach (CodeSnip snip, codeSnips) {
        if ((snip.position != position) ||
            !(snip.language & language))
            continue;

        QString code = snip.code();
        while (code.contains(startMarkup) && code.contains(endMarkup)) {
            int startBlock = code.indexOf(startMarkup) + startMarkup.size();
            int endBlock = code.indexOf(endMarkup);

            if ((startBlock == -1) || (endBlock == -1))
                break;

            QString codeBlock = code.mid(startBlock, endBlock - startBlock);
            QStringList rows = codeBlock.split("\n");
            int currenRow = 0;
            int offset = 0;

            foreach(QString row, rows) {
                foreach(QString invalidString, invalidStrings) {
                    row = row.remove(invalidString);
                }

                if (row.trimmed().size() == 0) {
                    if (currenRow == 0)
                        continue;
                    else
                        s << endl;
                }

                if (currenRow == 0) {
                    //find offset
                    for (int i=0, i_max = row.size(); i < i_max; i++) {
                        if (row[i] == ' ')
                            offset++;
                        else if (row[i] == '\n')
                            offset = 0;
                        else
                            break;
                    }
                }
                row = row.mid(offset);
                s << row << endl;
                currenRow++;
            }

            code = code.mid(endBlock+endMarkup.size());
        }
    }
}

void QtDocGenerator::writeInjectDocumentation(QTextStream &s,
                                            DocModification::Mode mode,
                                            const AbstractMetaClass *cppClass,
                                            const AbstractMetaFunction *func)
{
    Indentation indentation(INDENT);

    foreach (DocModification mod, cppClass->typeEntry()->docModifications()) {
        if (mod.mode() == mode) {
            bool modOk = func ? mod.signature() == func->minimalSignature() : mod.signature().isEmpty();

            if (modOk) {
                Documentation doc;
                Documentation::Format fmt;

                if (mod.format == TypeSystem::NativeCode)
                    fmt = Documentation::Native;
                else if (mod.format == TypeSystem::TargetLangCode)
                    fmt = Documentation::Target;
                else
                    continue;

                doc.setValue(mod.code() , fmt);
                s << INDENT;
                writeFormatedText(s, doc, cppClass);
            }
        }
    }

    s << endl;

    if (func) {
        writeDocSnips(s, getCodeSnips(func),
                       (mode == DocModification::Prepend ? CodeSnip::Beginning : CodeSnip::End),
                       TypeSystem::TargetLangCode);
    } else {
        writeDocSnips(s, cppClass->typeEntry()->codeSnips(),
                       (mode == DocModification::Prepend ? CodeSnip::Beginning : CodeSnip::End),
                       TypeSystem::TargetLangCode);
    }
}

void QtDocGenerator::writeFunctionSignature(QTextStream& s, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func)
{
    if (!func->isConstructor())
        s << getClassName(cppClass) << '.';
    s << getFuncName(func) << "(" << parseArgDocStyle(cppClass, func) << ")";
}

QString QtDocGenerator::translateToPythonType(const AbstractMetaType *type, const AbstractMetaClass *cppClass)
{
    QString originalType = translateType(type, cppClass, Generator::ExcludeConst | Generator::ExcludeReference);
    QString strType = originalType;

    //remove "*"
    strType.remove("*");
    TypeEntry *te = TypeDatabase::instance()->findType(originalType.trimmed());
    if (te) {
        return te->targetLangName();
    } else {
        //remove <, >
        strType.remove(">");
        strType.remove("<");

        //replace ::
        strType.replace("::", ".");

        //Translate ContainerType
        if (strType.contains("QList") || strType.contains("QVector")) {
            strType.replace("QList", "List of ");
            strType.replace("QVector", "List of ");
        } else if (strType.contains("QHash") || strType.contains("QMap")) {
            strType.remove("QHash");
            strType.remove("QMap");
            QStringList types = strType.split(",");
            strType = QString("Dictionary with keys of type %1 and values of type %2.")
                .arg(types[0]).arg(types[1]);
        }
        return strType;
    }
}

void QtDocGenerator::writeParamerteType(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaArgument *arg)
{
    s << INDENT << ":param " << arg->argumentName() << ": "
      << translateToPythonType(arg->type(), cppClass) << endl;
}

void QtDocGenerator::writeFunctionParametersType(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);

    s << endl;
    foreach (AbstractMetaArgument *arg, func->arguments()) {

        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        writeParamerteType(s, cppClass, arg);
    }

    if (!func->isConstructor() && func->type()) {
        s << INDENT << ":rtype: " << translateToPythonType(func->type(), cppClass) << endl;
    }
    s << endl;
}

void QtDocGenerator::writeFunction(QTextStream &s, bool writeDoc, const AbstractMetaClass *cppClass, const AbstractMetaFunction* func)
{
    writeFunctionSignature(s, cppClass, func);
    s << endl;

    if (writeDoc) {
        s << endl;
        writeFunctionParametersType(s, cppClass, func);
        s << endl;
        writeInjectDocumentation(s, DocModification::Prepend, cppClass, func);
        writeFormatedText(s, func->documentation(), cppClass);
        writeInjectDocumentation(s, DocModification::Append, cppClass, func);
    }
}

void QtDocGenerator::finishGeneration()
{
    if (classes().isEmpty())
        return;

    QFile input(outputDirectory() + '/' + subDirectoryForPackage(packageName()) + "/index.rst");
    input.open(QIODevice::WriteOnly);
    QTextStream s(&input);

    s << ".. module:: " << packageName() << endl << endl;

    QString title = packageName() + " contents";
    s << title << endl;
    s << createRepeatedChar(title.length(), '*') << endl << endl;
    s << ".. toctree::" << endl;

    /* Avoid showing "Detailed Description for *every* class in toc tree */
    Indentation indentation(INDENT);
    s << INDENT << ":maxdepth: 1" << endl << endl;

    QStringList classList;
    foreach (AbstractMetaClass *cls, classes()) {
        if (!shouldGenerate(cls))
            continue;
        classList << getClassName(cls);
    }
    classList.sort();

    foreach (QString clazz, classList)
        s << INDENT << clazz << endl;

    s << endl << endl;

    s << "Detailed Description" << endl;
    s << "--------------------" << endl << endl;

    if (m_moduleDoc.format() == Documentation::Native) {
        QtXmlToSphinx x(this, m_moduleDoc.value(), moduleName());
        s << x;
    } else {
        s << m_moduleDoc.value();
    }
}

bool QtDocGenerator::doSetup(const QMap<QString, QString>& args)
{
    m_libSourceDir = args.value("library-source-dir");
    setOutputDirectory(args.value("documentation-out-dir"));
    m_docDataDir = args.value("documentation-data-dir");
    m_codeSnippetDir = args.value("documentation-code-snippets-dir", m_libSourceDir);

    if (m_libSourceDir.isEmpty() || m_docDataDir.isEmpty()) {
        ReportHandler::warning("Documentation data dir and/or Qt source dir not informed, "
                               "documentation will not be extracted from Qt sources.");
        return false;
    } else {
        QtDocParser docParser;
        docParser.setPackageName(packageName());
        docParser.setDocumentationDataDirectory(m_docDataDir);
        docParser.setLibrarySourceDirectory(m_libSourceDir);
        foreach(AbstractMetaClass* cppClass, classes()) {
            docParser.fillDocumentation(cppClass);
        }
        m_moduleDoc = docParser.retrieveModuleDocumentation();
        return true;
    }
}


QMap<QString, QString> QtDocGenerator::options() const
{
    QMap<QString, QString> options;
    options.insert("library-source-dir", "Directory where library source code is located");
    options.insert("documentation-out-dir", "The directory where the generated documentation files will be written");
    options.insert("documentation-data-dir", "Directory with XML files generated by documentation tool (qdoc3 or Doxygen)");
    options.insert("documentation-code-snippets-dir", "Directory used to search code snippets used by the documentation");
    return options;
}

