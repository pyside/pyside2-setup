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

#include "typesystem.h"
#include "typedatabase.h"
#include "messages.h"
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>

#include <algorithm>

static QString strings_Object = QLatin1String("Object");
static QString strings_String = QLatin1String("String");
static QString strings_char = QLatin1String("char");
static QString strings_jchar = QLatin1String("jchar");
static QString strings_jobject = QLatin1String("jobject");

static inline QString callOperator() { return QStringLiteral("operator()"); }

PrimitiveTypeEntry::PrimitiveTypeEntry(const QString &name, const QVersionNumber &vr) :
    TypeEntry(name, PrimitiveType, vr),
    m_preferredTargetLangType(true)
{
}

QString PrimitiveTypeEntry::targetLangName() const
{
    return m_targetLangName;
}

QString PrimitiveTypeEntry::targetLangApiName() const
{
    return m_targetLangApiName;
}

PrimitiveTypeEntry *PrimitiveTypeEntry::basicReferencedTypeEntry() const
{
    if (!m_referencedTypeEntry)
        return nullptr;

    PrimitiveTypeEntry *baseReferencedTypeEntry = m_referencedTypeEntry->basicReferencedTypeEntry();
    return baseReferencedTypeEntry ? baseReferencedTypeEntry : m_referencedTypeEntry;
}

TypeEntry *PrimitiveTypeEntry::clone() const
{
    return new PrimitiveTypeEntry(*this);
}

PrimitiveTypeEntry::PrimitiveTypeEntry(const PrimitiveTypeEntry &) = default;

CodeSnipList TypeEntry::codeSnips() const
{
    return m_codeSnips;
}

QString Modification::accessModifierString() const
{
    if (isPrivate()) return QLatin1String("private");
    if (isProtected()) return QLatin1String("protected");
    if (isPublic()) return QLatin1String("public");
    if (isFriendly()) return QLatin1String("friendly");
    return QString();
}

FunctionModificationList ComplexTypeEntry::functionModifications(const QString &signature) const
{
    FunctionModificationList lst;
    for (int i = 0; i < m_functionMods.count(); ++i) {
        const FunctionModification &mod = m_functionMods.at(i);
        if (mod.matches(signature))
            lst << mod;
    }
    return lst;
}

FieldModification ComplexTypeEntry::fieldModification(const QString &name) const
{
    for (const auto &fieldMod : m_fieldMods) {
        if (fieldMod.name == name)
            return fieldMod;
    }
    FieldModification mod;
    mod.name = name;
    mod.modifiers = FieldModification::Readable | FieldModification::Writable;
    return mod;
}

QString ComplexTypeEntry::targetLangName() const
{
    return m_targetLangName.isEmpty() ?
        TypeEntry::targetLangName() : m_targetLangName;
}

void ComplexTypeEntry::setDefaultConstructor(const QString& defaultConstructor)
{
    m_defaultConstructor = defaultConstructor;
}
QString ComplexTypeEntry::defaultConstructor() const
{
    return m_defaultConstructor;
}
bool ComplexTypeEntry::hasDefaultConstructor() const
{
    return !m_defaultConstructor.isEmpty();
}

TypeEntry *ComplexTypeEntry::clone() const
{
    return new ComplexTypeEntry(*this);
}

// Take over parameters relevant for typedefs
void ComplexTypeEntry::useAsTypedef(const ComplexTypeEntry *source)
{
    TypeEntry::useAsTypedef(source);
    m_qualifiedCppName = source->m_qualifiedCppName;
    m_targetLangName = source->m_targetLangName;
    m_lookupName = source->m_lookupName;
    m_targetType = source->m_targetType;
}

ComplexTypeEntry::ComplexTypeEntry(const ComplexTypeEntry &) = default;

QString ContainerTypeEntry::targetLangName() const
{

    switch (m_type) {
    case StringListContainer: return QLatin1String("QStringList");
    case ListContainer: return QLatin1String("QList");
    case LinkedListContainer: return QLatin1String("QLinkedList");
    case VectorContainer: return QLatin1String("QVector");
    case StackContainer: return QLatin1String("QStack");
    case QueueContainer: return QLatin1String("QQueue");
    case SetContainer: return QLatin1String("QSet");
    case MapContainer: return QLatin1String("QMap");
    case MultiMapContainer: return QLatin1String("QMultiMap");
    case HashContainer: return QLatin1String("QHash");
    case MultiHashContainer: return QLatin1String("QMultiHash");
    case PairContainer: return QLatin1String("QPair");
    default:
        qWarning("bad type... %d", m_type);
        break;
    }
    return QString();
}

QString ContainerTypeEntry::qualifiedCppName() const
{
    if (m_type == StringListContainer)
        return QLatin1String("QStringList");
    return ComplexTypeEntry::qualifiedCppName();
}

TypeEntry *ContainerTypeEntry::clone() const
{
    return new ContainerTypeEntry(*this);
}

ContainerTypeEntry::ContainerTypeEntry(const ContainerTypeEntry &) = default;

QString EnumTypeEntry::targetLangQualifier() const
{
    TypeEntry *te = TypeDatabase::instance()->findType(m_qualifier);
    return te ? te->targetLangName() : m_qualifier;
}

QString EnumTypeEntry::qualifiedTargetLangName() const
{
    QString qualifiedName;
    QString pkg = targetLangPackage();
    QString qualifier = targetLangQualifier();

    if (!pkg.isEmpty())
        qualifiedName += pkg + QLatin1Char('.');
    if (!qualifier.isEmpty())
        qualifiedName += qualifier + QLatin1Char('.');
    qualifiedName += targetLangName();

    return qualifiedName;
}

QString EnumTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

QString FlagsTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

TypeEntry *EnumTypeEntry::clone() const
{
    return new EnumTypeEntry(*this);
}

EnumTypeEntry::EnumTypeEntry(const EnumTypeEntry &) = default;

TypeEntry *FlagsTypeEntry::clone() const
{
    return new FlagsTypeEntry(*this);
}

FlagsTypeEntry::FlagsTypeEntry(const FlagsTypeEntry &) = default;

QString FlagsTypeEntry::qualifiedTargetLangName() const
{
    return targetLangPackage() + QLatin1Char('.') + m_enum->targetLangQualifier()
        + QLatin1Char('.') + targetLangName();
}

QString FlagsTypeEntry::targetLangName() const
{
    return m_targetLangName;
}

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
    result += QLatin1String("\n// TEMPLATE - ") + m_name + QLatin1String(" - END");
    return result;
}


QString CodeSnipAbstract::code() const
{
    QString res;
    for (const CodeSnipFragment &codeFrag : codeList)
        res.append(codeFrag.code());

    return res;
}

QString CodeSnipFragment::code() const
{
    return m_instance ? m_instance->expandCode() : m_code;
}

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

QString FunctionModification::toString() const
{
    QString str = signature() + QLatin1String("->");
    if (modifiers & AccessModifierMask) {
        switch (modifiers & AccessModifierMask) {
        case Private: str += QLatin1String("private"); break;
        case Protected: str += QLatin1String("protected"); break;
        case Public: str += QLatin1String("public"); break;
        case Friendly: str += QLatin1String("friendly"); break;
        }
    }

    if (modifiers & Final) str += QLatin1String("final");
    if (modifiers & NonFinal) str += QLatin1String("non-final");

    if (modifiers & Readable) str += QLatin1String("readable");
    if (modifiers & Writable) str += QLatin1String("writable");

    if (modifiers & CodeInjection) {
        for (const CodeSnip &s : snips) {
            str += QLatin1String("\n//code injection:\n");
            str += s.code();
        }
    }

    if (modifiers & Rename) str += QLatin1String("renamed:") + renamedToName;

    if (modifiers & Deprecated) str += QLatin1String("deprecate");

    if (modifiers & ReplaceExpression) str += QLatin1String("replace-expression");

    return str;
}

static AddedFunction::TypeInfo parseType(const QString& signature,
                                         int startPos = 0, int *endPos = nullptr,
                                         QString *argumentName = nullptr)
{
    AddedFunction::TypeInfo result;
    static const QRegularExpression regex(QLatin1String("\\w"));
    Q_ASSERT(regex.isValid());
    int length = signature.length();
    int start = signature.indexOf(regex, startPos);
    if (start == -1) {
        if (signature.midRef(startPos + 1, 3) == QLatin1String("...")) { // varargs
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
        result.defaultValue = lst[1].trimmed();
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
            TypeInfo arg = parseType(signature, endPos, &endPos, &argumentName);
            if (!arg.name.isEmpty())
                m_arguments.append({argumentName, arg});
            // end of parameters...
            if (endPos >= signatureLength || signature[endPos] == QLatin1Char(')'))
                break;
        }
        // is const?
        m_isConst = signature.rightRef(signatureLength - endPos).contains(QLatin1String("const"));
    }
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
        const auto lines = code.splitRef(QLatin1Char('\n'));
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
    d << "modifiers=" << hex << showbase << modifiers << noshowbase << dec;
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
    if (!ti.defaultValue.isEmpty())
        d << " = " << ti.defaultValue;
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
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

AddedFunction::TypeInfo AddedFunction::TypeInfo::fromSignature(const QString& signature)
{
    return parseType(signature);
}

ComplexTypeEntry::ComplexTypeEntry(const QString &name, TypeEntry::Type t,
                                   const QVersionNumber &vr) :
    TypeEntry(name, t, vr),
    m_qualifiedCppName(name),
    m_polymorphicBase(false),
    m_genericClass(false),
    m_deleteInMainThread(false)
{
}

bool ComplexTypeEntry::isComplex() const
{
    return true;
}

QString ComplexTypeEntry::lookupName() const
{
    return m_lookupName.isEmpty() ? targetLangName() : m_lookupName;
}

QString ComplexTypeEntry::targetLangApiName() const
{
    return strings_jobject;
}

QString ContainerTypeEntry::typeName() const
{
    switch(m_type) {
        case LinkedListContainer:
            return QLatin1String("linked-list");
        case ListContainer:
            return QLatin1String("list");
        case StringListContainer:
            return QLatin1String("string-list");
        case VectorContainer:
            return QLatin1String("vector");
        case StackContainer:
            return QLatin1String("stack");
        case QueueContainer:
            return QLatin1String("queue");
        case SetContainer:
            return QLatin1String("set");
        case MapContainer:
            return QLatin1String("map");
        case MultiMapContainer:
            return QLatin1String("multi-map");
        case HashContainer:
            return QLatin1String("hash");
        case MultiHashContainer:
            return QLatin1String("multi-hash");
        case PairContainer:
            return QLatin1String("pair");
        case NoContainer:
        default:
            return QLatin1String("?");
    }
}

static const QSet<QString> &primitiveCppTypes()
{
    static QSet<QString> result;
    if (result.isEmpty()) {
        static const char *cppTypes[] = {
            "bool", "char", "double", "float", "int",
            "long", "long long", "short",
            "wchar_t"
        };
        for (const char *cppType : cppTypes)
            result.insert(QLatin1String(cppType));
    }
    return result;
}

void TypeEntry::setInclude(const Include &inc)
{
    // This is a workaround for preventing double inclusion of the QSharedPointer implementation
    // header, which does not use header guards. In the previous parser this was not a problem
    // because the Q_QDOC define was set, and the implementation header was never included.
    if (inc.name().endsWith(QLatin1String("qsharedpointer_impl.h"))) {
        QString path = inc.name();
        path.remove(QLatin1String("_impl"));
        m_include = Include(inc.type(), path);
    } else {
        m_include = inc;
    }
}

bool TypeEntry::isCppPrimitive() const
{
    if (!isPrimitive())
        return false;

    if (m_type == VoidType)
        return true;

    const PrimitiveTypeEntry *referencedType =
        static_cast<const PrimitiveTypeEntry *>(this)->basicReferencedTypeEntry();
    const QString &typeName = referencedType ? referencedType->name() : m_name;
    return typeName.contains(QLatin1Char(' ')) || primitiveCppTypes().contains(typeName);
}

TypeEntry::TypeEntry(const QString &name, TypeEntry::Type t, const QVersionNumber &vr) :
    m_name(name),
    m_type(t),
    m_version(vr)
{
}

TypeEntry::~TypeEntry()
{
    delete m_customConversion;
}

bool TypeEntry::hasCustomConversion() const
{
    return m_customConversion != nullptr;
}

void TypeEntry::setCustomConversion(CustomConversion* customConversion)
{
    m_customConversion = customConversion;
}

CustomConversion* TypeEntry::customConversion() const
{
    return m_customConversion;
}

TypeEntry *TypeEntry::clone() const
{
    return new TypeEntry(*this);
}

// Take over parameters relevant for typedefs
void TypeEntry::useAsTypedef(const TypeEntry *source)
{
    m_name = source->m_name;
    m_targetLangPackage = source->m_targetLangPackage;
    m_codeGeneration = source->m_codeGeneration;
    m_version = source->m_version;
}

TypeEntry::TypeEntry(const TypeEntry &) = default;

TypeSystemTypeEntry::TypeSystemTypeEntry(const QString &name, const QVersionNumber &vr) :
    TypeEntry(name, TypeSystemType, vr)
{
}

TypeEntry *TypeSystemTypeEntry::clone() const
{
    return new TypeSystemTypeEntry(*this);
}

TypeSystemTypeEntry::TypeSystemTypeEntry(const TypeSystemTypeEntry &) = default;

VoidTypeEntry::VoidTypeEntry() :
    TypeEntry(QLatin1String("void"), VoidType, QVersionNumber(0, 0))
{
}

TypeEntry *VoidTypeEntry::clone() const
{
    return new VoidTypeEntry(*this);
}

VoidTypeEntry::VoidTypeEntry(const VoidTypeEntry &) = default;

VarargsTypeEntry::VarargsTypeEntry() :
    TypeEntry(QLatin1String("..."), VarargsType, QVersionNumber(0, 0))
{
}

TypeEntry *VarargsTypeEntry::clone() const
{
    return new VarargsTypeEntry(*this);
}

VarargsTypeEntry::VarargsTypeEntry(const VarargsTypeEntry &) = default;

TemplateArgumentEntry::TemplateArgumentEntry(const QString &name, const QVersionNumber &vr) :
    TypeEntry(name, TemplateArgumentType, vr)
{
}

TypeEntry *TemplateArgumentEntry::clone() const
{
    return new TemplateArgumentEntry(*this);
}

TemplateArgumentEntry::TemplateArgumentEntry(const TemplateArgumentEntry &) = default;

ArrayTypeEntry::ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr) :
    TypeEntry(QLatin1String("Array"), ArrayType, vr),
    m_nestedType(nested_type)
{
    Q_ASSERT(m_nestedType);
}

QString ArrayTypeEntry::targetLangName() const
{
    return m_nestedType->targetLangName() + QLatin1String("[]");
}

QString ArrayTypeEntry::targetLangApiName() const
{
    return m_nestedType->isPrimitive()
        ? m_nestedType->targetLangApiName() + QLatin1String("Array")
        : QLatin1String("jobjectArray");
}

TypeEntry *ArrayTypeEntry::clone() const
{
    return new ArrayTypeEntry(*this);
}

ArrayTypeEntry::ArrayTypeEntry(const ArrayTypeEntry &) = default;

EnumTypeEntry::EnumTypeEntry(const QString &nspace, const QString &enumName,
                             const QVersionNumber &vr) :
    TypeEntry(nspace.isEmpty() ? enumName : nspace + QLatin1String("::") + enumName,
              EnumType, vr),
    m_qualifier(nspace),
    m_targetLangName(enumName)
{
}

QString EnumTypeEntry::targetLangName() const
{
    return m_targetLangName;
}

EnumValueTypeEntry::EnumValueTypeEntry(const QString &name, const QString &value,
                                       const EnumTypeEntry *enclosingEnum,
                                       const QVersionNumber &vr) :
    TypeEntry(name, TypeEntry::EnumValue, vr),
    m_value(value),
    m_enclosingEnum(enclosingEnum)
{
}

TypeEntry *EnumValueTypeEntry::clone() const
{
    return new EnumValueTypeEntry(*this);
}

EnumValueTypeEntry::EnumValueTypeEntry(const EnumValueTypeEntry &) = default;

FlagsTypeEntry::FlagsTypeEntry(const QString &name, const QVersionNumber &vr) :
    TypeEntry(name, FlagsType, vr)
{
}

/* A typedef entry allows for specifying template specializations in the
 * typesystem XML file. */
TypedefEntry::TypedefEntry(const QString &name, const QString &sourceType,
                           const QVersionNumber &vr)  :
    ComplexTypeEntry(name, TypedefType, vr),
    m_sourceType(sourceType)
{
}

TypeEntry *TypedefEntry::clone() const
{
    return new TypedefEntry(*this);
}

TypedefEntry::TypedefEntry(const TypedefEntry &) = default;

ContainerTypeEntry::ContainerTypeEntry(const QString &name, Type type,
                                       const QVersionNumber &vr) :
    ComplexTypeEntry(name, ContainerType, vr),
    m_type(type)
{
    setCodeGeneration(GenerateForSubclass);
}

SmartPointerTypeEntry::SmartPointerTypeEntry(const QString &name,
                                             const QString &getterName,
                                             const QString &smartPointerType,
                                             const QString &refCountMethodName,
                                             const QVersionNumber &vr) :
    ComplexTypeEntry(name, SmartPointerType, vr),
    m_getterName(getterName),
    m_smartPointerType(smartPointerType),
    m_refCountMethodName(refCountMethodName)
{
}

TypeEntry *SmartPointerTypeEntry::clone() const
{
    return new SmartPointerTypeEntry(*this);
}

SmartPointerTypeEntry::SmartPointerTypeEntry(const SmartPointerTypeEntry &) = default;

NamespaceTypeEntry::NamespaceTypeEntry(const QString &name, const QVersionNumber &vr) :
    ComplexTypeEntry(name, NamespaceType, vr)
{
}

TypeEntry *NamespaceTypeEntry::clone() const
{
    return new NamespaceTypeEntry(*this);
}

void NamespaceTypeEntry::setFilePattern(const QRegularExpression &r)
{
    m_filePattern = r;
    m_hasPattern = !m_filePattern.pattern().isEmpty();
    if (m_hasPattern)
        m_filePattern.optimize();
}

NamespaceTypeEntry::NamespaceTypeEntry(const NamespaceTypeEntry &) = default;

bool NamespaceTypeEntry::matchesFile(const QString &needle) const
{
    return m_filePattern.match(needle).hasMatch();
}

ValueTypeEntry::ValueTypeEntry(const QString &name, const QVersionNumber &vr) :
    ComplexTypeEntry(name, BasicValueType, vr)
{
}

bool ValueTypeEntry::isValue() const
{
    return true;
}

bool ValueTypeEntry::isNativeIdBased() const
{
    return true;
}

TypeEntry *ValueTypeEntry::clone() const
{
    return new ValueTypeEntry(*this);
}

ValueTypeEntry::ValueTypeEntry(const ValueTypeEntry &) = default;

ValueTypeEntry::ValueTypeEntry(const QString &name, Type t, const QVersionNumber &vr) :
    ComplexTypeEntry(name, t, vr)
{
}

struct CustomConversion::CustomConversionPrivate
{
    CustomConversionPrivate(const TypeEntry* ownerType)
        : ownerType(ownerType), replaceOriginalTargetToNativeConversions(false)
    {
    }
    const TypeEntry* ownerType;
    QString nativeToTargetConversion;
    bool replaceOriginalTargetToNativeConversions;
    TargetToNativeConversions targetToNativeConversions;
};

struct CustomConversion::TargetToNativeConversion::TargetToNativeConversionPrivate
{
    TargetToNativeConversionPrivate()
        : sourceType(nullptr)
    {
    }
    const TypeEntry* sourceType;
    QString sourceTypeName;
    QString sourceTypeCheck;
    QString conversion;
};

CustomConversion::CustomConversion(TypeEntry* ownerType)
{
    m_d = new CustomConversionPrivate(ownerType);
    if (ownerType)
        ownerType->setCustomConversion(this);
}

CustomConversion::~CustomConversion()
{
    qDeleteAll(m_d->targetToNativeConversions);
    delete m_d;
}

const TypeEntry* CustomConversion::ownerType() const
{
    return m_d->ownerType;
}

QString CustomConversion::nativeToTargetConversion() const
{
    return m_d->nativeToTargetConversion;
}

void CustomConversion::setNativeToTargetConversion(const QString& nativeToTargetConversion)
{
    m_d->nativeToTargetConversion = nativeToTargetConversion;
}

bool CustomConversion::replaceOriginalTargetToNativeConversions() const
{
    return m_d->replaceOriginalTargetToNativeConversions;
}

void CustomConversion::setReplaceOriginalTargetToNativeConversions(bool replaceOriginalTargetToNativeConversions)
{
    m_d->replaceOriginalTargetToNativeConversions = replaceOriginalTargetToNativeConversions;
}

bool CustomConversion::hasTargetToNativeConversions() const
{
    return !(m_d->targetToNativeConversions.isEmpty());
}

CustomConversion::TargetToNativeConversions& CustomConversion::targetToNativeConversions()
{
    return m_d->targetToNativeConversions;
}

const CustomConversion::TargetToNativeConversions& CustomConversion::targetToNativeConversions() const
{
    return m_d->targetToNativeConversions;
}

void CustomConversion::addTargetToNativeConversion(const QString& sourceTypeName,
                                                   const QString& sourceTypeCheck,
                                                   const QString& conversion)
{
    m_d->targetToNativeConversions.append(new TargetToNativeConversion(sourceTypeName, sourceTypeCheck, conversion));
}

CustomConversion::TargetToNativeConversion::TargetToNativeConversion(const QString& sourceTypeName,
                                                                     const QString& sourceTypeCheck,
                                                                     const QString& conversion)
{
    m_d = new TargetToNativeConversionPrivate;
    m_d->sourceTypeName = sourceTypeName;
    m_d->sourceTypeCheck = sourceTypeCheck;
    m_d->conversion = conversion;
}

CustomConversion::TargetToNativeConversion::~TargetToNativeConversion()
{
    delete m_d;
}

const TypeEntry* CustomConversion::TargetToNativeConversion::sourceType() const
{
    return m_d->sourceType;
}

void CustomConversion::TargetToNativeConversion::setSourceType(const TypeEntry* sourceType)
{
    m_d->sourceType = sourceType;
}

bool CustomConversion::TargetToNativeConversion::isCustomType() const
{
    return !(m_d->sourceType);
}

QString CustomConversion::TargetToNativeConversion::sourceTypeName() const
{
    return m_d->sourceTypeName;
}

QString CustomConversion::TargetToNativeConversion::sourceTypeCheck() const
{
    return m_d->sourceTypeCheck;
}

QString CustomConversion::TargetToNativeConversion::conversion() const
{
    return m_d->conversion;
}

void CustomConversion::TargetToNativeConversion::setConversion(const QString& conversion)
{
    m_d->conversion = conversion;
}

InterfaceTypeEntry::InterfaceTypeEntry(const QString &name, const QVersionNumber &vr) :
    ComplexTypeEntry(name, InterfaceType, vr)
{
}

bool InterfaceTypeEntry::isNativeIdBased() const
{
    return true;
}

QString InterfaceTypeEntry::qualifiedCppName() const
{
    const int len = ComplexTypeEntry::qualifiedCppName().length() - interfaceName(QString()).length();
    return ComplexTypeEntry::qualifiedCppName().left(len);
}

TypeEntry *InterfaceTypeEntry::clone() const
{
    return new InterfaceTypeEntry(*this);
}

InterfaceTypeEntry::InterfaceTypeEntry(const InterfaceTypeEntry &) = default;

FunctionTypeEntry::FunctionTypeEntry(const QString &name, const QString &signature,
                                     const QVersionNumber &vr) :
    TypeEntry(name, FunctionType, vr)
{
    addSignature(signature);
}

TypeEntry *FunctionTypeEntry::clone() const
{
    return new FunctionTypeEntry(*this);
}

FunctionTypeEntry::FunctionTypeEntry(const FunctionTypeEntry &) = default;

ObjectTypeEntry::ObjectTypeEntry(const QString &name, const QVersionNumber &vr)
    : ComplexTypeEntry(name, ObjectType, vr)
{
}

InterfaceTypeEntry *ObjectTypeEntry::designatedInterface() const
{
    return m_interface;
}

bool ObjectTypeEntry::isNativeIdBased() const
{
    return true;
}

TypeEntry *ObjectTypeEntry::clone() const
{
    return new ObjectTypeEntry(*this);
}

ObjectTypeEntry::ObjectTypeEntry(const ObjectTypeEntry &) = default;
