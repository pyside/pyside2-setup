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

#include "generator.h"
#include "ctypenames.h"
#include "abstractmetalang.h"
#include "parser/codemodel.h"
#include "messages.h"
#include "reporthandler.h"
#include "fileout.h"
#include "apiextractor.h"
#include "typesystem.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QDebug>
#include <typedatabase.h>

/**
 * DefaultValue is used for storing default values of types for which code is
 * generated in different contexts:
 *
 * Context             | Example: "Class *"            | Example: "Class" with default Constructor
 * --------------------+-------------------------------+------------------------------------------
 * Variable            |  var{nullptr};                | var;
 * initializations     |                               |
 * --------------------+-------------------------------+------------------------------------------
 * Return values       | return nullptr;               | return {}
 * --------------------+-------------------------------+------------------------------------------
 * constructor         | static_cast<Class *>(nullptr) | Class()
 * arguments lists     |                               |
 * (recursive, precise |                               |
 * matching).          |                               |
 */

DefaultValue::DefaultValue(Type t, QString value) :
    m_type(t), m_value(std::move(value))
{
}

DefaultValue::DefaultValue(QString customValue) :
    m_type(Custom), m_value(std::move(customValue))
{
}

QString DefaultValue::returnValue() const
{
    switch (m_type) {
    case DefaultValue::Error:
        return QLatin1String("#error");
    case DefaultValue::Boolean:
        return QLatin1String("false");
    case DefaultValue::CppScalar:
        return QLatin1String("0");
    case DefaultValue::Custom:
    case DefaultValue::Enum:
        return m_value;
    case DefaultValue::Pointer:
        return QLatin1String("nullptr");
    case DefaultValue::Void:
        return QString();
    case DefaultValue::DefaultConstructorWithDefaultValues:
        return m_value + QLatin1String("()");
    case DefaultValue::DefaultConstructor:
        break;
    }
    return QLatin1String("{}");
}

QString DefaultValue::initialization() const
{
    switch (m_type) {
    case DefaultValue::Error:
        return QLatin1String("#error");
    case DefaultValue::Boolean:
        return QLatin1String("{false}");
    case DefaultValue::CppScalar:
        return QLatin1String("{0}");
    case DefaultValue::Custom:
        return QLatin1String(" = ") + m_value;
    case DefaultValue::Enum:
        return QLatin1Char('{') + m_value + QLatin1Char('}');
    case DefaultValue::Pointer:
        return QLatin1String("{nullptr}");
    case DefaultValue::Void:
        Q_ASSERT(false);
        break;
    case DefaultValue::DefaultConstructor:
    case DefaultValue::DefaultConstructorWithDefaultValues:
        break;
    }
    return QString();
}

QString DefaultValue::constructorParameter() const
{
    switch (m_type) {
    case DefaultValue::Error:
        return QLatin1String("#error");
    case DefaultValue::Boolean:
        return QLatin1String("false");
    case DefaultValue::CppScalar: {
        // PYSIDE-846: Use static_cast in case of "unsigned long" and similar
        const QString cast = m_value.contains(QLatin1Char(' '))
            ? QLatin1String("static_cast<") + m_value + QLatin1Char('>')
            : m_value;
        return cast + QLatin1String("(0)");
    }
    case DefaultValue::Custom:
    case DefaultValue::Enum:
        return m_value;
    case DefaultValue::Pointer:
        // Be precise here to be able to differentiate between constructors
        // taking different pointer types, cf
        // QTreeWidgetItemIterator(QTreeWidget *) and
        // QTreeWidgetItemIterator(QTreeWidgetItemIterator *).
        return QLatin1String("static_cast<") + m_value + QLatin1String("*>(nullptr)");
    case DefaultValue::Void:
        Q_ASSERT(false);
        break;
    case DefaultValue::DefaultConstructor:
    case DefaultValue::DefaultConstructorWithDefaultValues:
        break;
    }
    return m_value + QLatin1String("()");
}

QString GeneratorContext::smartPointerWrapperName() const
{
    Q_ASSERT(m_type == SmartPointer);
    return m_preciseClassType->cppSignature();
}

struct Generator::GeneratorPrivate
{
    const ApiExtractor *apiextractor = nullptr;
    QString outDir;
    // License comment
    QString licenseComment;
    QString moduleName;
    QStringList instantiatedContainersNames;
    QVector<const AbstractMetaType *> instantiatedContainers;
    QVector<const AbstractMetaType *> instantiatedSmartPointers;
    AbstractMetaClassList m_invisibleTopNamespaces;
};

Generator::Generator() : m_d(new GeneratorPrivate)
{
}

Generator::~Generator()
{
    delete m_d;
}

bool Generator::setup(const ApiExtractor &extractor)
{
    m_d->apiextractor = &extractor;
    const auto moduleEntry = TypeDatabase::instance()->defaultTypeSystemType();
    if (!moduleEntry || !moduleEntry->generateCode()) {
        qCWarning(lcShiboken) << "Couldn't find the package name!!";
        return false;
    }

    collectInstantiatedContainersAndSmartPointers();

    for (auto c : classes()) {
        if (c->enclosingClass() == nullptr && c->isInvisibleNamespace()) {
            m_d->m_invisibleTopNamespaces.append(c);
            c->invisibleNamespaceRecursion([&](AbstractMetaClass *ic) {
                m_d->m_invisibleTopNamespaces.append(ic);
            });
        }
    }

    return doSetup();
}

QString Generator::getSimplifiedContainerTypeName(const AbstractMetaType *type)
{
    const QString signature = type->cppSignature();
    if (!type->typeEntry()->isContainer() && !type->typeEntry()->isSmartPointer())
        return signature;
    QString typeName = signature;
    if (type->isConstant())
        typeName.remove(0, sizeof("const ") / sizeof(char) - 1);
    switch (type->referenceType()) {
    case NoReference:
        break;
    case LValueReference:
        typeName.chop(1);
        break;
    case RValueReference:
        typeName.chop(2);
        break;
    }
    while (typeName.endsWith(QLatin1Char('*')) || typeName.endsWith(QLatin1Char(' ')))
        typeName.chop(1);
    return typeName;
}

// Strip a "const QSharedPtr<const Foo> &" or similar to "QSharedPtr<Foo>" (PYSIDE-1016/454)
const AbstractMetaType *canonicalSmartPtrInstantiation(const AbstractMetaType *type)
{
    AbstractMetaTypeList instantiations = type->instantiations();
    Q_ASSERT(instantiations.size() == 1);
    const bool needsFix = type->isConstant() || type->referenceType() != NoReference;
    const bool pointeeNeedsFix = instantiations.constFirst()->isConstant();
    if (!needsFix && !pointeeNeedsFix)
        return type;
    auto fixedType = type->copy();
    fixedType->setReferenceType(NoReference);
    fixedType->setConstant(false);
    if (pointeeNeedsFix) {
        auto fixedPointeeType = instantiations.constFirst()->copy();
        fixedPointeeType->setConstant(false);
        fixedType->setInstantiations(AbstractMetaTypeList(1, fixedPointeeType));
    }
    return fixedType;
}

static inline const TypeEntry *pointeeTypeEntry(const AbstractMetaType *smartPtrType)
{
    return smartPtrType->instantiations().constFirst()->typeEntry();
}

void Generator::addInstantiatedContainersAndSmartPointers(const AbstractMetaType *type,
                                                          const QString &context)
{
    if (!type)
        return;
    const AbstractMetaTypeList &instantiations = type->instantiations();
    for (const AbstractMetaType *t : instantiations)
        addInstantiatedContainersAndSmartPointers(t, context);
    const auto typeEntry = type->typeEntry();
    const bool isContainer = typeEntry->isContainer();
    if (!isContainer
        && !(typeEntry->isSmartPointer() && typeEntry->generateCode())) {
        return;
    }
    if (type->hasTemplateChildren()) {
        QString piece = isContainer ? QStringLiteral("container") : QStringLiteral("smart pointer");
        QString warning =
                QString::fromLatin1("Skipping instantiation of %1 '%2' because it has template"
                               " arguments.").arg(piece, type->originalTypeDescription());
        if (!context.isEmpty())
            warning.append(QStringLiteral(" Calling context: %1").arg(context));

        qCWarning(lcShiboken).noquote().nospace() << warning;
        return;

    }
    QString typeName = getSimplifiedContainerTypeName(type);
    if (isContainer) {
        if (!m_d->instantiatedContainersNames.contains(typeName)) {
            m_d->instantiatedContainersNames.append(typeName);
            m_d->instantiatedContainers.append(type);
        }
    } else {
        // Is smart pointer. Check if the (const?) pointee is already known
        auto pt = pointeeTypeEntry(type);
        const bool present =
            std::any_of(m_d->instantiatedSmartPointers.cbegin(), m_d->instantiatedSmartPointers.cend(),
                        [pt] (const AbstractMetaType *t) {
                            return pointeeTypeEntry(t) == pt;
                        });
        if (!present)
            m_d->instantiatedSmartPointers.append(canonicalSmartPtrInstantiation(type));
    }

}

void Generator::collectInstantiatedContainersAndSmartPointers(const AbstractMetaFunction *func)
{
    addInstantiatedContainersAndSmartPointers(func->type(), func->signature());
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument *arg : arguments)
        addInstantiatedContainersAndSmartPointers(arg->type(), func->signature());
}

void Generator::collectInstantiatedContainersAndSmartPointers(const AbstractMetaClass *metaClass)
{
    if (!metaClass->typeEntry()->generateCode())
        return;
    const AbstractMetaFunctionList &funcs = metaClass->functions();
    for (const AbstractMetaFunction *func : funcs)
        collectInstantiatedContainersAndSmartPointers(func);
    const AbstractMetaFieldList &fields = metaClass->fields();
    for (const AbstractMetaField *field : fields)
        addInstantiatedContainersAndSmartPointers(field->type(), field->name());
    const AbstractMetaClassList &innerClasses = metaClass->innerClasses();
    for (AbstractMetaClass *innerClass : innerClasses)
        collectInstantiatedContainersAndSmartPointers(innerClass);
}

void Generator::collectInstantiatedContainersAndSmartPointers()
{
    for (const AbstractMetaFunction *func : globalFunctions())
        collectInstantiatedContainersAndSmartPointers(func);
    for (const AbstractMetaClass *metaClass : classes())
        collectInstantiatedContainersAndSmartPointers(metaClass);
}

QVector<const AbstractMetaType *> Generator::instantiatedContainers() const
{
    return m_d->instantiatedContainers;
}

QVector<const AbstractMetaType *> Generator::instantiatedSmartPointers() const
{
    return m_d->instantiatedSmartPointers;
}

Generator::OptionDescriptions Generator::options() const
{
    return OptionDescriptions();
}

bool Generator::handleOption(const QString & /* key */, const QString & /* value */)
{
    return false;
}

const AbstractMetaClassList &Generator::classes() const
{
    return m_d->apiextractor->classes();
}

const AbstractMetaClassList &Generator::invisibleTopNamespaces() const
{
    return m_d->m_invisibleTopNamespaces;
}

AbstractMetaClassList Generator::classesTopologicalSorted(const Dependencies &additionalDependencies) const
{
    return m_d->apiextractor->classesTopologicalSorted(additionalDependencies);
}

const AbstractMetaFunctionList &Generator::globalFunctions() const
{
    return m_d->apiextractor->globalFunctions();
}

const AbstractMetaEnumList &Generator::globalEnums() const
{
    return m_d->apiextractor->globalEnums();
}

PrimitiveTypeEntryList Generator::primitiveTypes() const
{
    return m_d->apiextractor->primitiveTypes();
}

ContainerTypeEntryList Generator::containerTypes() const
{
    return m_d->apiextractor->containerTypes();
}

const AbstractMetaEnum *Generator::findAbstractMetaEnum(const TypeEntry *typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum *Generator::findAbstractMetaEnum(const AbstractMetaType *metaType) const
{
    return m_d->apiextractor->findAbstractMetaEnum(metaType->typeEntry());
}

QString Generator::licenseComment() const
{
    return m_d->licenseComment;
}

void Generator::setLicenseComment(const QString &licenseComment)
{
    m_d->licenseComment = licenseComment;
}

QString Generator::packageName() const
{
    return TypeDatabase::instance()->defaultPackageName();
}

QString Generator::moduleName() const
{
    if (m_d->moduleName.isEmpty()) {
        m_d->moduleName = packageName();
        m_d->moduleName.remove(0, m_d->moduleName.lastIndexOf(QLatin1Char('.')) + 1);
    }
    return m_d->moduleName;
}

QString Generator::outputDirectory() const
{
    return m_d->outDir;
}

void Generator::setOutputDirectory(const QString &outDir)
{
    m_d->outDir = outDir;
}

bool Generator::generateFileForContext(const GeneratorContext &context)
{
    const AbstractMetaClass *cls = context.metaClass();

    if (!shouldGenerate(cls))
        return true;

    const QString fileName = fileNameForContext(context);
    if (fileName.isEmpty())
        return true;

    QString filePath = outputDirectory() + QLatin1Char('/') + subDirectoryForClass(cls)
            + QLatin1Char('/') + fileName;
    FileOut fileOut(filePath);

    generateClass(fileOut.stream, context);

    return fileOut.done() != FileOut::Failure;
}

QString Generator::getFileNameBaseForSmartPointer(const AbstractMetaType *smartPointerType,
                                                  const AbstractMetaClass *smartPointerClass) const
{
    const AbstractMetaType *innerType = smartPointerType->getSmartPointerInnerType();
    QString fileName = smartPointerClass->qualifiedCppName().toLower();
    fileName.replace(QLatin1String("::"), QLatin1String("_"));
    fileName.append(QLatin1String("_"));
    fileName.append(innerType->name().toLower());

    return fileName;
}

GeneratorContext Generator::contextForClass(const AbstractMetaClass *c) const
{
    GeneratorContext result;
    result.m_metaClass = c;
    return result;
}

GeneratorContext Generator::contextForSmartPointer(const AbstractMetaClass *c,
                                                   const AbstractMetaType *t) const
{
    GeneratorContext result;
    result.m_metaClass = c;
    result.m_preciseClassType = t;
    result.m_type = GeneratorContext::SmartPointer;
    return result;
}

bool Generator::generate()
{
    const AbstractMetaClassList &classList = m_d->apiextractor->classes();
    for (AbstractMetaClass *cls : classList) {
        if (!generateFileForContext(contextForClass(cls)))
            return false;
    }

    const auto smartPointers = m_d->apiextractor->smartPointers();
    for (const AbstractMetaType *type : qAsConst(m_d->instantiatedSmartPointers)) {
        AbstractMetaClass *smartPointerClass =
            AbstractMetaClass::findClass(smartPointers, type->typeEntry());
        if (!smartPointerClass) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgCannotFindSmartPointer(type->cppSignature(),
                                                           smartPointers)));
            return false;
        }
        if (!generateFileForContext(contextForSmartPointer(smartPointerClass, type)))
            return false;
    }
    return finishGeneration();
}

bool Generator::shouldGenerateTypeEntry(const TypeEntry *type) const
{
    return (type->codeGeneration() & TypeEntry::GenerateTargetLang)
        && NamespaceTypeEntry::isVisibleScope(type);
}

bool Generator::shouldGenerate(const AbstractMetaClass *metaClass) const
{
    return shouldGenerateTypeEntry(metaClass->typeEntry());
}

void verifyDirectoryFor(const QString &file)
{
    QDir dir = QFileInfo(file).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("unable to create directory '%1'").arg(dir.absolutePath());
        }
    }
}

void Generator::replaceTemplateVariables(QString &code, const AbstractMetaFunction *func)
{
    const AbstractMetaClass *cpp_class = func->ownerClass();
    if (cpp_class)
        code.replace(QLatin1String("%TYPE"), cpp_class->name());

    const AbstractMetaArgumentList &argument = func->arguments();
    for (AbstractMetaArgument *arg : argument)
        code.replace(QLatin1Char('%') + QString::number(arg->argumentIndex() + 1), arg->name());

    //template values
    code.replace(QLatin1String("%RETURN_TYPE"), translateType(func->type(), cpp_class));
    code.replace(QLatin1String("%FUNCTION_NAME"), func->originalName());

    if (code.contains(QLatin1String("%ARGUMENT_NAMES"))) {
        QString str;
        QTextStream aux_stream(&str);
        writeArgumentNames(aux_stream, func, Generator::SkipRemovedArguments);
        code.replace(QLatin1String("%ARGUMENT_NAMES"), str);
    }

    if (code.contains(QLatin1String("%ARGUMENTS"))) {
        QString str;
        QTextStream aux_stream(&str);
        writeFunctionArguments(aux_stream, func, Options(SkipDefaultValues) | SkipRemovedArguments);
        code.replace(QLatin1String("%ARGUMENTS"), str);
    }
}

QTextStream &formatCode(QTextStream &s, const QString &code, Indentor &indentor)
{
    const auto lines= QStringView{code}.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        // Do not indent preprocessor lines
        if (!line.isEmpty() && !line.startsWith(QLatin1Char('#')))
            s << indentor;
        s << line << '\n';
    }
    return s;
}

AbstractMetaFunctionList Generator::implicitConversions(const TypeEntry *type) const
{
    if (type->isValue()) {
        if (const AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes(), type))
            return metaClass->implicitConversions();
    }
    return AbstractMetaFunctionList();
}

AbstractMetaFunctionList Generator::implicitConversions(const AbstractMetaType *metaType) const
{
    return implicitConversions(metaType->typeEntry());
}

bool Generator::isObjectType(const TypeEntry *type)
{
    if (type->isComplex())
        return Generator::isObjectType(static_cast<const ComplexTypeEntry *>(type));
    return type->isObject();
}
bool Generator::isObjectType(const ComplexTypeEntry *type)
{
    return type->isObject();
}
bool Generator::isObjectType(const AbstractMetaClass *metaClass)
{
    return Generator::isObjectType(metaClass->typeEntry());
}
bool Generator::isObjectType(const AbstractMetaType *metaType)
{
    return isObjectType(metaType->typeEntry());
}

bool Generator::isPointer(const AbstractMetaType *type)
{
    return type->indirections() > 0
            || type->isNativePointer()
            || type->isValuePointer();
}

bool Generator::isCString(const AbstractMetaType *type)
{
    return type->isNativePointer()
            && type->indirections() == 1
            && type->name() == QLatin1String("char");
}

bool Generator::isVoidPointer(const AbstractMetaType *type)
{
    return type->isNativePointer()
            && type->indirections() == 1
            && type->name() == QLatin1String("void");
}

QString Generator::getFullTypeName(const TypeEntry *type) const
{
    QString result = type->qualifiedCppName();
    if (type->isArray())
        type = static_cast<const ArrayTypeEntry *>(type)->nestedTypeEntry();
    if (!type->isCppPrimitive())
        result.prepend(QLatin1String("::"));
    return result;
}

QString Generator::getFullTypeName(const AbstractMetaType *type) const
{
    if (isCString(type))
        return QLatin1String("const char*");
    if (isVoidPointer(type))
        return QLatin1String("void*");
    if (type->typeEntry()->isContainer())
        return QLatin1String("::") + type->cppSignature();
    QString typeName;
    if (type->typeEntry()->isComplex() && type->hasInstantiations())
        typeName = getFullTypeNameWithoutModifiers(type);
    else
        typeName = getFullTypeName(type->typeEntry());
    return typeName + QString::fromLatin1("*").repeated(type->indirections());
}

QString Generator::getFullTypeName(const AbstractMetaClass *metaClass) const
{
    return QLatin1String("::") + metaClass->qualifiedCppName();
}

QString Generator::getFullTypeNameWithoutModifiers(const AbstractMetaType *type) const
{
    if (isCString(type))
        return QLatin1String("const char*");
    if (isVoidPointer(type))
        return QLatin1String("void*");
    if (!type->hasInstantiations())
        return getFullTypeName(type->typeEntry());
    QString typeName = type->cppSignature();
    if (type->isConstant())
        typeName.remove(0, sizeof("const ") / sizeof(char) - 1);
    switch (type->referenceType()) {
    case NoReference:
        break;
    case LValueReference:
        typeName.chop(1);
        break;
    case RValueReference:
        typeName.chop(2);
        break;
    }
    while (typeName.endsWith(QLatin1Char('*')) || typeName.endsWith(QLatin1Char(' ')))
        typeName.chop(1);
    return QLatin1String("::") + typeName;
}

DefaultValue Generator::minimalConstructor(const AbstractMetaType *type) const
{
    if (!type || (type->referenceType() ==  LValueReference && Generator::isObjectType(type)))
        return DefaultValue(DefaultValue::Error);

    if (type->isContainer()) {
        QString ctor = type->cppSignature();
        if (ctor.endsWith(QLatin1Char('*'))) {
            ctor.chop(1);
            return DefaultValue(DefaultValue::Pointer, ctor.trimmed());
        }
        if (ctor.startsWith(QLatin1String("const ")))
            ctor.remove(0, sizeof("const ") / sizeof(char) - 1);
        if (ctor.endsWith(QLatin1Char('&'))) {
            ctor.chop(1);
            ctor = ctor.trimmed();
        }
        return DefaultValue(DefaultValue::DefaultConstructor, QLatin1String("::") + ctor);
    }

    if (type->isNativePointer())
        return DefaultValue(DefaultValue::Pointer, type->typeEntry()->qualifiedCppName());
    if (Generator::isPointer(type))
        return DefaultValue(DefaultValue::Pointer, QLatin1String("::") + type->typeEntry()->qualifiedCppName());

    if (type->typeEntry()->isSmartPointer())
        return minimalConstructor(type->typeEntry());

    if (type->typeEntry()->isComplex()) {
        auto cType = static_cast<const ComplexTypeEntry *>(type->typeEntry());
        if (cType->hasDefaultConstructor())
            return DefaultValue(DefaultValue::Custom, cType->defaultConstructor());
        auto ctor = minimalConstructor(AbstractMetaClass::findClass(classes(), cType));
        if (ctor.isValid() && type->hasInstantiations()) {
            QString v = ctor.value();
            v.replace(getFullTypeName(cType), getFullTypeNameWithoutModifiers(type));
            ctor.setValue(v);
        }
        return ctor;
    }

    return minimalConstructor(type->typeEntry());
}

DefaultValue Generator::minimalConstructor(const TypeEntry *type) const
{
    if (!type)
        return DefaultValue(DefaultValue::Error);

    if (type->isCppPrimitive()) {
        const QString &name = type->qualifiedCppName();
        return name == QLatin1String("bool")
            ? DefaultValue(DefaultValue::Boolean)
            : DefaultValue(DefaultValue::CppScalar, name);
    }

    if (type->isEnum()) {
        const auto enumEntry = static_cast<const EnumTypeEntry *>(type);
        if (const auto *nullValue = enumEntry->nullValue())
            return DefaultValue(DefaultValue::Enum, nullValue->name());
        return DefaultValue(DefaultValue::Custom,
                            QLatin1String("static_cast< ::") + type->qualifiedCppName()
                            + QLatin1String(">(0)"));
    }

    if (type->isFlags()) {
        return DefaultValue(DefaultValue::Custom,
                            type->qualifiedCppName() + QLatin1String("(0)"));
    }

    if (type->isPrimitive()) {
        QString ctor = static_cast<const PrimitiveTypeEntry *>(type)->defaultConstructor();
        // If a non-C++ (i.e. defined by the user) primitive type does not have
        // a default constructor defined by the user, the empty constructor is
        // heuristically returned. If this is wrong the build of the generated
        // bindings will tell.
        return ctor.isEmpty()
            ? DefaultValue(DefaultValue::DefaultConstructorWithDefaultValues, QLatin1String("::")
                           + type->qualifiedCppName())
            : DefaultValue(DefaultValue::Custom, ctor);
    }

    if (type->isSmartPointer())
        return DefaultValue(DefaultValue::DefaultConstructor, type->qualifiedCppName());

    if (type->isComplex())
        return minimalConstructor(AbstractMetaClass::findClass(classes(), type));

    return DefaultValue(DefaultValue::Error);
}

static QString constructorCall(const QString &qualifiedCppName, const QStringList &args)
{
    return QLatin1String("::") + qualifiedCppName + QLatin1Char('(')
        + args.join(QLatin1String(", ")) + QLatin1Char(')');
}

DefaultValue Generator::minimalConstructor(const AbstractMetaClass *metaClass) const
{
    if (!metaClass)
        return DefaultValue(DefaultValue::Error);

    auto cType = static_cast<const ComplexTypeEntry *>(metaClass->typeEntry());
    if (cType->hasDefaultConstructor())
        return DefaultValue(DefaultValue::Custom, cType->defaultConstructor());

    const QString qualifiedCppName = cType->qualifiedCppName();
    // Obtain a list of constructors sorted by complexity and number of arguments
    QMultiMap<int, const AbstractMetaFunction *> candidates;
    const AbstractMetaFunctionList &constructors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
    for (const AbstractMetaFunction *ctor : constructors) {
        if (!ctor->isUserAdded() && !ctor->isPrivate()
            && ctor->functionType() == AbstractMetaFunction::ConstructorFunction) {
            // No arguments: Default constructible
            const auto &arguments = ctor->arguments();
            if (arguments.isEmpty()) {
                return DefaultValue(DefaultValue::DefaultConstructor,
                                    QLatin1String("::") + qualifiedCppName);
            }
            // First argument has unmodified default: Default constructible with values
            if (arguments.constFirst()->hasUnmodifiedDefaultValueExpression()) {
                return DefaultValue(DefaultValue::DefaultConstructorWithDefaultValues,
                                    QLatin1String("::") + qualifiedCppName);
            }
            // Examine arguments, exclude functions taking a self parameter
            bool simple = true;
            bool suitable = true;
            for (int i = 0, size = arguments.size();
                 suitable && i < size && !arguments.at(i)->hasOriginalDefaultValueExpression(); ++i) {
                const AbstractMetaArgument *arg = arguments.at(i);
                const TypeEntry *aType = arg->type()->typeEntry();
                suitable &= aType != cType;
                simple &= aType->isCppPrimitive() || aType->isEnum() || isPointer(arg->type());
            }
            if (suitable)
                candidates.insert(arguments.size() + (simple ? 0 : 100), ctor);
        }
    }

    for (auto it = candidates.cbegin(), end = candidates.cend(); it != end; ++it) {
        const AbstractMetaArgumentList &arguments = it.value()->arguments();
        QStringList args;
        bool ok = true;
        for (int i =0, size = arguments.size(); ok && i < size; ++i) {
            const AbstractMetaArgument *arg = arguments.at(i);
            if (arg->hasModifiedDefaultValueExpression()) {
                args << arg->defaultValueExpression(); // Spell out modified values
                break;
            }
            if (arg->hasOriginalDefaultValueExpression())
                break;
            auto argValue = minimalConstructor(arg->type());
            ok &= argValue.isValid();
            args << argValue.constructorParameter();
        }
        if (ok)
            return DefaultValue(DefaultValue::Custom, constructorCall(qualifiedCppName, args));
    }

    return DefaultValue(DefaultValue::Error);
}

// Should int be used for a (protected) enum when generating the public wrapper?
bool Generator::useEnumAsIntForProtectedHack(const AbstractMetaType *metaType) const
{
    if (metaType->isFlags())
        return true;
    if (!metaType->isEnum())
        return false;
    const AbstractMetaEnum *metaEnum = findAbstractMetaEnum(metaType);
    if (!metaEnum)
        return true;
    if (metaEnum->attributes() & AbstractMetaAttributes::Public) // No reason, type is public
        return false;
    // Only ordinary C-enums can be used as int, scoped enums fail when used
    // as function arguments.
    if (metaEnum->enumKind() == EnumKind::EnumClass)
        qWarning(lcShiboken, "%s", qPrintable(msgCannotUseEnumAsInt(metaEnum->name())));
    return true;
}

QString Generator::translateType(const AbstractMetaType *cType,
                                 const AbstractMetaClass *context,
                                 Options options) const
{
    QString s;
    static int constLen = strlen("const");

    if (context && cType &&
        context->typeEntry()->isGenericClass() &&
        cType->originalTemplateType()) {
        cType = cType->originalTemplateType();
    }

    if (!cType) {
        s = QLatin1String("void");
    } else if (cType->isArray()) {
        s = translateType(cType->arrayElementType(), context, options) + QLatin1String("[]");
    } else if ((options & Generator::EnumAsInts) && useEnumAsIntForProtectedHack(cType)) {
        s = intT();
    } else {
        if (options & Generator::OriginalName) {
            s = cType->originalTypeDescription().trimmed();
            if ((options & Generator::ExcludeReference) && s.endsWith(QLatin1Char('&')))
                s.chop(1);

            // remove only the last const (avoid remove template const)
            if (options & Generator::ExcludeConst) {
                int index = s.lastIndexOf(QLatin1String("const"));

                if (index >= (s.size() - (constLen + 1))) // (VarType const)  or (VarType const[*|&])
                    s = s.remove(index, constLen);
            }
        } else if (options & Generator::ExcludeConst || options & Generator::ExcludeReference) {
            AbstractMetaType *copyType = cType->copy();

            if (options & Generator::ExcludeConst)
                copyType->setConstant(false);

            if (options & Generator::ExcludeReference)
                copyType->setReferenceType(NoReference);

            s = copyType->cppSignature();
            if (!copyType->typeEntry()->isVoid() && !copyType->typeEntry()->isCppPrimitive())
                s.prepend(QLatin1String("::"));
            delete copyType;
        } else {
            s = cType->cppSignature();
        }
    }

    return s;
}


QString Generator::subDirectoryForClass(const AbstractMetaClass *clazz) const
{
    return subDirectoryForPackage(clazz->package());
}

QString Generator::subDirectoryForPackage(QString packageNameIn) const
{
    if (packageNameIn.isEmpty())
        packageNameIn = packageName();
    packageNameIn.replace(QLatin1Char('.'), QDir::separator());
    return packageNameIn;
}

template<typename T>
static QString getClassTargetFullName_(const T *t, bool includePackageName)
{
    QString name = t->name();
    const AbstractMetaClass *context = t->enclosingClass();
    while (context) {
        // If the type was marked as 'visible=false' we should not use it in
        // the type name
        if (NamespaceTypeEntry::isVisibleScope(context->typeEntry())) {
            name.prepend(QLatin1Char('.'));
            name.prepend(context->name());
        }
        context = context->enclosingClass();
    }
    if (includePackageName) {
        name.prepend(QLatin1Char('.'));
        name.prepend(t->package());
    }
    return name;
}

QString getClassTargetFullName(const AbstractMetaClass *metaClass, bool includePackageName)
{
    return getClassTargetFullName_(metaClass, includePackageName);
}

QString getClassTargetFullName(const AbstractMetaEnum *metaEnum, bool includePackageName)
{
    return getClassTargetFullName_(metaEnum, includePackageName);
}

QString getClassTargetFullName(const AbstractMetaType *metaType, bool includePackageName)
{
    QString name = metaType->cppSignature();
    name.replace(QLatin1String("::"), QLatin1String("_"));
    name.replace(QLatin1Char('<'), QLatin1Char('_'));
    name.remove(QLatin1Char('>'));
    name.remove(QLatin1Char(' '));
    if (includePackageName) {
        name.prepend(QLatin1Char('.'));
        name.prepend(metaType->package());
    }
    return name;
}

QString getFilteredCppSignatureString(QString signature)
{
    signature.replace(QLatin1String("::"), QLatin1String("_"));
    signature.replace(QLatin1Char('<'), QLatin1Char('_'));
    signature.replace(QLatin1Char('>'), QLatin1Char('_'));
    signature.replace(QLatin1Char(' '), QLatin1Char('_'));
    return signature;
}
