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

#include "qtxmltosphinx.h"
#include "fileout.h"
#include "messages.h"
#include "rstformat.h"
#include "qtdocgenerator.h"
#include <abstractmetafunction.h>
#include <abstractmetalang.h>
#include <reporthandler.h>
#include <typedatabase.h>
#include <typesystem.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QXmlStreamReader>

static inline QString nameAttribute() { return QStringLiteral("name"); }
static inline QString titleAttribute() { return QStringLiteral("title"); }
static inline QString fullTitleAttribute() { return QStringLiteral("fulltitle"); }
static inline QString briefAttribute() { return QStringLiteral("brief"); }

struct QtXmlToSphinx::LinkContext
{
    enum Type
    {
        Method = 0x1, Function = 0x2,
        FunctionMask = Method | Function,
        Class = 0x4, Attribute = 0x8, Module = 0x10,
        Reference = 0x20, External= 0x40
    };

    enum Flags { InsideBold = 0x1, InsideItalic = 0x2 };

    explicit LinkContext(const QString &ref) : linkRef(ref) {}

    QString linkRef;
    QString linkText;
    Type type = Reference;
    int flags = 0;
};

static const char *linkKeyWord(QtXmlToSphinx::LinkContext::Type type)
{
    switch (type) {
    case QtXmlToSphinx::LinkContext::Method:
        return ":meth:";
    case QtXmlToSphinx::LinkContext::Function:
        return ":func:";
    case QtXmlToSphinx::LinkContext::Class:
        return ":class:";
    case QtXmlToSphinx::LinkContext::Attribute:
        return ":attr:";
    case QtXmlToSphinx::LinkContext::Module:
        return ":mod:";
    case QtXmlToSphinx::LinkContext::Reference:
        return ":ref:";
    case QtXmlToSphinx::LinkContext::External:
        break;
    case QtXmlToSphinx::LinkContext::FunctionMask:
        break;
     }
    return "";
}

TextStream &operator<<(TextStream &str, const QtXmlToSphinx::LinkContext &linkContext)
{
    // Temporarily turn off bold/italic since links do not work within
    if (linkContext.flags & QtXmlToSphinx::LinkContext::InsideBold)
        str << "**";
    else if (linkContext.flags & QtXmlToSphinx::LinkContext::InsideItalic)
        str << '*';
    str << ' ' << linkKeyWord(linkContext.type) << '`';
    const bool isExternal = linkContext.type == QtXmlToSphinx::LinkContext::External;
    if (!linkContext.linkText.isEmpty()) {
        writeEscapedRstText(str, linkContext.linkText);
        if (isExternal && !linkContext.linkText.endsWith(QLatin1Char(' ')))
            str << ' ';
        str << '<';
    }
    // Convert page titles to RST labels
    str << (linkContext.type == QtXmlToSphinx::LinkContext::Reference
        ? toRstLabel(linkContext.linkRef) : linkContext.linkRef);
    if (!linkContext.linkText.isEmpty())
        str << '>';
    str << '`';
    if (isExternal)
        str << '_';
    str << ' ';
    if (linkContext.flags & QtXmlToSphinx::LinkContext::InsideBold)
        str << "**";
    else if (linkContext.flags & QtXmlToSphinx::LinkContext::InsideItalic)
        str << '*';
    return str;
}

enum class WebXmlTag {
    Unknown,
    heading, brief, para, italic, bold, see_also, snippet, dots, codeline,
    table, header, row, item, argument, teletype, link, inlineimage, image,
    list, term, raw, underline, superscript, code, badcode, legalese,
    rst, section, quotefile,
    // ignored tags
    generatedlist, tableofcontents, quotefromfile, skipto, target, page, group,
    // useless tags
    description, definition, printuntil, relation,
    // Doxygen tags
    title, ref, computeroutput, detaileddescription, name, listitem,
    parametername, parameteritem, ulink, itemizedlist, parameternamelist,
    parameterlist,
    // Doxygen ignored tags
    highlight, linebreak, programlisting, xreftitle, sp, entry, simplesect,
    verbatim, xrefsect, xrefdescription,
};

using WebXmlTagHash = QHash<QStringView, WebXmlTag>;

static const WebXmlTagHash &webXmlTagHash()
{
    static const WebXmlTagHash result = {
        {u"heading", WebXmlTag::heading},
        {u"brief", WebXmlTag::brief},
        {u"para", WebXmlTag::para},
        {u"italic", WebXmlTag::italic},
        {u"bold", WebXmlTag::bold},
        {u"see-also", WebXmlTag::see_also},
        {u"snippet", WebXmlTag::snippet},
        {u"dots", WebXmlTag::dots},
        {u"codeline", WebXmlTag::codeline},
        {u"table", WebXmlTag::table},
        {u"header", WebXmlTag::header},
        {u"row", WebXmlTag::row},
        {u"item", WebXmlTag::item},
        {u"argument", WebXmlTag::argument},
        {u"teletype", WebXmlTag::teletype},
        {u"link", WebXmlTag::link},
        {u"inlineimage", WebXmlTag::inlineimage},
        {u"image", WebXmlTag::image},
        {u"list", WebXmlTag::list},
        {u"term", WebXmlTag::term},
        {u"raw", WebXmlTag::raw},
        {u"underline", WebXmlTag::underline},
        {u"superscript", WebXmlTag::superscript},
        {u"code", WebXmlTag::code},
        {u"badcode", WebXmlTag::badcode},
        {u"legalese", WebXmlTag::legalese},
        {u"rst", WebXmlTag::rst},
        {u"section", WebXmlTag::section},
        {u"quotefile", WebXmlTag::quotefile},
        {u"generatedlist", WebXmlTag::generatedlist},
        {u"tableofcontents", WebXmlTag::tableofcontents},
        {u"quotefromfile", WebXmlTag::quotefromfile},
        {u"skipto", WebXmlTag::skipto},
        {u"target", WebXmlTag::target},
        {u"page", WebXmlTag::page},
        {u"group", WebXmlTag::group},
        {u"description", WebXmlTag::description},
        {u"definition", WebXmlTag::definition},
        {u"printuntil", WebXmlTag::printuntil},
        {u"relation", WebXmlTag::relation},
        {u"title", WebXmlTag::title},
        {u"ref", WebXmlTag::ref},
        {u"computeroutput", WebXmlTag::computeroutput},
        {u"detaileddescription", WebXmlTag::detaileddescription},
        {u"name", WebXmlTag::name},
        {u"listitem", WebXmlTag::listitem},
        {u"parametername", WebXmlTag::parametername},
        {u"parameteritem", WebXmlTag::parameteritem},
        {u"ulink", WebXmlTag::ulink},
        {u"itemizedlist", WebXmlTag::itemizedlist},
        {u"parameternamelist", WebXmlTag::parameternamelist},
        {u"parameterlist", WebXmlTag::parameterlist},
        {u"highlight", WebXmlTag::highlight},
        {u"linebreak", WebXmlTag::linebreak},
        {u"programlisting", WebXmlTag::programlisting},
        {u"xreftitle", WebXmlTag::xreftitle},
        {u"sp", WebXmlTag::sp},
        {u"entry", WebXmlTag::entry},
        {u"simplesect", WebXmlTag::simplesect},
        {u"verbatim", WebXmlTag::verbatim},
        {u"xrefsect", WebXmlTag::xrefsect},
        {u"xrefdescription", WebXmlTag::xrefdescription},
    };
    return result;
}

QtXmlToSphinx::QtXmlToSphinx(const QtDocGenerator *generator,
                             const QString& doc, const QString& context)
        : m_output(static_cast<QString *>(nullptr)),
          m_tableHasHeader(false), m_context(context), m_generator(generator),
          m_insideBold(false), m_insideItalic(false)
{
    m_result = transform(doc);
}

QtXmlToSphinx::~QtXmlToSphinx() = default;

void QtXmlToSphinx::callHandler(WebXmlTag t, QXmlStreamReader &r)
{
    switch (t) {
    case WebXmlTag::heading:
        handleHeadingTag(r);
        break;
    case WebXmlTag::brief:
    case WebXmlTag::para:
        handleParaTag(r);
        break;
    case WebXmlTag::italic:
        handleItalicTag(r);
        break;
    case WebXmlTag::bold:
        handleBoldTag(r);
        break;
    case WebXmlTag::see_also:
        handleSeeAlsoTag(r);
        break;
    case WebXmlTag::snippet:
        handleSnippetTag(r);
        break;
    case WebXmlTag::dots:
    case WebXmlTag::codeline:
        handleDotsTag(r);
        break;
    case WebXmlTag::table:
        handleTableTag(r);
        break;
    case WebXmlTag::header:
        handleRowTag(r);
        break;
    case WebXmlTag::row:
        handleRowTag(r);
        break;
    case WebXmlTag::item:
        handleItemTag(r);
        break;
    case WebXmlTag::argument:
        handleArgumentTag(r);
        break;
    case WebXmlTag::teletype:
        handleArgumentTag(r);
        break;
    case WebXmlTag::link:
        handleLinkTag(r);
        break;
    case WebXmlTag::inlineimage:
        handleInlineImageTag(r);
        break;
    case WebXmlTag::image:
        handleImageTag(r);
        break;
    case WebXmlTag::list:
        handleListTag(r);
        break;
    case WebXmlTag::term:
        handleTermTag(r);
        break;
    case WebXmlTag::raw:
        handleRawTag(r);
        break;
    case WebXmlTag::underline:
        handleItalicTag(r);
        break;
    case WebXmlTag::superscript:
        handleSuperScriptTag(r);
        break;
    case WebXmlTag::code:
    case WebXmlTag::badcode:
    case WebXmlTag::legalese:
        handleCodeTag(r);
        break;
    case WebXmlTag::rst:
        handleRstPassTroughTag(r);
        break;
    case WebXmlTag::section:
        handleAnchorTag(r);
        break;
    case WebXmlTag::quotefile:
        handleQuoteFileTag(r);
        break;
    case WebXmlTag::generatedlist:
    case WebXmlTag::tableofcontents:
    case WebXmlTag::quotefromfile:
    case WebXmlTag::skipto:
        handleIgnoredTag(r);
        break;
    case WebXmlTag::target:
        handleTargetTag(r);
        break;
    case WebXmlTag::page:
    case WebXmlTag::group:
        handlePageTag(r);
        break;
    case WebXmlTag::description:
    case WebXmlTag::definition:
    case WebXmlTag::printuntil:
    case WebXmlTag::relation:
        handleUselessTag(r);
        break;
    case WebXmlTag::title:
        handleHeadingTag(r);
        break;
    case WebXmlTag::ref:
    case WebXmlTag::computeroutput:
    case WebXmlTag::detaileddescription:
    case WebXmlTag::name:
        handleParaTag(r);
        break;
    case WebXmlTag::listitem:
    case WebXmlTag::parametername:
    case WebXmlTag::parameteritem:
        handleItemTag(r);
        break;
    case WebXmlTag::ulink:
        handleLinkTag(r);
        break;
    case WebXmlTag::itemizedlist:
    case WebXmlTag::parameternamelist:
    case WebXmlTag::parameterlist:
        handleListTag(r);
        break;
    case WebXmlTag::highlight:
    case WebXmlTag::linebreak:
    case WebXmlTag::programlisting:
    case WebXmlTag::xreftitle:
    case WebXmlTag::sp:
    case WebXmlTag::entry:
    case WebXmlTag::simplesect:
    case WebXmlTag::verbatim:
    case WebXmlTag::xrefsect:
    case WebXmlTag::xrefdescription:
        handleIgnoredTag(r);
        break;
    case WebXmlTag::Unknown:
        break;
    }
}

void QtXmlToSphinx::pushOutputBuffer()
{
    auto *buffer = new QString();
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

QString QtXmlToSphinx::expandFunction(const QString& function) const
{
    const int firstDot = function.indexOf(QLatin1Char('.'));
    const AbstractMetaClass *metaClass = nullptr;
    if (firstDot != -1) {
        const auto className = QStringView{function}.left(firstDot);
        for (const AbstractMetaClass *cls : m_generator->classes()) {
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

QString QtXmlToSphinx::resolveContextForMethod(const QString& methodName) const
{
    const auto currentClass = QStringView{m_context}.split(QLatin1Char('.')).constLast();

    const AbstractMetaClass *metaClass = nullptr;
    for (const AbstractMetaClass *cls : m_generator->classes()) {
        if (cls->name() == currentClass) {
            metaClass = cls;
            break;
        }
    }

    if (metaClass) {
        AbstractMetaFunctionList funcList;
        const AbstractMetaFunctionList &methods = metaClass->queryFunctionsByName(methodName);
        for (AbstractMetaFunction *func : methods) {
            if (methodName == func->name())
                funcList.append(func);
        }

        const AbstractMetaClass *implementingClass = nullptr;
        for (AbstractMetaFunction *func : qAsConst(funcList)) {
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
    Indentation indentation(m_output);
    if (doc.trimmed().isEmpty())
        return doc;

    pushOutputBuffer();

    QXmlStreamReader reader(doc);

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (reader.hasError()) {
            QString message;
            QTextStream(&message) << "XML Error "
                << reader.errorString() << " at " << reader.lineNumber()
                << ':' << reader.columnNumber() << '\n' << doc;
            m_output << message;
            qCWarning(lcShibokenDoc).noquote().nospace() << message;
            break;
        }

        if (token == QXmlStreamReader::StartElement) {
            WebXmlTag tag = webXmlTagHash().value(reader.name(), WebXmlTag::Unknown);
            if (!m_tagStack.isEmpty() && tag == WebXmlTag::raw)
                tag = WebXmlTag::Unknown;
            m_tagStack.push(tag);
        }

        if (!m_tagStack.isEmpty())
            callHandler(m_tagStack.top(), reader);

        if (token == QXmlStreamReader::EndElement) {
            m_tagStack.pop();
            m_lastTagName = reader.name().toString();
        }
    }

    if (!m_inlineImages.isEmpty()) {
        // Write out inline image definitions stored in handleInlineImageTag().
        m_output << '\n' << disableIndent;
        for (const InlineImage &img : qAsConst(m_inlineImages))
            m_output << ".. |" << img.tag << "| image:: " << img.href << '\n';
        m_output << '\n' << enableIndent;
        m_inlineImages.clear();
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
    QString resolvedPath;
    if (path.endsWith(QLatin1String(".cpp"))) {
        const QString pySnippet = path.left(path.size() - 3) + QLatin1String("py");
        resolvedPath = resolveFile(locations, pySnippet);
    }
    if (resolvedPath.isEmpty())
        resolvedPath = resolveFile(locations, path);
    if (resolvedPath.isEmpty()) {
        QTextStream(errorMessage) << "Could not resolve \"" << path << "\" in \""
           << locations.join(QLatin1String("\", \""));
        return QString(); // null
    }
    qCDebug(lcShibokenDoc).noquote().nospace() << "snippet file " << path
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
    static int headingSize = 0;
    static char type;
    static char types[] = { '-', '^' };
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        uint typeIdx = reader.attributes().value(QLatin1String("level")).toUInt();
        if (typeIdx >= sizeof(types))
            type = types[sizeof(types)-1];
        else
            type = types[typeIdx];
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << disableIndent << Pad(type, headingSize) << "\n\n"
            << enableIndent;
    } else if (token == QXmlStreamReader::Characters) {
        m_output << "\n\n" << disableIndent;
        headingSize = writeEscapedRstText(m_output, reader.text().trimmed());
        m_output << '\n' << enableIndent;
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

        m_output << result << "\n\n";
    } else if (token == QXmlStreamReader::Characters) {
        const auto  text = reader.text();
        const QChar end = m_output.lastChar();
        if (!text.isEmpty() && m_output.indentation() == 0 && !end.isNull()) {
            QChar start = text[0];
            if ((end == QLatin1Char('*') || end == QLatin1Char('`')) && start != QLatin1Char(' ') && !start.isPunct())
                m_output << '\\';
        }
        m_output << escape(text);
    }
}

void QtXmlToSphinx::handleItalicTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement) {
        m_insideItalic = !m_insideItalic;
        m_output << '*';
    } else if (token == QXmlStreamReader::Characters) {
        m_output << escape(reader.text().trimmed());
    }
}

void QtXmlToSphinx::handleBoldTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement) {
        m_insideBold = !m_insideBold;
        m_output << "**";
    } else if (token == QXmlStreamReader::Characters) {
        m_output << escape(reader.text().trimmed());
    }
}

void QtXmlToSphinx::handleArgumentTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement || token == QXmlStreamReader::EndElement)
        m_output << "``";
    else if (token == QXmlStreamReader::Characters)
        m_output << reader.text().trimmed();
}

static inline QString functionLinkType() { return QStringLiteral("function"); }
static inline QString classLinkType() { return QStringLiteral("class"); }

static inline QString fixLinkType(QStringView type)
{
    // TODO: create a flag PROPERTY-AS-FUNCTION to ask if the properties
    // are recognized as such or not in the binding
    if (type == QLatin1String("property"))
        return functionLinkType();
    if (type == QLatin1String("typedef"))
        return classLinkType();
    return type.toString();
}

static inline QString linkSourceAttribute(const QString &type)
{
    if (type == functionLinkType() || type == classLinkType())
        return QLatin1String("raw");
    return type == QLatin1String("enum") || type == QLatin1String("page")
        ? type : QLatin1String("href");
}

// "See also" links may appear as nested links:
//     <see-also>QAbstractXmlReceiver<link raw="isValid()" href="qxmlquery.html#isValid" type="function">isValid()</link>
//     which is handled in handleLinkTag
// or direct text:
//     <see-also>rootIsDecorated()</see-also>
//     which is handled here.

void QtXmlToSphinx::handleSeeAlsoTag(QXmlStreamReader& reader)
{
    switch (reader.tokenType()) {
    case QXmlStreamReader::StartElement:
        m_output << ".. seealso:: ";
        break;
    case QXmlStreamReader::Characters: {
        // Direct embedded link: <see-also>rootIsDecorated()</see-also>
        const auto  textR = reader.text().trimmed();
        if (!textR.isEmpty()) {
            const QString text = textR.toString();
            if (m_seeAlsoContext.isNull()) {
                const QString type = text.endsWith(QLatin1String("()"))
                    ? functionLinkType() : classLinkType();
                m_seeAlsoContext.reset(handleLinkStart(type, text));
            }
            handleLinkText(m_seeAlsoContext.data(), text);
        }
    }
        break;
    case QXmlStreamReader::EndElement:
        if (!m_seeAlsoContext.isNull()) { // direct, no nested </link> seen
            handleLinkEnd(m_seeAlsoContext.data());
            m_seeAlsoContext.reset();
        }
        m_output << "\n\n";
        break;
    default:
        break;
    }
}

static inline QString fallbackPathAttribute() { return QStringLiteral("path"); }

static inline bool snippetComparison()
{
    return ReportHandler::debugLevel() >= ReportHandler::FullDebug;
}

template <class Indent> // const char*/class Indentor
void formatSnippet(TextStream &str, Indent indent, const QString &snippet)
{
    const auto lines = QStringView{snippet}.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        if (!line.trimmed().isEmpty())
            str << indent << line;
        str << '\n';
    }
}

static QString msgSnippetComparison(const QString &location, const QString &identifier,
                                    const QString &pythonCode, const QString &fallbackCode)
{
    StringStream str;
    str.setTabWidth(2);
    str << "Python snippet " << location;
    if (!identifier.isEmpty())
        str << " [" << identifier << ']';
    str << ":\n" << indent << pythonCode << ensureEndl << outdent
        << "Corresponding fallback snippet:\n"
        << indent << fallbackCode << ensureEndl << outdent << "-- end --\n";
    return str;
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
            qCWarning(lcShibokenDoc, "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
        // Fall back to C++ snippet when "path" attribute is present.
        // Also read fallback snippet when comparison is desired.
        QString fallbackCode;
        if ((pythonCode.isEmpty() || snippetComparison())
            && reader.attributes().hasAttribute(fallbackPathAttribute())) {
            const QString fallback = reader.attributes().value(fallbackPathAttribute()).toString();
            if (QFileInfo::exists(fallback)) {
                if (pythonCode.isEmpty())
                    qCWarning(lcShibokenDoc, "%s", qPrintable(msgFallbackWarning(reader, m_context, m_lastTagName, location, identifier, fallback)));
                fallbackCode = readFromLocation(fallback, identifier, &errorMessage);
                if (!errorMessage.isEmpty())
                    qCWarning(lcShibokenDoc, "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
            }
        }

        if (!pythonCode.isEmpty() && !fallbackCode.isEmpty() && snippetComparison())
            qCDebug(lcShibokenDoc, "%s", qPrintable(msgSnippetComparison(location, identifier, pythonCode, fallbackCode)));

        if (!consecutiveSnippet)
            m_output << "::\n\n";

        Indentation indentation(m_output);
        const QString code = pythonCode.isEmpty() ? fallbackCode : pythonCode;
        if (code.isEmpty())
            m_output << "<Code snippet \"" << location << ':' << identifier << "\" not found>\n";
        else
            m_output << code << ensureEndl;
        m_output << '\n';
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
        } else {
            m_output << "::\n\n";
        }
        Indentation indentation(m_output);
        pushOutputBuffer();
        int indent = reader.attributes().value(QLatin1String("indent")).toInt();
        for (int i = 0; i < indent; ++i)
            m_output << ' ';
    } else if (token == QXmlStreamReader::Characters) {
        m_output << reader.text().toString();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << disableIndent << popOutputBuffer() << "\n\n\n" << enableIndent;
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
        m_currentTable.setHeaderEnabled(m_tableHasHeader);
        m_currentTable.normalize();
        m_output << ensureEndl;
        m_currentTable.format(m_output);
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
        m_currentTable.appendRow(TableRow(1, cell));
    }
}


void QtXmlToSphinx::handleItemTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        if (m_currentTable.isEmpty())
            m_currentTable.appendRow({});
        TableRow& row = m_currentTable.last();
        TableCell cell;
        cell.colSpan = reader.attributes().value(QLatin1String("colspan")).toShort();
        cell.rowSpan = reader.attributes().value(QLatin1String("rowspan")).toShort();
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
        m_currentTable.appendRow({});
    }
}

enum ListType { BulletList, OrderedList, EnumeratedList };

static inline ListType webXmlListType(QStringView t)
{
    if (t == QLatin1String("enum"))
        return EnumeratedList;
    if (t == QLatin1String("ordered"))
        return OrderedList;
    return BulletList;
}

void QtXmlToSphinx::handleListTag(QXmlStreamReader& reader)
{
    // BUG We do not support a list inside a table cell
    static ListType listType = BulletList;
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        listType = webXmlListType(reader.attributes().value(QLatin1String("type")));
        if (listType == EnumeratedList) {
            m_currentTable.appendRow(TableRow{TableCell(QLatin1String("Constant")),
                                              TableCell(QLatin1String("Description"))});
            m_tableHasHeader = true;
        }
        m_output.indent();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output.outdent();
        if (!m_currentTable.isEmpty()) {
            switch (listType) {
            case BulletList:
            case OrderedList: {
                m_output << '\n';
                const char *separator = listType == BulletList ? "* " : "#. ";
                const char *indentLine = listType == BulletList ? "  " : "   ";
                for (const TableCell &cell : m_currentTable.constFirst()) {
                    const auto itemLines = QStringView{cell.data}.split(QLatin1Char('\n'));
                    m_output << separator << itemLines.constFirst() << '\n';
                    for (int i = 1, max = itemLines.count(); i < max; ++i)
                        m_output << indentLine << itemLines[i] << '\n';
                }
                m_output << '\n';
            }
                break;
            case EnumeratedList:
                m_currentTable.setHeaderEnabled(m_tableHasHeader);
                m_currentTable.normalize();
                m_output << ensureEndl;
                m_currentTable.format(m_output);
                break;
            }
        }
        m_currentTable.clear();
    }
}

void QtXmlToSphinx::handleLinkTag(QXmlStreamReader& reader)
{
    switch (reader.tokenType()) {
    case QXmlStreamReader::StartElement: {
        // <link> embedded in <see-also> means the characters of <see-also> are no link.
        m_seeAlsoContext.reset();
        const QString type = fixLinkType(reader.attributes().value(QLatin1String("type")));
        const QString ref = reader.attributes().value(linkSourceAttribute(type)).toString();
        m_linkContext.reset(handleLinkStart(type, ref));
    }
        break;
    case QXmlStreamReader::Characters:
        Q_ASSERT(!m_linkContext.isNull());
        handleLinkText(m_linkContext.data(), reader.text().toString());
        break;
    case QXmlStreamReader::EndElement:
        Q_ASSERT(!m_linkContext.isNull());
        handleLinkEnd(m_linkContext.data());
        m_linkContext.reset();
        break;
    default:
        break;
    }
}

QtXmlToSphinx::LinkContext *QtXmlToSphinx::handleLinkStart(const QString &type, QString ref) const
{
    ref.replace(QLatin1String("::"), QLatin1String("."));
    ref.remove(QLatin1String("()"));
    auto *result = new LinkContext(ref);

    if (m_insideBold)
        result->flags |= LinkContext::InsideBold;
    else if (m_insideItalic)
        result->flags |= LinkContext::InsideItalic;

    if (type == functionLinkType() && !m_context.isEmpty()) {
        result->type = LinkContext::Method;
        const auto rawlinklist = QStringView{result->linkRef}.split(QLatin1Char('.'));
        if (rawlinklist.size() == 1 || rawlinklist.constFirst() == m_context) {
            QString context = resolveContextForMethod(rawlinklist.constLast().toString());
            if (!result->linkRef.startsWith(context))
                result->linkRef.prepend(context + QLatin1Char('.'));
        } else {
            result->linkRef = expandFunction(result->linkRef);
        }
    } else if (type == functionLinkType() && m_context.isEmpty()) {
        result->type = LinkContext::Function;
    } else if (type == classLinkType()) {
        result->type = LinkContext::Class;
        if (const TypeEntry *type = TypeDatabase::instance()->findType(result->linkRef)) {
            result->linkRef = type->qualifiedTargetLangName();
        } else { // fall back to the old heuristic if the type wasn't found.
            const auto rawlinklist = QStringView{result->linkRef}.split(QLatin1Char('.'));
            QStringList splittedContext = m_context.split(QLatin1Char('.'));
            if (rawlinklist.size() == 1 || rawlinklist.constFirst() == splittedContext.constLast()) {
                splittedContext.removeLast();
                result->linkRef.prepend(QLatin1Char('~') + splittedContext.join(QLatin1Char('.'))
                                                 + QLatin1Char('.'));
            }
        }
    } else if (type == QLatin1String("enum")) {
        result->type = LinkContext::Attribute;
    } else if (type == QLatin1String("page")) {
        // Module, external web page or reference
        if (result->linkRef == m_generator->moduleName())
            result->type = LinkContext::Module;
        else if (result->linkRef.startsWith(QLatin1String("http")))
            result->type = LinkContext::External;
        else
            result->type = LinkContext::Reference;
    } else if (type == QLatin1String("external")) {
        result->type = LinkContext::External;
    } else {
        result->type = LinkContext::Reference;
    }
    return result;
}

// <link raw="Model/View Classes" href="model-view-programming.html#model-view-classes"
//  type="page" page="Model/View Programming">Model/View Classes</link>
// <link type="page" page="http://doc.qt.io/qt-5/class.html">QML types</link>
// <link raw="Qt Quick" href="qtquick-index.html" type="page" page="Qt Quick">Qt Quick</link>
// <link raw="QObject" href="qobject.html" type="class">QObject</link>
// <link raw="Qt::Window" href="qt.html#WindowType-enum" type="enum" enum="Qt::WindowType">Qt::Window</link>
// <link raw="QNetworkSession::reject()" href="qnetworksession.html#reject" type="function">QNetworkSession::reject()</link>

static QString fixLinkText(const QtXmlToSphinx::LinkContext *linkContext,
                           QString linktext)
{
    if (linkContext->type == QtXmlToSphinx::LinkContext::External
        || linkContext->type == QtXmlToSphinx::LinkContext::Reference) {
        return linktext;
    }
    // For the language reference documentation, strip the module name.
    // Clear the link text if that matches the function/class/enumeration name.
    const int lastSep = linktext.lastIndexOf(QLatin1String("::"));
    if (lastSep != -1)
        linktext.remove(0, lastSep + 2);
    else
         QtXmlToSphinx::stripPythonQualifiers(&linktext);
    if (linkContext->linkRef == linktext)
        return QString();
    if ((linkContext->type & QtXmlToSphinx::LinkContext::FunctionMask) != 0
        && (linkContext->linkRef + QLatin1String("()")) == linktext) {
        return QString();
    }
    return  linktext;
}

void QtXmlToSphinx::handleLinkText(LinkContext *linkContext, const QString &linktext) const
{
    linkContext->linkText = fixLinkText(linkContext, linktext);
}

void QtXmlToSphinx::handleLinkEnd(LinkContext *linkContext)
{
    m_output << *linkContext;
}

// Copy images that are placed in a subdirectory "images" under the webxml files
// by qdoc to a matching subdirectory under the "rst/PySide6/<module>" directory
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
    qCDebug(lcShibokenDoc()).noquote().nospace() << __FUNCTION__ << " href=\""
        << href << "\", context=\"" << context << "\", docDataDir=\""
        << docDataDir << "\", outputDir=\"" << outputDir << "\", copied \""
        << source.fileName() << "\"->\"" << targetFileName << '"';
    return true;
}

bool QtXmlToSphinx::copyImage(const QString &href) const
{
    QString errorMessage;
    const bool result =
        ::copyImage(href, m_generator->docDataDir(), m_context,
                    m_generator->outputDirectory(), &errorMessage);
    if (!result)
        qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
    return result;
}

void QtXmlToSphinx::handleImageTag(QXmlStreamReader& reader)
{
    if (reader.tokenType() != QXmlStreamReader::StartElement)
        return;
    const QString href = reader.attributes().value(QLatin1String("href")).toString();
    if (copyImage(href))
        m_output << ".. image:: " <<  href << "\n\n";
}

void QtXmlToSphinx::handleInlineImageTag(QXmlStreamReader& reader)
{
    if (reader.tokenType() != QXmlStreamReader::StartElement)
        return;
    const QString href = reader.attributes().value(QLatin1String("href")).toString();
    if (!copyImage(href))
        return;
    // Handle inline images by substitution references. Insert a unique tag
    // enclosed by '|' and define it further down. Determine tag from the base
    //file name with number.
    QString tag = href;
    int pos = tag.lastIndexOf(QLatin1Char('/'));
    if (pos != -1)
        tag.remove(0, pos + 1);
    pos = tag.indexOf(QLatin1Char('.'));
    if (pos != -1)
        tag.truncate(pos);
    tag += QString::number(m_inlineImages.size() + 1);
    m_inlineImages.append(InlineImage{tag, href});
    m_output << '|' << tag << '|' << ' ';
}

void QtXmlToSphinx::handleRawTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        QString format = reader.attributes().value(QLatin1String("format")).toString();
        m_output << ".. raw:: " << format.toLower() << "\n\n";
    } else if (token == QXmlStreamReader::Characters) {
        Indentation indent(m_output);
        m_output << reader.text();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << "\n\n";
    }
}

void QtXmlToSphinx::handleCodeTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement) {
        m_output << "::\n\n" << indent;
    } else if (token == QXmlStreamReader::Characters) {
        Indentation indent(m_output);
        m_output << reader.text();
    } else if (token == QXmlStreamReader::EndElement) {
        m_output << outdent << "\n\n";
    }
}

void QtXmlToSphinx::handleUnknownTag(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType token = reader.tokenType();
    if (token == QXmlStreamReader::StartElement)
        qCDebug(lcShibokenDoc).noquote().nospace() << "Unknown QtDoc tag: \"" << reader.name().toString() << "\".";
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

void QtXmlToSphinx::handlePageTag(QXmlStreamReader &reader)
{
    if (reader.tokenType() != QXmlStreamReader::StartElement)
        return;

    m_output << disableIndent;

    const auto  title = reader.attributes().value(titleAttribute());
    if (!title.isEmpty())
        m_output << rstLabel(title.toString());

    const auto  fullTitle = reader.attributes().value(fullTitleAttribute());
    const int size = fullTitle.isEmpty()
       ? writeEscapedRstText(m_output, title)
       : writeEscapedRstText(m_output, fullTitle);

    m_output << '\n' << Pad('*', size) << "\n\n"
        << enableIndent;
}

void QtXmlToSphinx::handleTargetTag(QXmlStreamReader &reader)
{
    if (reader.tokenType() != QXmlStreamReader::StartElement)
        return;
    const auto  name = reader.attributes().value(nameAttribute());
    if (!name.isEmpty())
        m_output << rstLabel(name.toString());
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
            if (!m_context.isEmpty())
                anchor.prepend(m_context + QLatin1Char('_'));
            m_output << rstLabel(anchor);
        }
   } else if (token == QXmlStreamReader::EndElement) {
       m_opened_anchor.clear();
   }
}

void QtXmlToSphinx::handleRstPassTroughTag(QXmlStreamReader& reader)
{
    if (reader.tokenType() == QXmlStreamReader::Characters)
        m_output << reader.text();
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
            qCWarning(lcShibokenDoc, "%s", qPrintable(msgTagWarning(reader, m_context, m_lastTagName, errorMessage)));
        m_output << "::\n\n";
        Indentation indentation(m_output);
        if (code.isEmpty())
            m_output << "<Code snippet \"" << location << "\" not found>\n";
        else
            m_output << code << ensureEndl;
        m_output << '\n';
    }
}

bool QtXmlToSphinx::convertToRst(const QtDocGenerator *generator,
                                 const QString &sourceFileName,
                                 const QString &targetFileName,
                                 const QString &context, QString *errorMessage)
{
    QFile sourceFile(sourceFileName);
    if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = msgCannotOpenForReading(sourceFile);
        return false;
    }
    const QString doc = QString::fromUtf8(sourceFile.readAll());
    sourceFile.close();

    FileOut targetFile(targetFileName);
    QtXmlToSphinx x(generator, doc, context);
    targetFile.stream << x;
    return targetFile.done(errorMessage) != FileOut::Failure;
}

void QtXmlToSphinx::Table::normalize()
{
    if (m_normalized || isEmpty())
        return;

    //QDoc3 generates tables with wrong number of columns. We have to
    //check and if necessary, merge the last columns.
    int maxCols = -1;
    for (const auto &row : qAsConst(m_rows)) {
        if (row.count() > maxCols)
            maxCols = row.count();
    }
    if (maxCols <= 0)
        return;
    // add col spans
    for (int row = 0; row < m_rows.count(); ++row) {
        for (int col = 0; col < m_rows.at(row).count(); ++col) {
            QtXmlToSphinx::TableCell& cell = m_rows[row][col];
            bool mergeCols = (col >= maxCols);
            if (cell.colSpan > 0) {
                QtXmlToSphinx::TableCell newCell;
                newCell.colSpan = -1;
                for (int i = 0, max = cell.colSpan-1; i < max; ++i) {
                    m_rows[row].insert(col + 1, newCell);
                }
                cell.colSpan = 0;
                col++;
            } else if (mergeCols) {
                m_rows[row][maxCols - 1].data += QLatin1Char(' ') + cell.data;
            }
        }
    }

    // row spans
    const int numCols = m_rows.constFirst().count();
    for (int col = 0; col < numCols; ++col) {
        for (int row = 0; row < m_rows.count(); ++row) {
            if (col < m_rows[row].count()) {
                QtXmlToSphinx::TableCell& cell = m_rows[row][col];
                if (cell.rowSpan > 0) {
                    QtXmlToSphinx::TableCell newCell;
                    newCell.rowSpan = -1;
                    int targetRow = row + 1;
                    const int targetEndRow =
                        std::min(targetRow + cell.rowSpan - 1, int(m_rows.count()));
                    cell.rowSpan = 0;
                    for ( ; targetRow < targetEndRow; ++targetRow)
                        m_rows[targetRow].insert(col, newCell);
                    row++;
                }
            }
        }
    }
    m_normalized = true;
}

void QtXmlToSphinx::Table::format(TextStream& s) const
{
    if (isEmpty())
        return;

    if (!isNormalized()) {
        qCDebug(lcShibokenDoc) << "Attempt to print an unnormalized table!";
        return;
    }

    // calc width and height of each column and row
    const int headerColumnCount = m_rows.constFirst().count();
    QList<int> colWidths(headerColumnCount);
    QList<int> rowHeights(m_rows.count());
    for (int i = 0, maxI = m_rows.count(); i < maxI; ++i) {
        const QtXmlToSphinx::TableRow& row = m_rows.at(i);
        for (int j = 0, maxJ = std::min(row.count(), colWidths.size()); j < maxJ; ++j) {
            const auto rowLines = QStringView{row[j].data}.split(QLatin1Char('\n')); // cache this would be a good idea
            for (const auto &str : rowLines)
                colWidths[j] = std::max(colWidths[j], int(str.size()));
            rowHeights[i] = std::max(rowHeights[i], int(row[j].data.count(QLatin1Char('\n')) + 1));
        }
    }

    if (!*std::max_element(colWidths.begin(), colWidths.end()))
        return; // empty table (table with empty cells)

    // create a horizontal line to be used later.
    QString horizontalLine = QLatin1String("+");
    for (int i = 0, max = colWidths.count(); i < max; ++i) {
        horizontalLine += QString(colWidths.at(i), QLatin1Char('-'));
        horizontalLine += QLatin1Char('+');
    }

    // write table rows
    for (int i = 0, maxI = m_rows.count(); i < maxI; ++i) { // for each row
        const QtXmlToSphinx::TableRow& row = m_rows.at(i);

        // print line
        s << '+';
        for (int col = 0; col < headerColumnCount; ++col) {
            char c;
            if (col >= row.length() || row[col].rowSpan == -1)
                c = ' ';
            else if (i == 1 && hasHeader())
                c = '=';
            else
                c = '-';
            s << Pad(c, colWidths.at(col)) << '+';
        }
        s << '\n';


        // Print the table cells
        for (int rowLine = 0; rowLine < rowHeights[i]; ++rowLine) { // for each line in a row
            int j = 0;
            for (int maxJ = std::min(int(row.count()), headerColumnCount); j < maxJ; ++j) { // for each column
                const QtXmlToSphinx::TableCell& cell = row[j];
                const auto rowLines = QStringView{cell.data}.split(QLatin1Char('\n')); // FIXME: Cache this!!!

                if (!j || !cell.colSpan)
                    s << '|';
                else
                    s << ' ';
                const int width = colWidths.at(j);
                if (rowLine < rowLines.count())
                    s << AlignedField(rowLines.at(rowLine), width);
                else
                    s << Pad(' ', width);
            }
            for ( ; j < headerColumnCount; ++j) // pad
                s << '|' << Pad(' ', colWidths.at(j));
            s << "|\n";
        }
    }
    s << horizontalLine << "\n\n";
}

void QtXmlToSphinx::stripPythonQualifiers(QString *s)
{
    const int lastSep = s->lastIndexOf(QLatin1Char('.'));
    if (lastSep != -1)
        s->remove(0, lastSep + 1);
}
