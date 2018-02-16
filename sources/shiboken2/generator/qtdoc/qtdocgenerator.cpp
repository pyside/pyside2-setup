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

#include "qtdocgenerator.h"
#include <abstractmetalang.h>
#include <reporthandler.h>
#include <typesystem.h>
#include <qtdocparser.h>
#include <doxygenparser.h>
#include <typedatabase.h>
#include <algorithm>
#include <QtCore/QStack>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <fileout.h>
#include <limits>

static Indentor INDENT;

static bool shouldSkip(const AbstractMetaFunction* func)
{
    bool skipable =  func->isConstructor()
                     || func->isModifiedRemoved()
                     || func->declaringClass() != func->ownerClass()
                     || func->isCastOperator()
                     || func->name() == QLatin1String("operator=");

    // Search a const clone
    if (!skipable && !func->isConstant()) {
        const AbstractMetaArgumentList funcArgs = func->arguments();
        const AbstractMetaFunctionList &ownerFunctions = func->ownerClass()->functions();
        for (AbstractMetaFunction *f : ownerFunctions) {
            if (f != func
                && f->isConstant()
                && f->name() == func->name()
                && f->arguments().count() == funcArgs.count()) {
                // Compare each argument
                bool cloneFound = true;

                const AbstractMetaArgumentList fargs = f->arguments();
                for (int i = 0, max = funcArgs.count(); i < max; ++i) {
                    if (funcArgs.at(i)->type()->typeEntry() != fargs.at(i)->type()->typeEntry()) {
                        cloneFound = false;
                        break;
                    }
                }
                if (cloneFound)
                    return true;
            }
        }
    }
    return skipable;
}

static bool functionSort(const AbstractMetaFunction* func1, const AbstractMetaFunction* func2)
{
    return func1->name() < func2->name();
}

class Pad
{
public:
    explicit Pad(char c, int count) : m_char(c), m_count(count) {}

    void write(QTextStream &str) const
    {
        for (int i = 0; i < m_count; ++i)
            str << m_char;
    }

private:
    const char m_char;
    const int m_count;
};

inline QTextStream &operator<<(QTextStream &str, const Pad &pad)
{
    pad.write(str);
    return str;
}

static QString escape(QString str)
{
    str.replace(QLatin1Char('*'), QLatin1String("\\*"));
    str.replace(QLatin1Char('_'), QLatin1String("\\_"));
    return str;
}

static QString escape(const QStringRef& strref)
{
    QString str = strref.toString();
    return escape(str);
}

static QString msgTagWarning(const QXmlStreamReader &reader, const QString &context,
                             const QString &tag, const QString &message)
{
    QString result;
    QTextStream str(&result);
    str << "While handling <";
    const QStringRef currentTag = reader.name();
    if (currentTag.isEmpty())
        str << tag;
    else
        str << currentTag;
    str << "> in " << context << ", line "<< reader.lineNumber()
        << ": " << message;
    return result;
}

static QString msgFallbackWarning(const QXmlStreamReader &reader, const QString &context,
                                  const QString &tag, const QString &location, const QString &identifier,
                                  const QString &fallback)
{
    QString message = QLatin1String("Falling back to \"")
        + QDir::toNativeSeparators(fallback) + QLatin1String("\" for \"") + location
        + QLatin1Char('"');
    if (!identifier.isEmpty())
        message += QLatin1String(" [") + identifier + QLatin1Char(']');
    return msgTagWarning(reader, context, tag, message);
}

QtXmlToSphinx::QtXmlToSphinx(QtDocGenerator* generator, const QString& doc, const QString& context)
        : m_context(context), m_generator(generator), m_insideBold(false), m_insideItalic(false)
{
    m_handlerMap.insert(QLatin1String("heading"), &QtXmlToSphinx::handleHeadingTag);
    m_handlerMap.insert(QLatin1String("brief"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("para"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("italic"), &QtXmlToSphinx::handleItalicTag);
    m_handlerMap.insert(QLatin1String("bold"), &QtXmlToSphinx::handleBoldTag);
    m_handlerMap.insert(QLatin1String("see-also"), &QtXmlToSphinx::handleSeeAlsoTag);
    m_handlerMap.insert(QLatin1String("snippet"), &QtXmlToSphinx::handleSnippetTag);
    m_handlerMap.insert(QLatin1String("dots"), &QtXmlToSphinx::handleDotsTag);
    m_handlerMap.insert(QLatin1String("codeline"), &QtXmlToSphinx::handleDotsTag);
    m_handlerMap.insert(QLatin1String("table"), &QtXmlToSphinx::handleTableTag);
    m_handlerMap.insert(QLatin1String("header"), &QtXmlToSphinx::handleRowTag);
    m_handlerMap.insert(QLatin1String("row"), &QtXmlToSphinx::handleRowTag);
    m_handlerMap.insert(QLatin1String("item"), &QtXmlToSphinx::handleItemTag);
    m_handlerMap.insert(QLatin1String("argument"), &QtXmlToSphinx::handleArgumentTag);
    m_handlerMap.insert(QLatin1String("teletype"), &QtXmlToSphinx::handleArgumentTag);
    m_handlerMap.insert(QLatin1String("link"), &QtXmlToSphinx::handleLinkTag);
    m_handlerMap.insert(QLatin1String("inlineimage"), &QtXmlToSphinx::handleImageTag);
    m_handlerMap.insert(QLatin1String("image"), &QtXmlToSphinx::handleImageTag);
    m_handlerMap.insert(QLatin1String("list"), &QtXmlToSphinx::handleListTag);
    m_handlerMap.insert(QLatin1String("term"), &QtXmlToSphinx::handleTermTag);
    m_handlerMap.insert(QLatin1String("raw"), &QtXmlToSphinx::handleRawTag);
    m_handlerMap.insert(QLatin1String("underline"), &QtXmlToSphinx::handleItalicTag);
    m_handlerMap.insert(QLatin1String("superscript"), &QtXmlToSphinx::handleSuperScriptTag);
    m_handlerMap.insert(QLatin1String("code"), &QtXmlToSphinx::handleCodeTag);
    m_handlerMap.insert(QLatin1String("badcode"), &QtXmlToSphinx::handleCodeTag);
    m_handlerMap.insert(QLatin1String("legalese"), &QtXmlToSphinx::handleCodeTag);
    m_handlerMap.insert(QLatin1String("section"), &QtXmlToSphinx::handleAnchorTag);
    m_handlerMap.insert(QLatin1String("quotefile"), &QtXmlToSphinx::handleQuoteFileTag);

    // ignored tags
    m_handlerMap.insert(QLatin1String("generatedlist"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("tableofcontents"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("quotefromfile"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("skipto"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("target"), &QtXmlToSphinx::handleIgnoredTag);

    // useless tags
    m_handlerMap.insert(QLatin1String("description"), &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert(QLatin1String("definition"), &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert(QLatin1String("printuntil"), &QtXmlToSphinx::handleUselessTag);
    m_handlerMap.insert(QLatin1String("relation"), &QtXmlToSphinx::handleUselessTag);

    // Doxygen tags
    m_handlerMap.insert(QLatin1String("title"), &QtXmlToSphinx::handleHeadingTag);
    m_handlerMap.insert(QLatin1String("ref"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("computeroutput"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("detaileddescription"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("name"), &QtXmlToSphinx::handleParaTag);
    m_handlerMap.insert(QLatin1String("listitem"), &QtXmlToSphinx::handleItemTag);
    m_handlerMap.insert(QLatin1String("parametername"), &QtXmlToSphinx::handleItemTag);
    m_handlerMap.insert(QLatin1String("parameteritem"), &QtXmlToSphinx::handleItemTag);
    m_handlerMap.insert(QLatin1String("ulink"), &QtXmlToSphinx::handleLinkTag);
    m_handlerMap.insert(QLatin1String("itemizedlist"), &QtXmlToSphinx::handleListTag);
    m_handlerMap.insert(QLatin1String("parameternamelist"), &QtXmlToSphinx::handleListTag);
    m_handlerMap.insert(QLatin1String("parameterlist"), &QtXmlToSphinx::handleListTag);

    // Doxygen ignored tags
    m_handlerMap.insert(QLatin1String("highlight"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("linebreak"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("programlisting"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("xreftitle"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("sp"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("entry"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("simplesect"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("verbatim"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("xrefsect"), &QtXmlToSphinx::handleIgnoredTag);
    m_handlerMap.insert(QLatin1String("xrefdescription"), &QtXmlToSphinx::handleIgnoredTag);

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

QString QtXmlToSphinx::expandFunction(const QString& function)
{
    const int firstDot = function.indexOf(QLatin1Char('.'));
    const AbstractMetaClass *metaClass = nullptr;
    if (firstDot != -1) {
        const QStringRef className = function.leftRef(firstDot);
        const AbstractMetaClassList &classes = m_generator->classes();
        for (const AbstractMetaClass *cls : classes) {
            if (cls->name() == className) {
                metaClass = cls;
                break;
            }
        }
    }

    return metaClass
        ? metaClass->typeEntry()->qualifiedTargetLangName()
          + function.right(function.size() - firstDot)
        : function;
}

QString QtXmlToSphinx::resolveContextForMethod(const QString& methodName)
{
    const QStringRef currentClass = m_context.splitRef(QLatin1Char('.')).constLast();

    const AbstractMetaClass* metaClass = 0;
    const AbstractMetaClassList &classes = m_generator->classes();
    for (const AbstractMetaClass *cls : classes) {
        if (cls->name() == currentClass) {
            metaClass = cls;
            break;
        }
    }

    if (metaClass) {
        QList<const AbstractMetaFunction*> funcList;
        const AbstractMetaFunctionList &methods = metaClass->queryFunctionsByName(methodName);
        for (const AbstractMetaFunction *func : methods) {
            if (methodName == func->name())
                funcList.append(func);
        }

        const AbstractMetaClass* implementingClass = 0;
        for (const AbstractMetaFunction *func : qAsConst(funcList)) {
            implementingClass = func->implementingClass();
            if (implementingClass->name() == currentClass)
                break;
        }

        if (implementingClass)
            return implementingClass->typeEntry()->qualifiedTargetLangName();
    }

    return QLatin1Char('~') + m_context;
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
            const QString message = QLatin1String("XML Error: ") + reader.errorString()
                                    + QLatin1Char('\n') + doc;
            m_output << INDENT << message;
            qCWarning(lcShiboken).noquote().nospace() << message;
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

static QString resolveFile(const QStringList &locations, const QString &path)
{
    for (QString location : locations) {
        location.append(QLatin1Char('/'));
        location.append(path);
        if (QFileInfo::exists(location))
            return location;
    }
    return QString();
}

QString QtXmlToSphinx::readFromLocations(const QStringList &locations, const QString &path,
                                         const QString &identifier, QString *errorMessage)
{
    QString result;
    const QString resolvedPath = resolveFile(locations, path);
    if (resolvedPath.isEmpty()) {
        QTextStream(errorMessage) << "Could not resolve \"" << path << "\" in \""
           << locations.join(QLatin1String("\", \""));
        return QString(); // null
    }
    qCDebug(lcShiboken).noquote().nospace() << "snippet file " << path
        << " [" << identifier << ']' << " resolved to " << resolvedPath;
    return readFromLocation(resolvedPath, identifier, errorMessage);
}

QString QtXmlToSphinx::readFromLocation(const QString &location, const QString &identifier,
                                        QString *errorMessage)
{
    QFile inputFile;
    inputFile.setFileName(location);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        QTextStream(errorMessage) << "Could not read code snippet file: "
            << QDir::toNativeSeparators(inputFile.fileName())
            << ": " << inputFile.errorString();
        return QString(); // null
    }

    QString code = QLatin1String(""); // non-null
    if (identifier.isEmpty()) {
        while (!inputFile.atEnd())
            code += QString::fromUtf8(inputFile.readLine());
        return code;
    }

    const QRegularExpression searchString(QLatin1String("//!\\s*\\[")
                                          + identifier + QLatin1String("\\]"));
    Q_ASSERT(searchString.isValid());
    static const QRegularExpression codeSnippetCode(QLatin1String("//!\\s*\\[[\\w\\d\\s]+\\]"));
    Q_ASSERT(codeSnippetCode.isValid());

    bool getCode = false;

    while (!inputFile.atEnd()) {
        QString line = QString::fromUtf8(inputFile.readLine());
        if (getCode && !line.contains(searchString)) {
            line.remove(codeSnippetCode);
            code += line;
        } else if (line.contains(searchString)) {
            if (getCode)
                break;
            getCode = true;
        }
    }

    if (!getCode) {
        QTextStream(errorMessage) << "Code snippet file found ("
            << QDir::toNativeSeparators(location) << "), but snippet ["
            << identifier << "] not found.";
        return QString(); // null
    }

    return code;
}

void QtXmlToSphinx::handleHeadingTag(QXmlStreamReader& reader)
{
    static QString heading;
    static char type;
    static char types[] = { '-', '^' };
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        uint typeIdx = reader.attributes().value(QLatin1String("level")).toString().toUInt();
        if (typeIdx >= sizeof(types))
            type = types[sizeof(types)-1];
        else
            type = types[typeIdx];
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << Pad(type, heading.length()) << endl << endl;
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
        if (result.startsWith(QLatin1String("**Warning:**")))
            result.replace(0, 12, QLatin1String(".. warning:: "));
        else if (result.startsWith(QLatin1String("**Note:**")))
            result.replace(0, 9, QLatin1String(".. note:: "));

        m_output << INDENT << result << endl << endl;
    } else if (token == QXmlStreamReader::Characters) {
        QString text = escape(reader.text());
        if (!m_output.string()->isEmpty()) {
            QChar start = text[0];
            QChar end = m_output.string()->at(m_output.string()->length() - 1);
            if ((end == QLatin1Char('*') || end == QLatin1Char('`')) && start != QLatin1Char(' ') && !start.isPunct())
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

static inline QString fallbackPathAttribute() { return QStringLiteral("path"); }

static inline bool snippetComparison()
{
    return ReportHandler::debugLevel() >= ReportHandler::FullDebug;
}

template <class Indent> // const char*/class Indentor
void formatSnippet(QTextStream &str, Indent indent, const QString &snippet)
{
    const QVector<QStringRef> lines = snippet.splitRef(QLatin1Char('\n'));
    for (const QStringRef &line : lines) {
        if (!line.trimmed().isEmpty())
            str << indent << line;
        str << endl;
    }
}

static QString msgSnippetComparison(const QString &location, const QString &identifier,
                                    const QString &pythonCode, const QString &fallbackCode)
{
    QString result;
    QTextStream str(&result);
    str << "Python snippet " << location;
    if (!identifier.isEmpty())
        str << " [" << identifier << ']';
    str << ":\n";
    formatSnippet(str, "  ", pythonCode);
    str << "Corresponding fallback snippet:\n";
    formatSnippet(str, "  ", fallbackCode);
    str << "-- end --\n";
    return result;
}

void QtXmlToSphinx::handleSnippetTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        const bool consecutiveSnippet = m_lastTagName == QLatin1String("snippet")
            || m_lastTagName == QLatin1String("dots") || m_lastTagName == QLatin1String("codeline");
        if (consecutiveSnippet) {
            m_output.flush();
            m_output.string()->chop(2);
        }
        QString location = reader.attributes().value(QLatin1String("location")).toString();
        QString identifier = reader.attributes().value(QLatin1String("identifier")).toString();
        QString errorMessage;
        const QString pythonCode =
            readFromLocations(m_generator->codeSnippetDirs(), location, identifier, &errorMessage);
        if (!errorMessage.isEmpty())
            qCWarning(lcShiboken, "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
        // Fall back to C++ snippet when "path" attribute is present.
        // Also read fallback snippet when comparison is desired.
        QString fallbackCode;
        if ((pythonCode.isNull() || snippetComparison())
            && reader.attributes().hasAttribute(fallbackPathAttribute())) {
            const QString fallback = reader.attributes().value(fallbackPathAttribute()).toString();
            if (QFileInfo::exists(fallback)) {
                if (pythonCode.isNull())
                    qCWarning(lcShiboken, "%s", qPrintable(msgFallbackWarning(reader, m_context, m_lastTagName, location, identifier, fallback)));
                fallbackCode = readFromLocation(fallback, identifier, &errorMessage);
                if (!errorMessage.isEmpty())
                    qCWarning(lcShiboken, "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
            }
        }

        if (!pythonCode.isEmpty() && !fallbackCode.isEmpty() && snippetComparison())
            qCDebug(lcShiboken, "%s", qPrintable(msgSnippetComparison(location, identifier, pythonCode, fallbackCode)));

        if (!consecutiveSnippet)
            m_output << INDENT << "::\n\n";

        Indentation indentation(INDENT);
        const QString code = pythonCode.isNull() ? fallbackCode : pythonCode;
        if (code.isEmpty())
            m_output << INDENT << "<Code snippet \"" << location << ':' << identifier << "\" not found>" << endl;
        else
            formatSnippet(m_output, INDENT, code);
        m_output << endl;
    }
}
void QtXmlToSphinx::handleDotsTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        const bool consecutiveSnippet = m_lastTagName == QLatin1String("snippet")
            || m_lastTagName == QLatin1String("dots") || m_lastTagName == QLatin1String("codeline");
        if (consecutiveSnippet) {
            m_output.flush();
            m_output.string()->chop(2);
        }
        Indentation indentation(INDENT);
        pushOutputBuffer();
        m_output << INDENT;
        int indent = reader.attributes().value(QLatin1String("indent")).toString().toInt();
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
        m_output << reader.text().toString().replace(QLatin1String("::"), QLatin1String("."));
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
        cell.colSpan = reader.attributes().value(QLatin1String("colspan")).toString().toShort();
        cell.rowSpan = reader.attributes().value(QLatin1String("rowspan")).toString().toShort();
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
        m_tableHasHeader = reader.name() == QLatin1String("header");
        m_currentTable << TableRow();
    }
}

void QtXmlToSphinx::handleListTag(QXmlStreamReader& reader)
{
    // BUG We do not support a list inside a table cell
    static QString listType;
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        listType = reader.attributes().value(QLatin1String("type")).toString();
        if (listType == QLatin1String("enum")) {
            m_currentTable << (TableRow() << "Constant" << "Description");
            m_tableHasHeader = true;
        }
        INDENT.indent--;
    } else if (token == QXmlStreamReader::EndElement) {
        INDENT.indent++;
        if (!m_currentTable.isEmpty()) {
            if (listType == QLatin1String("bullet")) {
                m_output << endl;
                for (const TableCell &cell : m_currentTable.constFirst()) {
                    const QVector<QStringRef> itemLines = cell.data.splitRef(QLatin1Char('\n'));
                    m_output << INDENT << "* " << itemLines.constFirst() << endl;
                    for (int i = 1, max = itemLines.count(); i < max; ++i)
                        m_output << INDENT << "  " << itemLines[i] << endl;
                }
                m_output << endl;
            } else if (listType == QLatin1String("enum")) {
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
        l_linktagending = QLatin1String("` ");
        if (m_insideBold) {
            l_linktag.prepend(QLatin1String("**"));
            l_linktagending.append(QLatin1String("**"));
        } else if (m_insideItalic) {
            l_linktag.prepend(QLatin1Char('*'));
            l_linktagending.append(QLatin1Char('*'));
        }
        l_type = reader.attributes().value(QLatin1String("type")).toString();

        // TODO: create a flag PROPERTY-AS-FUNCTION to ask if the properties
        // are recognized as such or not in the binding
        if (l_type == QLatin1String("property"))
            l_type = QLatin1String("function");

        if (l_type == QLatin1String("typedef"))
            l_type = QLatin1String("class");

        QString linkSource;
        if (l_type == QLatin1String("function") || l_type == QLatin1String("class")) {
            linkSource  = QLatin1String("raw");
        } else if (l_type == QLatin1String("enum")) {
            linkSource  = QLatin1String("enum");
        } else if (l_type == QLatin1String("page")) {
            linkSource  = QLatin1String("page");
        } else {
            linkSource = QLatin1String("href");
        }

        l_linkref = reader.attributes().value(linkSource).toString();
        l_linkref.replace(QLatin1String("::"), QLatin1String("."));
        l_linkref.remove(QLatin1String("()"));

        if (l_type == QLatin1String("function") && !m_context.isEmpty()) {
            l_linktag = QLatin1String(" :meth:`");
            const QVector<QStringRef> rawlinklist = l_linkref.splitRef(QLatin1Char('.'));
            if (rawlinklist.size() == 1 || rawlinklist.constFirst() == m_context) {
                QString context = resolveContextForMethod(rawlinklist.constLast().toString());
                if (!l_linkref.startsWith(context))
                    l_linkref.prepend(context + QLatin1Char('.'));
            } else {
                l_linkref = expandFunction(l_linkref);
            }
        } else if (l_type == QLatin1String("function") && m_context.isEmpty()) {
            l_linktag = QLatin1String(" :func:`");
        } else if (l_type == QLatin1String("class")) {
            l_linktag = QLatin1String(" :class:`");
            TypeEntry* type = TypeDatabase::instance()->findType(l_linkref);
            if (type) {
                l_linkref = type->qualifiedTargetLangName();
            } else { // fall back to the old heuristic if the type wasn't found.
                const QVector<QStringRef> rawlinklist = l_linkref.splitRef(QLatin1Char('.'));
                QStringList splittedContext = m_context.split(QLatin1Char('.'));
                if (rawlinklist.size() == 1 || rawlinklist.constFirst() == splittedContext.constLast()) {
                    splittedContext.removeLast();
                    l_linkref.prepend(QLatin1Char('~') + splittedContext.join(QLatin1Char('.'))
                                      + QLatin1Char('.'));
                }
            }
        } else if (l_type == QLatin1String("enum")) {
            l_linktag = QLatin1String(" :attr:`");
        } else if (l_type == QLatin1String("page") && l_linkref == m_generator->moduleName()) {
            l_linktag = QLatin1String(" :mod:`");
        } else {
            l_linktag = QLatin1String(" :ref:`");
        }

    } else if (token == QXmlStreamReader::Characters) {
        QString linktext = reader.text().toString();
        linktext.replace(QLatin1String("::"), QLatin1String("."));
        const QStringRef item = l_linkref.splitRef(QLatin1Char('.')).constLast();
        if (l_linkref == linktext
            || (l_linkref + QLatin1String("()")) == linktext
            || item == linktext
            || (item + QLatin1String("()")) == linktext)
            l_linktext.clear();
        else
            l_linktext = linktext + QLatin1Char('<');
    } else if (token == QXmlStreamReader::EndElement) {
        if (!l_linktext.isEmpty())
            l_linktagending.prepend(QLatin1Char('>'));
        m_output << l_linktag << l_linktext << escape(l_linkref) << l_linktagending;
    }
}

// Copy images that are placed in a subdirectory "images" under the webxml files
// by qdoc to a matching subdirectory under the "rst/PySide2/<module>" directory
static bool copyImage(const QString &href, const QString &docDataDir,
                      const QString &context, const QString &outputDir,
                      QString *errorMessage)
{
    const QChar slash = QLatin1Char('/');
    const int lastSlash = href.lastIndexOf(slash);
    const QString imagePath = lastSlash != -1 ? href.left(lastSlash) : QString();
    const QString imageFileName = lastSlash != -1 ? href.right(href.size() - lastSlash - 1) : href;
    QFileInfo imageSource(docDataDir + slash + href);
    if (!imageSource.exists()) {
        QTextStream(errorMessage) << "Image " << href << " does not exist in "
            << QDir::toNativeSeparators(docDataDir);
        return false;
    }
    // Determine directory from context, "Pyside2.QtGui.QPainter" ->"Pyside2/QtGui".
    // FIXME: Not perfect yet, should have knowledge about namespaces (DataVis3D) or
    // nested classes "Pyside2.QtGui.QTouchEvent.QTouchPoint".
    QString relativeTargetDir = context;
    const int lastDot = relativeTargetDir.lastIndexOf(QLatin1Char('.'));
    if (lastDot != -1)
        relativeTargetDir.truncate(lastDot);
    relativeTargetDir.replace(QLatin1Char('.'), slash);
    if (!imagePath.isEmpty())
        relativeTargetDir += slash + imagePath;

    const QString targetDir = outputDir + slash + relativeTargetDir;
    const QString targetFileName = targetDir + slash + imageFileName;
    if (QFileInfo::exists(targetFileName))
        return true;
    if (!QFileInfo::exists(targetDir)) {
        const QDir outDir(outputDir);
        if (!outDir.mkpath(relativeTargetDir)) {
            QTextStream(errorMessage) << "Cannot create " << QDir::toNativeSeparators(relativeTargetDir)
                << " under " << QDir::toNativeSeparators(outputDir);
            return false;
        }
    }

    QFile source(imageSource.absoluteFilePath());
    if (!source.copy(targetFileName)) {
        QTextStream(errorMessage) << "Cannot copy " << QDir::toNativeSeparators(source.fileName())
            << " to " << QDir::toNativeSeparators(targetFileName) << ": "
            << source.errorString();
        return false;
    }
    qCDebug(lcShiboken()).noquote().nospace() << __FUNCTION__ << " href=\""
        << href << "\", context=\"" << context << "\", docDataDir=\""
        << docDataDir << "\", outputDir=\"" << outputDir << "\", copied \""
        << source.fileName() << "\"->\"" << targetFileName << '"';
    return true;
}

void QtXmlToSphinx::handleImageTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString href = reader.attributes().value(QLatin1String("href")).toString();
        QString errorMessage;
        if (!copyImage(href,m_generator->docDataDir(), m_context,
                       m_generator->outputDirectory(), &errorMessage)) {
            qCWarning(lcShiboken, "%s", qPrintable(errorMessage));
        }

        if (reader.name() == QLatin1String("image"))
            m_output << INDENT << ".. image:: " <<  href << endl << endl;
        else
            m_output << ".. image:: " << href << ' ';
    }
}

void QtXmlToSphinx::handleRawTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString format = reader.attributes().value(QLatin1String("format")).toString();
        m_output << INDENT << ".. raw:: " << format.toLower() << endl << endl;
    } else if (token == QXmlStreamReader::Characters) {
        const QVector<QStringRef> lst(reader.text().split(QLatin1Char('\n')));
        for (const QStringRef &row : lst)
            m_output << INDENT << INDENT << row << endl;
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << endl << endl;
    }
}

void QtXmlToSphinx::handleCodeTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        m_output << INDENT << "::" << endl << endl;
        INDENT.indent++;
    } else if (token == QXmlStreamReader::Characters) {
        const QVector<QStringRef> lst(reader.text().split(QLatin1Char('\n')));
        for (const QStringRef &row : lst)
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
        qCDebug(lcShiboken).noquote().nospace() << "Unknown QtDoc tag: \"" << reader.name().toString() << "\".";
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
        if (reader.attributes().hasAttribute(QLatin1String("id")))
            anchor = reader.attributes().value(QLatin1String("id")).toString();
        else if (reader.attributes().hasAttribute(QLatin1String("name")))
            anchor = reader.attributes().value(QLatin1String("name")).toString();
        if (!anchor.isEmpty() && m_opened_anchor != anchor) {
            m_opened_anchor = anchor;
            m_output << INDENT << ".. _" << m_context << "_" << anchor.toLower() << ":" << endl << endl;
        }
   } else if (token == QXmlStreamReader::EndElement) {
       m_opened_anchor.clear();
   }
}

void QtXmlToSphinx::handleQuoteFileTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::Characters) {
        QString location = reader.text().toString();
        location.prepend(m_generator->libSourceDir() + QLatin1Char('/'));
        QString errorMessage;
        QString code = readFromLocation(location, QString(), &errorMessage);
        if (!errorMessage.isEmpty())
            qCWarning(lcShiboken(), "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
        m_output << INDENT << "::\n\n";
        Indentation indentation(INDENT);
        if (code.isEmpty())
            m_output << INDENT << "<Code snippet \"" << location << "\" not found>" << endl;
        else
            formatCode(m_output, code, INDENT);
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

    //QDoc3 generates tables with wrong number of columns. We have to
    //check and if necessary, merge the last columns.
    int maxCols = self.at(0).count();
    // add col spans
    for (row = 0; row < count(); ++row) {
        for (col = 0; col < at(row).count(); ++col) {
            QtXmlToSphinx::TableCell& cell = self[row][col];
            bool mergeCols = (col >= maxCols);
            if (cell.colSpan > 0) {
                QtXmlToSphinx::TableCell newCell;
                newCell.colSpan = -1;
                for (int i = 0, max = cell.colSpan-1; i < max; ++i) {
                    self[row].insert(col+1, newCell);
                }
                cell.colSpan = 0;
                col++;
            } else if (mergeCols) {
                self[row][maxCols - 1].data += QLatin1Char(' ') + cell.data;
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
        qCDebug(lcShiboken) << "Attempt to print an unnormalized table!";
        return s;
    }

    // calc width and height of each column and row
    const int headerColumnCount = table.constFirst().count();
    QVector<int> colWidths(headerColumnCount);
    QVector<int> rowHeights(table.count());
    for (int i = 0, maxI = table.count(); i < maxI; ++i) {
        const QtXmlToSphinx::TableRow& row = table[i];
        for (int j = 0, maxJ = std::min(row.count(), colWidths.size()); j < maxJ; ++j) {
            const QVector<QStringRef> rowLines = row[j].data.splitRef(QLatin1Char('\n')); // cache this would be a good idea
            for (const QStringRef &str : rowLines)
                colWidths[j] = std::max(colWidths[j], str.count());
            rowHeights[i] = std::max(rowHeights[i], row[j].data.count(QLatin1Char('\n')) + 1);
        }
    }

    if (!*std::max_element(colWidths.begin(), colWidths.end()))
        return s; // empty table (table with empty cells)

    // create a horizontal line to be used later.
    QString horizontalLine = QLatin1String("+");
    for (int i = 0, max = colWidths.count(); i < max; ++i) {
        horizontalLine += QString(colWidths.at(i), QLatin1Char('-'));
        horizontalLine += QLatin1Char('+');
    }

    // write table rows
    for (int i = 0, maxI = table.count(); i < maxI; ++i) { // for each row
        const QtXmlToSphinx::TableRow& row = table[i];

        // print line
        s << INDENT << '+';
        for (int col = 0; col < headerColumnCount; ++col) {
            char c;
            if (col >= row.length() || row[col].rowSpan == -1)
                c = ' ';
            else if (i == 1 && table.hasHeader())
                c = '=';
            else
                c = '-';
            s << Pad(c, colWidths.at(col)) << '+';
        }
        s << endl;


        // Print the table cells
        for (int rowLine = 0; rowLine < rowHeights[i]; ++rowLine) { // for each line in a row
            int j = 0;
            for (int maxJ = std::min(row.count(), headerColumnCount); j < maxJ; ++j) { // for each column
                const QtXmlToSphinx::TableCell& cell = row[j];
                const QVector<QStringRef> rowLines = cell.data.splitRef(QLatin1Char('\n')); // FIXME: Cache this!!!
                if (!j) // First column, so we need print the identation
                    s << INDENT;

                if (!j || !cell.colSpan)
                    s << '|';
                else
                    s << ' ';
                if (rowLine < rowLines.count())
                    s << qSetFieldWidth(colWidths[j]) << left << rowLines.at(rowLine) << qSetFieldWidth(0);
                else
                    s << Pad(' ', colWidths.at(j));
            }
            for ( ; j < headerColumnCount; ++j) // pad
                s << '|' << Pad(' ', colWidths.at(j));
            s << '|' << endl;
        }
    }
    s << INDENT << horizontalLine << endl;
    s << endl;
    return s;
}

static QString getFuncName(const AbstractMetaFunction* cppFunc) {
    static bool hashInitialized = false;
    static QHash<QString, QString> operatorsHash;
    if (!hashInitialized) {
        operatorsHash.insert(QLatin1String("operator+"), QLatin1String("__add__"));
        operatorsHash.insert(QLatin1String("operator+="), QLatin1String("__iadd__"));
        operatorsHash.insert(QLatin1String("operator-"), QLatin1String("__sub__"));
        operatorsHash.insert(QLatin1String("operator-="), QLatin1String("__isub__"));
        operatorsHash.insert(QLatin1String("operator*"), QLatin1String("__mul__"));
        operatorsHash.insert(QLatin1String("operator*="), QLatin1String("__imul__"));
        operatorsHash.insert(QLatin1String("operator/"), QLatin1String("__div__"));
        operatorsHash.insert(QLatin1String("operator/="), QLatin1String("__idiv__"));
        operatorsHash.insert(QLatin1String("operator%"), QLatin1String("__mod__"));
        operatorsHash.insert(QLatin1String("operator%="), QLatin1String("__imod__"));
        operatorsHash.insert(QLatin1String("operator<<"), QLatin1String("__lshift__"));
        operatorsHash.insert(QLatin1String("operator<<="), QLatin1String("__ilshift__"));
        operatorsHash.insert(QLatin1String("operator>>"), QLatin1String("__rshift__"));
        operatorsHash.insert(QLatin1String("operator>>="), QLatin1String("__irshift__"));
        operatorsHash.insert(QLatin1String("operator&"), QLatin1String("__and__"));
        operatorsHash.insert(QLatin1String("operator&="), QLatin1String("__iand__"));
        operatorsHash.insert(QLatin1String("operator|"), QLatin1String("__or__"));
        operatorsHash.insert(QLatin1String("operator|="), QLatin1String("__ior__"));
        operatorsHash.insert(QLatin1String("operator^"), QLatin1String("__xor__"));
        operatorsHash.insert(QLatin1String("operator^="), QLatin1String("__ixor__"));
        operatorsHash.insert(QLatin1String("operator=="), QLatin1String("__eq__"));
        operatorsHash.insert(QLatin1String("operator!="), QLatin1String("__ne__"));
        operatorsHash.insert(QLatin1String("operator<"), QLatin1String("__lt__"));
        operatorsHash.insert(QLatin1String("operator<="), QLatin1String("__le__"));
        operatorsHash.insert(QLatin1String("operator>"), QLatin1String("__gt__"));
        operatorsHash.insert(QLatin1String("operator>="), QLatin1String("__ge__"));
        hashInitialized = true;
    }

    QHash<QString, QString>::const_iterator it = operatorsHash.constFind(cppFunc->name());
    QString result = it != operatorsHash.cend() ? it.value() : cppFunc->name();
    result.replace(QLatin1String("::"), QLatin1String("."));
    return result;
}

QtDocGenerator::QtDocGenerator() : m_docParser(0)
{
}

QtDocGenerator::~QtDocGenerator()
{
    delete m_docParser;
}

QString QtDocGenerator::fileNamePrefix() const
{
    return QLatin1String(".rst");
}

QString QtDocGenerator::fileNameForContext(GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    if (!context.forSmartPointer()) {
        return getClassTargetFullName(metaClass, false) + fileNamePrefix();
    } else {
        const AbstractMetaType *smartPointerType = context.preciseType();
        QString fileNameBase = getFileNameBaseForSmartPointer(smartPointerType, metaClass);
        return fileNameBase + fileNamePrefix();
    }
}

void QtDocGenerator::writeFormattedText(QTextStream &s, const Documentation &doc,
                                        const AbstractMetaClass *metaClass)
{
    QString metaClassName;

    if (metaClass)
        metaClassName = getClassTargetFullName(metaClass);

    if (doc.format() == Documentation::Native) {
        QtXmlToSphinx x(this, doc.value(), metaClassName);
        s << x;
    } else {
        const QString &value = doc.value();
        const QVector<QStringRef> lines = value.splitRef(QLatin1Char('\n'));
        int typesystemIndentation = std::numeric_limits<int>().max();
        // check how many spaces must be removed from the beginning of each line
        for (const QStringRef &line : lines) {
            const auto it = std::find_if(line.cbegin(), line.cend(),
                                         [] (QChar c) { return !c.isSpace(); });
            if (it != line.cend())
                typesystemIndentation = qMin(typesystemIndentation, int(it - line.cbegin()));
        }
        if (typesystemIndentation == std::numeric_limits<int>().max())
            typesystemIndentation = 0;
        for (const QStringRef &line : lines) {
            s << INDENT
                << (typesystemIndentation > 0 && typesystemIndentation < line.size()
                    ? line.right(line.size() - typesystemIndentation) : line)
                << endl;
        }
    }

    s << endl;
}

static void writeInheritedByList(QTextStream& s, const AbstractMetaClass* metaClass, const AbstractMetaClassList& allClasses)
{
    AbstractMetaClassList res;
    for (AbstractMetaClass *c : allClasses) {
        if (c != metaClass && c->inheritsFrom(metaClass))
            res << c;
    }

    if (res.isEmpty())
        return;

    s << "**Inherited by:** ";
    QStringList classes;
    for (AbstractMetaClass *c : qAsConst(res))
        classes << QLatin1String(":ref:`") + getClassTargetFullName(c, false) + QLatin1Char('`');
    s << classes.join(QLatin1String(", ")) << endl << endl;
}

void QtDocGenerator::generateClass(QTextStream &s, GeneratorContext &classContext)
{
    AbstractMetaClass *metaClass = classContext.metaClass();
    qCDebug(lcShiboken).noquote().nospace() << "Generating Documentation for " << metaClass->fullName();

    m_packages[metaClass->package()] << fileNameForContext(classContext);

    m_docParser->setPackageName(metaClass->package());
    m_docParser->fillDocumentation(const_cast<AbstractMetaClass*>(metaClass));

    s << ".. module:: " << metaClass->package() << endl;
    QString className = getClassTargetFullName(metaClass, false);
    s << ".. _" << className << ":" << endl << endl;

    s << className << endl;
    s << Pad('*', className.count()) << endl << endl;

    s << ".. inheritance-diagram:: " << className << endl
      << "    :parts: 2" << endl << endl; // TODO: This would be a parameter in the future...


    writeInheritedByList(s, metaClass, classes());

    if (metaClass->typeEntry() && (metaClass->typeEntry()->version() != 0))
        s << ".. note:: This class was introduced in Qt " << metaClass->typeEntry()->version() << endl;

    writeFunctionList(s, metaClass);

    //Function list
    AbstractMetaFunctionList functionList = metaClass->functions();
    qSort(functionList.begin(), functionList.end(), functionSort);

    s << "Detailed Description\n"
         "--------------------\n\n";

    writeInjectDocumentation(s, TypeSystem::DocModificationPrepend, metaClass, 0);
    if (!writeInjectDocumentation(s, TypeSystem::DocModificationReplace, metaClass, 0))
        writeFormattedText(s, metaClass->documentation(), metaClass);

    if (!metaClass->isNamespace())
        writeConstructors(s, metaClass);
    writeEnums(s, metaClass);
    if (!metaClass->isNamespace())
        writeFields(s, metaClass);


    for (AbstractMetaFunction *func : qAsConst(functionList)) {
        if (shouldSkip(func))
            continue;

        if (func->isStatic())
            s <<  ".. staticmethod:: ";
        else
            s <<  ".. method:: ";

        writeFunction(s, true, metaClass, func);
    }

    writeInjectDocumentation(s, TypeSystem::DocModificationAppend, metaClass, 0);
}

void QtDocGenerator::writeFunctionList(QTextStream& s, const AbstractMetaClass* cppClass)
{
    QStringList functionList;
    QStringList virtualList;
    QStringList signalList;
    QStringList slotList;
    QStringList staticFunctionList;

    const AbstractMetaFunctionList &classFunctions = cppClass->functions();
    for (AbstractMetaFunction *func : classFunctions) {
        if (shouldSkip(func))
            continue;

        QString className;
        if (!func->isConstructor())
            className =  getClassTargetFullName(cppClass) + QLatin1Char('.');
        else if (func->implementingClass() && func->implementingClass()->enclosingClass())
            className =  getClassTargetFullName(func->implementingClass()->enclosingClass()) + QLatin1Char('.');
        QString funcName = getFuncName(func);

        QString str = QLatin1String("def :meth:`");

        str += funcName;
        str += QLatin1Char('<');
        if (!funcName.startsWith(className))
            str += className;
        str += funcName;
        str += QLatin1String(">` (");
        str += parseArgDocStyle(cppClass, func);
        str += QLatin1Char(')');

        if (func->isStatic())
            staticFunctionList << str;
        else if (func->isVirtual())
            virtualList << str;
        else if (func->isSignal())
            signalList << str;
        else if (func->isSlot())
            slotList << str;
        else
            functionList << str;
    }

    if ((functionList.size() > 0) || (staticFunctionList.size() > 0)) {
        QtXmlToSphinx::Table functionTable;

        s << "Synopsis" << endl
          << "--------" << endl << endl;

        writeFunctionBlock(s, QLatin1String("Functions"), functionList);
        writeFunctionBlock(s, QLatin1String("Virtual functions"), virtualList);
        writeFunctionBlock(s, QLatin1String("Slots"), slotList);
        writeFunctionBlock(s, QLatin1String("Signals"), signalList);
        writeFunctionBlock(s, QLatin1String("Static functions"), staticFunctionList);
    }
}

void QtDocGenerator::writeFunctionBlock(QTextStream& s, const QString& title, QStringList& functions)
{
    if (functions.size() > 0) {
        s << title << endl
          << QString(title.size(), QLatin1Char('^')) << endl;

        qSort(functions);

        s << ".. container:: function_list" << endl << endl;
        Indentation indentation(INDENT);
        for (const QString &func : qAsConst(functions))
            s << '*' << INDENT << func << endl;

        s << endl << endl;
    }
}

void QtDocGenerator::writeEnums(QTextStream& s, const AbstractMetaClass* cppClass)
{
    static const QString section_title = QLatin1String(".. attribute:: ");

    const AbstractMetaEnumList &enums = cppClass->enums();
    for (AbstractMetaEnum *en : enums) {
        s << section_title << getClassTargetFullName(cppClass) << '.' << en->name() << endl << endl;
        writeFormattedText(s, en->documentation(), cppClass);

        if (en->typeEntry() && (en->typeEntry()->version() != 0))
            s << ".. note:: This enum was introduced or modified in Qt " << en->typeEntry()->version() << endl;
    }

}

void QtDocGenerator::writeFields(QTextStream& s, const AbstractMetaClass* cppClass)
{
    static const QString section_title = QLatin1String(".. attribute:: ");

    const AbstractMetaFieldList &fields = cppClass->fields();
    for (AbstractMetaField *field : fields) {
        s << section_title << getClassTargetFullName(cppClass) << "." << field->name() << endl << endl;
        //TODO: request for member ‘documentation’ is ambiguous
        writeFormattedText(s, field->AbstractMetaAttributes::documentation(), cppClass);
    }
}

void QtDocGenerator::writeConstructors(QTextStream& s, const AbstractMetaClass* cppClass)
{
    static const QString sectionTitle = QLatin1String(".. class:: ");
    static const QString sectionTitleSpace = QString(sectionTitle.size(), QLatin1Char(' '));

    AbstractMetaFunctionList lst = cppClass->queryFunctions(AbstractMetaClass::Constructors | AbstractMetaClass::Visible);
    for (int i = lst.size() - 1; i >= 0; --i) {
        if (lst.at(i)->isModifiedRemoved() || lst.at(i)->functionType() == AbstractMetaFunction::MoveConstructorFunction)
            lst.removeAt(i);
    }

    bool first = true;
    QHash<QString, AbstractMetaArgument*> arg_map;

    for (AbstractMetaFunction *func : qAsConst(lst)) {
        if (first) {
            first = false;
            s << sectionTitle;
        } else {
            s << sectionTitleSpace;
        }
        writeFunction(s, false, cppClass, func);
        const AbstractMetaArgumentList &arguments = func->arguments();
        for (AbstractMetaArgument *arg : arguments) {
            if (!arg_map.contains(arg->name())) {
                arg_map.insert(arg->name(), arg);
            }
        }
    }

    s << endl;

    for (QHash<QString, AbstractMetaArgument*>::const_iterator it = arg_map.cbegin(), end = arg_map.cend(); it != end; ++it) {
        Indentation indentation(INDENT);
        writeParameterType(s, cppClass, it.value());
    }

    s << endl;

    for (AbstractMetaFunction *func : qAsConst(lst))
        writeFormattedText(s, func->documentation(), cppClass);
}

QString QtDocGenerator::parseArgDocStyle(const AbstractMetaClass* cppClass, const AbstractMetaFunction* func)
{
    QString ret;
    int optArgs = 0;

    const AbstractMetaArgumentList &arguments = func->arguments();
    for (AbstractMetaArgument *arg : arguments) {

        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        bool thisIsoptional = !arg->defaultValueExpression().isEmpty();
        if (optArgs || thisIsoptional) {
            ret += QLatin1Char('[');
            optArgs++;
        }

        if (arg->argumentIndex() > 0)
            ret += QLatin1String(", ");

        ret += arg->name();

        if (thisIsoptional) {
            QString defValue = arg->defaultValueExpression();
            if (defValue == QLatin1String("QString()")) {
                defValue = QLatin1String("\"\"");
            } else if (defValue == QLatin1String("QStringList()")
                       || defValue.startsWith(QLatin1String("QVector"))
                       || defValue.startsWith(QLatin1String("QList"))) {
                defValue = QLatin1String("list()");
            } else if (defValue == QLatin1String("QVariant()")) {
                defValue = QLatin1String("None");
            } else {
                defValue.replace(QLatin1String("::"), QLatin1String("."));
                if (defValue == QLatin1String("0") && (arg->type()->isQObject() || arg->type()->isObject()))
                    defValue = QLatin1String("None");
            }
            ret += QLatin1Char('=') + defValue;
        }
    }

    ret += QString(optArgs, QLatin1Char(']'));
    return ret;
}

void QtDocGenerator::writeDocSnips(QTextStream &s,
                                 const CodeSnipList &codeSnips,
                                 TypeSystem::CodeSnipPosition position,
                                 TypeSystem::Language language)
{
    Indentation indentation(INDENT);
    QStringList invalidStrings;
    const static QString startMarkup = QLatin1String("[sphinx-begin]");
    const static QString endMarkup = QLatin1String("[sphinx-end]");

    invalidStrings << QLatin1String("*") << QLatin1String("//") << QLatin1String("/*") << QLatin1String("*/");

    for (const CodeSnip &snip : codeSnips) {
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
            const QStringList rows = codeBlock.split(QLatin1Char('\n'));
            int currenRow = 0;
            int offset = 0;

            for (QString row : rows) {
                for (const QString &invalidString : qAsConst(invalidStrings))
                    row.remove(invalidString);

                if (row.trimmed().size() == 0) {
                    if (currenRow == 0)
                        continue;
                    else
                        s << endl;
                }

                if (currenRow == 0) {
                    //find offset
                    for (int i=0, i_max = row.size(); i < i_max; i++) {
                        if (row[i] == QLatin1Char(' '))
                            offset++;
                        else if (row[i] == QLatin1Char('\n'))
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

bool QtDocGenerator::writeInjectDocumentation(QTextStream& s,
                                            TypeSystem::DocModificationMode mode,
                                            const AbstractMetaClass* cppClass,
                                            const AbstractMetaFunction* func)
{
    Indentation indentation(INDENT);
    bool didSomething = false;

    const DocModificationList &mods = cppClass->typeEntry()->docModifications();
    for (const DocModification &mod : mods) {
        if (mod.mode() == mode) {
            bool modOk = func ? mod.signature() == func->minimalSignature() : mod.signature().isEmpty();

            if (modOk) {
                Documentation doc;
                Documentation::Format fmt;

                if (mod.format() == TypeSystem::NativeCode)
                    fmt = Documentation::Native;
                else if (mod.format() == TypeSystem::TargetLangCode)
                    fmt = Documentation::Target;
                else
                    continue;

                doc.setValue(mod.code() , fmt);
                writeFormattedText(s, doc, cppClass);
                didSomething = true;
            }
        }
    }

    s << endl;

    // TODO: Deprecate the use of doc string on glue code.
    //       This is pre "add-function" and "inject-documentation" tags.
    const TypeSystem::CodeSnipPosition pos = mode == TypeSystem::DocModificationPrepend
        ? TypeSystem::CodeSnipPositionBeginning : TypeSystem::CodeSnipPositionEnd;
    if (func)
        writeDocSnips(s, func->injectedCodeSnips(), pos, TypeSystem::TargetLangCode);
    else
        writeDocSnips(s, cppClass->typeEntry()->codeSnips(), pos, TypeSystem::TargetLangCode);
    return didSomething;
}

void QtDocGenerator::writeFunctionSignature(QTextStream& s, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func)
{
    QString className;
    if (!func->isConstructor())
        className =  getClassTargetFullName(cppClass) + QLatin1Char('.');
    else if (func->implementingClass() && func->implementingClass()->enclosingClass())
        className =  getClassTargetFullName(func->implementingClass()->enclosingClass()) + QLatin1Char('.');

    QString funcName = getFuncName(func);
    if (!funcName.startsWith(className))
        funcName = className + funcName;

    s << funcName << "(" << parseArgDocStyle(cppClass, func) << ")";
}

QString QtDocGenerator::translateToPythonType(const AbstractMetaType* type, const AbstractMetaClass* cppClass)
{
    QString strType;
    if (type->name() == QLatin1String("QString")) {
        strType = QLatin1String("unicode");
    } else if (type->name() == QLatin1String("QVariant")) {
        strType = QLatin1String("object");
    } else if (type->name() == QLatin1String("QStringList")) {
        strType = QLatin1String("list of strings");
    } else if (type->isConstant() && type->name() == QLatin1String("char") && type->indirections() == 1) {
        strType = QLatin1String("str");
    } else if (type->name().startsWith(QLatin1String("unsigned short"))) {
        strType = QLatin1String("int");
    } else if (type->name().startsWith(QLatin1String("unsigned "))) { // uint and ulong
        strType = QLatin1String("long");
    } else if (type->isContainer()) {
        QString strType = translateType(type, cppClass, Options(ExcludeConst) | ExcludeReference);
        strType.remove(QLatin1Char('*'));
        strType.remove(QLatin1Char('>'));
        strType.remove(QLatin1Char('<'));
        strType.replace(QLatin1String("::"), QLatin1String("."));
        if (strType.contains(QLatin1String("QList")) || strType.contains(QLatin1String("QVector"))) {
            strType.replace(QLatin1String("QList"), QLatin1String("list of "));
            strType.replace(QLatin1String("QVector"), QLatin1String("list of "));
        } else if (strType.contains(QLatin1String("QHash")) || strType.contains(QLatin1String("QMap"))) {
            strType.remove(QLatin1String("QHash"));
            strType.remove(QLatin1String("QMap"));
            QStringList types = strType.split(QLatin1Char(','));
            strType = QString::fromLatin1("Dictionary with keys of type %1 and values of type %2.")
                                         .arg(types[0], types[1]);
        }
    } else {
        QString refTag;
        if (type->isEnum())
            refTag = QLatin1String("attr");
        else
            refTag = QLatin1String("class");
        strType = QLatin1Char(':') + refTag + QLatin1String(":`") + type->fullName() + QLatin1Char('`');
    }
    return strType;
}

void QtDocGenerator::writeParameterType(QTextStream& s, const AbstractMetaClass* cppClass, const AbstractMetaArgument* arg)
{
    s << INDENT << ":param " << arg->name() << ": "
      << translateToPythonType(arg->type(), cppClass) << endl;
}

void QtDocGenerator::writeFunctionParametersType(QTextStream &s, const AbstractMetaClass *cppClass,
                                                 const AbstractMetaFunction *func)
{
    Indentation indentation(INDENT);

    s << endl;
    const AbstractMetaArgumentList &funcArgs = func->arguments();
    for (AbstractMetaArgument *arg : funcArgs) {

        if (func->argumentRemoved(arg->argumentIndex() + 1))
            continue;

        writeParameterType(s, cppClass, arg);
    }

    if (!func->isConstructor() && func->type()) {

        QString retType;
        // check if the return type was modified
        const FunctionModificationList &mods = func->modifications();
        for (const FunctionModification &mod : mods) {
            for (const ArgumentModification &argMod : mod.argument_mods) {
                if (argMod.index == 0) {
                    retType = argMod.modified_type;
                    break;
                }
            }
        }

        if (retType.isEmpty())
            retType = translateToPythonType(func->type(), cppClass);
        s << INDENT << ":rtype: " << retType << endl;
    }
    s << endl;
}

void QtDocGenerator::writeFunction(QTextStream& s, bool writeDoc, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func)
{
    writeFunctionSignature(s, cppClass, func);
    s << endl;

    if (func->typeEntry() && (func->typeEntry()->version() != 0))
        s << ".. note:: This method was introduced in Qt " << func->typeEntry()->version() << endl;

    if (writeDoc) {
        s << endl;
        writeFunctionParametersType(s, cppClass, func);
        s << endl;
        writeInjectDocumentation(s, TypeSystem::DocModificationPrepend, cppClass, func);
        if (!writeInjectDocumentation(s, TypeSystem::DocModificationReplace, cppClass, func))
            writeFormattedText(s, func->documentation(), cppClass);
        writeInjectDocumentation(s, TypeSystem::DocModificationAppend, cppClass, func);
    }
}

static void writeFancyToc(QTextStream& s, const QStringList& items, int cols = 4)
{
    typedef QMap<QChar, QStringList> TocMap;
    TocMap tocMap;
    QChar Q = QLatin1Char('Q');
    QChar idx;
    for (QString item : items) {
        if (item.isEmpty())
            continue;
        if (item.startsWith(Q) && item.length() > 1)
            idx = item[1];
        item.chop(4); // Remove the .rst extension
        tocMap[idx] << item;
    }
    QtXmlToSphinx::Table table;
    QtXmlToSphinx::TableRow row;

    int itemsPerCol = (items.size() + tocMap.size()*2) / cols;
    QString currentColData;
    int i = 0;
    QTextStream ss(&currentColData);
    QMutableMapIterator<QChar, QStringList> it(tocMap);
    while (it.hasNext()) {
        it.next();
        qSort(it.value());

        if (i)
            ss << endl;

        ss << "**" << it.key() << "**" << endl << endl;
        i += 2; // a letter title is equivalent to two entries in space
        for (const QString &item : qAsConst(it.value())) {
            ss << "* :doc:`" << item << "`" << endl;
            ++i;

            // end of column detected!
            if (i > itemsPerCol) {
                ss.flush();
                QtXmlToSphinx::TableCell cell(currentColData);
                row << cell;
                currentColData.clear();
                i = 0;
            }
        }
    }
    if (i) {
        ss.flush();
        QtXmlToSphinx::TableCell cell(currentColData);
        row << cell;
        currentColData.clear();
        i = 0;
    }
    table << row;
    table.normalize();
    s << ".. container:: pysidetoc" << endl << endl;
    s << table;
}

bool QtDocGenerator::finishGeneration()
{
    if (classes().isEmpty())
        return true;

    QMap<QString, QStringList>::iterator it = m_packages.begin();
    for (; it != m_packages.end(); ++it) {
        QString key = it.key();
        key.replace(QLatin1Char('.'), QLatin1Char('/'));
        QString outputDir = outputDirectory() + QLatin1Char('/') + key;
        FileOut output(outputDir + QLatin1String("/index.rst"));
        QTextStream& s = output.stream;

        s << ".. module:: " << it.key() << endl << endl;

        QString title = it.key();
        s << title << endl;
        s << Pad('*', title.length()) << endl << endl;

        /* Avoid showing "Detailed Description for *every* class in toc tree */
        Indentation indentation(INDENT);

        // Search for extra-sections
        if (!m_extraSectionDir.isEmpty()) {
            QDir extraSectionDir(m_extraSectionDir);
            QStringList fileList = extraSectionDir.entryList(QStringList() << (it.key() + QLatin1String("?*.rst")), QDir::Files);
            QStringList::iterator it2 = fileList.begin();
            for (; it2 != fileList.end(); ++it2) {
                QString origFileName(*it2);
                it2->remove(0, it.key().count() + 1);
                QString newFilePath = outputDir + QLatin1Char('/') + *it2;
                if (QFile::exists(newFilePath))
                    QFile::remove(newFilePath);
                if (!QFile::copy(m_extraSectionDir + QLatin1Char('/') + origFileName, newFilePath)) {
                    qCDebug(lcShiboken).noquote().nospace() << "Error copying extra doc "
                        << QDir::toNativeSeparators(m_extraSectionDir + QLatin1Char('/') + origFileName)
                        << " to " << QDir::toNativeSeparators(newFilePath);
                }
            }
            it.value().append(fileList);
        }

        writeFancyToc(s, it.value());

        s << INDENT << ".. container:: hide" << endl << endl;
        {
            Indentation indentation(INDENT);
            s << INDENT << ".. toctree::" << endl;
            Indentation deeperIndentation(INDENT);
            s << INDENT << ":maxdepth: 1" << endl << endl;
            for (const QString &className : qAsConst(it.value()))
                s << INDENT << className << endl;
            s << endl << endl;
        }

        s << "Detailed Description" << endl;
        s << "--------------------" << endl << endl;

        // module doc is always wrong and C++istic, so go straight to the extra directory!
        QFile moduleDoc(m_extraSectionDir + QLatin1Char('/') + it.key() + QLatin1String(".rst"));
        if (moduleDoc.open(QIODevice::ReadOnly | QIODevice::Text)) {
            s << moduleDoc.readAll();
            moduleDoc.close();
        } else {
            // try the normal way
            Documentation moduleDoc = m_docParser->retrieveModuleDocumentation(it.key());
            if (moduleDoc.format() == Documentation::Native) {
                QString context = it.key();
                context.remove(0, context.lastIndexOf(QLatin1Char('.')) + 1);
                QtXmlToSphinx x(this, moduleDoc.value(), context);
                s << x;
            } else {
                s << moduleDoc.value();
            }
        }
    }
    return true;
}

bool QtDocGenerator::doSetup(const QMap<QString, QString>& args)
{
    m_libSourceDir = args.value(QLatin1String("library-source-dir"));
    m_docDataDir = args.value(QLatin1String("documentation-data-dir"));
#ifdef __WIN32__
#   define PATH_SEP ';'
#else
#   define PATH_SEP ':'
#endif
    m_codeSnippetDirs = args.value(QLatin1String("documentation-code-snippets-dir"), m_libSourceDir).split(QLatin1Char(PATH_SEP));
    m_extraSectionDir = args.value(QLatin1String("documentation-extra-sections-dir"));

    m_docParser = args.value(QLatin1String("doc-parser")) == QLatin1String("doxygen")
        ? static_cast<DocParser*>(new DoxygenParser)
        : static_cast<DocParser*>(new QtDocParser);
    qCDebug(lcShiboken).noquote().nospace() << "doc-parser: " << args.value(QLatin1String("doc-parser"));

    if (m_libSourceDir.isEmpty() || m_docDataDir.isEmpty()) {
        qCWarning(lcShiboken) << "Documentation data dir and/or Qt source dir not informed, "
                                 "documentation will not be extracted from Qt sources.";
        return false;
    } else {
        m_docParser->setDocumentationDataDirectory(m_docDataDir);
        m_docParser->setLibrarySourceDirectory(m_libSourceDir);
    }
    return true;
}


Generator::OptionDescriptions QtDocGenerator::options() const
{
    return OptionDescriptions()
        << qMakePair(QLatin1String("doc-parser"),
                     QLatin1String("The documentation parser used to interpret the documentation input files (qdoc3|doxygen)"))
        << qMakePair(QLatin1String("documentation-code-snippets-dir"),
                     QLatin1String("Directory used to search code snippets used by the documentation"))
        << qMakePair(QLatin1String("documentation-data-dir"),
                     QLatin1String("Directory with XML files generated by documentation tool (qdoc3 or Doxygen)"))
        << qMakePair(QLatin1String("documentation-extra-sections-dir"),
                     QLatin1String("Directory used to search for extra documentation sections"))
        << qMakePair(QLatin1String("library-source-dir"),
                     QLatin1String("Directory where library source code is located"));
}

