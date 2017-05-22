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

#include "generator.h"
#include "abstractmetalang.h"
#include "reporthandler.h"
#include "fileout.h"
#include "apiextractor.h"
#include "typesystem.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QDebug>
#include <typedatabase.h>

struct Generator::GeneratorPrivate {
    const ApiExtractor* apiextractor;
    QString outDir;
    // License comment
    QString licenseComment;
    QString packageName;
    int numGenerated;
    QStringList instantiatedContainersNames;
    QStringList instantiatedSmartPointerNames;
    QList<const AbstractMetaType *> instantiatedContainers;
    QList<const AbstractMetaType *> instantiatedSmartPointers;

};

Generator::Generator() : m_d(new GeneratorPrivate)
{
    m_d->numGenerated = 0;
    m_d->instantiatedContainers = QList<const AbstractMetaType *>();
    m_d->instantiatedSmartPointers = QList<const AbstractMetaType *>();
    m_d->instantiatedContainersNames = QStringList();
    m_d->instantiatedSmartPointerNames = QStringList();
}

Generator::~Generator()
{
    delete m_d;
}

bool Generator::setup(const ApiExtractor& extractor, const QMap< QString, QString > args)
{
    m_d->apiextractor = &extractor;
    TypeEntryHash allEntries = TypeDatabase::instance()->allEntries();
    TypeEntry* entryFound = 0;
    for (TypeEntryHash::const_iterator it = allEntries.cbegin(), end = allEntries.cend(); it != end; ++it) {
        foreach (TypeEntry *entry, it.value()) {
            if (entry->type() == TypeEntry::TypeSystemType && entry->generateCode()) {
                entryFound = entry;
                break;
            }
        }
        if (entryFound)
            break;
    }
    if (entryFound)
        m_d->packageName = entryFound->name();
    else
        qCWarning(lcShiboken) << "Couldn't find the package name!!";

    collectInstantiatedContainersAndSmartPointers();

    return doSetup(args);
}

QString Generator::getSimplifiedContainerTypeName(const AbstractMetaType* type)
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

void Generator::addInstantiatedContainersAndSmartPointers(const AbstractMetaType *type,
                                                          const QString &context)
{
    if (!type)
        return;
    foreach (const AbstractMetaType* t, type->instantiations())
        addInstantiatedContainersAndSmartPointers(t, context);
    if (!type->typeEntry()->isContainer() && !type->typeEntry()->isSmartPointer())
        return;
    bool isContainer = type->typeEntry()->isContainer();
    if (type->hasTemplateChildren()) {
        QString piece = isContainer ? QStringLiteral("container") : QStringLiteral("smart pointer");
        QString warning =
                QString::fromLatin1("Skipping instantiation of %1 '%2' because it has template"
                               " arguments.").arg(piece).arg(type->originalTypeDescription());
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
        // Is smart pointer.
        if (!m_d->instantiatedSmartPointerNames.contains(typeName)) {
            m_d->instantiatedSmartPointerNames.append(typeName);
            m_d->instantiatedSmartPointers.append(type);
        }
    }

}

void Generator::collectInstantiatedContainersAndSmartPointers(const AbstractMetaFunction *func)
{
    addInstantiatedContainersAndSmartPointers(func->type(), func->signature());
    foreach (const AbstractMetaArgument* arg, func->arguments())
        addInstantiatedContainersAndSmartPointers(arg->type(), func->signature());
}

void Generator::collectInstantiatedContainersAndSmartPointers(const AbstractMetaClass *metaClass)
{
    if (!metaClass->typeEntry()->generateCode())
        return;
    foreach (const AbstractMetaFunction* func, metaClass->functions())
        collectInstantiatedContainersAndSmartPointers(func);
    foreach (const AbstractMetaField* field, metaClass->fields())
        addInstantiatedContainersAndSmartPointers(field->type(), field->name());
    foreach (AbstractMetaClass* innerClass, metaClass->innerClasses())
        collectInstantiatedContainersAndSmartPointers(innerClass);
}

void Generator::collectInstantiatedContainersAndSmartPointers()
{
    foreach (const AbstractMetaFunction* func, globalFunctions())
        collectInstantiatedContainersAndSmartPointers(func);
    foreach (const AbstractMetaClass* metaClass, classes())
        collectInstantiatedContainersAndSmartPointers(metaClass);
}

QList<const AbstractMetaType*> Generator::instantiatedContainers() const
{
    return m_d->instantiatedContainers;
}

QList<const AbstractMetaType*> Generator::instantiatedSmartPointers() const
{
    return m_d->instantiatedSmartPointers;
}

QMap< QString, QString > Generator::options() const
{
    return QMap<QString, QString>();
}

AbstractMetaClassList Generator::classes() const
{
    return m_d->apiextractor->classes();
}

AbstractMetaClassList Generator::classesTopologicalSorted(const Dependencies &additionalDependencies) const
{
    return m_d->apiextractor->classesTopologicalSorted(additionalDependencies);
}

AbstractMetaFunctionList Generator::globalFunctions() const
{
    return m_d->apiextractor->globalFunctions();
}

AbstractMetaEnumList Generator::globalEnums() const
{
    return m_d->apiextractor->globalEnums();
}

QList<const PrimitiveTypeEntry*> Generator::primitiveTypes() const
{
    return m_d->apiextractor->primitiveTypes();
}

QList<const ContainerTypeEntry*> Generator::containerTypes() const
{
    return m_d->apiextractor->containerTypes();
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const EnumTypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const TypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const FlagsTypeEntry* typeEntry) const
{
    return m_d->apiextractor->findAbstractMetaEnum(typeEntry);
}

const AbstractMetaEnum* Generator::findAbstractMetaEnum(const AbstractMetaType* metaType) const
{
    return m_d->apiextractor->findAbstractMetaEnum(metaType);
}

QSet< QString > Generator::qtMetaTypeDeclaredTypeNames() const
{
    return m_d->apiextractor->qtMetaTypeDeclaredTypeNames();
}

QString Generator::licenseComment() const
{
    return m_d->licenseComment;
}

void Generator::setLicenseComment(const QString& licenseComment)
{
    m_d->licenseComment = licenseComment;
}

QString Generator::packageName() const
{
    return m_d->packageName;
}

QString Generator::moduleName() const
{
    QString& pkgName = m_d->packageName;
    return QString(pkgName).remove(0, pkgName.lastIndexOf(QLatin1Char('.')) + 1);
}

QString Generator::outputDirectory() const
{
    return m_d->outDir;
}

void Generator::setOutputDirectory(const QString &outDir)
{
    m_d->outDir = outDir;
}

int Generator::numGenerated() const
{
    return m_d->numGenerated;
}

inline void touchFile(const QString &filePath)
{
    QFile toucher(filePath);
    qint64 size = toucher.size();
    if (!toucher.open(QIODevice::ReadWrite)) {
        qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Failed to touch file '%1'")
                   .arg(QDir::toNativeSeparators(filePath));
        return;
    }
    toucher.resize(size+1);
    toucher.resize(size);
    toucher.close();
}

bool Generator::generateFileForContext(GeneratorContext &context)
{
    AbstractMetaClass *cls = context.metaClass();

    if (!shouldGenerate(cls))
        return true;

    const QString fileName = fileNameForContext(context);
    if (fileName.isEmpty())
        return true;
    if (ReportHandler::isDebug(ReportHandler::SparseDebug))
        qCDebug(lcShiboken) << "generating: " << fileName;

    QString filePath = outputDirectory() + QLatin1Char('/') + subDirectoryForClass(cls)
            + QLatin1Char('/') + fileName;
    FileOut fileOut(filePath);

    generateClass(fileOut.stream, context);

    FileOut::State state = fileOut.done();
    switch (state) {
    case FileOut::Failure:
        return false;
    case FileOut::Unchanged:
        // Even if contents is unchanged, the last file modification time should be updated,
        // so that the build system can rely on the fact the generated file is up-to-date.
        touchFile(filePath);
        break;
    case FileOut::Success:
        break;
    }

    ++m_d->numGenerated;
    return true;
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

bool Generator::generate()
{
    foreach (AbstractMetaClass *cls, m_d->apiextractor->classes()) {
        GeneratorContext context(cls);
        if (!generateFileForContext(context))
            return false;
    }

    foreach (const AbstractMetaType *type, instantiatedSmartPointers()) {
        AbstractMetaClass *smartPointerClass =
                AbstractMetaClass::findClass(m_d->apiextractor->smartPointers(), type->name());
        GeneratorContext context(smartPointerClass, type, true);
        if (!generateFileForContext(context))
            return false;
    }
    return finishGeneration();
}

bool Generator::shouldGenerateTypeEntry(const TypeEntry* type) const
{
    return type->codeGeneration() & TypeEntry::GenerateTargetLang;
}

bool Generator::shouldGenerate(const AbstractMetaClass* metaClass) const
{
    return shouldGenerateTypeEntry(metaClass->typeEntry());
}

void verifyDirectoryFor(const QFile &file)
{
    QDir dir = QFileInfo(file).dir();
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

    foreach (AbstractMetaArgument *arg, func->arguments())
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

QTextStream& formatCode(QTextStream &s, const QString& code, Indentor &indentor)
{
    // detect number of spaces before the first character
    QStringList lst(code.split(QLatin1Char('\n')));
    QRegExp nonSpaceRegex(QLatin1String("[^\\s]"));
    int spacesToRemove = 0;
    foreach(QString line, lst) {
        if (!line.trimmed().isEmpty()) {
            spacesToRemove = line.indexOf(nonSpaceRegex);
            if (spacesToRemove == -1)
                spacesToRemove = 0;
            break;
        }
    }

    static QRegExp emptyLine(QLatin1String("\\s*[\\r]?[\\n]?\\s*"));

    foreach(QString line, lst) {
        if (!line.isEmpty() && !emptyLine.exactMatch(line)) {
            while (line.end()->isSpace())
                line.chop(1);
            int limit = 0;
            for(int i = 0; i < spacesToRemove; ++i) {
                if (!line[i].isSpace())
                    break;
                limit++;
            }

            s << indentor << line.remove(0, limit);
        }
        s << endl;
    }
    return s;
}

AbstractMetaFunctionList Generator::implicitConversions(const TypeEntry* type) const
{
    if (type->isValue()) {
        if (const AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes(), type))
            return metaClass->implicitConversions();
    }
    return AbstractMetaFunctionList();
}

AbstractMetaFunctionList Generator::implicitConversions(const AbstractMetaType* metaType) const
{
    return implicitConversions(metaType->typeEntry());
}

bool Generator::isObjectType(const TypeEntry* type)
{
    if (type->isComplex())
        return Generator::isObjectType((const ComplexTypeEntry*)type);
    return type->isObject();
}
bool Generator::isObjectType(const ComplexTypeEntry* type)
{
    return type->isObject() || type->isQObject();
}
bool Generator::isObjectType(const AbstractMetaClass* metaClass)
{
    return Generator::isObjectType(metaClass->typeEntry());
}
bool Generator::isObjectType(const AbstractMetaType* metaType)
{
    return isObjectType(metaType->typeEntry());
}

bool Generator::isPointer(const AbstractMetaType* type)
{
    return type->indirections() > 0
            || type->isNativePointer()
            || type->isValuePointer();
}

bool Generator::isCString(const AbstractMetaType* type)
{
    return type->isNativePointer()
            && type->indirections() == 1
            && type->name() == QLatin1String("char");
}

bool Generator::isVoidPointer(const AbstractMetaType* type)
{
    return type->isNativePointer()
            && type->indirections() == 1
            && type->name() == QLatin1String("void");
}

QString Generator::getFullTypeName(const TypeEntry* type) const
{
    return type->isCppPrimitive()
        ? type->qualifiedCppName()
        : (QLatin1String("::") + type->qualifiedCppName());
}

QString Generator::getFullTypeName(const AbstractMetaType* type) const
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

QString Generator::getFullTypeName(const AbstractMetaClass* metaClass) const
{
    return QLatin1String("::") + metaClass->qualifiedCppName();
}

QString Generator::getFullTypeNameWithoutModifiers(const AbstractMetaType* type) const
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

QString Generator::minimalConstructor(const AbstractMetaType* type) const
{
    if (!type || (type->referenceType() ==  LValueReference && Generator::isObjectType(type)))
        return QString();

    if (type->isContainer()) {
        QString ctor = type->cppSignature();
        if (ctor.endsWith(QLatin1Char('*')))
            return QLatin1String("0");
        if (ctor.startsWith(QLatin1String("const ")))
            ctor.remove(0, sizeof("const ") / sizeof(char) - 1);
        if (ctor.endsWith(QLatin1Char('&'))) {
            ctor.chop(1);
            ctor = ctor.trimmed();
        }
        return QLatin1String("::") + ctor + QLatin1String("()");
    }

    if (type->isNativePointer())
        return QLatin1String("static_cast<") + type->typeEntry()->qualifiedCppName() + QLatin1String(" *>(0)");
    if (Generator::isPointer(type))
        return QLatin1String("static_cast< ::") + type->typeEntry()->qualifiedCppName() + QLatin1String(" *>(0)");

    if (type->typeEntry()->isComplex()) {
        const ComplexTypeEntry* cType = reinterpret_cast<const ComplexTypeEntry*>(type->typeEntry());
        QString ctor = cType->defaultConstructor();
        if (!ctor.isEmpty())
            return ctor;
        ctor = minimalConstructor(AbstractMetaClass::findClass(classes(), cType));
        if (type->hasInstantiations())
            ctor = ctor.replace(getFullTypeName(cType), getFullTypeNameWithoutModifiers(type));
        return ctor;
    }

    return minimalConstructor(type->typeEntry());
}

QString Generator::minimalConstructor(const TypeEntry* type) const
{
    if (!type)
        return QString();

    if (type->isCppPrimitive()) {
        const QString &name = type->qualifiedCppName();
        return name == QLatin1String("bool")
            ? QLatin1String("false") : name + QLatin1String("(0)");
    }

    if (type->isEnum())
        return QLatin1String("static_cast< ::") + type->qualifiedCppName() + QLatin1String(">(0)");

    if (type->isFlags())
        return type->qualifiedCppName() + QLatin1String("(0)");

    if (type->isPrimitive()) {
        QString ctor = reinterpret_cast<const PrimitiveTypeEntry*>(type)->defaultConstructor();
        // If a non-C++ (i.e. defined by the user) primitive type does not have
        // a default constructor defined by the user, the empty constructor is
        // heuristically returned. If this is wrong the build of the generated
        // bindings will tell.
        return ctor.isEmpty()
            ? (QLatin1String("::") + type->qualifiedCppName() + QLatin1String("()"))
            : ctor;
    }

    if (type->isComplex())
        return minimalConstructor(AbstractMetaClass::findClass(classes(), type));

    return QString();
}

QString Generator::minimalConstructor(const AbstractMetaClass* metaClass) const
{
    if (!metaClass)
        return QString();

    const ComplexTypeEntry* cType = reinterpret_cast<const ComplexTypeEntry*>(metaClass->typeEntry());
    if (cType->hasDefaultConstructor())
        return cType->defaultConstructor();

    AbstractMetaFunctionList constructors = metaClass->queryFunctions(AbstractMetaClass::Constructors);
    int maxArgs = 0;
    foreach (const AbstractMetaFunction* ctor, constructors) {
        if (ctor->isUserAdded() || ctor->isPrivate() || ctor->functionType() != AbstractMetaFunction::ConstructorFunction)
            continue;

        int numArgs = ctor->arguments().size();
        if (numArgs == 0) {
            maxArgs = 0;
            break;
        }
        if (numArgs > maxArgs)
            maxArgs = numArgs;
    }

    QString qualifiedCppName = metaClass->typeEntry()->qualifiedCppName();
    QStringList templateTypes;
    foreach (TypeEntry* templateType, metaClass->templateArguments())
        templateTypes << templateType->qualifiedCppName();

    // Empty constructor.
    if (maxArgs == 0)
        return QLatin1String("::") + qualifiedCppName + QLatin1String("()");

    QList<const AbstractMetaFunction*> candidates;

    // Constructors with C++ primitive types, enums or pointers only.
    // Start with the ones with fewer arguments.
    for (int i = 1; i <= maxArgs; ++i) {
        foreach (const AbstractMetaFunction* ctor, constructors) {
            if (ctor->isUserAdded() || ctor->isPrivate() || ctor->functionType() != AbstractMetaFunction::ConstructorFunction)
                continue;

            AbstractMetaArgumentList arguments = ctor->arguments();
            if (arguments.size() != i)
                continue;

            QStringList args;
            foreach (const AbstractMetaArgument* arg, arguments) {
                const TypeEntry* type = arg->type()->typeEntry();
                if (type == metaClass->typeEntry()) {
                    args.clear();
                    break;
                }

                if (!arg->originalDefaultValueExpression().isEmpty()) {
                    if (!arg->defaultValueExpression().isEmpty()
                        && arg->defaultValueExpression() != arg->originalDefaultValueExpression()) {
                        args << arg->defaultValueExpression();
                    }
                    break;
                }

                if (type->isCppPrimitive() || type->isEnum() || isPointer(arg->type())) {
                    QString argValue = minimalConstructor(arg->type());
                    if (argValue.isEmpty()) {
                        args.clear();
                        break;
                    }
                    args << argValue;
                } else {
                    args.clear();
                    break;
                }
            }

            if (!args.isEmpty())
                return QString::fromLatin1("::%1(%2)").arg(qualifiedCppName, args.join(QLatin1String(", ")));

            candidates << ctor;
        }
    }

    // Constructors with C++ primitive types, enums, pointers, value types,
    // and user defined primitive types.
    // Builds the minimal constructor recursively.
    foreach (const AbstractMetaFunction* ctor, candidates) {
        QStringList args;
        foreach (const AbstractMetaArgument* arg, ctor->arguments()) {
            if (arg->type()->typeEntry() == metaClass->typeEntry()) {
                args.clear();
                break;
            }
            QString argValue = minimalConstructor(arg->type());
            if (argValue.isEmpty()) {
                args.clear();
                break;
            }
            args << argValue;
        }
        if (!args.isEmpty()) {
            return QString::fromLatin1("::%1(%2)").arg(qualifiedCppName, args.join(QLatin1String(", ")));
        }
    }

    return QString();
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
    } else if (options & Generator::EnumAsInts && (cType->isEnum() || cType->isFlags())) {
        s = QLatin1String("int");
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
            AbstractMetaType* copyType = cType->copy();

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


QString Generator::subDirectoryForClass(const AbstractMetaClass* clazz) const
{
    return subDirectoryForPackage(clazz->package());
}

QString Generator::subDirectoryForPackage(QString packageName) const
{
    if (packageName.isEmpty())
        packageName = m_d->packageName;
    return QString(packageName).replace(QLatin1Char('.'), QDir::separator());
}

template<typename T>
static QString getClassTargetFullName_(const T* t, bool includePackageName)
{
    QString name = t->name();
    const AbstractMetaClass* context = t->enclosingClass();
    while (context) {
        name.prepend(QLatin1Char('.'));
        name.prepend(context->name());
        context = context->enclosingClass();
    }
    if (includePackageName) {
        name.prepend(QLatin1Char('.'));
        name.prepend(t->package());
    }
    return name;
}

QString getClassTargetFullName(const AbstractMetaClass* metaClass, bool includePackageName)
{
    return getClassTargetFullName_(metaClass, includePackageName);
}

QString getClassTargetFullName(const AbstractMetaEnum* metaEnum, bool includePackageName)
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
