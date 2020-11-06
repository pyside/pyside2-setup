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

#include "modifications.h"
#include "modifications_p.h"
#include "typedatabase.h"
#include "typeparser.h"
#include "typesystem.h"

#include <QtCore/QDebug>

#include <algorithm>
#include <limits>

static inline QString callOperator() { return QStringLiteral("operator()"); }

QString TemplateInstance::expandCode() const
{
    TemplateEntry *templateEntry = TypeDatabase::instance()->findTemplate(m_name);
    if (!templateEntry)
        qFatal("<insert-template> referring to non-existing template '%s'.", qPrintable(m_name));

    QString code = templateEntry->code();
    for (auto it = replaceRules.cbegin(), end = replaceRules.cend(); it != end; ++it)
        code.replace(it.key(), it.value());
    while (!code.isEmpty() && code.at(code.size() - 1).isSpace())
        code.chop(1);
    QString result = QLatin1String("// TEMPLATE - ") + m_name + QLatin1String(" - START");
    if (!code.startsWith(QLatin1Char('\n')))
        result += QLatin1Char('\n');
    result += code;
    result += QLatin1String("\n// TEMPLATE - ") + m_name + QLatin1String(" - END\n");
    return result;
}

// ---------------------- CodeSnipFragment
QString CodeSnipFragment::code() const
{
    return m_instance ? m_instance->expandCode() : m_code;
}

// ---------------------- CodeSnipAbstract
QString CodeSnipAbstract::code() const
{
    QString res;
    for (const CodeSnipFragment &codeFrag : codeList)
        res.append(codeFrag.code());

    return res;
}

void CodeSnipAbstract::addCode(const QString &code)
{
    codeList.append(CodeSnipFragment(fixSpaces(code)));
}

static inline int firstNonBlank(QStringView s)
{
    const auto it = std::find_if(s.cbegin(), s.cend(),
                                 [] (QChar c) { return !c.isSpace(); });
    return int(it - s.cbegin());
}

static inline bool isEmpty(QStringView s)
{
    return s.isEmpty()
        || std::all_of(s.cbegin(), s.cend(),
                       [] (QChar c) { return c.isSpace(); });
}

QString CodeSnipAbstract::dedent(const QString &code)
{
    if (code.isEmpty())
        return code;
    // Right trim if indent=0, or trim if single line
    if (!code.at(0).isSpace() || !code.contains(QLatin1Char('\n')))
        return code.trimmed();
    const auto lines = QStringView{code}.split(QLatin1Char('\n'));
    int spacesToRemove = std::numeric_limits<int>::max();
    for (const auto &line : lines) {
        if (!isEmpty(line)) {
            const int nonSpacePos = firstNonBlank(line);
            if (nonSpacePos < spacesToRemove)
                spacesToRemove = nonSpacePos;
            if (spacesToRemove == 0)
                return code;
        }
    }
    QString result;
    for (const auto &line : lines) {
        if (!isEmpty(line) && spacesToRemove < line.size())
            result += line.mid(spacesToRemove).toString();
        result += QLatin1Char('\n');
    }
    return result;
}

QString CodeSnipAbstract::fixSpaces(QString code)
{
    code.remove(QLatin1Char('\r'));
    // Check for XML <tag>\n<space>bla...
    if (code.startsWith(QLatin1String("\n ")))
        code.remove(0, 1);
    while (!code.isEmpty() && code.back().isSpace())
        code.chop(1);
    code = dedent(code);
    if (!code.isEmpty() && !code.endsWith(QLatin1Char('\n')))
        code.append(QLatin1Char('\n'));
    return code;
}

// Prepend a line to the code, observing indentation
void CodeSnipAbstract::prependCode(QString *code, QString firstLine)
{
    while (!code->isEmpty() && code->front() == QLatin1Char('\n'))
        code->remove(0, 1);
    if (!code->isEmpty() && code->front().isSpace()) {
        const int indent = firstNonBlank(*code);
        firstLine.prepend(QString(indent, QLatin1Char(' ')));
    }
    if (!firstLine.endsWith(QLatin1Char('\n')))
        firstLine += QLatin1Char('\n');
    code->prepend(firstLine);
}

// ---------------------- Modification
QString Modification::accessModifierString() const
{
    if (isPrivate()) return QLatin1String("private");
    if (isProtected()) return QLatin1String("protected");
    if (isPublic()) return QLatin1String("public");
    if (isFriendly()) return QLatin1String("friendly");
    return QString();
}

// ---------------------- FunctionModification
bool FunctionModification::setSignature(const QString &s, QString *errorMessage)
{
    if (s.startsWith(QLatin1Char('^'))) {
        m_signaturePattern.setPattern(s);
        if (!m_signaturePattern.isValid()) {
            if (errorMessage) {
                *errorMessage = QLatin1String("Invalid signature pattern: \"")
                    + s + QLatin1String("\": ") + m_signaturePattern.errorString();
            }
            return false;
        }
    } else {
        m_signature = s;
    }
    return true;
}

// Helpers to split a parameter list of <add-function>, <declare-function>
// (@ denoting names), like
// "void foo(QList<X,Y> &@list@ = QList<X,Y>{1,2}, int @b@=5, ...)"
namespace AddedFunctionParser {

bool Argument::equals(const Argument &rhs) const
{
    return type == rhs.type && name == rhs.name && defaultValue == rhs.defaultValue;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const Argument &a)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "Argument(type=\"" << a.type << '"';
    if (!a.name.isEmpty())
        d << ", name=\"" << a.name << '"';
    if (!a.defaultValue.isEmpty())
        d << ", defaultValue=\"" << a.defaultValue << '"';
    d << ')';
    return d;
}
#endif // QT_NO_DEBUG_STREAM

// Helper for finding the end of a function parameter, observing
// nested template parameters or lists.
static int parameterTokenEnd(int startPos, QStringView paramString)
{
    const int end = paramString.size();
    int nestingLevel = 0;
    for (int p = startPos; p < end; ++p) {
        switch (paramString.at(p).toLatin1()) {
        case ',':
            if (nestingLevel == 0)
                return p;
            break;
        case '<': // templates
        case '{': // initializer lists of default values
        case '(': // initialization, function pointers
        case '[': // array dimensions
            ++nestingLevel;
            break;
        case '>':
        case '}':
        case ')':
        case ']':
            --nestingLevel;
            break;
        }
    }
    return end;
}

// Split a function parameter list into string tokens containing one
// parameters (including default value, etc).
static QList<QStringView> splitParameterTokens(QStringView paramString)
{
    QList<QStringView> result;
    int startPos = 0;
    for ( ; startPos < paramString.size(); ) {
        int end = parameterTokenEnd(startPos, paramString);
        result.append(paramString.mid(startPos, end - startPos).trimmed());
        startPos = end + 1;
    }
    return result;
}

// Split a function parameter list
Arguments splitParameters(QStringView paramString, QString *errorMessage)
{
    Arguments result;
    const QList<QStringView> tokens = splitParameterTokens(paramString);

    for (const auto &t : tokens) {
        Argument argument;
        // Check defaultValue, "int @b@=5"
        const int equalPos = t.lastIndexOf(QLatin1Char('='));
        if (equalPos != -1) {
            const int defaultValuePos = equalPos + 1;
            argument.defaultValue =
                t.mid(defaultValuePos, t.size() - defaultValuePos).trimmed().toString();
        }
        QString typeString = (equalPos != -1 ? t.left(equalPos) : t).trimmed().toString();
        // Check @name@
        const int atPos = typeString.indexOf(QLatin1Char('@'));
        if (atPos != -1) {
            const int namePos = atPos + 1;
            const int nameEndPos = typeString.indexOf(QLatin1Char('@'), namePos);
            if (nameEndPos == -1) {
                if (errorMessage != nullptr) {
                    *errorMessage = QLatin1String("Mismatched @ in \"")
                                    + paramString.toString() + QLatin1Char('"');
                }
                return {};
            }
            argument.name = typeString.mid(namePos, nameEndPos - namePos).trimmed();
            typeString.remove(atPos, nameEndPos - atPos + 1);
        }
        argument.type = typeString.trimmed();
        result.append(argument);
    }

    return result;
}

} // namespace AddedFunctionParser

AddedFunction::AddedFunction(const QString &name, const QList<Argument> &arguments,
                             const TypeInfo &returnType) :
    m_name(name),
    m_arguments(arguments),
    m_returnType(returnType)
{
}

AddedFunction::AddedFunctionPtr
    AddedFunction::createAddedFunction(const QString &signatureIn, const QString &returnTypeIn,
                                       QString *errorMessage)

{
    errorMessage->clear();

    QList<Argument> arguments;
    const TypeInfo returnType = returnTypeIn.isEmpty()
                                ? TypeInfo::voidType()
                                : TypeParser::parse(returnTypeIn, errorMessage);
    if (!errorMessage->isEmpty())
        return {};

    QStringView signature = QStringView{signatureIn}.trimmed();

    // Skip past "operator()(...)"
    const int parenSearchStartPos = signature.startsWith(callOperator())
        ? callOperator().size() : 0;
    const int openParenPos = signature.indexOf(QLatin1Char('('), parenSearchStartPos);
    if (openParenPos < 0) {
        return AddedFunctionPtr(new AddedFunction(signature.toString(),
                                                  arguments, returnType));
    }

    const QString name = signature.left(openParenPos).trimmed().toString();
    const int closingParenPos = signature.lastIndexOf(QLatin1Char(')'));
    if (closingParenPos < 0) {
        *errorMessage = QLatin1String("Missing closing parenthesis");
        return {};
    }

    // Check for "foo() const"
    bool isConst = false;
    const int signatureLength = signature.length();
    const int qualifierLength = signatureLength - closingParenPos - 1;
    if (qualifierLength >= 5
        && signature.right(qualifierLength).contains(QLatin1String("const"))) {
        isConst = true;
    }

    const auto paramString = signature.mid(openParenPos + 1, closingParenPos - openParenPos - 1);
    const auto params = AddedFunctionParser::splitParameters(paramString, errorMessage);
    if (params.isEmpty() && !errorMessage->isEmpty())
        return {};
    for (const auto &p : params) {
        TypeInfo type = p.type == QLatin1String("...")
            ? TypeInfo::varArgsType() : TypeParser::parse(p.type, errorMessage);
        if (!errorMessage->isEmpty())
            return {};
        arguments.append({type, p.name, p.defaultValue});
    }

    AddedFunctionPtr result(new AddedFunction(name, arguments, returnType));
    result->setConstant(isConst);
    return result;
}

void DocModification::setCode(const QString &code)
{
    m_code = CodeSnipAbstract::fixSpaces(code);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const ReferenceCount &r)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "ReferenceCount(" << r.varName << ", action=" << r.action << ')';
    return d;
}

QDebug operator<<(QDebug d, const CodeSnip &s)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "CodeSnip(language=" << s.language << ", position=" << s.position << ", \"";
    for (const auto &f : s.codeList) {
        const QString &code = f.code();
        const auto lines = QStringView{code}.split(QLatin1Char('\n'));
        for (int i = 0, size = lines.size(); i < size; ++i) {
            if (i)
                d << "\\n";
            d << lines.at(i).trimmed();
        }
    }
    d << '"';
    if (!s.argumentMap.isEmpty()) {
        d << ", argumentMap{";
        for (auto it = s.argumentMap.cbegin(), end = s.argumentMap.cend(); it != end; ++it)
            d << it.key() << "->\"" << it.value() << '"';
        d << '}';
    }
    d << ')';
    return d;
}

void Modification::formatDebug(QDebug &d) const
{
    d << "modifiers=" << modifiers;
    if (removal)
      d << ", removal";
    if (!renamedToName.isEmpty())
        d << ", renamedToName=\"" << renamedToName << '"';
}

void FunctionModification::formatDebug(QDebug &d) const
{
    if (m_signature.isEmpty())
        d << "pattern=\"" << m_signaturePattern.pattern();
    else
        d << "signature=\"" << m_signature;
    d << "\", ";
    Modification::formatDebug(d);
    if (!association.isEmpty())
        d << ", association=\"" << association << '"';
    if (m_allowThread != TypeSystem::AllowThread::Unspecified)
        d << ", allowThread=" << int(m_allowThread);
    if (m_thread)
        d << ", thread";
    if (m_exceptionHandling != TypeSystem::ExceptionHandling::Unspecified)
        d << ", exceptionHandling=" << int(m_exceptionHandling);
    if (!snips.isEmpty())
        d << ", snips=(" << snips << ')';
    if (!argument_mods.isEmpty())
        d << ", argument_mods=(" << argument_mods << ')';
}

QDebug operator<<(QDebug d, const ArgumentOwner &a)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "ArgumentOwner(index=" << a.index << ", action=" << a.action << ')';
    return d;
}

QDebug operator<<(QDebug d, const ArgumentModification &a)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "ArgumentModification(index=" << a.index;
    if (a.removedDefaultExpression)
        d << ", removedDefaultExpression";
    if (a.removed)
        d << ", removed";
    if (a.noNullPointers)
        d << ", noNullPointers";
    if (a.array)
        d << ", array";
    if (!a.referenceCounts.isEmpty())
        d << ", referenceCounts=" << a.referenceCounts;
    if (!a.modified_type.isEmpty())
        d << ", modified_type=\"" << a.modified_type << '"';
    if (!a.replace_value.isEmpty())
        d << ", replace_value=\"" << a.replace_value << '"';
    if (!a.replacedDefaultExpression.isEmpty())
        d << ", replacedDefaultExpression=\"" << a.replacedDefaultExpression << '"';
    if (!a.ownerships.isEmpty())
        d << ", ownerships=" << a.ownerships;
    if (!a.renamed_to.isEmpty())
        d << ", renamed_to=\"" << a.renamed_to << '"';
     d << ", owner=" << a.owner << ')';
    return  d;
}

QDebug operator<<(QDebug d, const FunctionModification &fm)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "FunctionModification(";
    fm.formatDebug(d);
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AddedFunction::Argument &a)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "Argument(";
    d << a.typeInfo;
    if (!a.name.isEmpty())
        d << ' ' << a.name;
    if (!a.defaultValue.isEmpty())
        d << " = " << a.defaultValue;
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AddedFunction &af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AddedFunction(";
    if (af.access() == AddedFunction::Protected)
        d << "protected";
    if (af.isStatic())
        d << " static";
    d << af.returnType() << ' ' << af.name() << '(' << af.arguments() << ')';
    if (af.isConstant())
        d << " const";
    if (af.isDeclaration())
        d << " [declaration]";
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
