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

#include "typeparser.h"

#include <QtCore/QDebug>
#include <QtCore/QStack>
#include <QtCore/QTextStream>

class Scanner
{
public:
    enum Token {
        StarToken,
        AmpersandToken,
        LessThanToken,
        ColonToken,
        CommaToken,
        OpenParenToken,
        CloseParenToken,
        SquareBegin,
        SquareEnd,
        GreaterThanToken,

        ConstToken,
        Identifier,
        NoToken,
        InvalidToken
    };

    Scanner(const QString &s)
            : m_pos(0), m_length(s.length()), m_chars(s.constData())
    {
    }

    Token nextToken(QString *errorMessage = Q_NULLPTR);
    QString identifier() const;

    QString msgParseError(const QString &why) const;

private:
    int m_pos;
    int m_length;
    int m_tokenStart;
    const QChar *m_chars;
};

QString Scanner::identifier() const
{
    return QString(m_chars + m_tokenStart, m_pos - m_tokenStart);
}

Scanner::Token Scanner::nextToken(QString *errorMessage)
{
    Token tok = NoToken;

    // remove whitespace
    while (m_pos < m_length && m_chars[m_pos] == QLatin1Char(' '))
        ++m_pos;

    m_tokenStart = m_pos;

    while (m_pos < m_length) {

        const QChar &c = m_chars[m_pos];

        if (tok == NoToken) {
            switch (c.toLatin1()) {
            case '*': tok = StarToken; break;
            case '&': tok = AmpersandToken; break;
            case '<': tok = LessThanToken; break;
            case '>': tok = GreaterThanToken; break;
            case ',': tok = CommaToken; break;
            case '(': tok = OpenParenToken; break;
            case ')': tok = CloseParenToken; break;
            case '[': tok = SquareBegin; break;
            case ']' : tok = SquareEnd; break;
            case ':':
                tok = ColonToken;
                Q_ASSERT(m_pos + 1 < m_length);
                ++m_pos;
                break;
            default:
                if (c.isLetterOrNumber() || c == QLatin1Char('_')) {
                    tok = Identifier;
                } else {
                    QString message;
                    QTextStream (&message) << ": Unrecognized character in lexer at "
                        <<  m_pos << " : '" << c << '\'';
                    message = msgParseError(message);
                    if (errorMessage)
                        *errorMessage = message;
                    else
                        qWarning().noquote().nospace() << message;
                    return InvalidToken;
                }
                break;
            }
        }

        if (tok <= GreaterThanToken) {
            ++m_pos;
            break;
        }

        if (tok == Identifier) {
            if (c.isLetterOrNumber() || c == QLatin1Char('_'))
                ++m_pos;
            else
                break;
        }
    }

    if (tok == Identifier && m_pos - m_tokenStart == 5) {
        if (m_chars[m_tokenStart] == QLatin1Char('c')
            && m_chars[m_tokenStart + 1] == QLatin1Char('o')
            && m_chars[m_tokenStart + 2] == QLatin1Char('n')
            && m_chars[m_tokenStart + 3] == QLatin1Char('s')
            && m_chars[m_tokenStart + 4] == QLatin1Char('t'))
            tok = ConstToken;
    }

    return tok;

}

QString Scanner::msgParseError(const QString &why) const
{
    return QStringLiteral("TypeParser: Unable to parse \"")
        + QString(m_chars, m_length) + QStringLiteral("\": ") + why;
}

static TypeParser::Info invalidInfo()
{
    TypeParser::Info result;
    result.is_busted = true;
    return result;
}

TypeParser::Info TypeParser::parse(const QString &str, QString *errorMessage)
{
    Scanner scanner(str);

    Info info;
    QStack<Info *> stack;
    stack.push(&info);

    bool colon_prefix = false;
    bool in_array = false;
    QString array;

    Scanner::Token tok = scanner.nextToken(errorMessage);
    while (tok != Scanner::NoToken) {
        if (tok == Scanner::InvalidToken)
            return invalidInfo();

//         switch (tok) {
//         case Scanner::StarToken: printf(" - *\n"); break;
//         case Scanner::AmpersandToken: printf(" - &\n"); break;
//         case Scanner::LessThanToken: printf(" - <\n"); break;
//         case Scanner::GreaterThanToken: printf(" - >\n"); break;
//         case Scanner::ColonToken: printf(" - ::\n"); break;
//         case Scanner::CommaToken: printf(" - ,\n"); break;
//         case Scanner::ConstToken: printf(" - const\n"); break;
//         case Scanner::SquareBegin: printf(" - [\n"); break;
//         case Scanner::SquareEnd: printf(" - ]\n"); break;
//         case Scanner::Identifier: printf(" - '%s'\n", qPrintable(scanner.identifier())); break;
//         default:
//             break;
//         }

        switch (tok) {

        case Scanner::StarToken:
            ++stack.top()->indirections;
            break;

        case Scanner::AmpersandToken:
            switch (stack.top()->referenceType) {
            case NoReference:
                stack.top()->referenceType = LValueReference;
                break;
            case LValueReference:
                stack.top()->referenceType = RValueReference;
                break;
            case RValueReference:
                const QString message = scanner.msgParseError(QStringLiteral("Too many '&' qualifiers"));
                if (errorMessage)
                    *errorMessage = message;
                else
                    qWarning().noquote().nospace() << message;
                return invalidInfo();
            }
            break;
        case Scanner::LessThanToken:
            stack.top()->template_instantiations << Info();
            stack.push(&stack.top()->template_instantiations.last());
            break;

        case Scanner::CommaToken:
            stack.pop();
            stack.top()->template_instantiations << Info();
            stack.push(&stack.top()->template_instantiations.last());
            break;

        case Scanner::GreaterThanToken:
            stack.pop();
            break;

        case Scanner::ColonToken:
            colon_prefix = true;
            break;

        case Scanner::ConstToken:
            stack.top()->is_constant = true;
            break;

        case Scanner::OpenParenToken: // function pointers not supported
        case Scanner::CloseParenToken: {
            const QString message = scanner.msgParseError(QStringLiteral("Function pointers are not supported"));
            if (errorMessage)
                *errorMessage = message;
            else
                qWarning().noquote().nospace() << message;
            return invalidInfo();
        }

        case Scanner::Identifier:
            if (in_array) {
                array = scanner.identifier();
            } else if (colon_prefix || stack.top()->qualified_name.isEmpty()) {
                stack.top()->qualified_name << scanner.identifier();
                colon_prefix = false;
            } else {
                stack.top()->qualified_name.last().append(QLatin1Char(' ') + scanner.identifier());
            }
            break;

        case Scanner::SquareBegin:
            in_array = true;
            break;

        case Scanner::SquareEnd:
            in_array = false;
            stack.top()->arrays += array;
            break;


        default:
            break;
        }

        tok = scanner.nextToken();
    }

    return info;
}

QString TypeParser::Info::instantiationName() const
{
    QString s(qualified_name.join(QLatin1String("::")));
    if (!template_instantiations.isEmpty()) {
        QStringList insts;
        foreach (const Info &info, template_instantiations)
            insts << info.toString();
        s += QLatin1String("< ") + insts.join(QLatin1String(", ")) + QLatin1String(" >");
    }

    return s;
}

QString TypeParser::Info::toString() const
{
    QString s;

    if (is_constant)
        s += QLatin1String("const ");
    s += instantiationName();
    for (int i = 0; i < arrays.size(); ++i)
        s += QLatin1Char('[') + arrays.at(i) + QLatin1Char(']');
    s += QString(indirections, QLatin1Char('*'));
    switch (referenceType) {
    case NoReference:
        break;
    case LValueReference:
        s += QLatin1Char('&');
        break;
    case RValueReference:
        s += QLatin1String("&&");
        break;
    }
    return s;
}
