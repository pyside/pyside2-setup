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
#include "typedatabase.h"
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

// ---------------------- AddedFunction

static AddedFunction::TypeInfo parseType(const QString& signature,
                                         int startPos = 0, int *endPos = nullptr,
                                         QString *argumentName = nullptr,
                                         QString *defaultValue = nullptr)
{
    AddedFunction::TypeInfo result;
    static const QRegularExpression regex(QLatin1String("\\w"));
    Q_ASSERT(regex.isValid());
    int length = signature.length();
    int start = signature.indexOf(regex, startPos);
    if (start == -1) {
        if (QStringView{signature}.mid(startPos + 1, 3) == QLatin1String("...")) { // varargs
            if (endPos)
                *endPos = startPos + 4;
            result.name = QLatin1String("...");
        } else { // error
            if (endPos)
                *endPos = length;
        }
        return result;
    }

    int cantStop = 0;
    QString paramString;
    QChar c;
    int i = start;
    for (; i < length; ++i) {
        c = signature[i];
        if (c == QLatin1Char('<'))
            cantStop++;
        if (c == QLatin1Char('>'))
            cantStop--;
        if (cantStop < 0)
            break; // FIXME: report error?
        if ((c == QLatin1Char(')') || c == QLatin1Char(',')) && !cantStop)
            break;
        paramString += signature[i];
    }
    if (endPos)
        *endPos = i;

    // Check default value
    if (paramString.contains(QLatin1Char('='))) {
        QStringList lst = paramString.split(QLatin1Char('='));
        paramString = lst[0].trimmed();
        if (defaultValue != nullptr)
            *defaultValue = lst[1].trimmed();
    }

    // check constness
    if (paramString.startsWith(QLatin1String("const "))) {
        result.isConstant = true;
        paramString.remove(0, sizeof("const")/sizeof(char));
        paramString = paramString.trimmed();
    }

    // Extract argument name from "T<bla,blub>* @foo@"
    const int nameStartPos = paramString.indexOf(QLatin1Char('@'));
    if (nameStartPos != -1) {
        const int nameEndPos = paramString.indexOf(QLatin1Char('@'), nameStartPos + 1);
        if (nameEndPos > nameStartPos) {
            if (argumentName)
                *argumentName = paramString.mid(nameStartPos + 1, nameEndPos - nameStartPos - 1);
            paramString.remove(nameStartPos, nameEndPos - nameStartPos + 1);
            paramString = paramString.trimmed();
        }
    }

    // check reference
    if (paramString.endsWith(QLatin1Char('&'))) {
        result.isReference = true;
        paramString.chop(1);
        paramString = paramString.trimmed();
    }
    // check Indirections
    while (paramString.endsWith(QLatin1Char('*'))) {
        result.indirections++;
        paramString.chop(1);
        paramString = paramString.trimmed();
    }
    result.name = paramString;

    return result;
}

AddedFunction::AddedFunction(QString signature, const QString &returnType) :
    m_access(Public)
{
    Q_ASSERT(!returnType.isEmpty());
    m_returnType = parseType(returnType);
    signature = signature.trimmed();
    // Skip past "operator()(...)"
    const int parenStartPos = signature.startsWith(callOperator())
        ? callOperator().size() : 0;
    int endPos = signature.indexOf(QLatin1Char('('), parenStartPos);
    if (endPos < 0) {
        m_isConst = false;
        m_name = signature;
    } else {
        m_name = signature.left(endPos).trimmed();
        int signatureLength = signature.length();
        while (endPos < signatureLength) {
            QString argumentName;
            QString defaultValue;
            TypeInfo arg = parseType(signature, endPos, &endPos, &argumentName, &defaultValue);
            if (!arg.name.isEmpty())
                m_arguments.append({arg, argumentName, defaultValue});
            // end of parameters...
            if (endPos >= signatureLength || signature[endPos] == QLatin1Char(')'))
                break;
        }
        // is const?
        m_isConst = QStringView{signature}.right(signatureLength - endPos).contains(QLatin1String("const"));
    }
}

AddedFunction::TypeInfo AddedFunction::TypeInfo::fromSignature(const QString& signature)
{
    return parseType(signature);
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

QDebug operator<<(QDebug d, const AddedFunction::TypeInfo &ti)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TypeInfo(";
    if (ti.isConstant)
        d << "const";
    if (ti.indirections)
        d << QByteArray(ti.indirections, '*');
    if (ti.isReference)
        d << " &";
    d << ti.name;
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
