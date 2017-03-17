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

#include "typesystem.h"
#include "typesystem_p.h"
#include "typedatabase.h"
#include "reporthandler.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamAttributes>
#include <QtCore/QXmlStreamReader>

static QString strings_Object = QLatin1String("Object");
static QString strings_String = QLatin1String("String");
static QString strings_char = QLatin1String("char");
static QString strings_jchar = QLatin1String("jchar");
static QString strings_jobject = QLatin1String("jobject");

static inline QString colonColon() { return QStringLiteral("::"); }
static inline QString quoteAfterLineAttribute() { return QStringLiteral("quote-after-line"); }
static inline QString quoteBeforeLineAttribute() { return QStringLiteral("quote-before-line"); }
static inline QString nameAttribute() { return QStringLiteral("name"); }
static inline QString sinceAttribute() { return QStringLiteral("since"); }
static inline QString flagsAttribute() { return QStringLiteral("flags"); }

static QList<CustomConversion*> customConversionsForReview = QList<CustomConversion*>();

Handler::Handler(TypeDatabase* database, bool generate)
            : m_database(database), m_generate(generate ? TypeEntry::GenerateAll : TypeEntry::GenerateForSubclass)
{
    m_currentEnum = 0;
    m_current = 0;
    m_currentDroppedEntry = 0;
    m_currentDroppedEntryDepth = 0;
    m_ignoreDepth = 0;

    tagNames.insert(QLatin1String("rejection"), StackElement::Rejection);
    tagNames.insert(QLatin1String("custom-type"), StackElement::CustomTypeEntry);
    tagNames.insert(QLatin1String("primitive-type"), StackElement::PrimitiveTypeEntry);
    tagNames.insert(QLatin1String("container-type"), StackElement::ContainerTypeEntry);
    tagNames.insert(QLatin1String("object-type"), StackElement::ObjectTypeEntry);
    tagNames.insert(QLatin1String("value-type"), StackElement::ValueTypeEntry);
    tagNames.insert(QLatin1String("interface-type"), StackElement::InterfaceTypeEntry);
    tagNames.insert(QLatin1String("namespace-type"), StackElement::NamespaceTypeEntry);
    tagNames.insert(QLatin1String("enum-type"), StackElement::EnumTypeEntry);
    tagNames.insert(QLatin1String("function"), StackElement::FunctionTypeEntry);
    tagNames.insert(QLatin1String("extra-includes"), StackElement::ExtraIncludes);
    tagNames.insert(QLatin1String("include"), StackElement::Include);
    tagNames.insert(QLatin1String("inject-code"), StackElement::InjectCode);
    tagNames.insert(QLatin1String("modify-function"), StackElement::ModifyFunction);
    tagNames.insert(QLatin1String("modify-field"), StackElement::ModifyField);
    tagNames.insert(QLatin1String("access"), StackElement::Access);
    tagNames.insert(QLatin1String("remove"), StackElement::Removal);
    tagNames.insert(QLatin1String("rename"), StackElement::Rename);
    tagNames.insert(QLatin1String("typesystem"), StackElement::Root);
    tagNames.insert(QLatin1String("custom-constructor"), StackElement::CustomMetaConstructor);
    tagNames.insert(QLatin1String("custom-destructor"), StackElement::CustomMetaDestructor);
    tagNames.insert(QLatin1String("argument-map"), StackElement::ArgumentMap);
    tagNames.insert(QLatin1String("suppress-warning"), StackElement::SuppressedWarning);
    tagNames.insert(QLatin1String("load-typesystem"), StackElement::LoadTypesystem);
    tagNames.insert(QLatin1String("define-ownership"), StackElement::DefineOwnership);
    tagNames.insert(QLatin1String("replace-default-expression"), StackElement::ReplaceDefaultExpression);
    tagNames.insert(QLatin1String("reject-enum-value"), StackElement::RejectEnumValue);
    tagNames.insert(QLatin1String("replace-type"), StackElement::ReplaceType);
    tagNames.insert(QLatin1String("conversion-rule"), StackElement::ConversionRule);
    tagNames.insert(QLatin1String("native-to-target"), StackElement::NativeToTarget);
    tagNames.insert(QLatin1String("target-to-native"), StackElement::TargetToNative);
    tagNames.insert(QLatin1String("add-conversion"), StackElement::AddConversion);
    tagNames.insert(QLatin1String("modify-argument"), StackElement::ModifyArgument);
    tagNames.insert(QLatin1String("remove-argument"), StackElement::RemoveArgument);
    tagNames.insert(QLatin1String("remove-default-expression"), StackElement::RemoveDefaultExpression);
    tagNames.insert(QLatin1String("template"), StackElement::Template);
    tagNames.insert(QLatin1String("insert-template"), StackElement::TemplateInstanceEnum);
    tagNames.insert(QLatin1String("replace"), StackElement::Replace);
    tagNames.insert(QLatin1String("no-null-pointer"), StackElement::NoNullPointers);
    tagNames.insert(QLatin1String("reference-count"), StackElement::ReferenceCount);
    tagNames.insert(QLatin1String("parent"), StackElement::ParentOwner);
    tagNames.insert(QLatin1String("inject-documentation"), StackElement::InjectDocumentation);
    tagNames.insert(QLatin1String("modify-documentation"), StackElement::ModifyDocumentation);
    tagNames.insert(QLatin1String("add-function"), StackElement::AddFunction);
}

static QString msgReaderError(const QXmlStreamReader &reader, const QString &what)
{
    QString message;
    QTextStream str(&message);
    str << "Error: ";
    if (const QFile *file = qobject_cast<const QFile *>(reader.device()))
        str << "file=" << QDir::toNativeSeparators(file->fileName()) << ", ";
    str << "line=" << reader.lineNumber() << ", column=" << reader.columnNumber()
        << ", message=" << what;
    return message;
}

static QString msgReaderError(const QXmlStreamReader &reader)
{
    return msgReaderError(reader, reader.errorString());
}

bool Handler::parse(QXmlStreamReader &reader)
{
    m_error.clear();
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
            qCWarning(lcShiboken).noquote().nospace() << msgReaderError(reader);
            return false;
        case QXmlStreamReader::StartElement:
            if (!startElement(reader.name(), reader.attributes())) {
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

void Handler::fetchAttributeValues(const QString &name, const QXmlStreamAttributes &atts,
                                   QHash<QString, QString> *acceptedAttributes)
{
    Q_ASSERT(acceptedAttributes);

    for (int i = 0; i < atts.length(); ++i) {
        const QString key = atts.at(i).name().toString().toLower();
        if (!acceptedAttributes->contains(key)) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Unknown attribute for '%1': '%2'").arg(name, key);
        } else {
            acceptedAttributes->insert(key, atts.at(i).value().toString());
        }
    }
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
            ++m_currentDroppedEntryDepth;
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
            foreach (CustomConversion* customConversion, customConversionsForReview) {
                foreach (CustomConversion::TargetToNativeConversion* toNative, customConversion->targetToNativeConversions())
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

    if (m_current->type == StackElement::Root
        || m_current->type == StackElement::NamespaceTypeEntry
        || m_current->type == StackElement::InterfaceTypeEntry
        || m_current->type == StackElement::ObjectTypeEntry
        || m_current->type == StackElement::ValueTypeEntry
        || m_current->type == StackElement::PrimitiveTypeEntry) {
        StackElementContext* context = m_contextStack.pop();
        delete context;
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

bool Handler::convertBoolean(const QString &_value, const QString &attributeName, bool defaultValue)
{
    QString value = _value.toLower();
    if (value == QLatin1String("true") || value == QLatin1String("yes"))
        return true;
     else if (value == QLatin1String("false") || value == QLatin1String("no"))
        return false;
     else {
        QString warn = QStringLiteral("Boolean value '%1' not supported in attribute '%2'. Use 'yes' or 'no'. Defaulting to '%3'.")
                                      .arg(value, attributeName,
                                           defaultValue ? QLatin1String("yes") : QLatin1String("no"));

        qCWarning(lcShiboken).noquote().nospace() << warn;
        return defaultValue;
    }
}

static bool convertRemovalAttribute(const QString& removalAttribute, Modification& mod, QString& errorMsg)
{
    QString remove = removalAttribute.toLower();
    if (!remove.isEmpty()) {
        if (remove == QLatin1String("all")) {
            mod.removal = TypeSystem::All;
        } else if (remove == QLatin1String("target")) {
            mod.removal = TypeSystem::TargetLangAndNativeCode;
        } else {
            errorMsg = QString::fromLatin1("Bad removal type '%1'").arg(remove);
            return false;
        }
    }
    return true;
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
    static QRegExp whiteSpace(QLatin1String("\\s"));
    if (!funcName.startsWith(QLatin1String("operator ")) && funcName.contains(whiteSpace)) {
        return QString::fromLatin1("Error in <%1> tag signature attribute '%2'.\n"
                                   "White spaces aren't allowed in function names, "
                                   "and return types should not be part of the signature.")
                                   .arg(tag, signature);
    }
    return QString();
}

void Handler::addFlags(const QString &name, QString flagName,
                       const QHash<QString, QString> &attributes, double since)
{
    FlagsTypeEntry *ftype = new FlagsTypeEntry(QLatin1String("QFlags<") + name + QLatin1Char('>'), since);
    ftype->setOriginator(m_currentEnum);
    // Try to get the guess the qualified flag name
    const int lastSepPos = name.lastIndexOf(colonColon());
    if (lastSepPos >= 0 && !flagName.contains(colonColon()))
        flagName.prepend(name.left(lastSepPos + 2));

    ftype->setOriginalName(flagName);
    ftype->setCodeGeneration(m_generate);
    QString n = ftype->originalName();

    QStringList lst = n.split(colonColon());
    if (QStringList(lst.mid(0, lst.size() - 1)).join(colonColon()) != m_currentEnum->targetLangQualifier()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("enum %1 and flags %2 differ in qualifiers")
                              // avoid constFirst to stay Qt 5.5 compatible
                              .arg(m_currentEnum->targetLangQualifier(), lst.first());
    }

    ftype->setFlagsName(lst.last());
    m_currentEnum->setFlags(ftype);

    m_database->addFlagsType(ftype);
    m_database->addType(ftype);

    QString revision = attributes.value(QLatin1String("flags-revision"));
    if (revision.isEmpty())
        revision = attributes.value(QLatin1String("revision"));
    setTypeRevision(ftype, revision.toInt());
}

bool Handler::startElement(const QStringRef &n, const QXmlStreamAttributes &atts)
{
    if (m_ignoreDepth) {
        ++m_ignoreDepth;
        return true;
    }

    if (!m_defaultPackage.isEmpty() && atts.hasAttribute(sinceAttribute())) {
        TypeDatabase* td = TypeDatabase::instance();
        if (!td->checkApiVersion(m_defaultPackage, atts.value(sinceAttribute()).toUtf8())) {
            ++m_ignoreDepth;
            return true;
        }
    }

    const QString tagName = n.toString().toLower();
    if (tagName == QLatin1String("import-file"))
        return importFileElement(atts);

    const QHash<QString, StackElement::ElementType>::const_iterator tit = tagNames.constFind(tagName);
    if (tit == tagNames.constEnd()) {
        m_error = QStringLiteral("Unknown tag name: '%1'").arg(tagName);
        return false;
    }

    if (m_currentDroppedEntry) {
        ++m_currentDroppedEntryDepth;
        return true;
    }

    StackElement* element = new StackElement(m_current);
    element->type = tit.value();

    if (element->type == StackElement::Root && m_generate == TypeEntry::GenerateAll)
        customConversionsForReview.clear();

    if (element->type == StackElement::Root
        || element->type == StackElement::NamespaceTypeEntry
        || element->type == StackElement::InterfaceTypeEntry
        || element->type == StackElement::ObjectTypeEntry
        || element->type == StackElement::ValueTypeEntry
        || element->type == StackElement::PrimitiveTypeEntry) {
        m_contextStack.push(new StackElementContext());
    }

    if (element->type & StackElement::TypeEntryMask) {
        QHash<QString, QString> attributes;
        attributes.insert(nameAttribute(), QString());
        attributes.insert(QLatin1String("revision"), QLatin1String("0"));
        attributes.insert(sinceAttribute(), QLatin1String("0"));

        switch (element->type) {
        case StackElement::PrimitiveTypeEntry:
            attributes.insert(QLatin1String("target-lang-name"), QString());
            attributes.insert(QLatin1String("target-lang-api-name"), QString());
            attributes.insert(QLatin1String("preferred-conversion"), QLatin1String("yes"));
            attributes.insert(QLatin1String("preferred-target-lang-type"), QLatin1String("yes"));
            attributes.insert(QLatin1String("default-constructor"), QString());
            break;
        case StackElement::ContainerTypeEntry:
            attributes.insert(QLatin1String("type"), QString());
            break;
        case StackElement::EnumTypeEntry:
            attributes.insert(flagsAttribute(), QString());
            attributes.insert(QLatin1String("flags-revision"), QString());
            attributes.insert(QLatin1String("upper-bound"), QString());
            attributes.insert(QLatin1String("lower-bound"), QString());
            attributes.insert(QLatin1String("force-integer"), QLatin1String("no"));
            attributes.insert(QLatin1String("extensible"), QLatin1String("no"));
            attributes.insert(QLatin1String("identified-by-value"), QString());
            break;
        case StackElement::ValueTypeEntry:
            attributes.insert(QLatin1String("default-constructor"), QString());
            // fall throooough
        case StackElement::ObjectTypeEntry:
            attributes.insert(QLatin1String("force-abstract"), QLatin1String("no"));
            attributes.insert(QLatin1String("deprecated"), QLatin1String("no"));
            attributes.insert(QLatin1String("hash-function"), QString());
            attributes.insert(QLatin1String("stream"), QLatin1String("no"));
            // fall throooough
        case StackElement::InterfaceTypeEntry:
            attributes[QLatin1String("default-superclass")] = m_defaultSuperclass;
            attributes.insert(QLatin1String("polymorphic-id-expression"), QString());
            attributes.insert(QLatin1String("delete-in-main-thread"), QLatin1String("no"));
            attributes.insert(QLatin1String("held-type"), QString());
            attributes.insert(QLatin1String("copyable"), QString());
            // fall through
        case StackElement::NamespaceTypeEntry:
            attributes.insert(QLatin1String("target-lang-name"), QString());
            attributes[QLatin1String("package")] = m_defaultPackage;
            attributes.insert(QLatin1String("expense-cost"), QLatin1String("1"));
            attributes.insert(QLatin1String("expense-limit"), QLatin1String("none"));
            attributes.insert(QLatin1String("polymorphic-base"), QLatin1String("no"));
            attributes.insert(QLatin1String("generate"), QLatin1String("yes"));
            attributes.insert(QLatin1String("target-type"), QString());
            attributes.insert(QLatin1String("generic-class"), QLatin1String("no"));
            break;
        case StackElement::FunctionTypeEntry:
            attributes.insert(QLatin1String("signature"), QString());
            attributes.insert(QLatin1String("rename"), QString());
            break;
        default:
            { } // nada
        };

        fetchAttributeValues(tagName, atts, &attributes);
        QString name = attributes[nameAttribute()];
        double since = attributes[sinceAttribute()].toDouble();

        if (m_database->hasDroppedTypeEntries()) {
            QString identifier = getNamePrefix(element) + QLatin1Char('.');
            identifier += (element->type == StackElement::FunctionTypeEntry ? attributes[QLatin1String("signature")] : name);
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
        if (element->type == StackElement::FunctionTypeEntry) {
            QString signature = attributes[QLatin1String("signature")];
            name = signature.left(signature.indexOf(QLatin1Char('('))).trimmed();
            QString errorString = checkSignatureError(signature, QLatin1String("function"));
            if (!errorString.isEmpty()) {
                m_error = errorString;
                return false;
            }
            QString rename = attributes[QLatin1String("rename")];
            if (!rename.isEmpty()) {
                static QRegExp functionNameRegExp(QLatin1String("^[a-zA-Z_][a-zA-Z0-9_]*$"));
                if (!functionNameRegExp.exactMatch(rename)) {
                    m_error = QLatin1String("can not rename '") + signature + QLatin1String("', '")
                              + rename + QLatin1String("' is not a valid function name");
                    return false;
                }
                FunctionModification mod(since);
                mod.signature = signature;
                mod.renamedToName = attributes[QLatin1String("rename")];
                mod.modifiers |= Modification::Rename;
                m_contextStack.top()->functionMods << mod;
            }
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
            if (name.isEmpty()) {
                name = attributes[QLatin1String("identified-by-value")];
            } else if (!attributes[QLatin1String("identified-by-value")].isEmpty()) {
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
        case StackElement::PrimitiveTypeEntry: {
            QString targetLangName = attributes[QLatin1String("target-lang-name")];
            QString targetLangApiName = attributes[QLatin1String("target-lang-api-name")];
            QString preferredConversion = attributes[QLatin1String("preferred-conversion")].toLower();
            QString preferredTargetLangType = attributes[QLatin1String("preferred-target-lang-type")].toLower();
            QString defaultConstructor = attributes[QLatin1String("default-constructor")];

            if (targetLangName.isEmpty())
                targetLangName = name;
            if (targetLangApiName.isEmpty())
                targetLangApiName = name;

            PrimitiveTypeEntry *type = new PrimitiveTypeEntry(name, since);
            type->setCodeGeneration(m_generate);
            type->setTargetLangName(targetLangName);
            type->setTargetLangApiName(targetLangApiName);
            type->setTargetLangPackage(m_defaultPackage);
            type->setDefaultConstructor(defaultConstructor);

            bool preferred;
            preferred = convertBoolean(preferredConversion, QLatin1String("preferred-conversion"), true);
            type->setPreferredConversion(preferred);
            preferred = convertBoolean(preferredTargetLangType,
                                       QLatin1String("preferred-target-lang-type"), true);
            type->setPreferredTargetLangType(preferred);

            element->entry = type;
        }
        break;

        case StackElement::ContainerTypeEntry: {
            QString typeName = attributes[QLatin1String("type")];
            ContainerTypeEntry::Type containerType =
                    ContainerTypeEntry::containerTypeFromString(typeName);
            if (typeName.isEmpty()) {
                m_error = QLatin1String("no 'type' attribute specified");
                return false;
            } else if (containerType == ContainerTypeEntry::NoContainer) {
                m_error = QLatin1String("there is no container of type ") + typeName;
                return false;
            }

            ContainerTypeEntry *type = new ContainerTypeEntry(name, containerType, since);
            type->setCodeGeneration(m_generate);
            element->entry = type;
        }
        break;

        case StackElement::EnumTypeEntry: {
            QStringList names = name.split(colonColon());
            if (names.size() == 1)
                m_currentEnum = new EnumTypeEntry(QString(), name, since);
             else
                m_currentEnum =
                    new EnumTypeEntry(QStringList(names.mid(0, names.size() - 1)).join(colonColon()),
                                      names.last(), since);
            m_currentEnum->setAnonymous(!attributes[QLatin1String("identified-by-value")].isEmpty());
            element->entry = m_currentEnum;
            m_currentEnum->setCodeGeneration(m_generate);
            m_currentEnum->setTargetLangPackage(m_defaultPackage);
            m_currentEnum->setUpperBound(attributes[QLatin1String("upper-bound")]);
            m_currentEnum->setLowerBound(attributes[QLatin1String("lower-bound")]);
            m_currentEnum->setForceInteger(convertBoolean(attributes[QLatin1String("force-integer")], QLatin1String("force-integer"), false));
            m_currentEnum->setExtensible(convertBoolean(attributes[QLatin1String("extensible")], QLatin1String("extensible"), false));

            // put in the flags parallel...
            const QString flagNames = attributes.value(flagsAttribute());
            if (!flagNames.isEmpty()) {
                foreach (const QString &flagName, flagNames.split(QLatin1Char(',')))
                    addFlags(name, flagName.trimmed(), attributes, since);
            }
        }
        break;

        case StackElement::InterfaceTypeEntry: {
            ObjectTypeEntry *otype = new ObjectTypeEntry(name, since);
            QString targetLangName = attributes[QLatin1String("target-lang-name")];
            if (targetLangName.isEmpty())
                targetLangName = name;
            InterfaceTypeEntry *itype =
                new InterfaceTypeEntry(InterfaceTypeEntry::interfaceName(targetLangName), since);

            if (!convertBoolean(attributes[QLatin1String("generate")], QLatin1String("generate"), true))
                itype->setCodeGeneration(TypeEntry::GenerateForSubclass);
            else
                itype->setCodeGeneration(m_generate);
            otype->setDesignatedInterface(itype);
            itype->setOrigin(otype);
            element->entry = otype;
        }
        // fall through
        case StackElement::ValueTypeEntry: {
            if (!element->entry) {
                ValueTypeEntry* typeEntry = new ValueTypeEntry(name, since);
                QString defaultConstructor = attributes[QLatin1String("default-constructor")];
                if (!defaultConstructor.isEmpty())
                    typeEntry->setDefaultConstructor(defaultConstructor);
                element->entry = typeEntry;
            }

        // fall through
        case StackElement::NamespaceTypeEntry:
            if (!element->entry)
                element->entry = new NamespaceTypeEntry(name, since);

        // fall through
        case StackElement::ObjectTypeEntry:
            if (!element->entry)
                element->entry = new ObjectTypeEntry(name, since);

            element->entry->setStream(attributes[QLatin1String("stream")] == QLatin1String("yes"));

            ComplexTypeEntry *ctype = static_cast<ComplexTypeEntry *>(element->entry);
            ctype->setTargetLangPackage(attributes[QLatin1String("package")]);
            ctype->setDefaultSuperclass(attributes[QLatin1String("default-superclass")]);
            ctype->setGenericClass(convertBoolean(attributes[QLatin1String("generic-class")], QLatin1String("generic-class"), false));

            if (!convertBoolean(attributes[QLatin1String("generate")], QLatin1String("generate"), true))
                element->entry->setCodeGeneration(TypeEntry::GenerateForSubclass);
            else
                element->entry->setCodeGeneration(m_generate);

            QString targetLangName = attributes[QLatin1String("target-lang-name")];
            if (!targetLangName.isEmpty())
                ctype->setTargetLangName(targetLangName);

            // The expense policy
            QString limit = attributes[QLatin1String("expense-limit")];
            if (!limit.isEmpty() && limit != QLatin1String("none")) {
                ExpensePolicy ep;
                ep.limit = limit.toInt();
                ep.cost = attributes[QLatin1String("expense-cost")];
                ctype->setExpensePolicy(ep);
            }


            ctype->setIsPolymorphicBase(convertBoolean(attributes[QLatin1String("polymorphic-base")], QLatin1String("polymorphic-base"), false));
            ctype->setPolymorphicIdValue(attributes[QLatin1String("polymorphic-id-expression")]);
            //Copyable
            if (attributes[QLatin1String("copyable")].isEmpty())
                ctype->setCopyable(ComplexTypeEntry::Unknown);
             else {
                if (convertBoolean(attributes[QLatin1String("copyable")], QLatin1String("copyable"), false))
                    ctype->setCopyable(ComplexTypeEntry::CopyableSet);
                 else
                    ctype->setCopyable(ComplexTypeEntry::NonCopyableSet);

            }

            if (element->type == StackElement::ObjectTypeEntry || element->type == StackElement::ValueTypeEntry)
                ctype->setHashFunction(attributes[QLatin1String("hash-function")]);


            ctype->setHeldType(attributes[QLatin1String("held-type")]);

            if (element->type == StackElement::ObjectTypeEntry
                || element->type == StackElement::ValueTypeEntry) {
                if (convertBoolean(attributes[QLatin1String("force-abstract")], QLatin1String("force-abstract"), false))
                    ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::ForceAbstract);
                if (convertBoolean(attributes[QLatin1String("deprecated")], QLatin1String("deprecated"), false))
                    ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::Deprecated);
            }

            if (element->type == StackElement::InterfaceTypeEntry
                || element->type == StackElement::ValueTypeEntry
                || element->type == StackElement::ObjectTypeEntry) {
                if (convertBoolean(attributes[QLatin1String("delete-in-main-thread")], QLatin1String("delete-in-main-thread"), false))
                    ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::DeleteInMainThread);
            }

            QString targetType = attributes[QLatin1String("target-type")];
            if (!targetType.isEmpty() && element->entry->isComplex())
                static_cast<ComplexTypeEntry *>(element->entry)->setTargetType(targetType);

            // ctype->setInclude(Include(Include::IncludePath, ctype->name()));
            ctype = ctype->designatedInterface();
            if (ctype)
                ctype->setTargetLangPackage(attributes[QLatin1String("package")]);

        }
        break;
        case StackElement::FunctionTypeEntry: {
            QString signature = attributes[QLatin1String("signature")];
            signature = TypeDatabase::normalizedSignature(signature.toLatin1().constData());
            element->entry = m_database->findType(name);
            if (element->entry) {
                if (element->entry->type() == TypeEntry::FunctionType) {
                    reinterpret_cast<FunctionTypeEntry*>(element->entry)->addSignature(signature);
                } else {
                    m_error = QStringLiteral("%1 expected to be a function, but isn't! Maybe it was already declared as a class or something else.")
                                             .arg(name);
                    return false;
                }
            } else {
                element->entry = new FunctionTypeEntry(name, signature, since);
                element->entry->setCodeGeneration(m_generate);
            }
        }
        break;
        default:
            Q_ASSERT(false);
        };

        if (element->entry) {
            m_database->addType(element->entry);
            setTypeRevision(element->entry, attributes[QLatin1String("revision")].toInt());
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Type: %1 was rejected by typesystem").arg(name);
        }

    } else if (element->type == StackElement::InjectDocumentation) {
        // check the XML tag attributes
        QHash<QString, QString> attributes;
        attributes.insert(QLatin1String("mode"), QLatin1String("replace"));
        attributes.insert(QLatin1String("format"), QLatin1String("native"));
        attributes.insert(sinceAttribute(), QLatin1String("0"));

        fetchAttributeValues(tagName, atts, &attributes);
        double since = attributes[sinceAttribute()].toDouble();

        const int validParent = StackElement::TypeEntryMask
                                | StackElement::ModifyFunction
                                | StackElement::ModifyField;
        if (m_current->parent && m_current->parent->type & validParent) {
            QString modeName = attributes[QLatin1String("mode")];
            TypeSystem::DocModificationMode mode;
            if (modeName == QLatin1String("append")) {
                mode = TypeSystem::DocModificationAppend;
            } else if (modeName == QLatin1String("prepend")) {
                mode = TypeSystem::DocModificationPrepend;
            } else if (modeName == QLatin1String("replace")) {
                mode = TypeSystem::DocModificationReplace;
            } else {
                m_error = QLatin1String("Unknow documentation injection mode: ") + modeName;
                return false;
            }

            static QHash<QString, TypeSystem::Language> languageNames;
            if (languageNames.isEmpty()) {
                languageNames[QLatin1String("target")] = TypeSystem::TargetLangCode;
                languageNames[QLatin1String("native")] = TypeSystem::NativeCode;
            }

            QString format = attributes[QLatin1String("format")].toLower();
            TypeSystem::Language lang = languageNames.value(format, TypeSystem::NoLanguage);
            if (lang == TypeSystem::NoLanguage) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(format);
                return false;
            }

            QString signature = m_current->type & StackElement::TypeEntryMask ? QString() : m_currentSignature;
            DocModification mod(mode, signature, since);
            mod.format = lang;
            m_contextStack.top()->docModifications << mod;
        } else {
            m_error = QLatin1String("inject-documentation must be inside modify-function, "
                      "modify-field or other tags that creates a type");
            return false;
        }
    } else if (element->type == StackElement::ModifyDocumentation) {
        // check the XML tag attributes
        QHash<QString, QString> attributes;
        attributes.insert(QLatin1String("xpath"), QString());
        attributes.insert(sinceAttribute(), QLatin1String("0"));
        fetchAttributeValues(tagName, atts, &attributes);
        double since = attributes[sinceAttribute()].toDouble();

        const int validParent = StackElement::TypeEntryMask
                                | StackElement::ModifyFunction
                                | StackElement::ModifyField;
        if (m_current->parent && m_current->parent->type & validParent) {
            QString signature = (m_current->type & StackElement::TypeEntryMask) ? QString() : m_currentSignature;
            m_contextStack.top()->docModifications << DocModification(attributes[QLatin1String("xpath")], signature, since);
        } else {
            m_error = QLatin1String("modify-documentation must be inside modify-function, "
                      "modify-field or other tags that creates a type");
            return false;
        }
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

        QHash<QString, QString> attributes;
        attributes.insert(sinceAttribute(), QLatin1String("0"));
        switch (element->type) {
        case StackElement::Root:
            attributes.insert(QLatin1String("package"), QString());
            attributes.insert(QLatin1String("default-superclass"), QString());
            break;
        case StackElement::LoadTypesystem:
            attributes.insert(nameAttribute(), QString());
            attributes.insert(QLatin1String("generate"), QLatin1String("yes"));
            break;
        case StackElement::NoNullPointers:
            attributes.insert(QLatin1String("default-value"), QString());
            break;
        case StackElement::SuppressedWarning:
            attributes.insert(QLatin1String("text"), QString());
            break;
        case StackElement::ReplaceDefaultExpression:
            attributes.insert(QLatin1String("with"), QString());
            break;
        case StackElement::DefineOwnership:
            attributes.insert(QLatin1String("class"), QLatin1String("target"));
            attributes.insert(QLatin1String("owner"), QString());
            break;
        case StackElement::AddFunction:
            attributes.insert(QLatin1String("signature"), QString());
            attributes.insert(QLatin1String("return-type"), QLatin1String("void"));
            attributes.insert(QLatin1String("access"), QLatin1String("public"));
            attributes.insert(QLatin1String("static"), QLatin1String("no"));
            break;
        case StackElement::ModifyFunction:
            attributes.insert(QLatin1String("signature"), QString());
            attributes.insert(QLatin1String("access"), QString());
            attributes.insert(QLatin1String("remove"), QString());
            attributes.insert(QLatin1String("rename"), QString());
            attributes.insert(QLatin1String("deprecated"), QLatin1String("no"));
            attributes.insert(QLatin1String("associated-to"), QString());
            attributes.insert(QLatin1String("virtual-slot"), QLatin1String("no"));
            attributes.insert(QLatin1String("thread"), QLatin1String("no"));
            attributes.insert(QLatin1String("allow-thread"), QLatin1String("no"));
            break;
        case StackElement::ModifyArgument:
            attributes.insert(QLatin1String("index"), QString());
            attributes.insert(QLatin1String("replace-value"), QString());
            attributes.insert(QLatin1String("invalidate-after-use"), QLatin1String("no"));
            break;
        case StackElement::ModifyField:
            attributes.insert(nameAttribute(), QString());
            attributes.insert(QLatin1String("write"), QLatin1String("true"));
            attributes.insert(QLatin1String("read"), QLatin1String("true"));
            attributes.insert(QLatin1String("remove"), QString());
            break;
        case StackElement::Access:
            attributes.insert(QLatin1String("modifier"), QString());
            break;
        case StackElement::Include:
            attributes.insert(QLatin1String("file-name"), QString());
            attributes.insert(QLatin1String("location"), QString());
            break;
        case StackElement::CustomMetaConstructor:
            attributes[nameAttribute()] = topElement.entry->name().toLower() + QLatin1String("_create");
            attributes.insert(QLatin1String("param-name"), QLatin1String("copy"));
            break;
        case StackElement::CustomMetaDestructor:
            attributes[nameAttribute()] = topElement.entry->name().toLower() + QLatin1String("_delete");
            attributes.insert(QLatin1String("param-name"), QLatin1String("copy"));
            break;
        case StackElement::ReplaceType:
            attributes.insert(QLatin1String("modified-type"), QString());
            break;
        case StackElement::InjectCode:
            attributes.insert(QLatin1String("class"), QLatin1String("target"));
            attributes.insert(QLatin1String("position"), QLatin1String("beginning"));
            attributes.insert(QLatin1String("file"), QString());
            break;
        case StackElement::ConversionRule:
            attributes.insert(QLatin1String("class"), QString());
            attributes.insert(QLatin1String("file"), QString());
            break;
        case StackElement::TargetToNative:
            attributes.insert(QLatin1String("replace"), QLatin1String("yes"));
            break;
        case StackElement::AddConversion:
            attributes.insert(QLatin1String("type"), QString());
            attributes.insert(QLatin1String("check"), QString());
            break;
        case StackElement::RejectEnumValue:
            attributes.insert(nameAttribute(), QString());
            break;
        case StackElement::ArgumentMap:
            attributes.insert(QLatin1String("index"), QLatin1String("1"));
            attributes.insert(QLatin1String("meta-name"), QString());
            break;
        case StackElement::Rename:
            attributes.insert(QLatin1String("to"), QString());
            break;
        case StackElement::Rejection:
            attributes.insert(QLatin1String("class"), QLatin1String("*"));
            attributes.insert(QLatin1String("function-name"), QLatin1String("*"));
            attributes.insert(QLatin1String("field-name"), QLatin1String("*"));
            attributes.insert(QLatin1String("enum-name"), QLatin1String("*"));
            break;
        case StackElement::Removal:
            attributes.insert(QLatin1String("class"), QLatin1String("all"));
            break;
        case StackElement::Template:
            attributes.insert(nameAttribute(), QString());
            break;
        case StackElement::TemplateInstanceEnum:
            attributes.insert(nameAttribute(), QString());
            break;
        case StackElement::Replace:
            attributes.insert(QLatin1String("from"), QString());
            attributes.insert(QLatin1String("to"), QString());
            break;
        case StackElement::ReferenceCount:
            attributes.insert(QLatin1String("action"), QString());
            attributes.insert(QLatin1String("variable-name"), QString());
            break;
        case StackElement::ParentOwner:
            attributes.insert(QLatin1String("index"), QString());
            attributes.insert(QLatin1String("action"), QString());
        default:
            { };
        };

        double since = 0;
        if (attributes.count() > 0) {
            fetchAttributeValues(tagName, atts, &attributes);
            since = attributes[sinceAttribute()].toDouble();
        }

        switch (element->type) {
        case StackElement::Root:
            m_defaultPackage = attributes[QLatin1String("package")];
            m_defaultSuperclass = attributes[QLatin1String("default-superclass")];
            element->type = StackElement::Root;
            {
                TypeSystemTypeEntry* moduleEntry = reinterpret_cast<TypeSystemTypeEntry*>(
                        m_database->findType(m_defaultPackage));
                element->entry = moduleEntry ? moduleEntry : new TypeSystemTypeEntry(m_defaultPackage, since);
                element->entry->setCodeGeneration(m_generate);
            }

            if ((m_generate == TypeEntry::GenerateForSubclass ||
                 m_generate == TypeEntry::GenerateNothing) && !m_defaultPackage.isEmpty())
                TypeDatabase::instance()->addRequiredTargetImport(m_defaultPackage);

            if (!element->entry->qualifiedCppName().isEmpty())
                m_database->addType(element->entry);
            break;
        case StackElement::LoadTypesystem: {
            QString name = attributes[nameAttribute()];
            if (name.isEmpty()) {
                m_error = QLatin1String("No typesystem name specified");
                return false;
            }
            bool generateChild = (convertBoolean(attributes[QLatin1String("generate")], QLatin1String("generate"), true) && (m_generate == TypeEntry::GenerateAll));
            if (!m_database->parseFile(name, generateChild)) {
                m_error = QStringLiteral("Failed to parse: '%1'").arg(name);
                return false;
            }
        }
        break;
        case StackElement::RejectEnumValue: {
            if (!m_currentEnum) {
                m_error = QLatin1String("<reject-enum-value> node must be used inside a <enum-type> node");
                return false;
            }
            QString name = attributes[nameAttribute()];
        } break;
        case StackElement::ReplaceType: {
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("Type replacement can only be specified for argument modifications");
                return false;
            }

            if (attributes[QLatin1String("modified-type")].isEmpty()) {
                m_error = QLatin1String("Type replacement requires 'modified-type' attribute");
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().modified_type = attributes[QLatin1String("modified-type")];
        }
        break;
        case StackElement::ConversionRule: {
            if (topElement.type != StackElement::ModifyArgument
                && topElement.type != StackElement::ValueTypeEntry
                && topElement.type != StackElement::PrimitiveTypeEntry
                && topElement.type != StackElement::ContainerTypeEntry) {
                m_error = QLatin1String("Conversion rules can only be specified for argument modification, "
                          "value-type, primitive-type or container-type conversion.");
                return false;
            }

            static QHash<QString, TypeSystem::Language> languageNames;
            if (languageNames.isEmpty()) {
                languageNames[QLatin1String("target")] = TypeSystem::TargetLangCode;
                languageNames[QLatin1String("native")] = TypeSystem::NativeCode;
            }

            QString languageAttribute = attributes[QLatin1String("class")].toLower();
            TypeSystem::Language lang = languageNames.value(languageAttribute, TypeSystem::NoLanguage);

            if (topElement.type == StackElement::ModifyArgument) {
               if (lang == TypeSystem::NoLanguage) {
                    m_error = QStringLiteral("unsupported class attribute: '%1'").arg(lang);
                    return false;
                }

                CodeSnip snip(since);
                snip.language = lang;
                m_contextStack.top()->functionMods.last().argument_mods.last().conversion_rules.append(snip);
            } else {
                if (topElement.entry->hasConversionRule() || topElement.entry->hasCustomConversion()) {
                    m_error = QLatin1String("Types can have only one conversion rule");
                    return false;
                }

                // The old conversion rule tag that uses a file containing the conversion
                // will be kept temporarily for compatibility reasons.
                QString sourceFile = attributes[QLatin1String("file")];
                if (!sourceFile.isEmpty()) {
                    if (m_generate != TypeEntry::GenerateForSubclass
                        && m_generate != TypeEntry::GenerateNothing) {

                        const char* conversionFlag = NATIVE_CONVERSION_RULE_FLAG;
                        if (lang == TypeSystem::TargetLangCode)
                            conversionFlag = TARGET_CONVERSION_RULE_FLAG;

                        QFile conversionSource(sourceFile);
                        if (conversionSource.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            topElement.entry->setConversionRule(QLatin1String(conversionFlag) + QString::fromUtf8(conversionSource.readAll()));
                        } else {
                            qCWarning(lcShiboken).noquote().nospace()
                                << "File containing conversion code for "
                                << topElement.entry->name() << " type does not exist or is not readable: "
                                << sourceFile;
                        }
                    }
                }

                CustomConversion* customConversion = new CustomConversion(static_cast<TypeEntry*>(m_current->entry));
                customConversionsForReview.append(customConversion);
            }
        }
        break;
        case StackElement::NativeToTarget: {
            if (topElement.type != StackElement::ConversionRule) {
                m_error = QLatin1String("Native to Target conversion code can only be specified for custom conversion rules.");
                return false;
            }
            m_contextStack.top()->codeSnips << CodeSnip(0);
        }
        break;
        case StackElement::TargetToNative: {
            if (topElement.type != StackElement::ConversionRule) {
                m_error = QLatin1String("Target to Native conversions can only be specified for custom conversion rules.");
                return false;
            }
            bool replace = attributes[QLatin1String("replace")] == QLatin1String("yes");
            static_cast<TypeEntry*>(m_current->entry)->customConversion()->setReplaceOriginalTargetToNativeConversions(replace);
        }
        break;
        case StackElement::AddConversion: {
            if (topElement.type != StackElement::TargetToNative) {
                m_error = QLatin1String("Target to Native conversions can only be added inside 'target-to-native' tags.");
                return false;
            }
            QString sourceTypeName = attributes[QLatin1String("type")];
            if (sourceTypeName.isEmpty()) {
                m_error = QLatin1String("Target to Native conversions must specify the input type with the 'type' attribute.");
                return false;
            }
            QString typeCheck = attributes[QLatin1String("check")];
            static_cast<TypeEntry*>(m_current->entry)->customConversion()->addTargetToNativeConversion(sourceTypeName, typeCheck);
            m_contextStack.top()->codeSnips << CodeSnip(0);
        }
        break;
        case StackElement::ModifyArgument: {
            if (topElement.type != StackElement::ModifyFunction
                && topElement.type != StackElement::AddFunction) {
                m_error = QString::fromLatin1("argument modification requires function"
                                              " modification as parent, was %1")
                          .arg(topElement.type, 0, 16);
                return false;
            }

            QString index = attributes[QLatin1String("index")];
            if (index == QLatin1String("return"))
                index = QLatin1String("0");
            else if (index == QLatin1String("this"))
                index = QLatin1String("-1");

            bool ok = false;
            int idx = index.toInt(&ok);
            if (!ok) {
                m_error = QStringLiteral("Cannot convert '%1' to integer").arg(index);
                return false;
            }

            QString replace_value = attributes[QLatin1String("replace-value")];

            if (!replace_value.isEmpty() && idx) {
                m_error = QLatin1String("replace-value is only supported for return values (index=0).");
                return false;
            }

            ArgumentModification argumentModification = ArgumentModification(idx, since);
            argumentModification.replace_value = replace_value;
            argumentModification.resetAfterUse = convertBoolean(attributes[QLatin1String("invalidate-after-use")], QLatin1String("invalidate-after-use"), false);
            m_contextStack.top()->functionMods.last().argument_mods.append(argumentModification);
        }
        break;
        case StackElement::NoNullPointers: {
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("no-null-pointer requires argument modification as parent");
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().noNullPointers = true;
            if (!m_contextStack.top()->functionMods.last().argument_mods.last().index)
                m_contextStack.top()->functionMods.last().argument_mods.last().nullPointerDefaultValue = attributes[QLatin1String("default-value")];
             else if (!attributes[QLatin1String("default-value")].isEmpty())
                qCWarning(lcShiboken) << "default values for null pointer guards are only effective for return values";

        }
        break;
        case StackElement::DefineOwnership: {
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("define-ownership requires argument modification as parent");
                return false;
            }

            static QHash<QString, TypeSystem::Language> languageNames;
            if (languageNames.isEmpty()) {
                languageNames[QLatin1String("target")] = TypeSystem::TargetLangCode;
                languageNames[QLatin1String("native")] = TypeSystem::NativeCode;
            }

            QString classAttribute = attributes[QLatin1String("class")].toLower();
            TypeSystem::Language lang = languageNames.value(classAttribute, TypeSystem::NoLanguage);
            if (lang == TypeSystem::NoLanguage) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(classAttribute);
                return false;
            }

            static QHash<QString, TypeSystem::Ownership> ownershipNames;
            if (ownershipNames.isEmpty()) {
                ownershipNames[QLatin1String("target")] = TypeSystem::TargetLangOwnership;
                ownershipNames[QLatin1String("c++")] = TypeSystem::CppOwnership;
                ownershipNames[QLatin1String("default")] = TypeSystem::DefaultOwnership;
            }

            QString ownershipAttribute = attributes[QLatin1String("owner")].toLower();
            TypeSystem::Ownership owner = ownershipNames.value(ownershipAttribute, TypeSystem::InvalidOwnership);
            if (owner == TypeSystem::InvalidOwnership) {
                m_error = QStringLiteral("unsupported owner attribute: '%1'").arg(ownershipAttribute);
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().ownerships[lang] = owner;
        }
        break;
        case StackElement::SuppressedWarning:
            if (attributes[QLatin1String("text")].isEmpty())
                qCWarning(lcShiboken) << "Suppressed warning with no text specified";
            else
                m_database->addSuppressedWarning(attributes[QLatin1String("text")]);
            break;
        case StackElement::ArgumentMap: {
            if (!(topElement.type & StackElement::CodeSnipMask)) {
                m_error = QLatin1String("Argument maps requires code injection as parent");
                return false;
            }

            bool ok;
            int pos = attributes[QLatin1String("index")].toInt(&ok);
            if (!ok) {
                m_error = QStringLiteral("Can't convert position '%1' to integer")
                          .arg(attributes[QLatin1String("position")]);
                return false;
            }

            if (pos <= 0) {
                m_error = QStringLiteral("Argument position %1 must be a positive number").arg(pos);
                return false;
            }

            QString meta_name = attributes[QLatin1String("meta-name")];
            if (meta_name.isEmpty())
                qCWarning(lcShiboken) << "Empty meta name in argument map";


            if (topElement.type == StackElement::InjectCodeInFunction)
                m_contextStack.top()->functionMods.last().snips.last().argumentMap[pos] = meta_name;
             else {
                qCWarning(lcShiboken) << "Argument maps are only useful for injection of code "
                                       "into functions.";
            }
        }
        break;
        case StackElement::Removal: {
            if (topElement.type != StackElement::ModifyFunction) {
                m_error = QLatin1String("Function modification parent required");
                return false;
            }

            static QHash<QString, TypeSystem::Language> languageNames;
            if (languageNames.isEmpty()) {
                languageNames.insert(QLatin1String("target"), TypeSystem::TargetLangAndNativeCode);
                languageNames.insert(QLatin1String("all"), TypeSystem::All);
            }

            QString languageAttribute = attributes[QLatin1String("class")].toLower();
            TypeSystem::Language lang = languageNames.value(languageAttribute, TypeSystem::NoLanguage);
            if (lang == TypeSystem::NoLanguage) {
                m_error = QStringLiteral("unsupported class attribute: '%1'").arg(languageAttribute);
                return false;
            }

            m_contextStack.top()->functionMods.last().removal = lang;
        }
        break;
        case StackElement::Rename:
        case StackElement::Access: {
            if (topElement.type != StackElement::ModifyField
                && topElement.type != StackElement::ModifyFunction
                && topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("Function, field  or argument modification parent required");
                return false;
            }

            Modification *mod = 0;
            if (topElement.type == StackElement::ModifyFunction)
                mod = &m_contextStack.top()->functionMods.last();
            else if (topElement.type == StackElement::ModifyField)
                mod = &m_contextStack.top()->fieldMods.last();

            QString modifier;
            if (element->type == StackElement::Rename) {
                modifier = QLatin1String("rename");
                QString renamed_to = attributes[QLatin1String("to")];
                if (renamed_to.isEmpty()) {
                    m_error = QLatin1String("Rename modifier requires 'to' attribute");
                    return false;
                }

                if (topElement.type == StackElement::ModifyFunction)
                    mod->setRenamedTo(renamed_to);
                else if (topElement.type == StackElement::ModifyField)
                    mod->setRenamedTo(renamed_to);
                else
                    m_contextStack.top()->functionMods.last().argument_mods.last().renamed_to = renamed_to;
            } else
                modifier = attributes[QLatin1String("modifier")].toLower();


            if (modifier.isEmpty()) {
                m_error = QLatin1String("No access modification specified");
                return false;
            }

            static QHash<QString, FunctionModification::Modifiers> modifierNames;
            if (modifierNames.isEmpty()) {
                modifierNames[QLatin1String("private")] = Modification::Private;
                modifierNames[QLatin1String("public")] = Modification::Public;
                modifierNames[QLatin1String("protected")] = Modification::Protected;
                modifierNames[QLatin1String("friendly")] = Modification::Friendly;
                modifierNames[QLatin1String("rename")] = Modification::Rename;
                modifierNames[QLatin1String("final")] = Modification::Final;
                modifierNames[QLatin1String("non-final")] = Modification::NonFinal;
            }

            if (!modifierNames.contains(modifier)) {
                m_error = QStringLiteral("Unknown access modifier: '%1'").arg(modifier);
                return false;
            }

            if (mod)
                mod->modifiers |= modifierNames[modifier];
        }
        break;
        case StackElement::RemoveArgument:
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("Removing argument requires argument modification as parent");
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().removed = true;
            break;

        case StackElement::ModifyField: {
            QString name = attributes[nameAttribute()];
            if (name.isEmpty())
                break;
            FieldModification fm;
            fm.name = name;
            fm.modifiers = 0;

            if (!convertRemovalAttribute(attributes[QLatin1String("remove")], fm, m_error))
                return false;

            QString read = attributes[QLatin1String("read")];
            QString write = attributes[QLatin1String("write")];

            if (read == QLatin1String("true")) fm.modifiers |= FieldModification::Readable;
            if (write == QLatin1String("true")) fm.modifiers |= FieldModification::Writable;

            m_contextStack.top()->fieldMods << fm;
        }
        break;
        case StackElement::AddFunction: {
            if (!(topElement.type & (StackElement::ComplexTypeEntryMask | StackElement::Root))) {
                m_error = QString::fromLatin1("Add function requires a complex type or a root tag as parent"
                                              ", was=%1").arg(topElement.type, 0, 16);
                return false;
            }
            QString signature = attributes[QLatin1String("signature")];

            signature = TypeDatabase::normalizedSignature(signature.toLocal8Bit().constData());
            if (signature.isEmpty()) {
                m_error = QLatin1String("No signature for the added function");
                return false;
            }

            QString errorString = checkSignatureError(signature, QLatin1String("add-function"));
            if (!errorString.isEmpty()) {
                m_error = errorString;
                return false;
            }

            AddedFunction func(signature, attributes[QLatin1String("return-type")], since);
            func.setStatic(attributes[QLatin1String("static")] == QLatin1String("yes"));
            if (!signature.contains(QLatin1Char('(')))
                signature += QLatin1String("()");
            m_currentSignature = signature;

            QString access = attributes[QLatin1String("access")].toLower();
            if (!access.isEmpty()) {
                if (access == QLatin1String("protected")) {
                    func.setAccess(AddedFunction::Protected);
                } else if (access == QLatin1String("public")) {
                    func.setAccess(AddedFunction::Public);
                } else {
                    m_error = QString::fromLatin1("Bad access type '%1'").arg(access);
                    return false;
                }
            }

            m_contextStack.top()->addedFunctions << func;

            FunctionModification mod(since);
            mod.signature = m_currentSignature;
            m_contextStack.top()->functionMods << mod;
        }
        break;
        case StackElement::ModifyFunction: {
            if (!(topElement.type & StackElement::ComplexTypeEntryMask)) {
                m_error = QString::fromLatin1("Modify function requires complex type as parent"
                                              ", was=%1").arg(topElement.type, 0, 16);
                return false;
            }
            QString signature = attributes[QLatin1String("signature")];

            signature = TypeDatabase::normalizedSignature(signature.toLocal8Bit().constData());
            if (signature.isEmpty()) {
                m_error = QLatin1String("No signature for modified function");
                return false;
            }

            QString errorString = checkSignatureError(signature, QLatin1String("modify-function"));
            if (!errorString.isEmpty()) {
                m_error = errorString;
                return false;
            }

            FunctionModification mod(since);
            m_currentSignature = mod.signature = signature;

            QString access = attributes[QLatin1String("access")].toLower();
            if (!access.isEmpty()) {
                if (access == QLatin1String("private"))
                    mod.modifiers |= Modification::Private;
                else if (access == QLatin1String("protected"))
                    mod.modifiers |= Modification::Protected;
                else if (access == QLatin1String("public"))
                    mod.modifiers |= Modification::Public;
                else if (access == QLatin1String("final"))
                    mod.modifiers |= Modification::Final;
                else if (access == QLatin1String("non-final"))
                    mod.modifiers |= Modification::NonFinal;
                else {
                    m_error = QString::fromLatin1("Bad access type '%1'").arg(access);
                    return false;
                }
            }

            if (convertBoolean(attributes[QLatin1String("deprecated")], QLatin1String("deprecated"), false))
                mod.modifiers |= Modification::Deprecated;

            if (!convertRemovalAttribute(attributes[QLatin1String("remove")], mod, m_error))
                return false;

            QString rename = attributes[QLatin1String("rename")];
            if (!rename.isEmpty()) {
                mod.renamedToName = rename;
                mod.modifiers |= Modification::Rename;
            }

            QString association = attributes[QLatin1String("associated-to")];
            if (!association.isEmpty())
                mod.association = association;

            mod.setIsThread(convertBoolean(attributes[QLatin1String("thread")], QLatin1String("thread"), false));
            mod.setAllowThread(convertBoolean(attributes[QLatin1String("allow-thread")], QLatin1String("allow-thread"), false));

            mod.modifiers |= (convertBoolean(attributes[QLatin1String("virtual-slot")], QLatin1String("virtual-slot"), false) ? Modification::VirtualSlot : 0);

            m_contextStack.top()->functionMods << mod;
        }
        break;
        case StackElement::ReplaceDefaultExpression:
            if (!(topElement.type & StackElement::ModifyArgument)) {
                m_error = QLatin1String("Replace default expression only allowed as child of argument modification");
                return false;
            }

            if (attributes[QLatin1String("with")].isEmpty()) {
                m_error = QLatin1String("Default expression replaced with empty string. Use remove-default-expression instead.");
                return false;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().replacedDefaultExpression = attributes[QLatin1String("with")];
            break;
        case StackElement::RemoveDefaultExpression:
            m_contextStack.top()->functionMods.last().argument_mods.last().removedDefaultExpression = true;
            break;
        case StackElement::CustomMetaConstructor:
        case StackElement::CustomMetaDestructor: {
            CustomFunction *func = new CustomFunction(attributes[nameAttribute()]);
            func->paramName = attributes[QLatin1String("param-name")];
            element->value.customFunction = func;
        }
        break;
        case StackElement::ReferenceCount: {
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("reference-count must be child of modify-argument");
                return false;
            }

            ReferenceCount rc;

            static QHash<QString, ReferenceCount::Action> actions;
            if (actions.isEmpty()) {
                actions[QLatin1String("add")] = ReferenceCount::Add;
                actions[QLatin1String("add-all")] = ReferenceCount::AddAll;
                actions[QLatin1String("remove")] = ReferenceCount::Remove;
                actions[QLatin1String("set")] = ReferenceCount::Set;
                actions[QLatin1String("ignore")] = ReferenceCount::Ignore;
            }
            rc.action = actions.value(attributes[QLatin1String("action")].toLower(), ReferenceCount::Invalid);
            rc.varName = attributes[QLatin1String("variable-name")];

            if (rc.action == ReferenceCount::Invalid) {
                m_error = QLatin1String("unrecognized value for action attribute. supported actions:");
                foreach (const QString &action, actions.keys())
                    m_error += QLatin1Char(' ') + action;
            }

            m_contextStack.top()->functionMods.last().argument_mods.last().referenceCounts.append(rc);
        }
        break;

        case StackElement::ParentOwner: {
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = QLatin1String("parent-policy must be child of modify-argument");
                return false;
            }

            ArgumentOwner ao;

            QString index = attributes[QLatin1String("index")];
            if (index == QLatin1String("return"))
                index = QLatin1String("0");
            else if (index == QLatin1String("this"))
                index = QLatin1String("-1");

            bool ok = false;
            int idx = index.toInt(&ok);
            if (!ok) {
                m_error = QStringLiteral("Cannot convert '%1' to integer").arg(index);
                return false;
            }

            static QHash<QString, ArgumentOwner::Action> actions;
            if (actions.isEmpty()) {
                actions[QLatin1String("add")] = ArgumentOwner::Add;
                actions[QLatin1String("remove")] = ArgumentOwner::Remove;
            }

            ao.action = actions.value(attributes[QLatin1String("action")].toLower(), ArgumentOwner::Invalid);
            if (!ao.action) {
                m_error = QLatin1String("Invalid parent actionr");
                return false;
            }
            ao.index = idx;
            m_contextStack.top()->functionMods.last().argument_mods.last().owner = ao;
        }
        break;


        case StackElement::InjectCode: {
            if (!(topElement.type & StackElement::ComplexTypeEntryMask)
                && (topElement.type != StackElement::AddFunction)
                && (topElement.type != StackElement::ModifyFunction)
                && (topElement.type != StackElement::Root)) {
                m_error = QLatin1String("wrong parent type for code injection");
                return false;
            }

            static QHash<QString, TypeSystem::Language> languageNames;
            if (languageNames.isEmpty()) {
                languageNames[QLatin1String("target")] = TypeSystem::TargetLangCode; // em algum lugar do cpp
                languageNames[QLatin1String("native")] = TypeSystem::NativeCode; // em algum lugar do cpp
                languageNames[QLatin1String("shell")] = TypeSystem::ShellCode; // coloca no header, mas antes da declaracao da classe
                languageNames[QLatin1String("shell-declaration")] = TypeSystem::ShellDeclaration; // coloca no header, dentro da declaracao da classe
                languageNames[QLatin1String("library-initializer")] = TypeSystem::PackageInitializer;
                languageNames[QLatin1String("destructor-function")] = TypeSystem::DestructorFunction;
                languageNames[QLatin1String("constructors")] = TypeSystem::Constructors;
                languageNames[QLatin1String("interface")] = TypeSystem::Interface;
            }

            QString className = attributes[QLatin1String("class")].toLower();
            if (!languageNames.contains(className)) {
                m_error = QStringLiteral("Invalid class specifier: '%1'").arg(className);
                return false;
            }


            static QHash<QString, TypeSystem::CodeSnipPosition> positionNames;
            if (positionNames.isEmpty()) {
                positionNames.insert(QLatin1String("beginning"), TypeSystem::CodeSnipPositionBeginning);
                positionNames.insert(QLatin1String("end"), TypeSystem::CodeSnipPositionEnd);
                // QtScript
                positionNames.insert(QLatin1String("declaration"), TypeSystem::CodeSnipPositionDeclaration);
                positionNames.insert(QLatin1String("prototype-initialization"), TypeSystem::CodeSnipPositionPrototypeInitialization);
                positionNames.insert(QLatin1String("constructor-initialization"), TypeSystem::CodeSnipPositionConstructorInitialization);
                positionNames.insert(QLatin1String("constructor"), TypeSystem::CodeSnipPositionConstructor);
            }

            QString position = attributes[QLatin1String("position")].toLower();
            if (!positionNames.contains(position)) {
                m_error = QStringLiteral("Invalid position: '%1'").arg(position);
                return false;
            }

            CodeSnip snip(since);
            snip.language = languageNames[className];
            snip.position = positionNames[position];
            bool in_file = false;

            QString file_name = attributes[QLatin1String("file")];

            //Handler constructor....
            if (m_generate != TypeEntry::GenerateForSubclass &&
                m_generate != TypeEntry::GenerateNothing &&
                !file_name.isEmpty()) {
                if (QFile::exists(file_name)) {
                    QFile codeFile(file_name);
                    if (codeFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
                        QString content = QLatin1String("// ========================================================================\n"
                                                        "// START of custom code block [file: ");
                        content += file_name;
                        content += QLatin1String("]\n");
                        content += QString::fromUtf8(codeFile.readAll());
                        content += QLatin1String("\n// END of custom code block [file: ");
                        content += file_name;
                        content += QLatin1String("]\n// ========================================================================\n");
                        snip.addCode(content);
                        in_file = true;
                    }
                } else {
                    qCWarning(lcShiboken).noquote().nospace()
                        << "File for inject code not exist: " << QDir::toNativeSeparators(file_name);
                }

            }

            if (snip.language == TypeSystem::Interface && topElement.type != StackElement::InterfaceTypeEntry) {
                m_error = QLatin1String("Interface code injections must be direct child of an interface type entry");
                return false;
            }

            if (topElement.type == StackElement::ModifyFunction || topElement.type == StackElement::AddFunction) {
                FunctionModification mod = m_contextStack.top()->functionMods.last();
                if (snip.language == TypeSystem::ShellDeclaration) {
                    m_error = QLatin1String("no function implementation in shell declaration in which to inject code");
                    return false;
                }

                m_contextStack.top()->functionMods.last().snips << snip;
                if (in_file)
                    m_contextStack.top()->functionMods.last().modifiers |= FunctionModification::CodeInjection;
                element->type = StackElement::InjectCodeInFunction;
            } else if (topElement.type == StackElement::Root) {
                element->entry->addCodeSnip(snip);
            } else if (topElement.type != StackElement::Root) {
                m_contextStack.top()->codeSnips << snip;
            }

        }
        break;
        case StackElement::Include: {
            QString location = attributes[QLatin1String("location")].toLower();

            static QHash<QString, Include::IncludeType> locationNames;
            if (locationNames.isEmpty()) {
                locationNames[QLatin1String("global")] = Include::IncludePath;
                locationNames[QLatin1String("local")] = Include::LocalPath;
                locationNames[QLatin1String("target")] = Include::TargetLangImport;
            }

            if (!locationNames.contains(location)) {
                m_error = QStringLiteral("Location not recognized: '%1'").arg(location);
                return false;
            }

            Include::IncludeType loc = locationNames[location];
            Include inc(loc, attributes[QLatin1String("file-name")]);

            ComplexTypeEntry *ctype = static_cast<ComplexTypeEntry *>(element->entry);
            if (topElement.type & (StackElement::ComplexTypeEntryMask | StackElement::PrimitiveTypeEntry)) {
                element->entry->setInclude(inc);
            } else if (topElement.type == StackElement::ExtraIncludes) {
                element->entry->addExtraInclude(inc);
            } else {
                m_error = QLatin1String("Only supported parent tags are primitive-type, complex types or extra-includes");
                return false;
            }

            inc = ctype->include();
            IncludeList lst = ctype->extraIncludes();
            ctype = ctype->designatedInterface();
            if (ctype) {
                ctype->setExtraIncludes(lst);
                ctype->setInclude(inc);
            }
        }
        break;
        case StackElement::Rejection: {
            QString cls = attributes[QLatin1String("class")];
            QString function = attributes[QLatin1String("function-name")];
            QString field = attributes[QLatin1String("field-name")];
            QString enum_ = attributes[QLatin1String("enum-name")];
            if (cls == QLatin1String("*") && function == QLatin1String("*") && field == QLatin1String("*") && enum_ == QLatin1String("*")) {
                m_error = QLatin1String("bad reject entry, neither 'class', 'function-name' nor "
                          "'field' specified");
                return false;
            }
            m_database->addRejection(cls, function, field, enum_);
        }
        break;
        case StackElement::Template:
            element->value.templateEntry = new TemplateEntry(attributes[nameAttribute()], since);
            break;
        case StackElement::TemplateInstanceEnum:
            if (!(topElement.type & StackElement::CodeSnipMask) &&
                (topElement.type != StackElement::Template) &&
                (topElement.type != StackElement::CustomMetaConstructor) &&
                (topElement.type != StackElement::CustomMetaDestructor) &&
                (topElement.type != StackElement::NativeToTarget) &&
                (topElement.type != StackElement::AddConversion) &&
                (topElement.type != StackElement::ConversionRule)) {
                m_error = QLatin1String("Can only insert templates into code snippets, templates, custom-constructors, "\
                          "custom-destructors, conversion-rule, native-to-target or add-conversion tags.");
                return false;
            }
            element->value.templateInstance = new TemplateInstance(attributes[nameAttribute()], since);
            break;
        case StackElement::Replace:
            if (topElement.type != StackElement::TemplateInstanceEnum) {
                m_error = QLatin1String("Can only insert replace rules into insert-template.");
                return false;
            }
            element->parent->value.templateInstance->addReplaceRule(attributes[QLatin1String("from")], attributes[QLatin1String("to")]);
            break;
        default:
            break; // nada
        };
    }

    m_current = element;
    return true;
}

PrimitiveTypeEntry *PrimitiveTypeEntry::basicReferencedTypeEntry() const
{
    if (!m_referencedTypeEntry)
        return 0;

    PrimitiveTypeEntry *baseReferencedTypeEntry = m_referencedTypeEntry->basicReferencedTypeEntry();
    if (baseReferencedTypeEntry)
        return baseReferencedTypeEntry;
    else
        return m_referencedTypeEntry;
}

typedef QHash<const PrimitiveTypeEntry*, QString> PrimitiveTypeEntryTargetLangPackageMap;
Q_GLOBAL_STATIC(PrimitiveTypeEntryTargetLangPackageMap, primitiveTypeEntryTargetLangPackages);

void PrimitiveTypeEntry::setTargetLangPackage(const QString& package)
{
    primitiveTypeEntryTargetLangPackages()->insert(this, package);
}
QString PrimitiveTypeEntry::targetLangPackage() const
{
    if (!primitiveTypeEntryTargetLangPackages()->contains(this))
        return this->::TypeEntry::targetLangPackage();
    return primitiveTypeEntryTargetLangPackages()->value(this);
}

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
        if (mod.signature == signature)
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

// The things we do not to break the ABI...
typedef QHash<const ComplexTypeEntry*, QString> ComplexTypeEntryDefaultConstructorMap;
Q_GLOBAL_STATIC(ComplexTypeEntryDefaultConstructorMap, complexTypeEntryDefaultConstructors);

void ComplexTypeEntry::setDefaultConstructor(const QString& defaultConstructor)
{
    if (!defaultConstructor.isEmpty())
        complexTypeEntryDefaultConstructors()->insert(this, defaultConstructor);
}
QString ComplexTypeEntry::defaultConstructor() const
{
    if (!complexTypeEntryDefaultConstructors()->contains(this))
        return QString();
    return complexTypeEntryDefaultConstructors()->value(this);
}
bool ComplexTypeEntry::hasDefaultConstructor() const
{
    return complexTypeEntryDefaultConstructors()->contains(this);
}

QString ContainerTypeEntry::targetLangPackage() const
{
    return QString();
}

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

QString EnumTypeEntry::targetLangQualifier() const
{
    TypeEntry *te = TypeDatabase::instance()->findType(m_qualifier);
    if (te)
        return te->targetLangName();
    else
        return m_qualifier;
}

QString EnumTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

QString FlagsTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

void EnumTypeEntry::addEnumValueRedirection(const QString &rejected, const QString &usedValue)
{
    m_enumRedirections << EnumValueRedirection(rejected, usedValue);
}

QString EnumTypeEntry::enumValueRedirection(const QString &value) const
{
    for (int i = 0; i < m_enumRedirections.size(); ++i)
        if (m_enumRedirections.at(i).rejected == value)
            return m_enumRedirections.at(i).used;
    return QString();
}

QString FlagsTypeEntry::qualifiedTargetLangName() const
{
    return targetLangPackage() + QLatin1Char('.') + m_enum->targetLangQualifier()
        + QLatin1Char('.') + targetLangName();
}

/*!
 * The Visual Studio 2002 compiler doesn't support these symbols,
 * which our typedefs unforntuatly expand to.
 */
QString fixCppTypeName(const QString &name)
{
    if (name == QLatin1String("long long"))
        return QLatin1String("qint64");
    else if (name == QLatin1String("unsigned long long"))
        return QLatin1String("quint64");
    return name;
}

QString TemplateInstance::expandCode() const
{
    TemplateEntry *templateEntry = TypeDatabase::instance()->findTemplate(m_name);
    if (templateEntry) {
        typedef QHash<QString, QString>::const_iterator ConstIt;
        QString code = templateEntry->code();
        for (ConstIt it = replaceRules.begin(), end = replaceRules.end(); it != end; ++it)
            code.replace(it.key(), it.value());
        while (!code.isEmpty() && code.at(code.size() - 1).isSpace())
            code.chop(1);
        QString result = QLatin1String("// TEMPLATE - ") + m_name + QLatin1String(" - START");
        if (!code.startsWith(QLatin1Char('\n')))
            result += QLatin1Char('\n');
        result += code;
        result += QLatin1String("\n// TEMPLATE - ") + m_name + QLatin1String(" - END");
        return result;
    } else {
        qCWarning(lcShiboken).noquote().nospace()
            << "insert-template referring to non-existing template '" << m_name << '\'';
    }

    return QString();
}


QString CodeSnipAbstract::code() const
{
    QString res;
    foreach (const CodeSnipFragment &codeFrag, codeList)
        res.append(codeFrag.code());

    return res;
}

QString CodeSnipFragment::code() const
{
    if (m_instance)
        return m_instance->expandCode();
    else
        return m_code;
}

QString FunctionModification::toString() const
{
    QString str = signature + QLatin1String("->");
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
        foreach (const CodeSnip &s, snips) {
            str += QLatin1String("\n//code injection:\n");
            str += s.code();
        }
    }

    if (modifiers & Rename) str += QLatin1String("renamed:") + renamedToName;

    if (modifiers & Deprecated) str += QLatin1String("deprecate");

    if (modifiers & ReplaceExpression) str += QLatin1String("replace-expression");

    return str;
}

bool FunctionModification::operator!=(const FunctionModification& other) const
{
    return !(*this == other);
}

bool FunctionModification::operator==(const FunctionModification& other) const
{
    if (signature != other.signature)
        return false;

    if (association != other.association)
        return false;

    if (modifiers != other.modifiers)
        return false;

    if (removal != other.removal)
        return false;

    if (m_thread != other.m_thread)
        return false;

    if (m_allowThread != other.m_allowThread)
        return false;

    if (m_version != other.m_version)
        return false;

    return true;
}

static AddedFunction::TypeInfo parseType(const QString& signature, int startPos = 0, int* endPos = 0)
{
    AddedFunction::TypeInfo result;
    QRegExp regex(QLatin1String("\\w"));
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

AddedFunction::AddedFunction(QString signature, QString returnType, double vr) : m_access(Public), m_version(vr)
{
    Q_ASSERT(!returnType.isEmpty());
    m_returnType = parseType(returnType);
    signature = signature.trimmed();
    int endPos = signature.indexOf(QLatin1Char('('));
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
        m_isConst = signature.right(signatureLength - endPos).contains(QLatin1String("const"));
    }
}

#ifndef QT_NO_DEBUG_STREAM
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

QString ComplexTypeEntry::targetLangApiName() const
{
    return strings_jobject;
}
QString StringTypeEntry::targetLangApiName() const
{
    return strings_jobject;
}
QString StringTypeEntry::targetLangName() const
{
    return strings_String;
}
QString StringTypeEntry::targetLangPackage() const
{
    return QString();
}
QString CharTypeEntry::targetLangApiName() const
{
    return strings_jchar;
}
QString CharTypeEntry::targetLangName() const
{
    return strings_char;
}
QString VariantTypeEntry::targetLangApiName() const
{
    return strings_jobject;
}
QString VariantTypeEntry::targetLangName() const
{
    return strings_Object;
}
QString VariantTypeEntry::targetLangPackage() const
{
    return QString();
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

static bool strLess(const char* a, const char* b)
{
    return ::strcmp(a, b) < 0;
}

bool TypeEntry::isCppPrimitive() const
{
    if (!isPrimitive())
        return false;

    const PrimitiveTypeEntry *referencedType =
        static_cast<const PrimitiveTypeEntry *>(this)->basicReferencedTypeEntry();
    QByteArray typeName = (referencedType ? referencedType->name() : m_name).toUtf8();

    if (typeName.contains(' ') || m_type == VoidType)
        return true;
    // Keep this sorted!!
    static const char* cppTypes[] = { "bool", "char", "double", "float", "int", "long", "long long", "short", "wchar_t" };
    const int N = sizeof(cppTypes)/sizeof(char*);

    const char** res = qBinaryFind(&cppTypes[0], &cppTypes[N], typeName.constData(), strLess);

    return res != &cppTypes[N];
}

// Again, stuff to avoid ABI breakage.
typedef QHash<const TypeEntry*, CustomConversion*> TypeEntryCustomConversionMap;
Q_GLOBAL_STATIC(TypeEntryCustomConversionMap, typeEntryCustomConversionMap);

TypeEntry::~TypeEntry()
{
    if (typeEntryCustomConversionMap()->contains(this)) {
        CustomConversion* customConversion = typeEntryCustomConversionMap()->value(this);
        typeEntryCustomConversionMap()->remove(this);
        delete customConversion;
    }
}

bool TypeEntry::hasCustomConversion() const
{
    return typeEntryCustomConversionMap()->contains(this);
}
void TypeEntry::setCustomConversion(CustomConversion* customConversion)
{
    if (customConversion)
        typeEntryCustomConversionMap()->insert(this, customConversion);
    else if (typeEntryCustomConversionMap()->contains(this))
        typeEntryCustomConversionMap()->remove(this);
}
CustomConversion* TypeEntry::customConversion() const
{
    if (typeEntryCustomConversionMap()->contains(this))
        return typeEntryCustomConversionMap()->value(this);
    return 0;
}

/*
static void injectCode(ComplexTypeEntry *e,
                       const char *signature,
                       const QByteArray &code,
                       const ArgumentMap &args)
{
    CodeSnip snip;
    snip.language = TypeSystem::NativeCode;
    snip.position = CodeSnip::Beginning;
    snip.addCode(QString::fromLatin1(code));
    snip.argumentMap = args;

    FunctionModification mod;
    mod.signature = QMetaObject::normalizedSignature(signature);
    mod.snips << snip;
    mod.modifiers = Modification::CodeInjection;
}
*/

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
    foreach (TargetToNativeConversion* targetToNativeConversion, m_d->targetToNativeConversions)
        delete targetToNativeConversion;
    m_d->targetToNativeConversions.clear();
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
