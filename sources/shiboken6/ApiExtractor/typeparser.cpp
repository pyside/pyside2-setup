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

#include "typeparser.h"
#include <typeinfo.h>

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
        VolatileToken,
        Identifier,
        NoToken,
        InvalidToken
    };

    Scanner(const QString &s)
            : m_pos(0), m_length(s.length()), m_tokenStart(-1), m_chars(s.constData())
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

    if (tok == Identifier) {
        switch (m_pos - m_tokenStart) {
        case 5:
            if (m_chars[m_tokenStart] == QLatin1Char('c')
                && m_chars[m_tokenStart + 1] == QLatin1Char('o')
                && m_chars[m_tokenStart + 2] == QLatin1Char('n')
                && m_chars[m_tokenStart + 3] == QLatin1Char('s')
                && m_chars[m_tokenStart + 4] == QLatin1Char('t')) {
                tok = ConstToken;
            }
                break;
        case 8:
            if (m_chars[m_tokenStart] == QLatin1Char('v')
                && m_chars[m_tokenStart + 1] == QLatin1Char('o')
                && m_chars[m_tokenStart + 2] == QLatin1Char('l')
                && m_chars[m_tokenStart + 3] == QLatin1Char('a')
                && m_chars[m_tokenStart + 4] == QLatin1Char('t')
                && m_chars[m_tokenStart + 5] == QLatin1Char('i')
                && m_chars[m_tokenStart + 6] == QLatin1Char('l')
                && m_chars[m_tokenStart + 7] == QLatin1Char('e')) {
                tok = VolatileToken;
            }
            break;
        }
    }

    return tok;

}

QString Scanner::msgParseError(const QString &why) const
{
    return QStringLiteral("TypeParser: Unable to parse \"")
        + QString(m_chars, m_length) + QStringLiteral("\": ") + why;
}

TypeInfo TypeParser::parse(const QString &str, QString *errorMessage)
{
    Scanner scanner(str);

    QStack<TypeInfo> stack;
    stack.push(TypeInfo());

    bool colon_prefix = false;
    bool in_array = false;
    QString array;
    bool seenStar = false;

    Scanner::Token tok = scanner.nextToken(errorMessage);
    while (tok != Scanner::NoToken) {
        if (tok == Scanner::InvalidToken)
            return TypeInfo();

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
            seenStar = true;
            stack.top().addIndirection(Indirection::Pointer);
            break;

        case Scanner::AmpersandToken:
            switch (stack.top().referenceType()) {
            case NoReference:
                stack.top().setReferenceType(LValueReference);
                break;
            case LValueReference:
                stack.top().setReferenceType(RValueReference);
                break;
            case RValueReference:
                const QString message = scanner.msgParseError(QStringLiteral("Too many '&' qualifiers"));
                if (errorMessage)
                    *errorMessage = message;
                else
                    qWarning().noquote().nospace() << message;
                return TypeInfo();
            }
            break;
        case Scanner::LessThanToken:
            stack.push(TypeInfo());
            break;

        case Scanner::CommaToken:
        {
            auto i = stack.pop();
            stack.top().addInstantiation(i); // Add after populating to prevent detach
            stack.push(TypeInfo());
        }
            break;

        case Scanner::GreaterThanToken: {
            auto i = stack.pop();
            stack.top().addInstantiation(i); // Add after populating to prevent detach
        }
            break;

        case Scanner::ColonToken:
            colon_prefix = true;
            break;

        case Scanner::ConstToken:
            if (seenStar) { // "int *const": Last indirection is const.
                auto indirections = stack.top().indirectionsV();
                Q_ASSERT(!indirections.isEmpty());
                indirections[0] = Indirection::ConstPointer;
                stack.top().setIndirectionsV(indirections);
            } else {
                stack.top().setConstant(true);
            }
            break;

        case Scanner::VolatileToken:
            stack.top().setVolatile(true);
            break;

        case Scanner::OpenParenToken: // function pointers not supported
        case Scanner::CloseParenToken: {
            const QString message = scanner.msgParseError(QStringLiteral("Function pointers are not supported"));
            if (errorMessage)
                *errorMessage = message;
            else
                qWarning().noquote().nospace() << message;
            return TypeInfo();
        }

        case Scanner::Identifier:
            if (in_array) {
                array = scanner.identifier();
            } else if (colon_prefix || stack.top().qualifiedName().isEmpty()) {
                stack.top().addName(scanner.identifier());
                colon_prefix = false;
            } else {
                QStringList qualifiedName = stack.top().qualifiedName();
                qualifiedName.last().append(QLatin1Char(' ') + scanner.identifier());
                stack.top().setQualifiedName(qualifiedName);
            }
            break;

        case Scanner::SquareBegin:
            in_array = true;
            break;

        case Scanner::SquareEnd:
            in_array = false;
            stack.top().addArrayElement(array);
            break;


        default:
            break;
        }

        tok = scanner.nextToken();
    }

    Q_ASSERT(!stack.isEmpty());
    return stack.constFirst();
}
