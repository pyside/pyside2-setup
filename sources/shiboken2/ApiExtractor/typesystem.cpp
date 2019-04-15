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
#include "typesystem_p.h"
#include "typedatabase.h"
#include "messages.h"
#include "reporthandler.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>
#include <QtCore/QStringView>
#include <QtCore/QStringAlgorithms>
#include <QtCore/QXmlStreamAttributes>
#include <QtCore/QXmlStreamReader>

#include <algorithm>

const char *TARGET_CONVERSION_RULE_FLAG = "0";
const char *NATIVE_CONVERSION_RULE_FLAG = "1";

static QString strings_Object = QLatin1String("Object");
static QString strings_String = QLatin1String("String");
static QString strings_char = QLatin1String("char");
static QString strings_jchar = QLatin1String("jchar");
static QString strings_jobject = QLatin1String("jobject");

static inline QString allowThreadAttribute() { return QStringLiteral("allow-thread"); }
static inline QString colonColon() { return QStringLiteral("::"); }
static inline QString copyableAttribute() { return QStringLiteral("copyable"); }
static inline QString accessAttribute() { return QStringLiteral("access"); }
static inline QString actionAttribute() { return QStringLiteral("action"); }
static inline QString quoteAfterLineAttribute() { return QStringLiteral("quote-after-line"); }
static inline QString quoteBeforeLineAttribute() { return QStringLiteral("quote-before-line"); }
static inline QString textAttribute() { return QStringLiteral("text"); }
static inline QString nameAttribute() { return QStringLiteral("name"); }
static inline QString sinceAttribute() { return QStringLiteral("since"); }
static inline QString defaultSuperclassAttribute() { return QStringLiteral("default-superclass"); }
static inline QString deleteInMainThreadAttribute() { return QStringLiteral("delete-in-main-thread"); }
static inline QString deprecatedAttribute() { return QStringLiteral("deprecated"); }
static inline QString exceptionHandlingAttribute() { return QStringLiteral("exception-handling"); }
static inline QString extensibleAttribute() { return QStringLiteral("extensible"); }
static inline QString flagsAttribute() { return QStringLiteral("flags"); }
static inline QString forceAbstractAttribute() { return QStringLiteral("force-abstract"); }
static inline QString forceIntegerAttribute() { return QStringLiteral("force-integer"); }
static inline QString formatAttribute() { return QStringLiteral("format"); }
static inline QString classAttribute() { return QStringLiteral("class"); }
static inline QString generateAttribute() { return QStringLiteral("generate"); }
static inline QString genericClassAttribute() { return QStringLiteral("generic-class"); }
static inline QString indexAttribute() { return QStringLiteral("index"); }
static inline QString invalidateAfterUseAttribute() { return QStringLiteral("invalidate-after-use"); }
static inline QString locationAttribute() { return QStringLiteral("location"); }
static inline QString modifiedTypeAttribute() { return QStringLiteral("modified-type"); }
static inline QString modifierAttribute() { return QStringLiteral("modifier"); }
static inline QString ownershipAttribute() { return QStringLiteral("owner"); }
static inline QString packageAttribute() { return QStringLiteral("package"); }
static inline QString positionAttribute() { return QStringLiteral("position"); }
static inline QString preferredConversionAttribute() { return QStringLiteral("preferred-conversion"); }
static inline QString preferredTargetLangTypeAttribute() { return QStringLiteral("preferred-target-lang-type"); }
static inline QString removeAttribute() { return QStringLiteral("remove"); }
static inline QString renameAttribute() { return QStringLiteral("rename"); }
static inline QString readAttribute() { return QStringLiteral("read"); }
static inline QString writeAttribute() { return QStringLiteral("write"); }
static inline QString replaceAttribute() { return QStringLiteral("replace"); }
static inline QString toAttribute() { return QStringLiteral("to"); }
static inline QString signatureAttribute() { return QStringLiteral("signature"); }
static inline QString snippetAttribute() { return QStringLiteral("snippet"); }
static inline QString staticAttribute() { return QStringLiteral("static"); }
static inline QString threadAttribute() { return QStringLiteral("thread"); }
static inline QString sourceAttribute() { return QStringLiteral("source"); }
static inline QString streamAttribute() { return QStringLiteral("stream"); }
static inline QString xPathAttribute() { return QStringLiteral("xpath"); }
static inline QString virtualSlotAttribute() { return QStringLiteral("virtual-slot"); }
static inline QString enumIdentifiedByValueAttribute() { return QStringLiteral("identified-by-value"); }

static inline QString noAttributeValue() { return QStringLiteral("no"); }
static inline QString yesAttributeValue() { return QStringLiteral("yes"); }
static inline QString trueAttributeValue() { return QStringLiteral("true"); }
static inline QString falseAttributeValue() { return QStringLiteral("false"); }

static inline QString callOperator() { return QStringLiteral("operator()"); }

static QVector<CustomConversion *> customConversionsForReview;

// Set a regular expression for rejection from text. By legacy, those are fixed
// strings, except for '*' meaning 'match all'. Enclosing in "^..$"
// indicates regular expression.
static bool setRejectionRegularExpression(const QString &patternIn,
                                          QRegularExpression *re,
                                          QString *errorMessage)
{
    QString pattern;
    if (patternIn.startsWith(QLatin1Char('^')) && patternIn.endsWith(QLatin1Char('$')))
        pattern = patternIn;
    else if (patternIn == QLatin1String("*"))
        pattern = QStringLiteral("^.*$");
    else
        pattern = QLatin1Char('^') + QRegularExpression::escape(patternIn) + QLatin1Char('$');
    re->setPattern(pattern);
    if (!re->isValid()) {
        *errorMessage = QLatin1String("Invalid pattern \"") + patternIn
            + QLatin1String("\": ") + re->errorString();
        return false;
    }
    return true;
}

// Extract a snippet from a file within annotation "// @snippet label".
static QString extractSnippet(const QString &code, const QString &snippetLabel)
{
    if (snippetLabel.isEmpty())
        return code;
    const QString pattern = QStringLiteral(R"(^\s*//\s*@snippet\s+)")
        + QRegularExpression::escape(snippetLabel)
        + QStringLiteral(R"(\s*$)");
    const QRegularExpression snippetRe(pattern);
    Q_ASSERT(snippetRe.isValid());

    bool useLine = false;
    QString result;
    const auto lines = code.splitRef(QLatin1Char('\n'));
    for (const QStringRef &line : lines) {
        if (snippetRe.match(line).hasMatch()) {
            useLine = !useLine;
            if (!useLine)
                break; // End of snippet reached
        } else if (useLine)
            result += line.toString() + QLatin1Char('\n');
    }
    return result;
}

template <class EnumType, Qt::CaseSensitivity cs = Qt::CaseInsensitive>
struct EnumLookup
{
    QStringView name;
    EnumType value;
};

template <class EnumType, Qt::CaseSensitivity cs>
bool operator==(const EnumLookup<EnumType, cs> &e1, const EnumLookup<EnumType, cs> &e2)
{
    return e1.name.compare(e2.name, cs) == 0;
}

template <class EnumType, Qt::CaseSensitivity cs>
bool operator<(const EnumLookup<EnumType, cs> &e1, const EnumLookup<EnumType, cs> &e2)
{
    return e1.name.compare(e2.name, cs) < 0;
}

// Helper macros to define lookup functions that take a QStringView needle
// and an optional default return value.
#define ENUM_LOOKUP_BEGIN(EnumType, caseSensitivity, functionName, defaultReturnValue) \
static EnumType functionName(QStringView needle, EnumType defaultValue = defaultReturnValue) \
{ \
    typedef EnumLookup<EnumType, caseSensitivity> HaystackEntry; \
    static const HaystackEntry haystack[] =

#define ENUM_LOOKUP_LINEAR_SEARCH() \
    const auto end = haystack + sizeof(haystack) / sizeof(haystack[0]); \
    const auto it = std::find(haystack, end, HaystackEntry{needle, defaultValue}); \
    return it != end ? it->value : defaultValue; \
}

#define ENUM_LOOKUP_BINARY_SEARCH() \
    const auto end = haystack + sizeof(haystack) / sizeof(haystack[0]); \
    const HaystackEntry needleEntry{needle, defaultValue}; \
    const auto lb = std::lower_bound(haystack, end, needleEntry); \
    return lb != end && *lb == needleEntry ? lb->value : defaultValue; \
}

ENUM_LOOKUP_BEGIN(TypeSystem::AllowThread, Qt::CaseInsensitive,
                  allowThreadFromAttribute, TypeSystem::AllowThread::Unspecified)
    {
        {QStringViewLiteral("yes"), TypeSystem::AllowThread::Allow},
        {QStringViewLiteral("true"), TypeSystem::AllowThread::Allow},
        {QStringViewLiteral("auto"), TypeSystem::AllowThread::Auto},
        {QStringViewLiteral("no"), TypeSystem::AllowThread::Disallow},
        {QStringViewLiteral("false"), TypeSystem::AllowThread::Disallow},
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(TypeSystem::Language, Qt::CaseInsensitive,
                  languageFromAttribute, TypeSystem::NoLanguage)
    {
        {QStringViewLiteral("all"), TypeSystem::All}, // sorted!
        {QStringViewLiteral("constructors"), TypeSystem::Constructors},
        {QStringViewLiteral("destructor-function"), TypeSystem::DestructorFunction},
        {QStringViewLiteral("interface"), TypeSystem::Interface},
        {QStringViewLiteral("library-initializer"), TypeSystem::PackageInitializer},
        {QStringViewLiteral("native"), TypeSystem::NativeCode}, // em algum lugar do cpp
        {QStringViewLiteral("shell"), TypeSystem::ShellCode}, // coloca no header, mas antes da declaracao da classe
        {QStringViewLiteral("shell-declaration"), TypeSystem::ShellDeclaration},
        {QStringViewLiteral("target"), TypeSystem::TargetLangCode}  // em algum lugar do cpp
    };
ENUM_LOOKUP_BINARY_SEARCH()

ENUM_LOOKUP_BEGIN(TypeSystem::Ownership, Qt::CaseInsensitive,
                   ownershipFromFromAttribute, TypeSystem::InvalidOwnership)
    {
        {QStringViewLiteral("target"), TypeSystem::TargetLangOwnership},
        {QStringViewLiteral("c++"), TypeSystem::CppOwnership},
        {QStringViewLiteral("default"), TypeSystem::DefaultOwnership}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(AddedFunction::Access, Qt::CaseInsensitive,
                  addedFunctionAccessFromAttribute, AddedFunction::InvalidAccess)
    {
        {QStringViewLiteral("public"), AddedFunction::Public},
        {QStringViewLiteral("protected"), AddedFunction::Protected},
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(Modification::Modifiers, Qt::CaseSensitive,
                  modifierFromAttribute, Modification::InvalidModifier)
    {
        {QStringViewLiteral("private"), Modification::Private},
        {QStringViewLiteral("public"), Modification::Public},
        {QStringViewLiteral("protected"), Modification::Protected},
        {QStringViewLiteral("friendly"), Modification::Friendly},
        {QStringViewLiteral("rename"), Modification::Rename},
        {QStringViewLiteral("final"), Modification::Final},
        {QStringViewLiteral("non-final"), Modification::NonFinal}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(ReferenceCount::Action, Qt::CaseInsensitive,
                  referenceCountFromAttribute, ReferenceCount::Invalid)
    {
        {QStringViewLiteral("add"), ReferenceCount::Add},
        {QStringViewLiteral("add-all"), ReferenceCount::AddAll},
        {QStringViewLiteral("remove"), ReferenceCount::Remove},
        {QStringViewLiteral("set"), ReferenceCount::Set},
        {QStringViewLiteral("ignore"), ReferenceCount::Ignore}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(ArgumentOwner::Action, Qt::CaseInsensitive,
                  argumentOwnerActionFromAttribute, ArgumentOwner::Invalid)
    {
        {QStringViewLiteral("add"), ArgumentOwner::Add},
        {QStringViewLiteral("remove"), ArgumentOwner::Remove}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(TypeSystem::CodeSnipPosition, Qt::CaseInsensitive,
                  codeSnipPositionFromAttribute, TypeSystem::CodeSnipPositionInvalid)
    {
        {QStringViewLiteral("beginning"), TypeSystem::CodeSnipPositionBeginning},
        {QStringViewLiteral("end"), TypeSystem::CodeSnipPositionEnd},
        {QStringViewLiteral("declaration"), TypeSystem::CodeSnipPositionDeclaration},
        {QStringViewLiteral("prototype-initialization"), TypeSystem::CodeSnipPositionPrototypeInitialization},
        {QStringViewLiteral("constructor-initialization"), TypeSystem::CodeSnipPositionConstructorInitialization},
        {QStringViewLiteral("constructor"), TypeSystem::CodeSnipPositionConstructor}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(Include::IncludeType, Qt::CaseInsensitive,
                  locationFromAttribute, Include::InvalidInclude)
    {
        {QStringViewLiteral("global"), Include::IncludePath},
        {QStringViewLiteral("local"), Include::LocalPath},
        {QStringViewLiteral("target"), Include::TargetLangImport}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(TypeSystem::DocModificationMode, Qt::CaseInsensitive,
                  docModificationFromAttribute, TypeSystem::DocModificationInvalid)
    {
        {QStringViewLiteral("append"), TypeSystem::DocModificationAppend},
        {QStringViewLiteral("prepend"), TypeSystem::DocModificationPrepend},
        {QStringViewLiteral("replace"), TypeSystem::DocModificationReplace}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(ContainerTypeEntry::Type, Qt::CaseSensitive,
                  containerTypeFromAttribute, ContainerTypeEntry::NoContainer)
    {
        {QStringViewLiteral("list"), ContainerTypeEntry::ListContainer},
        {QStringViewLiteral("string-list"), ContainerTypeEntry::StringListContainer},
        {QStringViewLiteral("linked-list"), ContainerTypeEntry::LinkedListContainer},
        {QStringViewLiteral("vector"), ContainerTypeEntry::VectorContainer},
        {QStringViewLiteral("stack"), ContainerTypeEntry::StackContainer},
        {QStringViewLiteral("queue"), ContainerTypeEntry::QueueContainer},
        {QStringViewLiteral("set"), ContainerTypeEntry::SetContainer},
        {QStringViewLiteral("map"), ContainerTypeEntry::MapContainer},
        {QStringViewLiteral("multi-map"), ContainerTypeEntry::MultiMapContainer},
        {QStringViewLiteral("hash"), ContainerTypeEntry::HashContainer},
        {QStringViewLiteral("multi-hash"), ContainerTypeEntry::MultiHashContainer},
        {QStringViewLiteral("pair"), ContainerTypeEntry::PairContainer}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(TypeRejection::MatchType, Qt::CaseSensitive,
                  typeRejectionFromAttribute, TypeRejection::Invalid)
    {
        {QStringViewLiteral("class"), TypeRejection::ExcludeClass},
        {QStringViewLiteral("function-name"), TypeRejection::Function},
        {QStringViewLiteral("field-name"), TypeRejection::Field},
        {QStringViewLiteral("enum-name"), TypeRejection::Enum },
        {QStringViewLiteral("argument-type"), TypeRejection::ArgumentType},
        {QStringViewLiteral("return-type"), TypeRejection::ReturnType}
    };
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(TypeSystem::ExceptionHandling, Qt::CaseSensitive,
                  exceptionHandlingFromAttribute, TypeSystem::ExceptionHandling::Unspecified)
{
    {QStringViewLiteral("no"), TypeSystem::ExceptionHandling::Off},
    {QStringViewLiteral("false"), TypeSystem::ExceptionHandling::Off},
    {QStringViewLiteral("auto-off"), TypeSystem::ExceptionHandling::AutoDefaultToOff},
    {QStringViewLiteral("auto-on"), TypeSystem::ExceptionHandling::AutoDefaultToOn},
    {QStringViewLiteral("yes"), TypeSystem::ExceptionHandling::On},
    {QStringViewLiteral("true"), TypeSystem::ExceptionHandling::On},
};
ENUM_LOOKUP_LINEAR_SEARCH()

ENUM_LOOKUP_BEGIN(StackElement::ElementType, Qt::CaseInsensitive,
                  elementFromTag, StackElement::None)
    {
        {QStringViewLiteral("access"), StackElement::Access}, // sorted!
        {QStringViewLiteral("add-conversion"), StackElement::AddConversion},
        {QStringViewLiteral("add-function"), StackElement::AddFunction},
        {QStringViewLiteral("argument-map"), StackElement::ArgumentMap},
        {QStringViewLiteral("array"), StackElement::Array},
        {QStringViewLiteral("container-type"), StackElement::ContainerTypeEntry},
        {QStringViewLiteral("conversion-rule"), StackElement::ConversionRule},
        {QStringViewLiteral("custom-constructor"), StackElement::CustomMetaConstructor},
        {QStringViewLiteral("custom-destructor"), StackElement::CustomMetaDestructor},
        {QStringViewLiteral("custom-type"), StackElement::CustomTypeEntry},
        {QStringViewLiteral("define-ownership"), StackElement::DefineOwnership},
        {QStringViewLiteral("enum-type"), StackElement::EnumTypeEntry},
        {QStringViewLiteral("extra-includes"), StackElement::ExtraIncludes},
        {QStringViewLiteral("function"), StackElement::FunctionTypeEntry},
        {QStringViewLiteral("include"), StackElement::Include},
        {QStringViewLiteral("inject-code"), StackElement::InjectCode},
        {QStringViewLiteral("inject-documentation"), StackElement::InjectDocumentation},
        {QStringViewLiteral("insert-template"), StackElement::TemplateInstanceEnum},
        {QStringViewLiteral("interface-type"), StackElement::InterfaceTypeEntry},
        {QStringViewLiteral("load-typesystem"), StackElement::LoadTypesystem},
        {QStringViewLiteral("modify-argument"), StackElement::ModifyArgument},
        {QStringViewLiteral("modify-documentation"), StackElement::ModifyDocumentation},
        {QStringViewLiteral("modify-field"), StackElement::ModifyField},
        {QStringViewLiteral("modify-function"), StackElement::ModifyFunction},
        {QStringViewLiteral("namespace-type"), StackElement::NamespaceTypeEntry},
        {QStringViewLiteral("native-to-target"), StackElement::NativeToTarget},
        {QStringViewLiteral("no-null-pointer"), StackElement::NoNullPointers},
        {QStringViewLiteral("object-type"), StackElement::ObjectTypeEntry},
        {QStringViewLiteral("parent"), StackElement::ParentOwner},
        {QStringViewLiteral("primitive-type"), StackElement::PrimitiveTypeEntry},
        {QStringViewLiteral("reference-count"), StackElement::ReferenceCount},
        {QStringViewLiteral("reject-enum-value"), StackElement::RejectEnumValue},
        {QStringViewLiteral("rejection"), StackElement::Rejection},
        {QStringViewLiteral("remove"), StackElement::Removal},
        {QStringViewLiteral("remove-argument"), StackElement::RemoveArgument},
        {QStringViewLiteral("remove-default-expression"), StackElement::RemoveDefaultExpression},
        {QStringViewLiteral("rename"), StackElement::Rename},
        {QStringViewLiteral("replace"), StackElement::Replace},
        {QStringViewLiteral("replace-default-expression"), StackElement::ReplaceDefaultExpression},
        {QStringViewLiteral("replace-type"), StackElement::ReplaceType},
        {QStringViewLiteral("smart-pointer-type"), StackElement::SmartPointerTypeEntry},
        {QStringViewLiteral("suppress-warning"), StackElement::SuppressedWarning},
        {QStringViewLiteral("target-to-native"), StackElement::TargetToNative},
        {QStringViewLiteral("template"), StackElement::Template},
        {QStringViewLiteral("typedef-type"), StackElement::TypedefTypeEntry},
        {QStringViewLiteral("typesystem"), StackElement::Root},
        {QStringViewLiteral("value-type"), StackElement::ValueTypeEntry},
    };
ENUM_LOOKUP_BINARY_SEARCH()

static int indexOfAttribute(const QXmlStreamAttributes &atts,
                            QStringView name)
{
    for (int i = 0, size = atts.size(); i < size; ++i) {
        if (atts.at(i).qualifiedName() == name)
            return i;
    }
    return -1;
}

static QString msgMissingAttribute(const QString &a)
{
    return QLatin1String("Required attribute '") + a
        + QLatin1String("' missing.");
}

QTextStream &operator<<(QTextStream &str, const QXmlStreamAttribute &attribute)
{
    str << attribute.qualifiedName() << "=\"" << attribute.value() << '"';
    return str;
}

static QString msgInvalidAttributeValue(const QXmlStreamAttribute &attribute)
{
    QString result;
    QTextStream(&result) << "Invalid attribute value:" << attribute;
    return result;
}

static QString msgUnusedAttributes(const QStringRef &tag, const QXmlStreamAttributes &attributes)
{
    QString result;
    QTextStream str(&result);
    str << attributes.size() << " attributes(s) unused on <" << tag << ">: ";
    for (int i = 0, size = attributes.size(); i < size; ++i) {
        if (i)
            str << ", ";
        str << attributes.at(i);
    }
    return result;
}

Handler::Handler(TypeDatabase *database, bool generate) :
    m_database(database),
    m_generate(generate ? TypeEntry::GenerateAll : TypeEntry::GenerateForSubclass)
{
}

static QString readerFileName(const QXmlStreamReader &reader)
{
    const QFile *file = qobject_cast<const QFile *>(reader.device());
    return file != nullptr ? file->fileName() : QString();
}

static QString msgReaderMessage(const QXmlStreamReader &reader,
                                const char *type,
                                const QString &what)
{
    QString message;
    QTextStream str(&message);
    str << type << ": ";
    const QString fileName = readerFileName(reader);
    if (fileName.isEmpty())
        str << "<stdin>:";
    else
        str << QDir::toNativeSeparators(fileName) << ':';
    str << reader.lineNumber() << ':' << reader.columnNumber()
        << ": " << what;
    return message;
}

static QString msgReaderWarning(const QXmlStreamReader &reader, const QString &what)
{
    return  msgReaderMessage(reader, "Warning", what);
}

static QString msgReaderError(const QXmlStreamReader &reader, const QString &what)
{
    return  msgReaderMessage(reader, "Error", what);
}

static QString msgUnimplementedElementWarning(const QXmlStreamReader &reader,
                                              const QStringRef &name)
{
    const QString message = QLatin1String("The element \"") +
        name + QLatin1String("\" is not implemented.");
    return msgReaderMessage(reader, "Warning", message);
}

static QString msgUnimplementedAttributeWarning(const QXmlStreamReader &reader,
                                                const QStringRef &name)
{
    const QString message = QLatin1String("The attribute \"") +
        name + QLatin1String("\" is not implemented.");
    return msgReaderMessage(reader, "Warning", message);
}

static inline QString msgUnimplementedAttributeWarning(const QXmlStreamReader &reader,
                                                       const QXmlStreamAttribute &attribute)
{
    return msgUnimplementedAttributeWarning(reader, attribute.qualifiedName());
}

static QString
    msgUnimplementedAttributeValueWarning(const QXmlStreamReader &reader,
                                          QStringView name, QStringView value)
{
    QString message;
    QTextStream(&message) << "The value \"" << value
        << "\" of the attribute \"" << name << "\" is not implemented.";
    return msgReaderMessage(reader, "Warning", message);
}

static inline
    QString msgUnimplementedAttributeValueWarning(const QXmlStreamReader &reader,
                                                  const QXmlStreamAttribute &attribute)
{
    return msgUnimplementedAttributeValueWarning(reader,
                                                 attribute.qualifiedName(),
                                                 attribute.value());
}

static QString msgInvalidVersion(const QStringRef &version, const QString &package = QString())
{
    QString result;
    QTextStream str(&result);
    str << "Invalid version \"" << version << '"';
    if (!package.isEmpty())
        str << "\" specified for package " << package;
    str << '.';
    return result;
}

static bool addRejection(TypeDatabase *database, QXmlStreamAttributes *attributes,
                         QString *errorMessage)
{
    const int classIndex = indexOfAttribute(*attributes, classAttribute());
    if (classIndex == -1) {
        *errorMessage = msgMissingAttribute(classAttribute());
        return false;
    }

    TypeRejection rejection;
    const QString className = attributes->takeAt(classIndex).value().toString();
    if (!setRejectionRegularExpression(className, &rejection.className, errorMessage))
        return false;

    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        const TypeRejection::MatchType type = typeRejectionFromAttribute(name);
        switch (type) {
        case TypeRejection::Function:
        case TypeRejection::Field:
        case TypeRejection::Enum:
        case TypeRejection::ArgumentType:
        case TypeRejection::ReturnType: {
            const QString pattern = attributes->takeAt(i).value().toString();
            if (!setRejectionRegularExpression(pattern, &rejection.pattern, errorMessage))
                return false;
            rejection.matchType = type;
            database->addRejection(rejection);
            return true;
        }
            break;
        default:
            break;
        }
    }

    // Special case: When all fields except class are empty, completely exclude class
    if (className == QLatin1String("*")) {
        *errorMessage = QLatin1String("bad reject entry, neither 'class', 'function-name'"
                                      " nor 'field' specified");
        return false;
    }
    rejection.matchType = TypeRejection::ExcludeClass;
    database->addRejection(rejection);
    return true;
}

bool Handler::parse(QXmlStreamReader &reader)
{
    m_error.clear();
    m_currentPath.clear();
    const QString fileName = readerFileName(reader);
    if (!fileName.isEmpty())
        m_currentPath = QFileInfo(fileName).absolutePath();

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
            m_error = msgReaderError(reader, reader.errorString());
            return false;
        case QXmlStreamReader::StartElement:
            if (!startElement(reader)) {
                m_error = msgReaderError(reader, m_error);
                return false;
            }

            break;
        case QXmlStreamReader::EndElement:
            if (!endElement(reader.name())) {
                m_error = msgReaderError(reader, m_error);
                return false;
            }
            break;
        case QXmlStreamReader::Characters:
            if (!characters(reader.text())) {
                m_error = msgReaderError(reader, m_error);
                return false;
            }
            break;
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }
    return true;
}

bool Handler::endElement(const QStringRef &localName)
{
    if (m_ignoreDepth) {
        --m_ignoreDepth;
        return true;
    }

    if (m_currentDroppedEntry) {
        if (m_currentDroppedEntryDepth == 1) {
            m_current = m_currentDroppedEntry->parent;
            delete m_currentDroppedEntry;
            m_currentDroppedEntry = 0;
            m_currentDroppedEntryDepth = 0;
        } else {
            --m_currentDroppedEntryDepth;
        }
        return true;
    }

    if (!localName.compare(QLatin1String("import-file"), Qt::CaseInsensitive))
        return true;

    if (!m_current)
        return true;

    switch (m_current->type) {
    case StackElement::Root:
        if (m_generate == TypeEntry::GenerateAll) {
            TypeDatabase::instance()->addGlobalUserFunctions(m_contextStack.top()->addedFunctions);
            TypeDatabase::instance()->addGlobalUserFunctionModifications(m_contextStack.top()->functionMods);
            for (CustomConversion *customConversion : qAsConst(customConversionsForReview)) {
                const CustomConversion::TargetToNativeConversions &toNatives = customConversion->targetToNativeConversions();
                for (CustomConversion::TargetToNativeConversion *toNative : toNatives)
                    toNative->setSourceType(m_database->findType(toNative->sourceTypeName()));
            }
        }
        break;
    case StackElement::ObjectTypeEntry:
    case StackElement::ValueTypeEntry:
    case StackElement::InterfaceTypeEntry:
    case StackElement::NamespaceTypeEntry: {
        ComplexTypeEntry *centry = static_cast<ComplexTypeEntry *>(m_current->entry);
        centry->setAddedFunctions(m_contextStack.top()->addedFunctions);
        centry->setFunctionModifications(m_contextStack.top()->functionMods);
        centry->setFieldModifications(m_contextStack.top()->fieldMods);
        centry->setCodeSnips(m_contextStack.top()->codeSnips);
        centry->setDocModification(m_contextStack.top()->docModifications);

        if (centry->designatedInterface()) {
            centry->designatedInterface()->setCodeSnips(m_contextStack.top()->codeSnips);
            centry->designatedInterface()->setFunctionModifications(m_contextStack.top()->functionMods);
        }
    }
    break;
    case StackElement::AddFunction: {
        // Leaving add-function: Assign all modifications to the added function
        StackElementContext *top = m_contextStack.top();
        const int modIndex = top->addedFunctionModificationIndex;
        top->addedFunctionModificationIndex = -1;
        Q_ASSERT(modIndex >= 0);
        Q_ASSERT(!top->addedFunctions.isEmpty());
        while (modIndex < top->functionMods.size())
            top->addedFunctions.last()->modifications.append(top->functionMods.takeAt(modIndex));
    }
        break;
    case StackElement::NativeToTarget:
    case StackElement::AddConversion: {
        CustomConversion* customConversion = static_cast<TypeEntry*>(m_current->entry)->customConversion();
        if (!customConversion) {
            m_error = QLatin1String("CustomConversion object is missing.");
            return false;
        }

        QString code = m_contextStack.top()->codeSnips.takeLast().code();
        if (m_current->type == StackElement::AddConversion) {
            if (customConversion->targetToNativeConversions().isEmpty()) {
                m_error = QLatin1String("CustomConversion's target to native conversions missing.");
                return false;
            }
            customConversion->targetToNativeConversions().last()->setConversion(code);
        } else {
            customConversion->setNativeToTargetConversion(code);
        }
    }
    break;
    case StackElement::CustomMetaConstructor: {
        m_current->entry->setCustomConstructor(*m_current->value.customFunction);
        delete m_current->value.customFunction;
    }
    break;
    case StackElement::CustomMetaDestructor: {
        m_current->entry->setCustomDestructor(*m_current->value.customFunction);
        delete m_current->value.customFunction;
    }
    break;
    case StackElement::EnumTypeEntry:
        m_current->entry->setDocModification(m_contextStack.top()->docModifications);
        m_contextStack.top()->docModifications = DocModificationList();
        m_currentEnum = 0;
        break;
    case StackElement::Template:
        m_database->addTemplate(m_current->value.templateEntry);
        break;
    case StackElement::TemplateInstanceEnum:
        switch (m_current->parent->type) {
        case StackElement::InjectCode:
            if (m_current->parent->parent->type == StackElement::Root) {
                CodeSnipList snips = m_current->parent->entry->codeSnips();
                CodeSnip snip = snips.takeLast();
                snip.addTemplateInstance(m_current->value.templateInstance);
                snips.append(snip);
                m_current->parent->entry->setCodeSnips(snips);
                break;
            }
            Q_FALLTHROUGH();
        case StackElement::NativeToTarget:
        case StackElement::AddConversion:
            m_contextStack.top()->codeSnips.last().addTemplateInstance(m_current->value.templateInstance);
            break;
        case StackElement::Template:
            m_current->parent->value.templateEntry->addTemplateInstance(m_current->value.templateInstance);
            break;
        case StackElement::CustomMetaConstructor:
        case StackElement::CustomMetaDestructor:
            m_current->parent->value.customFunction->addTemplateInstance(m_current->value.templateInstance);
            break;
        case StackElement::ConversionRule:
            m_contextStack.top()->functionMods.last().argument_mods.last().conversion_rules.last().addTemplateInstance(m_current->value.templateInstance);
            break;
        case StackElement::InjectCodeInFunction:
            m_contextStack.top()->functionMods.last().snips.last().addTemplateInstance(m_current->value.templateInstance);
            break;
        default:
            break; // nada
        };
        break;
    default:
        break;
    }

    switch (m_current->type) {
    case StackElement::Root:
    case StackElement::NamespaceTypeEntry:
    case StackElement::InterfaceTypeEntry:
    case StackElement::ObjectTypeEntry:
    case StackElement::ValueTypeEntry:
    case StackElement::PrimitiveTypeEntry:
    case StackElement::TypedefTypeEntry:
        delete m_contextStack.pop();
        break;
    default:
        break;
    }

    StackElement *child = m_current;
    m_current = m_current->parent;
    delete(child);

    return true;
}

template <class String> // QString/QStringRef
bool Handler::characters(const String &ch)
{
    if (m_currentDroppedEntry || m_ignoreDepth)
        return true;

    if (m_current->type == StackElement::Template) {
        m_current->value.templateEntry->addCode(ch);
        return true;
    }

    if (m_current->type == StackElement::CustomMetaConstructor || m_current->type == StackElement::CustomMetaDestructor) {
        m_current->value.customFunction->addCode(ch);
        return true;
    }

    if (m_current->type == StackElement::ConversionRule
        && m_current->parent->type == StackElement::ModifyArgument) {
        m_contextStack.top()->functionMods.last().argument_mods.last().conversion_rules.last().addCode(ch);
        return true;
    }

    if (m_current->type == StackElement::NativeToTarget || m_current->type == StackElement::AddConversion) {
       m_contextStack.top()->codeSnips.last().addCode(ch);
       return true;
    }

    if (m_current->parent) {
        if ((m_current->type & StackElement::CodeSnipMask)) {
            CodeSnipList snips;
            switch (m_current->parent->type) {
            case StackElement::Root:
                snips = m_current->parent->entry->codeSnips();
                snips.last().addCode(ch);
                m_current->parent->entry->setCodeSnips(snips);
                break;
            case StackElement::ModifyFunction:
            case StackElement::AddFunction:
                m_contextStack.top()->functionMods.last().snips.last().addCode(ch);
                m_contextStack.top()->functionMods.last().modifiers |= FunctionModification::CodeInjection;
                break;
            case StackElement::NamespaceTypeEntry:
            case StackElement::ObjectTypeEntry:
            case StackElement::ValueTypeEntry:
            case StackElement::InterfaceTypeEntry:
                m_contextStack.top()->codeSnips.last().addCode(ch);
                break;
            default:
                Q_ASSERT(false);
            };
            return true;
        }
    }

    if (m_current->type & StackElement::DocumentationMask)
        m_contextStack.top()->docModifications.last().setCode(ch);

    return true;
}

bool Handler::importFileElement(const QXmlStreamAttributes &atts)
{
    const QString fileName = atts.value(nameAttribute()).toString();
    if (fileName.isEmpty()) {
        m_error = QLatin1String("Required attribute 'name' missing for include-file tag.");
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.setFileName(QLatin1String(":/trolltech/generator/") + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_error = QString::fromLatin1("Could not open file: '%1'").arg(QDir::toNativeSeparators(fileName));
            return false;
        }
    }

    const QStringRef quoteFrom = atts.value(quoteAfterLineAttribute());
    bool foundFromOk = quoteFrom.isEmpty();
    bool from = quoteFrom.isEmpty();

    const QStringRef quoteTo = atts.value(quoteBeforeLineAttribute());
    bool foundToOk = quoteTo.isEmpty();
    bool to = true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (from && to && line.contains(quoteTo)) {
            to = false;
            foundToOk = true;
            break;
        }
        if (from && to)
            characters(line + QLatin1Char('\n'));
        if (!from && line.contains(quoteFrom)) {
            from = true;
            foundFromOk = true;
        }
    }
    if (!foundFromOk || !foundToOk) {
        QString fromError = QStringLiteral("Could not find quote-after-line='%1' in file '%2'.")
                                           .arg(quoteFrom.toString(), fileName);
        QString toError = QStringLiteral("Could not find quote-before-line='%1' in file '%2'.")
                                         .arg(quoteTo.toString(), fileName);

        if (!foundToOk)
            m_error = toError;
        if (!foundFromOk)
            m_error = fromError;
        if (!foundFromOk && !foundToOk)
            m_error = fromError + QLatin1Char(' ') + toError;
        return false;
    }

    return true;
}

static bool convertBoolean(QStringView value, const QString &attributeName, bool defaultValue)
{
#ifdef QTBUG_69389_FIXED
    if (value.compare(trueAttributeValue(), Qt::CaseInsensitive) == 0
        || value.compare(yesAttributeValue(), Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (value.compare(falseAttributeValue(), Qt::CaseInsensitive) == 0
        || value.compare(noAttributeValue(), Qt::CaseInsensitive) == 0) {
        return false;
    }
#else
    if (QtPrivate::compareStrings(value, trueAttributeValue(), Qt::CaseInsensitive) == 0
        || QtPrivate::compareStrings(value, yesAttributeValue(), Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (QtPrivate::compareStrings(value, falseAttributeValue(), Qt::CaseInsensitive) == 0
        || QtPrivate::compareStrings(value, noAttributeValue(), Qt::CaseInsensitive) == 0) {
        return false;
    }
#endif
    const QString warn = QStringLiteral("Boolean value '%1' not supported in attribute '%2'. Use 'yes' or 'no'. Defaulting to '%3'.")
                                      .arg(value)
                                      .arg(attributeName,
                                           defaultValue ? yesAttributeValue() : noAttributeValue());

    qCWarning(lcShiboken).noquote().nospace() << warn;
    return defaultValue;
}

static bool convertRemovalAttribute(QStringView remove, Modification& mod, QString& errorMsg)
{
    if (remove.isEmpty())
        return true;
#ifdef QTBUG_69389_FIXED
    if (remove.compare(QStringViewLiteral("all"), Qt::CaseInsensitive) == 0) {
#else
    if (QtPrivate::compareStrings(remove, QStringViewLiteral("all"), Qt::CaseInsensitive) == 0) {
#endif
        mod.removal = TypeSystem::All;
        return true;
    }
#ifdef QTBUG_69389_FIXED
    if (remove.compare(QStringViewLiteral("target"), Qt::CaseInsensitive) == 0) {
#else
    if (QtPrivate::compareStrings(remove, QStringViewLiteral("target"), Qt::CaseInsensitive) == 0) {
#endif
        mod.removal = TypeSystem::TargetLangAndNativeCode;
        return true;
    }
    errorMsg = QString::fromLatin1("Bad removal type '%1'").arg(remove);
    return false;
}

static void getNamePrefixRecursive(StackElement* element, QStringList& names)
{
    if (!element->parent || !element->parent->entry)
        return;
    getNamePrefixRecursive(element->parent, names);
    names << element->parent->entry->name();
}

static QString getNamePrefix(StackElement* element)
{
    QStringList names;
    getNamePrefixRecursive(element, names);
    return names.join(QLatin1Char('.'));
}

// Returns empty string if there's no error.
static QString checkSignatureError(const QString& signature, const QString& tag)
{
    QString funcName = signature.left(signature.indexOf(QLatin1Char('('))).trimmed();
    static const QRegularExpression whiteSpace(QStringLiteral("\\s"));
    Q_ASSERT(whiteSpace.isValid());
    if (!funcName.startsWith(QLatin1String("operator ")) && funcName.contains(whiteSpace)) {
        return QString::fromLatin1("Error in <%1> tag signature attribute '%2'.\n"
                                   "White spaces aren't allowed in function names, "
                                   "and return types should not be part of the signature.")
                                   .arg(tag, signature);
    }
    return QString();
}

void Handler::applyCommonAttributes(TypeEntry *type, QXmlStreamAttributes *attributes) const
{
    type->setCodeGeneration(m_generate);
    const int revisionIndex =
        indexOfAttribute(*attributes, QStringViewLiteral("revision"));
    if (revisionIndex != -1)
        type->setRevision(attributes->takeAt(revisionIndex).value().toInt());
}

FlagsTypeEntry *
    Handler::parseFlagsEntry(const QXmlStreamReader &,
                             EnumTypeEntry *enumEntry,
                             const QString &name, QString flagName,
                             const QVersionNumber &since,
                             QXmlStreamAttributes *attributes)

{
    FlagsTypeEntry *ftype = new FlagsTypeEntry(QLatin1String("QFlags<") + name + QLatin1Char('>'), since);
    ftype->setOriginator(enumEntry);
    ftype->setTargetLangPackage(enumEntry->targetLangPackage());
    // Try to get the guess the qualified flag name
    const int lastSepPos = name.lastIndexOf(colonColon());
    if (lastSepPos >= 0 && !flagName.contains(colonColon()))
        flagName.prepend(name.left(lastSepPos + 2));

    ftype->setOriginalName(flagName);
    applyCommonAttributes(ftype, attributes);
    QString n = ftype->originalName();

    QStringList lst = n.split(colonColon());
    const QString &targetLangQualifier = enumEntry->targetLangQualifier();
    if (QStringList(lst.mid(0, lst.size() - 1)).join(colonColon()) != targetLangQualifier) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("enum %1 and flags %2 differ in qualifiers")
                              .arg(targetLangQualifier, lst.constFirst());
    }

    ftype->setFlagsName(lst.constLast());
    enumEntry->setFlags(ftype);

    m_database->addFlagsType(ftype);
    m_database->addType(ftype);

    const int revisionIndex =
        indexOfAttribute(*attributes, QStringViewLiteral("flags-revision"));
    ftype->setRevision(revisionIndex != -1
                       ? attributes->takeAt(revisionIndex).value().toInt()
                       : enumEntry->revision());
    return ftype;
}

SmartPointerTypeEntry *
    Handler::parseSmartPointerEntry(const QXmlStreamReader &,
                                    const QString &name, const QVersionNumber &since,
                                    QXmlStreamAttributes *attributes)
{
    QString smartPointerType;
    QString getter;
    QString refCountMethodName;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("type")) {
             smartPointerType = attributes->takeAt(i).value().toString();
        } else if (name == QLatin1String("getter")) {
            getter = attributes->takeAt(i).value().toString();
        } else if (name == QLatin1String("ref-count-method")) {
            refCountMethodName = attributes->takeAt(i).value().toString();
        }
    }

    if (smartPointerType.isEmpty()) {
        m_error = QLatin1String("No type specified for the smart pointer. Currently supported types: 'shared',");
        return nullptr;
    }
    if (smartPointerType != QLatin1String("shared")) {
        m_error = QLatin1String("Currently only the 'shared' type is supported.");
        return nullptr;
    }

    if (getter.isEmpty()) {
        m_error = QLatin1String("No function getter name specified for getting the raw pointer held by the smart pointer.");
        return nullptr;
    }

    QString signature = getter + QLatin1String("()");
    signature = TypeDatabase::normalizedSignature(signature);
    if (signature.isEmpty()) {
        m_error = QLatin1String("No signature for the smart pointer getter found.");
        return nullptr;
    }

    QString errorString = checkSignatureError(signature,
                                              QLatin1String("smart-pointer-type"));
    if (!errorString.isEmpty()) {
        m_error = errorString;
        return nullptr;
    }

    SmartPointerTypeEntry *type =
        new SmartPointerTypeEntry(name, getter, smartPointerType, refCountMethodName, since);
    applyCommonAttributes(type, attributes);
    return type;
}

PrimitiveTypeEntry *
    Handler::parsePrimitiveTypeEntry(const QXmlStreamReader &reader,
                                     const QString &name, const QVersionNumber &since,
                                     QXmlStreamAttributes *attributes)
{
    PrimitiveTypeEntry *type = new PrimitiveTypeEntry(name, since);
    applyCommonAttributes(type, attributes);
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("target-lang-name")) {
             type->setTargetLangName(attributes->takeAt(i).value().toString());
        } else if (name == QLatin1String("target-lang-api-name")) {
            type->setTargetLangApiName(attributes->takeAt(i).value().toString());
        } else if (name == preferredConversionAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == preferredTargetLangTypeAttribute()) {
            const bool v = convertBoolean(attributes->takeAt(i).value(),
                                          preferredTargetLangTypeAttribute(), true);
            type->setPreferredTargetLangType(v);
        } else if (name == QLatin1String("default-constructor")) {
             type->setDefaultConstructor(attributes->takeAt(i).value().toString());
        }
    }

    if (type->targetLangName().isEmpty())
        type->setTargetLangName(type->name());
    if (type->targetLangApiName().isEmpty())
        type->setTargetLangApiName(type->name());
    type->setTargetLangPackage(m_defaultPackage);
    return type;
}

ContainerTypeEntry *
    Handler::parseContainerTypeEntry(const QXmlStreamReader &,
                                     const QString &name, const QVersionNumber &since,
                                     QXmlStreamAttributes *attributes)
{
    const int typeIndex = indexOfAttribute(*attributes, QStringViewLiteral("type"));
    if (typeIndex == -1) {
        m_error = QLatin1String("no 'type' attribute specified");
        return nullptr;
    }
    const QStringRef typeName = attributes->takeAt(typeIndex).value();
    ContainerTypeEntry::Type containerType = containerTypeFromAttribute(typeName);
    if (containerType == ContainerTypeEntry::NoContainer) {
        m_error = QLatin1String("there is no container of type ") + typeName.toString();
        return nullptr;
    }
    ContainerTypeEntry *type = new ContainerTypeEntry(name, containerType, since);
    applyCommonAttributes(type, attributes);
    return type;
}

EnumTypeEntry *
    Handler::parseEnumTypeEntry(const QXmlStreamReader &reader,
                                const QString &fullName, const QVersionNumber &since,
                                QXmlStreamAttributes *attributes)
{
    QString scope;
    QString name = fullName;
    const int sep = fullName.lastIndexOf(colonColon());
    if (sep != -1) {
        scope = fullName.left(sep);
        name = fullName.right(fullName.size() - sep - 2);
    }
    EnumTypeEntry *entry = new EnumTypeEntry(scope, name, since);
    applyCommonAttributes(entry, attributes);
    entry->setTargetLangPackage(m_defaultPackage);

    QString flagNames;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("upper-bound")) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == QLatin1String("lower-bound")) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == forceIntegerAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == extensibleAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == flagsAttribute()) {
            flagNames = attributes->takeAt(i).value().toString();
        }
    }

    // put in the flags parallel...
    if (!flagNames.isEmpty()) {
        const QStringList &flagNameList = flagNames.split(QLatin1Char(','));
        for (const QString &flagName : flagNameList)
            parseFlagsEntry(reader, entry, fullName, flagName.trimmed(), since, attributes);
    }
    return entry;
}

ObjectTypeEntry *
    Handler::parseInterfaceTypeEntry(const QXmlStreamReader &,
                                     const QString &name, const QVersionNumber &since,
                                     QXmlStreamAttributes *attributes)
{
    ObjectTypeEntry *otype = new ObjectTypeEntry(name, since);
    applyCommonAttributes(otype, attributes);
    QString targetLangName = name;
    bool generate = true;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("target-lang-name")) {
            targetLangName = attributes->takeAt(i).value().toString();
        } else if (name == generateAttribute()) {
            generate = convertBoolean(attributes->takeAt(i).value(),
                                      generateAttribute(), true);
        }
    }

    InterfaceTypeEntry *itype =
        new InterfaceTypeEntry(InterfaceTypeEntry::interfaceName(targetLangName), since);

    if (generate)
        itype->setCodeGeneration(m_generate);
    else
        itype->setCodeGeneration(TypeEntry::GenerateForSubclass);

    otype->setDesignatedInterface(itype);
    itype->setOrigin(otype);
    return otype;
}

ValueTypeEntry *
    Handler::parseValueTypeEntry(const QXmlStreamReader &,
                                 const QString &name, const QVersionNumber &since,
                                 QXmlStreamAttributes *attributes)
{
    ValueTypeEntry *typeEntry = new ValueTypeEntry(name, since);
    applyCommonAttributes(typeEntry, attributes);
    const int defaultCtIndex =
        indexOfAttribute(*attributes, QStringViewLiteral("default-constructor"));
    if (defaultCtIndex != -1)
         typeEntry->setDefaultConstructor(attributes->takeAt(defaultCtIndex).value().toString());
    return typeEntry;
}

FunctionTypeEntry *
    Handler::parseFunctionTypeEntry(const QXmlStreamReader &,
                                    const QString &name, const QVersionNumber &since,
                                    QXmlStreamAttributes *attributes)
{
    const int signatureIndex = indexOfAttribute(*attributes, signatureAttribute());
    if (signatureIndex == -1) {
        m_error =  msgMissingAttribute(signatureAttribute());
        return nullptr;
    }
    const QString signature =
        TypeDatabase::normalizedSignature(attributes->takeAt(signatureIndex).value().toString());

    TypeEntry *existingType = m_database->findType(name);

    if (!existingType) {
        FunctionTypeEntry *result = new FunctionTypeEntry(name, signature, since);
        applyCommonAttributes(result, attributes);
        return result;
    }

    if (existingType->type() != TypeEntry::FunctionType) {
        m_error = QStringLiteral("%1 expected to be a function, but isn't! Maybe it was already declared as a class or something else.")
                 .arg(name);
        return nullptr;
    }

    FunctionTypeEntry *result = reinterpret_cast<FunctionTypeEntry *>(existingType);
    result->addSignature(signature);
    return result;
}

TypedefEntry *
 Handler::parseTypedefEntry(const QXmlStreamReader &, const QString &name,
                            const QVersionNumber &since,
                            QXmlStreamAttributes *attributes)
{
    if (m_current && m_current->type != StackElement::Root
        && m_current->type != StackElement::NamespaceTypeEntry) {
        m_error = QLatin1String("typedef entries must be nested in namespaces or type system.");
        return nullptr;
    }
    const int sourceIndex = indexOfAttribute(*attributes, sourceAttribute());
    if (sourceIndex == -1) {
        m_error =  msgMissingAttribute(sourceAttribute());
        return nullptr;
    }
    const QString sourceType = attributes->takeAt(sourceIndex).value().toString();
    auto result = new TypedefEntry(name, sourceType, since);
    applyCommonAttributes(result, attributes);
    return result;
}

void Handler::applyComplexTypeAttributes(const QXmlStreamReader &reader,
                                         ComplexTypeEntry *ctype,
                                         QXmlStreamAttributes *attributes) const
{
    bool generate = true;
    ctype->setCopyable(ComplexTypeEntry::Unknown);
    auto exceptionHandling = m_exceptionHandling;
    auto allowThread = m_allowThread;

    QString package = m_defaultPackage;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == streamAttribute()) {
            ctype->setStream(convertBoolean(attributes->takeAt(i).value(), streamAttribute(), false));
        } else if (name == generateAttribute()) {
            generate = convertBoolean(attributes->takeAt(i).value(), generateAttribute(), true);
        } else if (name ==packageAttribute()) {
            package = attributes->takeAt(i).value().toString();
        } else if (name == defaultSuperclassAttribute()) {
            ctype->setDefaultSuperclass(attributes->takeAt(i).value().toString());
        } else if (name == genericClassAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
            const bool v = convertBoolean(attributes->takeAt(i).value(), genericClassAttribute(), false);
            ctype->setGenericClass(v);
        } else if (name == QLatin1String("target-lang-name")) {
            ctype->setTargetLangName(attributes->takeAt(i).value().toString());
        } else if (name == QLatin1String("polymorphic-base")) {
            ctype->setPolymorphicIdValue(attributes->takeAt(i).value().toString());
        } else if (name == QLatin1String("polymorphic-id-expression")) {
            ctype->setPolymorphicIdValue(attributes->takeAt(i).value().toString());
        } else if (name == copyableAttribute()) {
            const bool v = convertBoolean(attributes->takeAt(i).value(), copyableAttribute(), false);
            ctype->setCopyable(v ? ComplexTypeEntry::CopyableSet : ComplexTypeEntry::NonCopyableSet);
        } else if (name == exceptionHandlingAttribute()) {
            const auto attribute = attributes->takeAt(i);
            const auto v = exceptionHandlingFromAttribute(attribute.value());
            if (v != TypeSystem::ExceptionHandling::Unspecified) {
                exceptionHandling = v;
            } else {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgInvalidAttributeValue(attribute)));
            }
        } else if (name == allowThreadAttribute()) {
            const auto attribute = attributes->takeAt(i);
            const auto v = allowThreadFromAttribute(attribute.value());
            if (v != TypeSystem::AllowThread::Unspecified) {
                allowThread = v;
            } else {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgInvalidAttributeValue(attribute)));
            }
        } else if (name == QLatin1String("held-type")) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == QLatin1String("hash-function")) {
            ctype->setHashFunction(attributes->takeAt(i).value().toString());
        } else if (name == forceAbstractAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == deprecatedAttribute()) {
            if (convertBoolean(attributes->takeAt(i).value(), deprecatedAttribute(), false))
                ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::Deprecated);
        } else if (name == deleteInMainThreadAttribute()) {
            if (convertBoolean(attributes->takeAt(i).value(), deleteInMainThreadAttribute(), false))
                ctype->setDeleteInMainThread(true);
        } else if (name == QLatin1String("target-type")) {
            ctype->setTargetType(attributes->takeAt(i).value().toString());
        }
    }

    if (exceptionHandling != TypeSystem::ExceptionHandling::Unspecified)
         ctype->setExceptionHandling(exceptionHandling);
    if (allowThread != TypeSystem::AllowThread::Unspecified)
        ctype->setAllowThread(allowThread);

    // The generator code relies on container's package being empty.
    if (ctype->type() != TypeEntry::ContainerType)
        ctype->setTargetLangPackage(package);

    if (InterfaceTypeEntry *di = ctype->designatedInterface())
        di->setTargetLangPackage(package);

    if (generate)
        ctype->setCodeGeneration(m_generate);
    else
        ctype->setCodeGeneration(TypeEntry::GenerateForSubclass);
}

bool Handler::parseRenameFunction(const QXmlStreamReader &,
                                  QString *name, QXmlStreamAttributes *attributes)
{
    QString signature;
    QString rename;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == signatureAttribute()) {
            // Do not remove as it is needed for the type entry later on
            signature = attributes->at(i).value().toString();
        } else if (name == renameAttribute()) {
            rename = attributes->takeAt(i).value().toString();
        }
    }

    if (signature.isEmpty()) {
        m_error = msgMissingAttribute(signatureAttribute());
        return false;
    }

    *name = signature.left(signature.indexOf(QLatin1Char('('))).trimmed();

    QString errorString = checkSignatureError(signature, QLatin1String("function"));
    if (!errorString.isEmpty()) {
        m_error = errorString;
        return false;
    }

    if (!rename.isEmpty()) {
        static const QRegularExpression functionNameRegExp(QLatin1String("^[a-zA-Z_][a-zA-Z0-9_]*$"));
        Q_ASSERT(functionNameRegExp.isValid());
        if (!functionNameRegExp.match(rename).hasMatch()) {
            m_error = QLatin1String("can not rename '") + signature + QLatin1String("', '")
                      + rename + QLatin1String("' is not a valid function name");
            return false;
        }
        FunctionModification mod;
        if (!mod.setSignature(signature, &m_error))
            return false;
        mod.renamedToName = rename;
        mod.modifiers |= Modification::Rename;
        m_contextStack.top()->functionMods << mod;
    }
    return true;
}

bool Handler::parseInjectDocumentation(const QXmlStreamReader &,
                                       QXmlStreamAttributes *attributes)
{
    const int validParent = StackElement::TypeEntryMask
                            | StackElement::ModifyFunction
                            | StackElement::ModifyField;
    if (!m_current->parent || (m_current->parent->type & validParent) == 0) {
        m_error = QLatin1String("inject-documentation must be inside modify-function, "
                                "modify-field or other tags that creates a type");
        return false;
    }

    TypeSystem::DocModificationMode mode = TypeSystem::DocModificationReplace;
    TypeSystem::Language lang = TypeSystem::NativeCode;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("mode")) {
            const QStringRef modeName = attributes->takeAt(i).value();
            mode = docModificationFromAttribute(modeName);
            if (mode == TypeSystem::DocModificationInvalid) {
                m_error = QLatin1String("Unknown documentation injection mode: ") + modeName;
                return false;
            }
        } else if (name == formatAttribute()) {
            const QStringRef format = attributes->takeAt(i).value();
            lang = languageFromAttribute(format);
            if (lang != TypeSystem::TargetLangCode && lang != TypeSystem::NativeCode) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(format);
                return false;
            }
        }
    }

    QString signature = m_current->type & StackElement::TypeEntryMask
        ? QString() : m_currentSignature;
    DocModification mod(mode, signature);
    mod.setFormat(lang);
    m_contextStack.top()->docModifications << mod;
    return true;
}

bool Handler::parseModifyDocumentation(const QXmlStreamReader &,
                                       QXmlStreamAttributes *attributes)
{
    const int validParent = StackElement::TypeEntryMask
                            | StackElement::ModifyFunction
                            | StackElement::ModifyField;
    if (!m_current->parent || (m_current->parent->type & validParent) == 0) {
        m_error = QLatin1String("modify-documentation must be inside modify-function, "
                                "modify-field or other tags that creates a type");
        return false;
    }

    const int xpathIndex = indexOfAttribute(*attributes, xPathAttribute());
    if (xpathIndex == -1) {
        m_error = msgMissingAttribute(xPathAttribute());
        return false;
    }

    const QString xpath = attributes->takeAt(xpathIndex).value().toString();
    QString signature = (m_current->type & StackElement::TypeEntryMask) ? QString() : m_currentSignature;
    m_contextStack.top()->docModifications
        << DocModification(xpath, signature);
    return true;
}

// m_exceptionHandling
TypeSystemTypeEntry *Handler::parseRootElement(const QXmlStreamReader &,
                                               const QVersionNumber &since,
                                               QXmlStreamAttributes *attributes)
{
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == packageAttribute()) {
            m_defaultPackage = attributes->takeAt(i).value().toString();
        } else if (name == defaultSuperclassAttribute()) {
            m_defaultSuperclass = attributes->takeAt(i).value().toString();
        } else if (name == exceptionHandlingAttribute()) {
            const auto attribute = attributes->takeAt(i);
            const auto v = exceptionHandlingFromAttribute(attribute.value());
            if (v != TypeSystem::ExceptionHandling::Unspecified) {
                m_exceptionHandling = v;
            } else {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgInvalidAttributeValue(attribute)));
            }
        } else if (name == allowThreadAttribute()) {
            const auto attribute = attributes->takeAt(i);
            const auto v = allowThreadFromAttribute(attribute.value());
            if (v != TypeSystem::AllowThread::Unspecified) {
                m_allowThread = v;
            } else {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgInvalidAttributeValue(attribute)));
            }
        }
    }

    TypeSystemTypeEntry *moduleEntry =
        const_cast<TypeSystemTypeEntry *>(m_database->findTypeSystemType(m_defaultPackage));
    const bool add = moduleEntry == nullptr;
    if (add)
        moduleEntry = new TypeSystemTypeEntry(m_defaultPackage, since);
    moduleEntry->setCodeGeneration(m_generate);

    if ((m_generate == TypeEntry::GenerateForSubclass ||
         m_generate == TypeEntry::GenerateNothing) && !m_defaultPackage.isEmpty())
        TypeDatabase::instance()->addRequiredTargetImport(m_defaultPackage);

    if (add)
        m_database->addTypeSystemType(moduleEntry);
    return moduleEntry;
}

bool Handler::loadTypesystem(const QXmlStreamReader &,
                             QXmlStreamAttributes *attributes)
{
    QString typeSystemName;
    bool generateChild = true;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == nameAttribute())
            typeSystemName = attributes->takeAt(i).value().toString();
         else if (name == generateAttribute())
            generateChild = convertBoolean(attributes->takeAt(i).value(), generateAttribute(), true);
    }
    if (typeSystemName.isEmpty()) {
            m_error = QLatin1String("No typesystem name specified");
            return false;
    }
    const bool result =
        m_database->parseFile(typeSystemName, m_currentPath, generateChild
                              && m_generate == TypeEntry::GenerateAll);
    if (!result)
        m_error = QStringLiteral("Failed to parse: '%1'").arg(typeSystemName);
    return result;
}

bool Handler::parseRejectEnumValue(const QXmlStreamReader &,
                                   QXmlStreamAttributes *attributes)
{
    if (!m_currentEnum) {
        m_error = QLatin1String("<reject-enum-value> node must be used inside a <enum-type> node");
        return false;
    }
    const int nameIndex = indexOfAttribute(*attributes, nameAttribute());
    if (nameIndex == -1) {
        m_error = msgMissingAttribute(nameAttribute());
        return false;
    }
    m_currentEnum->addEnumValueRejection(attributes->takeAt(nameIndex).value().toString());
    return true;
}

bool Handler::parseReplaceArgumentType(const QXmlStreamReader &,
                                       const StackElement &topElement,
                                       QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("Type replacement can only be specified for argument modifications");
        return false;
    }
    const int modifiedTypeIndex = indexOfAttribute(*attributes, modifiedTypeAttribute());
    if (modifiedTypeIndex == -1) {
        m_error = QLatin1String("Type replacement requires 'modified-type' attribute");
        return false;
    }
    m_contextStack.top()->functionMods.last().argument_mods.last().modified_type =
        attributes->takeAt(modifiedTypeIndex).value().toString();
    return true;
}

bool Handler::parseCustomConversion(const QXmlStreamReader &,
                                    const StackElement &topElement,
                                    QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument
        && topElement.type != StackElement::ValueTypeEntry
        && topElement.type != StackElement::PrimitiveTypeEntry
        && topElement.type != StackElement::ContainerTypeEntry) {
        m_error = QLatin1String("Conversion rules can only be specified for argument modification, "
                                "value-type, primitive-type or container-type conversion.");
        return false;
    }

    QString sourceFile;
    QString snippetLabel;
    TypeSystem::Language lang = TypeSystem::NativeCode;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == classAttribute()) {
            const QStringRef languageAttribute = attributes->takeAt(i).value();
            lang = languageFromAttribute(languageAttribute);
            if (lang != TypeSystem::TargetLangCode && lang != TypeSystem::NativeCode) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(languageAttribute);
                return false;
            }
        } else if (name == QLatin1String("file")) {
            sourceFile = attributes->takeAt(i).value().toString();
        } else if (name == snippetAttribute()) {
            snippetLabel = attributes->takeAt(i).value().toString();
        }
    }

    if (topElement.type == StackElement::ModifyArgument) {
        CodeSnip snip;
        snip.language = lang;
        m_contextStack.top()->functionMods.last().argument_mods.last().conversion_rules.append(snip);
        return true;
    }

    if (topElement.entry->hasConversionRule() || topElement.entry->hasCustomConversion()) {
        m_error = QLatin1String("Types can have only one conversion rule");
        return false;
    }

    // The old conversion rule tag that uses a file containing the conversion
    // will be kept temporarily for compatibility reasons.
    if (!sourceFile.isEmpty()) {
        if (m_generate != TypeEntry::GenerateForSubclass
                && m_generate != TypeEntry::GenerateNothing) {

            const char* conversionFlag = NATIVE_CONVERSION_RULE_FLAG;
            if (lang == TypeSystem::TargetLangCode)
                conversionFlag = TARGET_CONVERSION_RULE_FLAG;

            QFile conversionSource(sourceFile);
            if (conversionSource.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString conversionRule =
                    extractSnippet(QString::fromUtf8(conversionSource.readAll()), snippetLabel);
                topElement.entry->setConversionRule(QLatin1String(conversionFlag) + conversionRule);
            } else {
                qCWarning(lcShiboken).noquote().nospace()
                    << "File containing conversion code for "
                    << topElement.entry->name() << " type does not exist or is not readable: "
                    << sourceFile;
            }
        }
    }

    CustomConversion* customConversion = new CustomConversion(m_current->entry);
    customConversionsForReview.append(customConversion);
    return true;
}

bool Handler::parseNativeToTarget(const QXmlStreamReader &,
                                  const StackElement &topElement,
                                  QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ConversionRule) {
        m_error = QLatin1String("Native to Target conversion code can only be specified for custom conversion rules.");
        return false;
    }
    CodeSnip snip;
    if (!readFileSnippet(attributes, &snip))
        return false;
    m_contextStack.top()->codeSnips.append(snip);
    return true;
}

bool Handler::parseAddConversion(const QXmlStreamReader &,
                                 const StackElement &topElement,
                                 QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::TargetToNative) {
        m_error = QLatin1String("Target to Native conversions can only be added inside 'target-to-native' tags.");
        return false;
    }
    QString sourceTypeName;
    QString typeCheck;
    CodeSnip snip;
    if (!readFileSnippet(attributes, &snip))
        return false;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("type"))
             sourceTypeName = attributes->takeAt(i).value().toString();
         else if (name == QLatin1String("check"))
            typeCheck = attributes->takeAt(i).value().toString();
    }
    if (sourceTypeName.isEmpty()) {
        m_error = QLatin1String("Target to Native conversions must specify the input type with the 'type' attribute.");
        return false;
    }
    m_current->entry->customConversion()->addTargetToNativeConversion(sourceTypeName, typeCheck);
    m_contextStack.top()->codeSnips.append(snip);
    return true;
}

static bool parseIndex(const QString &index, int *result, QString *errorMessage)
{
    bool ok = false;
    *result = index.toInt(&ok);
    if (!ok)
        *errorMessage = QStringLiteral("Cannot convert '%1' to integer").arg(index);
    return ok;
}

static bool parseArgumentIndex(const QString &index, int *result, QString *errorMessage)
{
    if (index == QLatin1String("return")) {
        *result = 0;
        return true;
    }
    if (index == QLatin1String("this")) {
        *result = -1;
        return true;
    }
    return parseIndex(index, result, errorMessage);
}

bool Handler::parseModifyArgument(const QXmlStreamReader &,
                                  const StackElement &topElement, QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyFunction
            && topElement.type != StackElement::AddFunction) {
        m_error = QString::fromLatin1("argument modification requires function"
                                      " modification as parent, was %1")
                                      .arg(topElement.type, 0, 16);
        return false;
    }

    QString index;
    QString replaceValue;
    bool resetAfterUse = false;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == indexAttribute()) {
             index = attributes->takeAt(i).value().toString();
        } else if (name == QLatin1String("replace-value")) {
            replaceValue = attributes->takeAt(i).value().toString();
        } else if (name == invalidateAfterUseAttribute()) {
            resetAfterUse = convertBoolean(attributes->takeAt(i).value(),
                                           invalidateAfterUseAttribute(), false);
        }
    }

    if (index.isEmpty()) {
        m_error = msgMissingAttribute(indexAttribute());
        return false;
    }

    int idx;
    if (!parseArgumentIndex(index, &idx, &m_error))
        return false;

    if (!replaceValue.isEmpty() && idx) {
        m_error = QLatin1String("replace-value is only supported for return values (index=0).");
        return false;
    }

    ArgumentModification argumentModification = ArgumentModification(idx);
    argumentModification.replace_value = replaceValue;
    argumentModification.resetAfterUse = resetAfterUse;
    m_contextStack.top()->functionMods.last().argument_mods.append(argumentModification);
    return true;
}

bool Handler::parseNoNullPointer(const QXmlStreamReader &reader,
                                 const StackElement &topElement, QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("no-null-pointer requires argument modification as parent");
        return false;
    }

    ArgumentModification &lastArgMod = m_contextStack.top()->functionMods.last().argument_mods.last();
    lastArgMod.noNullPointers = true;

    const int defaultValueIndex =
        indexOfAttribute(*attributes, QStringViewLiteral("default-value"));
    if (defaultValueIndex != -1) {
        const QXmlStreamAttribute attribute = attributes->takeAt(defaultValueIndex);
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgUnimplementedAttributeWarning(reader, attribute)));
    }
    return true;
}

bool Handler::parseDefineOwnership(const QXmlStreamReader &,
                                   const StackElement &topElement,
                                   QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("define-ownership requires argument modification as parent");
        return false;
    }

    TypeSystem::Language lang = TypeSystem::TargetLangCode;
    QString ownership;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == classAttribute()) {
            const QStringRef className = attributes->takeAt(i).value();
            lang = languageFromAttribute(className);
            if (lang != TypeSystem::TargetLangCode && lang != TypeSystem::NativeCode) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(className);
                return false;
            }
        } else if (name == ownershipAttribute()) {
            ownership = attributes->takeAt(i).value().toString();
        }
    }
    const TypeSystem::Ownership owner = ownershipFromFromAttribute(ownership);
    if (owner == TypeSystem::InvalidOwnership) {
        m_error = QStringLiteral("unsupported owner attribute: '%1'").arg(ownership);
        return false;
    }
    m_contextStack.top()->functionMods.last().argument_mods.last().ownerships[lang] = owner;
    return true;
}

bool Handler::parseArgumentMap(const QXmlStreamReader &,
                               const StackElement &topElement,
                               QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & StackElement::CodeSnipMask)) {
        m_error = QLatin1String("Argument maps requires code injection as parent");
        return false;
    }

    int pos = 1;
    QString metaName;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == indexAttribute()) {
            if (!parseIndex(attributes->takeAt(i).value().toString(), &pos, &m_error))
                return false;
            if (pos <= 0) {
                m_error = QStringLiteral("Argument position %1 must be a positive number").arg(pos);
                return false;
            }
        } else if (name == QLatin1String("meta-name")) {
            metaName = attributes->takeAt(i).value().toString();
        }
    }

    if (metaName.isEmpty())
        qCWarning(lcShiboken) << "Empty meta name in argument map";

    if (topElement.type == StackElement::InjectCodeInFunction) {
        m_contextStack.top()->functionMods.last().snips.last().argumentMap[pos] = metaName;
    } else {
        qCWarning(lcShiboken) << "Argument maps are only useful for injection of code "
                                 "into functions.";
    }
    return true;
}

bool Handler::parseRemoval(const QXmlStreamReader &,
                           const StackElement &topElement,
                           QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyFunction) {
        m_error = QLatin1String("Function modification parent required");
        return false;
    }

    TypeSystem::Language lang = TypeSystem::All;
    const int classIndex = indexOfAttribute(*attributes, classAttribute());
    if (classIndex != -1) {
        const QStringRef value = attributes->takeAt(classIndex).value();
        lang = languageFromAttribute(value);
        if (lang == TypeSystem::TargetLangCode) // "target" means TargetLangAndNativeCode here
            lang = TypeSystem::TargetLangAndNativeCode;
        if (lang != TypeSystem::TargetLangAndNativeCode && lang != TypeSystem::All) {
            m_error = QStringLiteral("unsupported class attribute: '%1'").arg(value);
            return false;
        }
    }
    m_contextStack.top()->functionMods.last().removal = lang;
    return true;
}

bool Handler::parseRename(const QXmlStreamReader &reader,
                          StackElement::ElementType type,
                          const StackElement &topElement,
                          QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyField
        && topElement.type != StackElement::ModifyFunction
        && topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("Function, field  or argument modification parent required");
        return false;
    }

    Modification *mod = nullptr;
    if (topElement.type == StackElement::ModifyFunction)
        mod = &m_contextStack.top()->functionMods.last();
    else if (topElement.type == StackElement::ModifyField)
        mod = &m_contextStack.top()->fieldMods.last();

    Modification::Modifiers modifierFlag = Modification::Rename;
    if (type == StackElement::Rename) {
        const int toIndex = indexOfAttribute(*attributes, toAttribute());
        if (toIndex == -1) {
            m_error = msgMissingAttribute(toAttribute());
            return false;
        }
        const QString renamed_to = attributes->takeAt(toIndex).value().toString();
        if (topElement.type == StackElement::ModifyFunction)
            mod->setRenamedTo(renamed_to);
        else if (topElement.type == StackElement::ModifyField)
            mod->setRenamedTo(renamed_to);
        else
            m_contextStack.top()->functionMods.last().argument_mods.last().renamed_to = renamed_to;
    } else {
        const int modifierIndex = indexOfAttribute(*attributes, modifierAttribute());
        if (modifierIndex == -1) {
            m_error = msgMissingAttribute(modifierAttribute());
            return false;
        }
        const QStringRef modifier = attributes->takeAt(modifierIndex).value();
        modifierFlag = modifierFromAttribute(modifier);
        if (modifierFlag == Modification::InvalidModifier) {
            m_error = QStringLiteral("Unknown access modifier: '%1'").arg(modifier);
            return false;
        }
        if (modifierFlag == Modification::Friendly) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeValueWarning(reader, modifierAttribute(), modifier)));
        }
    }

    if (mod)
        mod->modifiers |= modifierFlag;
    return true;
}

bool Handler::parseModifyField(const QXmlStreamReader &reader,
                               QXmlStreamAttributes *attributes)
{
    FieldModification fm;
    fm.modifiers = FieldModification::Readable | FieldModification::Writable;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == nameAttribute()) {
            fm.name = attributes->takeAt(i).value().toString();
        } else if (name == removeAttribute()) {
            if (!convertRemovalAttribute(attributes->takeAt(i).value(), fm, m_error))
                return false;
        }  else if (name == readAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
            if (!convertBoolean(attributes->takeAt(i).value(), readAttribute(), true))
                fm.modifiers &= ~FieldModification::Readable;
        } else if (name == writeAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
            if (!convertBoolean(attributes->takeAt(i).value(), writeAttribute(), true))
                fm.modifiers &= ~FieldModification::Writable;
        }
    }
    if (fm.name.isEmpty()) {
        m_error = msgMissingAttribute(nameAttribute());
        return false;
    }
    m_contextStack.top()->fieldMods << fm;
    return true;
}

bool Handler::parseAddFunction(const QXmlStreamReader &,
                               const StackElement &topElement,
                               QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & (StackElement::ComplexTypeEntryMask | StackElement::Root))) {
        m_error = QString::fromLatin1("Add function requires a complex type or a root tag as parent"
                                      ", was=%1").arg(topElement.type, 0, 16);
        return false;
    }
    QString originalSignature;
    QString returnType = QLatin1String("void");
    bool staticFunction = false;
    QString access;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("signature")) {
            originalSignature = attributes->takeAt(i).value().toString();
        } else if (name == QLatin1String("return-type")) {
            returnType = attributes->takeAt(i).value().toString();
        } else if (name == staticAttribute()) {
            staticFunction = convertBoolean(attributes->takeAt(i).value(),
                                            staticAttribute(), false);
        } else if (name == accessAttribute()) {
            access = attributes->takeAt(i).value().toString();
        }
    }

    QString signature = TypeDatabase::normalizedSignature(originalSignature);
    if (signature.isEmpty()) {
        m_error = QLatin1String("No signature for the added function");
        return false;
    }

    QString errorString = checkSignatureError(signature, QLatin1String("add-function"));
    if (!errorString.isEmpty()) {
        m_error = errorString;
        return false;
    }

    AddedFunctionPtr func(new AddedFunction(signature, returnType));
    func->setStatic(staticFunction);
    if (!signature.contains(QLatin1Char('(')))
        signature += QLatin1String("()");
    m_currentSignature = signature;

    if (!access.isEmpty()) {
        const AddedFunction::Access a = addedFunctionAccessFromAttribute(access);
        if (a == AddedFunction::InvalidAccess) {
            m_error = QString::fromLatin1("Bad access type '%1'").arg(access);
            return false;
        }
        func->setAccess(a);
    }

    m_contextStack.top()->addedFunctions << func;
    m_contextStack.top()->addedFunctionModificationIndex =
        m_contextStack.top()->functionMods.size();

    FunctionModification mod;
    if (!mod.setSignature(m_currentSignature, &m_error))
        return false;
    mod.setOriginalSignature(originalSignature);
    m_contextStack.top()->functionMods << mod;
    return true;
}

bool Handler::parseModifyFunction(const QXmlStreamReader &reader,
                                  const StackElement &topElement,
                                  QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & StackElement::ComplexTypeEntryMask)) {
        m_error = QString::fromLatin1("Modify function requires complex type as parent"
                                      ", was=%1").arg(topElement.type, 0, 16);
        return false;
    }

    QString originalSignature;
    QString access;
    QString removal;
    QString rename;
    QString association;
    bool deprecated = false;
    bool isThread = false;
    TypeSystem::ExceptionHandling exceptionHandling = TypeSystem::ExceptionHandling::Unspecified;
    TypeSystem::AllowThread allowThread = TypeSystem::AllowThread::Unspecified;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("signature")) {
            originalSignature = attributes->takeAt(i).value().toString();
        } else if (name == accessAttribute()) {
            access = attributes->takeAt(i).value().toString();
        } else if (name == renameAttribute()) {
            rename = attributes->takeAt(i).value().toString();
        } else if (name == QLatin1String("associated-to")) {
            association = attributes->takeAt(i).value().toString();
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        } else if (name == removeAttribute()) {
            removal = attributes->takeAt(i).value().toString();
        } else if (name == deprecatedAttribute()) {
            deprecated = convertBoolean(attributes->takeAt(i).value(),
                                        deprecatedAttribute(), false);
        } else if (name == threadAttribute()) {
            isThread = convertBoolean(attributes->takeAt(i).value(),
                                      threadAttribute(), false);
        } else if (name == allowThreadAttribute()) {
            const QXmlStreamAttribute attribute = attributes->takeAt(i);
            allowThread = allowThreadFromAttribute(attribute.value());
            if (allowThread == TypeSystem::AllowThread::Unspecified) {
                m_error = msgInvalidAttributeValue(attribute);
                return false;
            }
        } else if (name == exceptionHandlingAttribute()) {
            const auto attribute = attributes->takeAt(i);
            exceptionHandling = exceptionHandlingFromAttribute(attribute.value());
            if (exceptionHandling == TypeSystem::ExceptionHandling::Unspecified) {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgInvalidAttributeValue(attribute)));
            }
        } else if (name == virtualSlotAttribute()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeWarning(reader, name)));
        }
    }

    const QString signature = TypeDatabase::normalizedSignature(originalSignature);
    if (signature.isEmpty()) {
        m_error = QLatin1String("No signature for modified function");
        return false;
    }

    QString errorString = checkSignatureError(signature, QLatin1String("modify-function"));
    if (!errorString.isEmpty()) {
        m_error = errorString;
        return false;
    }

    FunctionModification mod;
    if (!mod.setSignature(signature, &m_error))
        return false;
    mod.setOriginalSignature(originalSignature);
    mod.setExceptionHandling(exceptionHandling);
    m_currentSignature = signature;

    if (!access.isEmpty()) {
        const Modification::Modifiers m = modifierFromAttribute(access);
        if ((m & (Modification::AccessModifierMask | Modification::FinalMask)) == 0) {
            m_error = QString::fromLatin1("Bad access type '%1'").arg(access);
            return false;
        }
        if (m == Modification::Final || m == Modification::NonFinal) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedAttributeValueWarning(reader,
                      accessAttribute(), access)));
        }
        mod.modifiers |= m;
    }

    if (deprecated)
        mod.modifiers |= Modification::Deprecated;

    if (!removal.isEmpty() && !convertRemovalAttribute(removal, mod, m_error))
        return false;

    if (!rename.isEmpty()) {
        mod.renamedToName = rename;
        mod.modifiers |= Modification::Rename;
    }

    if (!association.isEmpty())
        mod.association = association;

    mod.setIsThread(isThread);
    if (allowThread != TypeSystem::AllowThread::Unspecified)
        mod.setAllowThread(allowThread);

    m_contextStack.top()->functionMods << mod;
    return true;
}

bool Handler::parseReplaceDefaultExpression(const QXmlStreamReader &,
                                            const StackElement &topElement,
                                            QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & StackElement::ModifyArgument)) {
        m_error = QLatin1String("Replace default expression only allowed as child of argument modification");
        return false;
    }
    const int withIndex = indexOfAttribute(*attributes, QStringViewLiteral("with"));
    if (withIndex == -1 || attributes->at(withIndex).value().isEmpty()) {
        m_error = QLatin1String("Default expression replaced with empty string. Use remove-default-expression instead.");
        return false;
    }

    m_contextStack.top()->functionMods.last().argument_mods.last().replacedDefaultExpression =
        attributes->takeAt(withIndex).value().toString();
    return true;
}

CustomFunction *
    Handler::parseCustomMetaConstructor(const QXmlStreamReader &,
                                        StackElement::ElementType type,
                                        const StackElement &topElement,
                                        QXmlStreamAttributes *attributes)
{
    QString functionName = topElement.entry->name().toLower()
        + (type == StackElement::CustomMetaConstructor
           ? QLatin1String("_create") : QLatin1String("_delete"));
    QString paramName = QLatin1String("copy");
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == nameAttribute())
            functionName = attributes->takeAt(i).value().toString();
        else if (name == QLatin1String("param-name"))
            paramName = attributes->takeAt(i).value().toString();
    }
    CustomFunction *func = new CustomFunction(functionName);
    func->paramName = paramName;
    return func;
}

bool Handler::parseReferenceCount(const QXmlStreamReader &reader,
                                  const StackElement &topElement,
                                  QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("reference-count must be child of modify-argument");
        return false;
    }

    ReferenceCount rc;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == actionAttribute()) {
            const QXmlStreamAttribute attribute = attributes->takeAt(i);
            rc.action = referenceCountFromAttribute(attribute.value());
            switch (rc.action) {
            case ReferenceCount::Invalid:
                m_error = QLatin1String("unrecognized value '") + attribute.value()
                          + QLatin1String("' for action attribute.");
                return false;
            case ReferenceCount::AddAll:
            case ReferenceCount::Ignore:
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgUnimplementedAttributeValueWarning(reader, attribute)));
                break;
            default:
                break;
            }
        } else if (name == QLatin1String("variable-name")) {
            rc.varName = attributes->takeAt(i).value().toString();
        }
    }

    m_contextStack.top()->functionMods.last().argument_mods.last().referenceCounts.append(rc);
    return true;
}

bool Handler::parseParentOwner(const QXmlStreamReader &,
                               const StackElement &topElement,
                               QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::ModifyArgument) {
        m_error = QLatin1String("parent-policy must be child of modify-argument");
        return false;
    }
    ArgumentOwner ao;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == indexAttribute()) {
            const QString index = attributes->takeAt(i).value().toString();
            if (!parseArgumentIndex(index, &ao.index, &m_error))
                return false;
        } else if (name == actionAttribute()) {
            const QStringRef action = attributes->takeAt(i).value();
            ao.action = argumentOwnerActionFromAttribute(action);
            if (ao.action == ArgumentOwner::Invalid) {
                m_error = QLatin1String("Invalid parent actionr '") + action + QLatin1String("'.");
                return false;
            }
        }
    }
    m_contextStack.top()->functionMods.last().argument_mods.last().owner = ao;
    return true;
}

bool Handler::readFileSnippet(QXmlStreamAttributes *attributes, CodeSnip *snip)
{
    QString fileName;
    QString snippetLabel;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("file")) {
            fileName = attributes->takeAt(i).value().toString();
        } else if (name == snippetAttribute()) {
            snippetLabel = attributes->takeAt(i).value().toString();
        }
    }
    if (fileName.isEmpty())
        return true;
    const QString resolved = m_database->modifiedTypesystemFilepath(fileName, m_currentPath);
    if (!QFile::exists(resolved)) {
        m_error = QLatin1String("File for inject code not exist: ")
            + QDir::toNativeSeparators(fileName);
        return false;
    }
    QFile codeFile(resolved);
    if (!codeFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        m_error = msgCannotOpenForReading(codeFile);
        return false;
    }
    QString source = fileName;
    if (!snippetLabel.isEmpty())
        source += QLatin1String(" (") + snippetLabel + QLatin1Char(')');
    QString content;
    QTextStream str(&content);
    str << "// ========================================================================\n"
           "// START of custom code block [file: "
        << source << "]\n"
        << extractSnippet(QString::fromUtf8(codeFile.readAll()), snippetLabel)
        << "\n// END of custom code block [file: " << source
        << "]\n// ========================================================================\n";
    snip->addCode(content);
    return true;
}

bool Handler::parseInjectCode(const QXmlStreamReader &,
                              const StackElement &topElement,
                              StackElement* element, QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & StackElement::ComplexTypeEntryMask)
        && (topElement.type != StackElement::AddFunction)
        && (topElement.type != StackElement::ModifyFunction)
        && (topElement.type != StackElement::Root)) {
        m_error = QLatin1String("wrong parent type for code injection");
        return false;
    }

    TypeSystem::CodeSnipPosition position = TypeSystem::CodeSnipPositionBeginning;
    TypeSystem::Language lang = TypeSystem::TargetLangCode;
    CodeSnip snip;
    if (!readFileSnippet(attributes, &snip))
        return false;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == classAttribute()) {
            const QStringRef className = attributes->takeAt(i).value();
            lang = languageFromAttribute(className);
            if (lang == TypeSystem::NoLanguage) {
                m_error = QStringLiteral("Invalid class specifier: '%1'").arg(className);
                return false;
            }
        } else if (name == positionAttribute()) {
            const QStringRef value = attributes->takeAt(i).value();
            position = codeSnipPositionFromAttribute(value);
            if (position == TypeSystem::CodeSnipPositionInvalid) {
                m_error = QStringLiteral("Invalid position: '%1'").arg(value);
                return false;
            }
        }
    }

    snip.position = position;
    snip.language = lang;

    if (snip.language == TypeSystem::Interface
        && topElement.type != StackElement::InterfaceTypeEntry) {
        m_error = QLatin1String("Interface code injections must be direct child of an interface type entry");
        return false;
    }

    if (topElement.type == StackElement::ModifyFunction
        || topElement.type == StackElement::AddFunction) {
        if (snip.language == TypeSystem::ShellDeclaration) {
            m_error = QLatin1String("no function implementation in shell declaration in which to inject code");
            return false;
        }

        FunctionModification &mod = m_contextStack.top()->functionMods.last();
        mod.snips << snip;
        if (!snip.code().isEmpty())
            mod.modifiers |= FunctionModification::CodeInjection;
        element->type = StackElement::InjectCodeInFunction;
    } else if (topElement.type == StackElement::Root) {
        element->entry->addCodeSnip(snip);
    } else if (topElement.type != StackElement::Root) {
        m_contextStack.top()->codeSnips << snip;
    }
    return true;
}

bool Handler::parseInclude(const QXmlStreamReader &,
                           const StackElement &topElement,
                           TypeEntry *entry, QXmlStreamAttributes *attributes)
{
    QString fileName;
    QString location;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("file-name"))
            fileName = attributes->takeAt(i).value().toString();
        else if (name == locationAttribute())
            location = attributes->takeAt(i).value().toString();
    }
    const Include::IncludeType loc = locationFromAttribute(location);
    if (loc == Include::InvalidInclude) {
        m_error = QStringLiteral("Location not recognized: '%1'").arg(location);
        return false;
    }

    Include inc(loc, fileName);
    if (topElement.type
        & (StackElement::ComplexTypeEntryMask | StackElement::PrimitiveTypeEntry)) {
        entry->setInclude(inc);
    } else if (topElement.type == StackElement::ExtraIncludes) {
        entry->addExtraInclude(inc);
    } else {
        m_error = QLatin1String("Only supported parent tags are primitive-type, complex types or extra-includes");
        return false;
    }
    if (InterfaceTypeEntry *di = entry->designatedInterface()) {
        di->setInclude(entry->include());
        di->setExtraIncludes(entry->extraIncludes());
    }
    return true;
}

TemplateInstance *
    Handler::parseTemplateInstanceEnum(const QXmlStreamReader &,
                                       const StackElement &topElement,
                                       QXmlStreamAttributes *attributes)
{
    if (!(topElement.type & StackElement::CodeSnipMask) &&
        (topElement.type != StackElement::Template) &&
        (topElement.type != StackElement::CustomMetaConstructor) &&
        (topElement.type != StackElement::CustomMetaDestructor) &&
        (topElement.type != StackElement::NativeToTarget) &&
        (topElement.type != StackElement::AddConversion) &&
        (topElement.type != StackElement::ConversionRule)) {
        m_error = QLatin1String("Can only insert templates into code snippets, templates, custom-constructors, "\
                                "custom-destructors, conversion-rule, native-to-target or add-conversion tags.");
        return nullptr;
    }
    const int nameIndex = indexOfAttribute(*attributes, nameAttribute());
    if (nameIndex == -1) {
        m_error = msgMissingAttribute(nameAttribute());
        return nullptr;
    }
    return new TemplateInstance(attributes->takeAt(nameIndex).value().toString());
}

bool Handler::parseReplace(const QXmlStreamReader &,
                           const StackElement &topElement,
                           StackElement *element, QXmlStreamAttributes *attributes)
{
    if (topElement.type != StackElement::TemplateInstanceEnum) {
        m_error = QLatin1String("Can only insert replace rules into insert-template.");
        return false;
    }
    QString from;
    QString to;
    for (int i = attributes->size() - 1; i >= 0; --i) {
        const QStringRef name = attributes->at(i).qualifiedName();
        if (name == QLatin1String("from"))
            from = attributes->takeAt(i).value().toString();
        else if (name == toAttribute())
            to = attributes->takeAt(i).value().toString();
    }
    element->parent->value.templateInstance->addReplaceRule(from, to);
    return true;
}

bool Handler::startElement(const QXmlStreamReader &reader)
{
    if (m_ignoreDepth) {
        ++m_ignoreDepth;
        return true;
    }

    const QStringRef tagName = reader.name();
    QXmlStreamAttributes attributes = reader.attributes();

    QVersionNumber since(0, 0);
    int index = indexOfAttribute(attributes, sinceAttribute());
    if (index != -1) {
        const QStringRef sinceSpec = attributes.takeAt(index).value();
        since = QVersionNumber::fromString(sinceSpec.toString());
        if (since.isNull()) {
            m_error = msgInvalidVersion(sinceSpec, m_defaultPackage);
            return false;
        }
    }

    if (!m_defaultPackage.isEmpty() && since > QVersionNumber(0, 0)) {
        TypeDatabase* td = TypeDatabase::instance();
        if (!td->checkApiVersion(m_defaultPackage, since)) {
            ++m_ignoreDepth;
            return true;
        }
    }

    if (tagName.compare(QLatin1String("import-file"), Qt::CaseInsensitive) == 0)
        return importFileElement(attributes);

    const StackElement::ElementType elementType = elementFromTag(tagName);
    if (elementType == StackElement::None) {
        m_error = QStringLiteral("Unknown tag name: '%1'").arg(tagName);
        return false;
    }

    if (m_currentDroppedEntry) {
        ++m_currentDroppedEntryDepth;
        return true;
    }

    StackElement* element = new StackElement(m_current);
    element->type = elementType;

    if (element->type == StackElement::Root && m_generate == TypeEntry::GenerateAll)
        customConversionsForReview.clear();

    if (element->type == StackElement::CustomMetaConstructor
        || element->type == StackElement::CustomMetaDestructor) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgUnimplementedElementWarning(reader, tagName)));
    }

    switch (element->type) {
    case StackElement::Root:
    case StackElement::NamespaceTypeEntry:
    case StackElement::InterfaceTypeEntry:
    case StackElement::ObjectTypeEntry:
    case StackElement::ValueTypeEntry:
    case StackElement::PrimitiveTypeEntry:
    case StackElement::TypedefTypeEntry:
        m_contextStack.push(new StackElementContext());
        break;
    default:
        break;
    }

    if (element->type & StackElement::TypeEntryMask) {
        QString name;
        if (element->type != StackElement::FunctionTypeEntry) {
            const int nameIndex = indexOfAttribute(attributes, nameAttribute());
            if (nameIndex != -1) {
                name = attributes.takeAt(nameIndex).value().toString();
            } else if (element->type != StackElement::EnumTypeEntry) { // anonymous enum?
                m_error = msgMissingAttribute(nameAttribute());
                return false;
            }
        }

        if (m_database->hasDroppedTypeEntries()) {
            QString identifier = getNamePrefix(element) + QLatin1Char('.');
            identifier += element->type == StackElement::FunctionTypeEntry
                ? attributes.value(signatureAttribute()).toString()
                : name;
            if (m_database->shouldDropTypeEntry(identifier)) {
                m_currentDroppedEntry = element;
                m_currentDroppedEntryDepth = 1;
                if (ReportHandler::isDebug(ReportHandler::SparseDebug)) {
                      qCDebug(lcShiboken)
                          << QStringLiteral("Type system entry '%1' was intentionally dropped from generation.").arg(identifier);
                }
                return true;
            }
        }

        // The top level tag 'function' has only the 'signature' tag
        // and we should extract the 'name' value from it.
        if (element->type == StackElement::FunctionTypeEntry
            && !parseRenameFunction(reader, &name, &attributes)) {
                return false;
        }

        // We need to be able to have duplicate primitive type entries,
        // or it's not possible to cover all primitive target language
        // types (which we need to do in order to support fake meta objects)
        if (element->type != StackElement::PrimitiveTypeEntry
            && element->type != StackElement::FunctionTypeEntry) {
            TypeEntry *tmp = m_database->findType(name);
            if (tmp)
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("Duplicate type entry: '%1'").arg(name);
        }

        if (element->type == StackElement::EnumTypeEntry) {
            const int enumIdentifiedByIndex = indexOfAttribute(attributes, enumIdentifiedByValueAttribute());
            const QString identifiedByValue = enumIdentifiedByIndex != -1
                ? attributes.takeAt(enumIdentifiedByIndex).value().toString() : QString();
            if (name.isEmpty()) {
                name = identifiedByValue;
            } else if (!identifiedByValue.isEmpty()) {
                m_error = QLatin1String("can't specify both 'name' and 'identified-by-value' attributes");
                return false;
            }
        }

        // Fix type entry name using nesting information.
        if (element->type & StackElement::TypeEntryMask
            && element->parent && element->parent->type != StackElement::Root) {
            name = element->parent->entry->name() + colonColon() + name;
        }


        if (name.isEmpty()) {
            m_error = QLatin1String("no 'name' attribute specified");
            return false;
        }

        switch (element->type) {
        case StackElement::CustomTypeEntry:
            element->entry = new TypeEntry(name, TypeEntry::CustomType, since);
            break;
        case StackElement::PrimitiveTypeEntry:
            element->entry = parsePrimitiveTypeEntry(reader, name, since, &attributes);
            if (Q_UNLIKELY(!element->entry))
                return false;
            break;
        case StackElement::ContainerTypeEntry:
            if (ContainerTypeEntry *ce = parseContainerTypeEntry(reader, name, since, &attributes)) {
                applyComplexTypeAttributes(reader, ce, &attributes);
                element->entry = ce;
            } else {
                return false;
            }
            break;

        case StackElement::SmartPointerTypeEntry:
            if (SmartPointerTypeEntry *se = parseSmartPointerEntry(reader, name, since, &attributes)) {
                applyComplexTypeAttributes(reader, se, &attributes);
                element->entry = se;
            } else {
                return false;
            }
            break;
        case StackElement::EnumTypeEntry:
            m_currentEnum = parseEnumTypeEntry(reader, name, since, &attributes);
            if (Q_UNLIKELY(!m_currentEnum))
                return false;
            element->entry = m_currentEnum;
            break;

        case StackElement::InterfaceTypeEntry:
            if (ObjectTypeEntry *oe = parseInterfaceTypeEntry(reader, name, since, &attributes)) {
                applyComplexTypeAttributes(reader, oe, &attributes);
                element->entry = oe;
            } else {
                return false;
            }
            break;
        case StackElement::ValueTypeEntry:
           if (ValueTypeEntry *ve = parseValueTypeEntry(reader, name, since, &attributes)) {
               applyComplexTypeAttributes(reader, ve, &attributes);
               element->entry = ve;
           } else {
               return false;
           }
           break;
        case StackElement::NamespaceTypeEntry:
            element->entry = new NamespaceTypeEntry(name, since);
            applyCommonAttributes(element->entry, &attributes);
            applyComplexTypeAttributes(reader, static_cast<ComplexTypeEntry *>(element->entry), &attributes);
            break;
        case StackElement::ObjectTypeEntry:
            element->entry = new ObjectTypeEntry(name, since);
            applyCommonAttributes(element->entry, &attributes);
            applyComplexTypeAttributes(reader, static_cast<ComplexTypeEntry *>(element->entry), &attributes);
            break;
        case StackElement::FunctionTypeEntry:
            element->entry = parseFunctionTypeEntry(reader, name, since, &attributes);
            if (Q_UNLIKELY(!element->entry))
                return false;
            break;
        case StackElement::TypedefTypeEntry:
            if (TypedefEntry *te = parseTypedefEntry(reader, name, since, &attributes)) {
                applyComplexTypeAttributes(reader, te, &attributes);
                element->entry = te;
            } else {
                return false;
            }
            break;
        default:
            Q_ASSERT(false);
        };

        if (element->entry) {
            if (!m_database->addType(element->entry, &m_error))
                return false;
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Type: %1 was rejected by typesystem").arg(name);
        }

    } else if (element->type == StackElement::InjectDocumentation) {
        if (!parseInjectDocumentation(reader, &attributes))
            return false;
    } else if (element->type == StackElement::ModifyDocumentation) {
        if (!parseModifyDocumentation(reader, &attributes))
            return false;
    } else if (element->type != StackElement::None) {
        bool topLevel = element->type == StackElement::Root
                        || element->type == StackElement::SuppressedWarning
                        || element->type == StackElement::Rejection
                        || element->type == StackElement::LoadTypesystem
                        || element->type == StackElement::InjectCode
                        || element->type == StackElement::ExtraIncludes
                        || element->type == StackElement::ConversionRule
                        || element->type == StackElement::AddFunction
                        || element->type == StackElement::Template;

        if (!topLevel && m_current->type == StackElement::Root) {
            m_error = QStringLiteral("Tag requires parent: '%1'").arg(tagName);
            return false;
        }

        StackElement topElement = !m_current ? StackElement(0) : *m_current;
        element->entry = topElement.entry;

        switch (element->type) {
        case StackElement::Root:
            element->entry = parseRootElement(reader, since, &attributes);
            element->type = StackElement::Root;
            break;
        case StackElement::LoadTypesystem:
            if (!loadTypesystem(reader, &attributes))
                return false;
            break;
        case StackElement::RejectEnumValue:
            if (!parseRejectEnumValue(reader, &attributes))
                return false;
            break;
        case StackElement::ReplaceType:
            if (!parseReplaceArgumentType(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::ConversionRule:
            if (!Handler::parseCustomConversion(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::NativeToTarget:
            if (!parseNativeToTarget(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::TargetToNative: {
            if (topElement.type != StackElement::ConversionRule) {
                m_error = QLatin1String("Target to Native conversions can only be specified for custom conversion rules.");
                return false;
            }
            const int replaceIndex = indexOfAttribute(attributes, replaceAttribute());
            const bool replace = replaceIndex == -1
                || convertBoolean(attributes.takeAt(replaceIndex).value(),
                                  replaceAttribute(), true);
            m_current->entry->customConversion()->setReplaceOriginalTargetToNativeConversions(replace);
        }
        break;
        case StackElement::AddConversion:
            if (!parseAddConversion(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::ModifyArgument:
            if (!parseModifyArgument(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::NoNullPointers:
            if (!parseNoNullPointer(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::DefineOwnership:
            if (!parseDefineOwnership(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::SuppressedWarning: {
            const int textIndex = indexOfAttribute(attributes, textAttribute());
            if (textIndex == -1) {
                qCWarning(lcShiboken) << "Suppressed warning with no text specified";
            } else {
                const QString suppressedWarning =
                    attributes.takeAt(textIndex).value().toString();
                if (!m_database->addSuppressedWarning(suppressedWarning, &m_error))
                    return false;
            }
        }
            break;
        case StackElement::ArgumentMap:
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnimplementedElementWarning(reader, tagName)));
            if (!parseArgumentMap(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::Removal:
            if (!parseRemoval(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::Rename:
        case StackElement::Access:
            if (!parseRename(reader, element->type, topElement, &attributes))
                return false;
            break;
        case StackElement::RemoveArgument:
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("Removing argument requires argument modification as parent");
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().removed = true;
            break;

        case StackElement::ModifyField:
            if (!parseModifyField(reader, &attributes))
                return false;
            break;
        case StackElement::AddFunction:
            if (!parseAddFunction(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::ModifyFunction:
            if (!parseModifyFunction(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::ReplaceDefaultExpression:
            if (!parseReplaceDefaultExpression(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::RemoveDefaultExpression:
            m_contextStack.top()->functionMods.last().argument_mods.last().removedDefaultExpression = true;
            break;
        case StackElement::CustomMetaConstructor:
        case StackElement::CustomMetaDestructor:
            element->value.customFunction =
                parseCustomMetaConstructor(reader, element->type, topElement, &attributes);
            break;
        case StackElement::ReferenceCount:
            if (!parseReferenceCount(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::ParentOwner:
            if (!parseParentOwner(reader, topElement, &attributes))
                return false;
            break;
        case StackElement::Array:
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("array must be child of modify-argument");
                return false;
            }
            m_contextStack.top()->functionMods.last().argument_mods.last().array = true;
            break;
        case StackElement::InjectCode:
            if (!parseInjectCode(reader, topElement, element, &attributes))
                return false;
            break;
        case StackElement::Include:
            if (!parseInclude(reader, topElement, element->entry, &attributes))
                return false;
            break;
        case StackElement::Rejection:
            if (!addRejection(m_database, &attributes, &m_error))
                return false;
            break;
        case StackElement::Template: {
            const int nameIndex = indexOfAttribute(attributes, nameAttribute());
            if (nameIndex == -1) {
                m_error = msgMissingAttribute(nameAttribute());
                return false;
            }
            element->value.templateEntry =
                new TemplateEntry(attributes.takeAt(nameIndex).value().toString());
        }
            break;
        case StackElement::TemplateInstanceEnum:
            element->value.templateInstance =
                parseTemplateInstanceEnum(reader, topElement, &attributes);
            if (!element->value.templateInstance)
                return false;
            break;
        case StackElement::Replace:
            if (!parseReplace(reader, topElement, element, &attributes))
                return false;
            break;
        default:
            break; // nada
        };
    }

    if (!attributes.isEmpty()) {
        const QString message = msgUnusedAttributes(tagName, attributes);
        qCWarning(lcShiboken, "%s", qPrintable(msgReaderWarning(reader, message)));
    }

    m_current = element;
    return true;
}

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
        return 0;

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
    for (int i = 0; i < m_fieldMods.size(); ++i)
        if (m_fieldMods.at(i).name == name)
            return m_fieldMods.at(i);
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

/*!
 * The Visual Studio 2002 compiler doesn't support these symbols,
 * which our typedefs unforntuatly expand to.
 */
QString fixCppTypeName(const QString &name)
{
    if (name == QLatin1String("long long"))
        return QLatin1String("qint64");
    if (name == QLatin1String("unsigned long long"))
        return QLatin1String("quint64");
    return name;
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

static AddedFunction::TypeInfo parseType(const QString& signature, int startPos = 0, int* endPos = 0)
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
            TypeInfo arg = parseType(signature, endPos, &endPos);
            if (!arg.name.isEmpty())
                m_arguments.append(arg);
            // end of parameters...
            if (signature[endPos] == QLatin1Char(')'))
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

NamespaceTypeEntry::NamespaceTypeEntry(const NamespaceTypeEntry &) = default;

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
        : sourceType(0)
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
