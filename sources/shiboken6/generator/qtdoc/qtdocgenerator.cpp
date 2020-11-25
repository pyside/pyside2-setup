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

#include "qtdocgenerator.h"
#include "qtxmltosphinx.h"
#include "rstformat.h"
#include "ctypenames.h"
#include <abstractmetaenum.h>
#include <abstractmetafield.h>
#include <abstractmetafunction.h>
#include <abstractmetalang.h>
#include <fileout.h>
#include <messages.h>
#include <modifications.h>
#include <propertyspec.h>
#include <reporthandler.h>
#include <textstream.h>
#include <typesystem.h>
#include <qtdocparser.h>
#include <doxygenparser.h>

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <algorithm>
#include <limits>

static inline QString additionalDocumentationOption() { return QStringLiteral("additional-documentation"); }

static inline QString briefStartElement() { return QStringLiteral("<brief>"); }
static inline QString briefEndElement() { return QStringLiteral("</brief>"); }

static inline QString none() { return QStringLiteral("None"); }

static bool shouldSkip(const AbstractMetaFunction* func)
{
    // Constructors go to separate section
    if (DocParser::skipForQuery(func) || func->isConstructor())
        return true;

    // Search a const clone (QImage::bits() vs QImage::bits() const)
    if (func->isConstant())
        return false;

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
                if (funcArgs.at(i).type().typeEntry() != fargs.at(i).type().typeEntry()) {
                    cloneFound = false;
                    break;
                }
            }
            if (cloneFound)
                return true;
        }
    }
    return false;
}

static bool functionSort(const AbstractMetaFunction* func1, const AbstractMetaFunction* func2)
{
    return func1->name() < func2->name();
}

static inline QVersionNumber versionOf(const TypeEntry *te)
{
    if (te) {
        const auto version = te->version();
        if (!version.isNull() && version > QVersionNumber(0, 0))
            return version;
    }
    return QVersionNumber();
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

QtDocGenerator::QtDocGenerator() : m_docParser(nullptr)
{
}

QtDocGenerator::~QtDocGenerator()
{
    delete m_docParser;
}

QString QtDocGenerator::fileNameSuffix() const
{
    return QLatin1String(".rst");
}

bool QtDocGenerator::shouldGenerate(const AbstractMetaClass *cls) const
{
    return Generator::shouldGenerate(cls)
        && cls->typeEntry()->type() != TypeEntry::SmartPointerType;
}

QString QtDocGenerator::fileNameForContext(const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    if (!context.forSmartPointer()) {
        return metaClass->name() + fileNameSuffix();
    }
    const AbstractMetaType &smartPointerType = context.preciseType();
    QString fileNameBase = getFileNameBaseForSmartPointer(smartPointerType, metaClass);
    return fileNameBase + fileNameSuffix();
}

void QtDocGenerator::writeFormattedText(TextStream &s, const Documentation &doc,
                                        const AbstractMetaClass *metaClass,
                                        Documentation::Type docType) const
{
    QString metaClassName;

    if (metaClass)
        metaClassName = metaClass->fullName();

    if (doc.format() == Documentation::Native) {
        QtXmlToSphinx x(this, doc.value(docType), metaClassName);
        s << x;
    } else {
        const QString &value = doc.value(docType);
        const auto lines = QStringView{value}.split(QLatin1Char('\n'));
        int typesystemIndentation = std::numeric_limits<int>::max();
        // check how many spaces must be removed from the beginning of each line
        for (const auto &line : lines) {
            const auto it = std::find_if(line.cbegin(), line.cend(),
                                         [] (QChar c) { return !c.isSpace(); });
            if (it != line.cend())
                typesystemIndentation = qMin(typesystemIndentation, int(it - line.cbegin()));
        }
        if (typesystemIndentation == std::numeric_limits<int>::max())
            typesystemIndentation = 0;
        for (const auto &line : lines) {
            s << (typesystemIndentation > 0 && typesystemIndentation < line.size()
                    ? line.right(line.size() - typesystemIndentation) : line)
                << '\n';
        }
    }

    s << '\n';
}

static void writeInheritedByList(TextStream& s, const AbstractMetaClass* metaClass, const AbstractMetaClassList& allClasses)
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
        classes << QLatin1String(":ref:`") + c->name() + QLatin1Char('`');
    s << classes.join(QLatin1String(", ")) << "\n\n";
}

// Extract the <brief> section from a WebXML (class) documentation and remove it
// from the source.
static bool extractBrief(Documentation *sourceDoc, Documentation *brief)
{
    if (sourceDoc->format() != Documentation::Native)
        return false;
    QString value = sourceDoc->value();
    const int briefStart = value.indexOf(briefStartElement());
    if (briefStart < 0)
        return false;
    const int briefEnd = value.indexOf(briefEndElement(), briefStart + briefStartElement().size());
    if (briefEnd < briefStart)
        return false;
    const int briefLength = briefEnd + briefEndElement().size() - briefStart;
    brief->setFormat(Documentation::Native);
    QString briefValue = value.mid(briefStart, briefLength);
    briefValue.insert(briefValue.size() - briefEndElement().size(),
                      QLatin1String("<rst> More_...</rst>"));
    brief->setValue(briefValue);
    value.remove(briefStart, briefLength);
    sourceDoc->setValue(value);
    return true;
}

void QtDocGenerator::generateClass(TextStream &s, const GeneratorContext &classContext)
{
    const AbstractMetaClass *metaClass = classContext.metaClass();
    qCDebug(lcShibokenDoc).noquote().nospace() << "Generating Documentation for " << metaClass->fullName();

    m_packages[metaClass->package()] << fileNameForContext(classContext);

    m_docParser->setPackageName(metaClass->package());
    m_docParser->fillDocumentation(const_cast<AbstractMetaClass*>(metaClass));

    QString className = metaClass->name();
    s << ".. _" << className << ":" << "\n\n";
    s << ".. currentmodule:: " << metaClass->package() << "\n\n\n";

    s << className << '\n';
    s << Pad('*', className.count()) << "\n\n";

    auto documentation = metaClass->documentation();
    Documentation brief;
    if (extractBrief(&documentation, &brief))
        writeFormattedText(s, brief.value(), metaClass);

    s << ".. inheritance-diagram:: " << metaClass->fullName()<< '\n'
      << "    :parts: 2\n\n";
    // TODO: This would be a parameter in the future...


    writeInheritedByList(s, metaClass, classes());

    const auto version = versionOf(metaClass->typeEntry());
    if (!version.isNull())
        s << rstVersionAdded(version);
    if (metaClass->attributes().testFlag(AbstractMetaAttributes::Deprecated))
        s << rstDeprecationNote("class");

    writeFunctionList(s, metaClass);

    //Function list
    AbstractMetaFunctionList functionList = metaClass->functions();
    std::sort(functionList.begin(), functionList.end(), functionSort);

    s << "\nDetailed Description\n"
           "--------------------\n\n"
        << ".. _More:\n";

    writeInjectDocumentation(s, TypeSystem::DocModificationPrepend, metaClass, nullptr);
    if (!writeInjectDocumentation(s, TypeSystem::DocModificationReplace, metaClass, nullptr))
        writeFormattedText(s, documentation.value(), metaClass);

    if (!metaClass->isNamespace())
        writeConstructors(s, metaClass);
    writeEnums(s, metaClass);
    if (!metaClass->isNamespace())
        writeFields(s, metaClass);


    QStringList uniqueFunctions;
    for (AbstractMetaFunction *func : qAsConst(functionList)) {
        if (shouldSkip(func))
            continue;

        if (func->isStatic())
            s <<  ".. staticmethod:: ";
        else
            s <<  ".. method:: ";

        writeFunction(s, metaClass, func, !uniqueFunctions.contains(func->name()));
        uniqueFunctions.append(func->name());
    }

    writeInjectDocumentation(s, TypeSystem::DocModificationAppend, metaClass, nullptr);
}

void QtDocGenerator::writeFunctionList(TextStream& s, const AbstractMetaClass* cppClass)
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
            className = cppClass->fullName() + QLatin1Char('.');
        else if (func->implementingClass() && func->implementingClass()->enclosingClass())
            className = func->implementingClass()->enclosingClass()->fullName() + QLatin1Char('.');
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

    if (!functionList.isEmpty() || !staticFunctionList.isEmpty()) {
        QtXmlToSphinx::Table functionTable;

        s << "\nSynopsis\n--------\n\n";

        writeFunctionBlock(s, QLatin1String("Functions"), functionList);
        writeFunctionBlock(s, QLatin1String("Virtual functions"), virtualList);
        writeFunctionBlock(s, QLatin1String("Slots"), slotList);
        writeFunctionBlock(s, QLatin1String("Signals"), signalList);
        writeFunctionBlock(s, QLatin1String("Static functions"), staticFunctionList);
    }
}

void QtDocGenerator::writeFunctionBlock(TextStream& s, const QString& title, QStringList& functions)
{
    if (!functions.isEmpty()) {
        s << title << '\n'
          << Pad('^', title.size()) << '\n';

        std::sort(functions.begin(), functions.end());

        s << ".. container:: function_list\n\n";
        Indentation indentation(s);
        for (const QString &func : qAsConst(functions))
            s << "* " << func << '\n';
        s << "\n\n";
    }
}

void QtDocGenerator::writeEnums(TextStream& s, const AbstractMetaClass* cppClass) const
{
    static const QString section_title = QLatin1String(".. attribute:: ");

    for (const AbstractMetaEnum &en : cppClass->enums()) {
        s << section_title << cppClass->fullName() << '.' << en.name() << "\n\n";
        writeFormattedText(s, en.documentation().value(), cppClass);
        const auto version = versionOf(en.typeEntry());
        if (!version.isNull())
            s << rstVersionAdded(version);
    }

}

void QtDocGenerator::writeFields(TextStream& s, const AbstractMetaClass* cppClass) const
{
    static const QString section_title = QLatin1String(".. attribute:: ");

    for (const AbstractMetaField &field : cppClass->fields()) {
        s << section_title << cppClass->fullName() << "." << field.name() << "\n\n";
        writeFormattedText(s, field.documentation().value(), cppClass);
    }
}

void QtDocGenerator::writeConstructors(TextStream& s, const AbstractMetaClass* cppClass) const
{
    static const QString sectionTitle = QLatin1String(".. class:: ");

    AbstractMetaFunctionList lst = cppClass->queryFunctions(AbstractMetaClass::Constructors | AbstractMetaClass::Visible);
    for (int i = lst.size() - 1; i >= 0; --i) {
        if (lst.at(i)->isModifiedRemoved() || lst.at(i)->functionType() == AbstractMetaFunction::MoveConstructorFunction)
            lst.removeAt(i);
    }

    bool first = true;
    QHash<QString, AbstractMetaArgument> arg_map;

    if (lst.isEmpty()) {
        s << sectionTitle << cppClass->fullName();
    } else {
        QByteArray pad;
        for (AbstractMetaFunction *func : qAsConst(lst)) {
            s << pad;
            if (first) {
                first = false;
                s << sectionTitle;
                pad = QByteArray(sectionTitle.size(), ' ');
            }
            s << functionSignature(cppClass, func) << "\n\n";

            const auto version = versionOf(func->typeEntry());
            if (!version.isNull())
                s << pad << rstVersionAdded(version);
            if (func->attributes().testFlag(AbstractMetaAttributes::Deprecated))
                s << pad << rstDeprecationNote("constructor");

            const AbstractMetaArgumentList &arguments = func->arguments();
            for (const AbstractMetaArgument &arg : arguments) {
                if (!arg_map.contains(arg.name())) {
                    arg_map.insert(arg.name(), arg);
                }
            }
        }
    }

    s << '\n';

    for (auto it = arg_map.cbegin(), end = arg_map.cend(); it != end; ++it) {
        s.indent(2);
        writeParameterType(s, cppClass, it.value());
        s.outdent(2);
    }

    s << '\n';

    for (AbstractMetaFunction *func : qAsConst(lst))
        writeFormattedText(s, func->documentation().value(), cppClass);
}

QString QtDocGenerator::parseArgDocStyle(const AbstractMetaClass* /* cppClass */,
                                         const AbstractMetaFunction* func)
{
    QString ret;
    int optArgs = 0;

    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument &arg : arguments) {

        if (func->argumentRemoved(arg.argumentIndex() + 1))
            continue;

        bool thisIsoptional = !arg.defaultValueExpression().isEmpty();
        if (optArgs || thisIsoptional) {
            ret += QLatin1Char('[');
            optArgs++;
        }

        if (arg.argumentIndex() > 0)
            ret += QLatin1String(", ");

        ret += arg.name();

        if (thisIsoptional) {
            QString defValue = arg.defaultValueExpression();
            if (defValue == QLatin1String("QString()")) {
                defValue = QLatin1String("\"\"");
            } else if (defValue == QLatin1String("QStringList()")
                       || defValue.startsWith(QLatin1String("QVector"))
                       || defValue.startsWith(QLatin1String("QList"))) {
                defValue = QLatin1String("list()");
            } else if (defValue == QLatin1String("QVariant()")) {
                defValue = none();
            } else {
                defValue.replace(QLatin1String("::"), QLatin1String("."));
                if (defValue == QLatin1String("nullptr"))
                    defValue = none();
                else if (defValue == QLatin1String("0") && arg.type().isObject())
                    defValue = none();
            }
            ret += QLatin1Char('=') + defValue;
        }
    }

    ret += QString(optArgs, QLatin1Char(']'));
    return ret;
}

void QtDocGenerator::writeDocSnips(TextStream &s,
                                 const CodeSnipList &codeSnips,
                                 TypeSystem::CodeSnipPosition position,
                                 TypeSystem::Language language)
{
    Indentation indentation(s);
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
            int currentRow = 0;
            int offset = 0;

            for (QString row : rows) {
                for (const QString &invalidString : qAsConst(invalidStrings))
                    row.remove(invalidString);

                if (row.trimmed().size() == 0) {
                    if (currentRow == 0)
                        continue;
                    s << '\n';
                }

                if (currentRow == 0) {
                    //find offset
                    for (auto c : row) {
                        if (c == QLatin1Char(' '))
                            offset++;
                        else if (c == QLatin1Char('\n'))
                            offset = 0;
                        else
                            break;
                    }
                }
                s << QStringView{row}.mid(offset) << '\n';
                currentRow++;
            }

            code = code.mid(endBlock+endMarkup.size());
        }
    }
}

bool QtDocGenerator::writeInjectDocumentation(TextStream& s,
                                            TypeSystem::DocModificationMode mode,
                                            const AbstractMetaClass* cppClass,
                                            const AbstractMetaFunction* func)
{
    Indentation indentation(s);
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

                doc.setValue(mod.code(), Documentation::Detailed, fmt);
                writeFormattedText(s, doc.value(), cppClass);
                didSomething = true;
            }
        }
    }

    s << '\n';

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

QString QtDocGenerator::functionSignature(const AbstractMetaClass* cppClass, const AbstractMetaFunction* func)
{
    QString funcName;

    funcName = cppClass->fullName();
    if (!func->isConstructor())
        funcName += QLatin1Char('.') + getFuncName(func);

    return funcName + QLatin1Char('(') + parseArgDocStyle(cppClass, func)
        + QLatin1Char(')');
}

QString QtDocGenerator::translateToPythonType(const AbstractMetaType &type,
                                              const AbstractMetaClass* cppClass) const
{
    static const QStringList nativeTypes = {boolT(), floatT(), intT(),
        QLatin1String("object"),
        QLatin1String("str")
    };
    const QString name = type.name();
    if (nativeTypes.contains(name))
        return name;

    static const QMap<QString, QString> typeMap = {
        { QLatin1String("PyObject"), QLatin1String("object") },
        { QLatin1String("QString"), QLatin1String("str") },
        { QLatin1String("uchar"), QLatin1String("str") },
        { QLatin1String("QStringList"), QLatin1String("list of strings") },
        { qVariantT(), QLatin1String("object") },
        { QLatin1String("quint32"), intT() },
        { QLatin1String("uint32_t"), intT() },
        { QLatin1String("quint64"), intT() },
        { QLatin1String("qint64"), intT() },
        { QLatin1String("size_t"), intT() },
        { QLatin1String("int64_t"), intT() },
        { QLatin1String("qreal"), floatT() }
    };
    const auto found = typeMap.find(name);
    if (found != typeMap.end())
        return found.value();

    QString strType;
    if (type.isConstant() && name == QLatin1String("char") && type.indirections() == 1) {
        strType = QLatin1String("str");
    } else if (name.startsWith(unsignedShortT())) {
        strType = intT();
    } else if (name.startsWith(unsignedT())) { // uint and ulong
        strType = intT();
    } else if (type.isContainer()) {
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
        const AbstractMetaClass *k = AbstractMetaClass::findClass(classes(), type.typeEntry());
        strType = k ? k->fullName() : type.name();
        strType = QStringLiteral(":any:`") + strType + QLatin1Char('`');
    }
    return strType;
}

void QtDocGenerator::writeParameterType(TextStream& s, const AbstractMetaClass* cppClass,
                                        const AbstractMetaArgument &arg) const
{
    s << ":param " << arg.name() << ": "
      << translateToPythonType(arg.type(), cppClass) << '\n';
}

void QtDocGenerator::writeFunctionParametersType(TextStream &s, const AbstractMetaClass *cppClass,
                                                 const AbstractMetaFunction *func) const
{
    s << '\n';
    const AbstractMetaArgumentList &funcArgs = func->arguments();
    for (const AbstractMetaArgument &arg : funcArgs) {

        if (func->argumentRemoved(arg.argumentIndex() + 1))
            continue;

        writeParameterType(s, cppClass, arg);
    }

    if (!func->isConstructor() && !func->isVoid()) {

        QString retType;
        // check if the return type was modified
        for (const auto &mod : func->modifications()) {
            for (const ArgumentModification &argMod : mod.argument_mods()) {
                if (argMod.index == 0) {
                    retType = argMod.modified_type;
                    break;
                }
            }
        }

        if (retType.isEmpty())
            retType = translateToPythonType(func->type(), cppClass);
        s << ":rtype: " << retType << '\n';
    }
    s << '\n';
}

void QtDocGenerator::writeFunction(TextStream& s, const AbstractMetaClass* cppClass,
                                   const AbstractMetaFunction* func, bool indexed)
{
    s << functionSignature(cppClass, func);

    {
        Indentation indentation(s);
        if (!indexed)
            s << "\n:noindex:";
        s << "\n\n";
        writeFunctionParametersType(s, cppClass, func);
        const auto version = versionOf(func->typeEntry());
        if (!version.isNull())
            s << rstVersionAdded(version);
        if (func->attributes().testFlag(AbstractMetaAttributes::Deprecated))
            s << rstDeprecationNote("function");
    }
    writeInjectDocumentation(s, TypeSystem::DocModificationPrepend, cppClass, func);
    if (!writeInjectDocumentation(s, TypeSystem::DocModificationReplace, cppClass, func)) {
        writeFormattedText(s, func->documentation(), cppClass, Documentation::Brief);
        writeFormattedText(s, func->documentation(), cppClass, Documentation::Detailed);
    }
    writeInjectDocumentation(s, TypeSystem::DocModificationAppend, cppClass, func);
}

static void writeFancyToc(TextStream& s, const QStringList& items, int cols = 2)
{
    using TocMap = QMap<QChar, QStringList>;
    TocMap tocMap;
    QChar Q = QLatin1Char('Q');
    QChar idx;
    for (QString item : items) {
        if (item.isEmpty())
            continue;
        item.chop(4); // Remove the .rst extension
        // skip namespace if necessary
        const QString className = item.split(QLatin1Char('.')).last();
        if (className.startsWith(Q) && className.length() > 1)
            idx = className[1];
        else
            idx = className[0];
        tocMap[idx] << item;
    }
    QtXmlToSphinx::Table table;
    QtXmlToSphinx::TableRow row;

    int itemsPerCol = (items.size() + tocMap.size()*2) / cols;
    QString currentColData;
    int i = 0;
    TextStream ss(&currentColData);
    QMutableMapIterator<QChar, QStringList> it(tocMap);
    while (it.hasNext()) {
        it.next();
        std::sort(it.value().begin(), it.value().end());

        if (i)
            ss << '\n';

        ss << "**" << it.key() << "**\n\n";
        i += 2; // a letter title is equivalent to two entries in space
        for (const QString &item : qAsConst(it.value())) {
            ss << "* :doc:`" << item << "`\n";
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
    table.appendRow(row);
    table.normalize();
    s << ".. container:: pysidetoc\n\n";
    table.format(s);
}

bool QtDocGenerator::finishGeneration()
{
    if (!classes().isEmpty())
        writeModuleDocumentation();
    if (!m_additionalDocumentationList.isEmpty())
        writeAdditionalDocumentation();
    return true;
}

void QtDocGenerator::writeModuleDocumentation()
{
    QMap<QString, QStringList>::iterator it = m_packages.begin();
    for (; it != m_packages.end(); ++it) {
        QString key = it.key();
        key.replace(QLatin1Char('.'), QLatin1Char('/'));
        QString outputDir = outputDirectory() + QLatin1Char('/') + key;
        FileOut output(outputDir + QLatin1String("/index.rst"));
        TextStream& s = output.stream;

        const QString &title = it.key();
        s << ".. module:: " << title << "\n\n"
            << title << '\n'
            << Pad('*', title.length()) << "\n\n";

        /* Avoid showing "Detailed Description for *every* class in toc tree */
        Indentation indentation(s);
        // Store the it.key() in a QString so that it can be stripped off unwanted
        // information when neeeded. For example, the RST files in the extras directory
        // doesn't include the PySide# prefix in their names.
        const QString moduleName = it.key();
        const int lastIndex = moduleName.lastIndexOf(QLatin1Char('.'));

        // Search for extra-sections
        if (!m_extraSectionDir.isEmpty()) {
            QDir extraSectionDir(m_extraSectionDir);
            if (!extraSectionDir.exists())
                qCWarning(lcShibokenDoc) << m_extraSectionDir << "doesn't exist";

            QStringList fileList = extraSectionDir.entryList(QStringList() << (moduleName.mid(lastIndex + 1) + QLatin1String("?*.rst")), QDir::Files);
            QStringList::iterator it2 = fileList.begin();
            for (; it2 != fileList.end(); ++it2) {
                QString origFileName(*it2);
                it2->remove(0, moduleName.indexOf(QLatin1Char('.')));
                QString newFilePath = outputDir + QLatin1Char('/') + *it2;
                if (QFile::exists(newFilePath))
                    QFile::remove(newFilePath);
                if (!QFile::copy(m_extraSectionDir + QLatin1Char('/') + origFileName, newFilePath)) {
                    qCDebug(lcShibokenDoc).noquote().nospace() << "Error copying extra doc "
                        << QDir::toNativeSeparators(m_extraSectionDir + QLatin1Char('/') + origFileName)
                        << " to " << QDir::toNativeSeparators(newFilePath);
                }
            }
            it.value().append(fileList);
        }

        writeFancyToc(s, it.value());

        s << ".. container:: hide\n\n" << indent
            << ".. toctree::\n" << indent
            << ":maxdepth: 1\n\n";
        for (const QString &className : qAsConst(it.value()))
            s << className << '\n';
        s << "\n\n" << outdent << outdent
            << "Detailed Description\n--------------------\n\n";

        // module doc is always wrong and C++istic, so go straight to the extra directory!
        QFile moduleDoc(m_extraSectionDir + QLatin1Char('/') + moduleName.mid(lastIndex + 1) + QLatin1String(".rst"));
        if (moduleDoc.open(QIODevice::ReadOnly | QIODevice::Text)) {
            s << moduleDoc.readAll();
            moduleDoc.close();
        } else {
            // try the normal way
            Documentation moduleDoc = m_docParser->retrieveModuleDocumentation(it.key());
            if (moduleDoc.format() == Documentation::Native) {
                QString context = it.key();
                QtXmlToSphinx::stripPythonQualifiers(&context);
                QtXmlToSphinx x(this, moduleDoc.value(), context);
                s << x;
            } else {
                s << moduleDoc.value();
            }
        }
    }
}

static inline QString msgNonExistentAdditionalDocFile(const QString &dir,
                                                      const QString &fileName)
{
    const QString result = QLatin1Char('"') + fileName
        + QLatin1String("\" does not exist in ")
        + QDir::toNativeSeparators(dir) + QLatin1Char('.');
    return result;
}

void QtDocGenerator::writeAdditionalDocumentation() const
{
    QFile additionalDocumentationFile(m_additionalDocumentationList);
    if (!additionalDocumentationFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(lcShibokenDoc, "%s",
                  qPrintable(msgCannotOpenForReading(additionalDocumentationFile)));
        return;
    }

    QDir outDir(outputDirectory());
    const QString rstSuffix = fileNameSuffix();

    QString errorMessage;
    int successCount = 0;
    int count = 0;

    QString targetDir = outDir.absolutePath();

    while (!additionalDocumentationFile.atEnd()) {
        const QByteArray lineBA = additionalDocumentationFile.readLine().trimmed();
        if (lineBA.isEmpty() || lineBA.startsWith('#'))
            continue;
        const QString line = QFile::decodeName(lineBA);
        // Parse "[directory]" specification
        if (line.size() > 2 && line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            const QString dir = line.mid(1, line.size() - 2);
            if (dir.isEmpty() || dir == QLatin1String(".")) {
                targetDir = outDir.absolutePath();
            } else {
                if (!outDir.exists(dir) && !outDir.mkdir(dir)) {
                    qCWarning(lcShibokenDoc, "Cannot create directory %s under %s",
                              qPrintable(dir),
                              qPrintable(QDir::toNativeSeparators(outputDirectory())));
                    break;
                }
                targetDir = outDir.absoluteFilePath(dir);
            }
        } else {
            // Normal file entry
            QFileInfo fi(m_docDataDir + QLatin1Char('/') + line);
            if (fi.isFile()) {
                const QString rstFileName = fi.baseName() + rstSuffix;
                const QString rstFile = targetDir + QLatin1Char('/') + rstFileName;
                const QString context = targetDir.mid(targetDir.lastIndexOf(QLatin1Char('/')) + 1);
                if (QtXmlToSphinx::convertToRst(this, fi.absoluteFilePath(),
                                                rstFile, context, &errorMessage)) {
                    ++successCount;
                    qCDebug(lcShibokenDoc).nospace().noquote() << __FUNCTION__
                        << " converted " << fi.fileName()
                        << ' ' << rstFileName;
                } else {
                    qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
                }
            } else {
                qCWarning(lcShibokenDoc, "%s",
                          qPrintable(msgNonExistentAdditionalDocFile(m_docDataDir, line)));
            }
            ++count;
        }
    }
    additionalDocumentationFile.close();

    qCInfo(lcShibokenDoc, "Created %d/%d additional documentation files.",
           successCount, count);
}

#ifdef __WIN32__
#   define PATH_SEP ';'
#else
#   define PATH_SEP ':'
#endif

bool QtDocGenerator::doSetup()
{
    if (m_codeSnippetDirs.isEmpty())
        m_codeSnippetDirs = m_libSourceDir.split(QLatin1Char(PATH_SEP));

    if (!m_docParser)
        m_docParser = new QtDocParser;

    if (m_libSourceDir.isEmpty() || m_docDataDir.isEmpty()) {
        qCWarning(lcShibokenDoc) << "Documentation data dir and/or Qt source dir not informed, "
                                 "documentation will not be extracted from Qt sources.";
        return false;
    }

    m_docParser->setDocumentationDataDirectory(m_docDataDir);
    m_docParser->setLibrarySourceDirectory(m_libSourceDir);
    return true;
}


Generator::OptionDescriptions QtDocGenerator::options() const
{
    return OptionDescriptions()
        << qMakePair(QLatin1String("doc-parser=<parser>"),
                     QLatin1String("The documentation parser used to interpret the documentation\n"
                                   "input files (qdoc|doxygen)"))
        << qMakePair(QLatin1String("documentation-code-snippets-dir=<dir>"),
                     QLatin1String("Directory used to search code snippets used by the documentation"))
        << qMakePair(QLatin1String("documentation-data-dir=<dir>"),
                     QLatin1String("Directory with XML files generated by documentation tool"))
        << qMakePair(QLatin1String("documentation-extra-sections-dir=<dir>"),
                     QLatin1String("Directory used to search for extra documentation sections"))
        << qMakePair(QLatin1String("library-source-dir=<dir>"),
                     QLatin1String("Directory where library source code is located"))
        << qMakePair(additionalDocumentationOption() + QLatin1String("=<file>"),
                     QLatin1String("List of additional XML files to be converted to .rst files\n"
                                   "(for example, tutorials)."));
}

bool QtDocGenerator::handleOption(const QString &key, const QString &value)
{
    if (key == QLatin1String("library-source-dir")) {
        m_libSourceDir = value;
        return true;
    }
    if (key == QLatin1String("documentation-data-dir")) {
        m_docDataDir = value;
        return true;
    }
    if (key == QLatin1String("documentation-code-snippets-dir")) {
        m_codeSnippetDirs = value.split(QLatin1Char(PATH_SEP));
        return true;
    }
    if (key == QLatin1String("documentation-extra-sections-dir")) {
        m_extraSectionDir = value;
        return true;
    }
    if (key == QLatin1String("doc-parser")) {
        qCDebug(lcShibokenDoc).noquote().nospace() << "doc-parser: " << value;
        if (value == QLatin1String("doxygen"))
            m_docParser = new DoxygenParser;
        return true;
    }
    if (key == additionalDocumentationOption()) {
        m_additionalDocumentationList = value;
        return true;
    }
    return false;
}
