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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "include.h"

#include <QtCore/QStringList>
#include <QtCore/QScopedPointer>

//Used to identify the conversion rule to avoid break API
extern const char *TARGET_CONVERSION_RULE_FLAG;
extern const char *NATIVE_CONVERSION_RULE_FLAG;

class CustomFunction;
class CustomConversion;
class EnumValueTypeEntry;
class FlagsTypeEntry;
class SourceLocation;
class TypeSystemTypeEntry;

class TypeEntryPrivate;
class TemplateArgumentEntryPrivate;
class ArrayTypeEntryPrivate;
class PrimitiveTypeEntryPrivate;
class EnumTypeEntryPrivate;
class EnumValueTypeEntryPrivate;
class FlagsTypeEntryPrivate;
class ComplexTypeEntryPrivate;
class TypedefEntryPrivate;
class ContainerTypeEntryPrivate;
class SmartPointerTypeEntryPrivate;
class NamespaceTypeEntryPrivate;
class FunctionTypeEntryPrivate;
struct TargetToNativeConversionPrivate;

QT_BEGIN_NAMESPACE
class QDebug;
class QRegularExpression;
class QTextStream;
class QVersionNumber;
QT_END_NAMESPACE

struct TypeSystemProperty
{
    bool isValid() const { return !name.isEmpty() && !read.isEmpty() && !type.isEmpty(); }

    QString type;
    QString name;
    QString read;
    QString write;
    QString reset;
    QString designable;
    // Indicates whether actual code is generated instead of relying on libpyside.
    bool generateGetSetDef = false;
};

class TypeEntry
{
    Q_GADGET
public:
    Q_DISABLE_COPY_MOVE(TypeEntry)

    enum Type {
        PrimitiveType,
        VoidType,
        VarargsType,
        FlagsType,
        EnumType,
        EnumValue,
        ConstantValueType,
        TemplateArgumentType,
        BasicValueType,
        ContainerType,
        ObjectType,
        NamespaceType,
        ArrayType,
        TypeSystemType,
        CustomType,
        FunctionType,
        SmartPointerType,
        TypedefType
    };
    Q_ENUM(Type)

    enum CodeGeneration {
        GenerateNothing,     // Rejection, private type, ConstantValueTypeEntry or similar
        GenerationDisabled,  // generate='no' in type system
        GenerateCode,        // Generate code
        GenerateForSubclass, // Inherited from a loaded dependent type system.
    };
    Q_ENUM(CodeGeneration)

    explicit TypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                       const TypeEntry *parent);
    virtual ~TypeEntry();

    Type type() const;

    const TypeEntry *parent() const;
    void setParent(const TypeEntry *p);
    bool isChildOf(const TypeEntry *p) const;
    const TypeSystemTypeEntry *typeSystemTypeEntry() const;
    // cf AbstractMetaClass::targetLangEnclosingClass()
    const TypeEntry *targetLangEnclosingEntry() const;

    bool isPrimitive() const;
    bool isEnum() const;
    bool isFlags() const;
    bool isObject() const;
    bool isNamespace() const;
    bool isContainer() const;
    bool isSmartPointer() const;
    bool isArray() const;
    bool isTemplateArgument() const;
    bool isVoid() const;
    bool isVarargs() const;
    bool isCustom() const;
    bool isTypeSystem() const;
    bool isFunction() const;
    bool isEnumValue() const;

    bool stream() const;
    void setStream(bool b);

    // The type's name in C++, fully qualified
    QString name() const;
    // C++ excluding inline namespaces
    QString shortName() const;
    // Name as specified in XML
    QString entryName() const;

    CodeGeneration codeGeneration() const;
    void setCodeGeneration(CodeGeneration cg);

    // Returns true if code must be generated for this entry,
    // it will return false in case of types coming from typesystems
    // included for reference only.
    // NOTE: 'GenerateForSubclass' means 'generate="no"'
    //       on 'load-typesystem' tag
    bool generateCode() const;

    int revision() const;
    void setRevision(int r); // see typedatabase.cpp
    int sbkIndex() const; // see typedatabase.cpp
    void setSbkIndex(int i);

    virtual QString qualifiedCppName() const;

    /**
     *   Its type's name in target language API
     *   The target language API name represents how this type is
     *   referred on low level code for the target language.
     *   Examples: for Java this would be a JNI name, for Python
     *   it should represent the CPython type name.
     *   /return string representing the target language API name
     *   for this type entry
     */
    virtual QString targetLangApiName() const;

    // The type's name in TargetLang
    QString targetLangName() const; // "Foo.Bar"
    void setTargetLangName(const QString &n);
    QString targetLangEntryName() const; // "Bar"

    // The package
    QString targetLangPackage() const;
    void setTargetLangPackage(const QString &p);

    QString qualifiedTargetLangName() const;

    void setCustomConstructor(const CustomFunction &func);
    CustomFunction customConstructor() const;

    void setCustomDestructor(const CustomFunction &func);
    CustomFunction customDestructor() const;

    virtual bool isValue() const;
    virtual bool isComplex() const;

    CodeSnipList codeSnips() const;
    void setCodeSnips(const CodeSnipList &codeSnips);
    void addCodeSnip(const CodeSnip &codeSnip);

    void setDocModification(const DocModificationList& docMods);
    DocModificationList docModifications() const;

    const IncludeList &extraIncludes() const;
    void setExtraIncludes(const IncludeList &includes);
    void addExtraInclude(const Include &newInclude);

    Include include() const;
    void setInclude(const Include &inc);

    // Replace conversionRule arg to CodeSnip in future version
    /// Set the type convertion rule
    void setConversionRule(const QString& conversionRule);

    /// Returns the type convertion rule
    QString conversionRule() const;

    /// Returns true if there are any conversiton rule for this type, false otherwise.
    bool hasConversionRule() const;

    QVersionNumber version() const;

    /// TODO-CONVERTER: mark as deprecated
    bool hasNativeConversionRule() const;

    /// TODO-CONVERTER: mark as deprecated
    bool hasTargetConversionRule() const;

    bool isCppPrimitive() const;

    bool hasCustomConversion() const;
    void setCustomConversion(CustomConversion* customConversion);
    CustomConversion* customConversion() const;

    // View on: Type to use for function argument conversion, fex
    // std::string_view -> std::string for foo(std::string_view).
    // cf AbstractMetaType::viewOn()
    TypeEntry *viewOn() const;
    void setViewOn(TypeEntry *v);

    virtual TypeEntry *clone() const;

    void useAsTypedef(const TypeEntry *source);

    SourceLocation sourceLocation() const;
    void setSourceLocation(const SourceLocation &sourceLocation);

    // Query functions for generators
    /// Returns true if the type is a primitive but not a C++ primitive.
    bool isUserPrimitive() const;
    /// Returns true if the type passed has a Python wrapper for it.
    /// Although namespace has a Python wrapper, it's not considered a type.
    bool isWrapperType() const;
    /// Returns true if the type is a C++ integral primitive,
    /// i.e. bool, char, int, long, and their unsigned counterparts.
    bool isCppIntegralPrimitive() const;
    /// Returns true if the type is an extended C++ primitive, a void*,
    /// a const char*, or a std::string (cf isCppPrimitive()).
    bool isExtendedCppPrimitive() const;

#ifndef QT_NO_DEBUG_STREAM
    virtual void formatDebug(QDebug &d) const;
#endif

protected:
    explicit TypeEntry(TypeEntryPrivate *d);

    const TypeEntryPrivate *d_func() const;
    TypeEntryPrivate *d_func();

    virtual QString buildTargetLangName() const;

private:
    bool setRevisionHelper(int r);
    int sbkIndexHelper() const;
    QScopedPointer<TypeEntryPrivate> m_d;
};

class TypeSystemTypeEntry : public TypeEntry
{
public:
    explicit TypeSystemTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                 const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    explicit TypeSystemTypeEntry(TypeEntryPrivate *d);
};

class VoidTypeEntry : public TypeEntry
{
public:
    VoidTypeEntry();

    TypeEntry *clone() const override;

protected:
    explicit VoidTypeEntry(TypeEntryPrivate *d);
};

class VarargsTypeEntry : public TypeEntry
{
public:
    VarargsTypeEntry();

    TypeEntry *clone() const override;

protected:
    explicit VarargsTypeEntry(TypeEntryPrivate *d);
};

class TemplateArgumentEntry : public TypeEntry
{
public:
    explicit TemplateArgumentEntry(const QString &entryName, const QVersionNumber &vr,
                                   const TypeEntry *parent);

    int ordinal() const;
    void setOrdinal(int o);

    TypeEntry *clone() const override;

protected:
    explicit TemplateArgumentEntry(TemplateArgumentEntryPrivate *d);
};

class ArrayTypeEntry : public TypeEntry
{
public:
    explicit ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr,
                            const TypeEntry *parent);

    void setNestedTypeEntry(TypeEntry *nested);
    const TypeEntry *nestedTypeEntry() const;

    QString targetLangApiName() const override;

    TypeEntry *clone() const override;

protected:
    explicit ArrayTypeEntry(ArrayTypeEntryPrivate *d);

    QString buildTargetLangName() const override;
};

class PrimitiveTypeEntry : public TypeEntry
{
public:
    explicit PrimitiveTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                const TypeEntry *parent);

    QString targetLangApiName() const override;
    void setTargetLangApiName(const QString &targetLangApiName);

    QString defaultConstructor() const;
    void setDefaultConstructor(const QString& defaultConstructor);
    bool hasDefaultConstructor() const;

    /**
     *   The PrimitiveTypeEntry pointed by this type entry if it
     *   represents a typedef).
     *   /return the type referenced by the typedef, or a null pointer
     *   if the current object is not an typedef
     */
    PrimitiveTypeEntry *referencedTypeEntry() const;

    /**
     *   Defines type referenced by this entry.
     *   /param referencedTypeEntry type referenced by this entry
     */
    void setReferencedTypeEntry(PrimitiveTypeEntry* referencedTypeEntry);

    /**
     *   Finds the most basic primitive type that the typedef represents,
     *   i.e. a type that is not an typedef'ed.
     *   /return the most basic non-typedef'ed primitive type represented
     *   by this typedef
     */
    PrimitiveTypeEntry* basicReferencedTypeEntry() const;

    bool preferredTargetLangType() const;
    void setPreferredTargetLangType(bool b);

    TypeEntry *clone() const override;

protected:
    explicit PrimitiveTypeEntry(PrimitiveTypeEntryPrivate *d);
};

class EnumTypeEntry : public TypeEntry
{
public:
    explicit EnumTypeEntry(const QString &entryName,
                           const QVersionNumber &vr,
                           const TypeEntry *parent);

    QString targetLangQualifier() const;

    QString targetLangApiName() const override;

    QString qualifier() const;

    const EnumValueTypeEntry *nullValue() const;
    void setNullValue(const EnumValueTypeEntry *n);

    void setFlags(FlagsTypeEntry *flags);
    FlagsTypeEntry *flags() const;

    bool isEnumValueRejected(const QString &name) const;
    void addEnumValueRejection(const QString &name);
    QStringList enumValueRejections() const;

    TypeEntry *clone() const override;
#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    explicit EnumTypeEntry(EnumTypeEntryPrivate *d);
};

// EnumValueTypeEntry is used for resolving integer type templates
// like array<EnumValue>. Note: Dummy entries for integer values will
// be created for non-type template parameters, where m_enclosingEnum==nullptr.
class EnumValueTypeEntry : public TypeEntry
{
public:
    explicit EnumValueTypeEntry(const QString& name, const QString& value,
                                const EnumTypeEntry* enclosingEnum,
                                bool isScopedEnum, const QVersionNumber &vr);

    QString value() const;
    const EnumTypeEntry* enclosingEnum() const;

    TypeEntry *clone() const override;

protected:
    explicit EnumValueTypeEntry(EnumValueTypeEntryPrivate *d);
};

class FlagsTypeEntry : public TypeEntry
{
public:
    explicit FlagsTypeEntry(const QString &entryName, const QVersionNumber &vr,
                            const TypeEntry *parent);

    QString targetLangApiName() const override;

    QString originalName() const;
    void setOriginalName(const QString &s);

    QString flagsName() const;
    void setFlagsName(const QString &name);

    EnumTypeEntry *originator() const;
    void setOriginator(EnumTypeEntry *e);

    TypeEntry *clone() const override;

protected:
    explicit FlagsTypeEntry(FlagsTypeEntryPrivate *d);

    QString buildTargetLangName() const override;
};

// For primitive values, typically to provide a dummy type for
// example the '2' in non-type template 'Array<2>'.
class ConstantValueTypeEntry : public TypeEntry
{
public:
    explicit  ConstantValueTypeEntry(const QString& name,
                                     const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    explicit ConstantValueTypeEntry(TypeEntryPrivate *d);
};

class ComplexTypeEntry : public TypeEntry
{
public:
    enum TypeFlag {
        DisableWrapper     = 0x1,
        Deprecated         = 0x4
    };
    Q_DECLARE_FLAGS(TypeFlags, TypeFlag)

    enum CopyableFlag {
        CopyableSet,
        NonCopyableSet,
        Unknown
    };

    explicit ComplexTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                              const TypeEntry *parent);

    bool isComplex() const override;

    QString targetLangApiName() const override;

    TypeFlags typeFlags() const;
    void setTypeFlags(TypeFlags flags);

    FunctionModificationList functionModifications() const;
    void setFunctionModifications(const FunctionModificationList &functionModifications);
    void addFunctionModification(const FunctionModification &functionModification);
    FunctionModificationList functionModifications(const QString &signature) const;

    AddedFunctionList addedFunctions() const;
    void setAddedFunctions(const AddedFunctionList &addedFunctions);
    void addNewFunction(const AddedFunctionPtr &addedFunction);

    FieldModification fieldModification(const QString &name) const;
    void setFieldModifications(const FieldModificationList &mods);
    FieldModificationList fieldModifications() const;

    const QList<TypeSystemProperty> &properties() const;
    void addProperty(const TypeSystemProperty &p);

    QString defaultSuperclass() const;
    void setDefaultSuperclass(const QString &sc);

    QString qualifiedCppName() const override;

    void setIsPolymorphicBase(bool on);
    bool isPolymorphicBase() const;

    void setPolymorphicIdValue(const QString &value);
    QString polymorphicIdValue() const;

    QString targetType() const;
    void setTargetType(const QString &code);

    bool isGenericClass() const;
    void setGenericClass(bool isGeneric);

    bool deleteInMainThread() const;
    void setDeleteInMainThread(bool d);

    CopyableFlag copyable() const;
    void setCopyable(CopyableFlag flag);

    QString hashFunction() const;
    void setHashFunction(const QString &hashFunction);

    void setBaseContainerType(const ComplexTypeEntry *baseContainer);

    const ComplexTypeEntry *baseContainerType() const;

    TypeSystem::ExceptionHandling exceptionHandling() const;
    void setExceptionHandling(TypeSystem::ExceptionHandling e);

    TypeSystem::AllowThread allowThread() const;
    void setAllowThread(TypeSystem::AllowThread allowThread);

    QString defaultConstructor() const;
    void setDefaultConstructor(const QString& defaultConstructor);
    bool hasDefaultConstructor() const;

    TypeEntry *clone() const override;

    void useAsTypedef(const ComplexTypeEntry *source);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &debug) const override;
#endif
protected:
    explicit ComplexTypeEntry(ComplexTypeEntryPrivate *d);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ComplexTypeEntry::TypeFlags)

class TypedefEntry : public ComplexTypeEntry
{
public:
    explicit TypedefEntry(const QString &entryName,
                          const QString &sourceType,
                          const QVersionNumber &vr,
                          const TypeEntry *parent);

    QString sourceType() const;
    void setSourceType(const QString &s);

    TypeEntry *clone() const override;

    ComplexTypeEntry *source() const;
    void setSource(ComplexTypeEntry *source);

    ComplexTypeEntry *target() const;
    void setTarget(ComplexTypeEntry *target);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    explicit TypedefEntry(TypedefEntryPrivate *d);
};

class ContainerTypeEntry : public ComplexTypeEntry
{
    Q_GADGET
public:
    enum ContainerKind {
        NoContainer,
        ListContainer,
        StringListContainer,
        LinkedListContainer,
        VectorContainer,
        StackContainer,
        QueueContainer,
        SetContainer,
        MapContainer,
        MultiMapContainer,
        HashContainer,
        MultiHashContainer,
        PairContainer,
    };
    Q_ENUM(ContainerKind)

    explicit ContainerTypeEntry(const QString &entryName, ContainerKind containerKind,
                                const QVersionNumber &vr, const TypeEntry *parent);

    ContainerKind containerKind() const;

    QString typeName() const;
    QString qualifiedCppName() const override;

    TypeEntry *clone() const override;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    explicit ContainerTypeEntry(ContainerTypeEntryPrivate *d);
};

class SmartPointerTypeEntry : public ComplexTypeEntry
{
public:
    using Instantiations = QVector<const TypeEntry *>;

    explicit SmartPointerTypeEntry(const QString &entryName,
                                   const QString &getterName,
                                   const QString &smartPointerType,
                                   const QString &refCountMethodName,
                                   const QVersionNumber &vr,
                                   const TypeEntry *parent);

    QString getter() const;

    QString refCountMethodName() const;

    TypeEntry *clone() const override;

    Instantiations instantiations() const;
    void setInstantiations(const Instantiations &i);
    bool matchesInstantiation(const TypeEntry *e) const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    SmartPointerTypeEntry(SmartPointerTypeEntryPrivate *d);
};

class NamespaceTypeEntry : public ComplexTypeEntry
{
public:
    explicit NamespaceTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                const TypeEntry *parent);

    TypeEntry *clone() const override;

    const NamespaceTypeEntry *extends() const;
    void setExtends(const NamespaceTypeEntry *e);

    const QRegularExpression &filePattern() const; // restrict files
    void setFilePattern(const QRegularExpression &r);

    bool hasPattern() const;

    bool matchesFile(const QString &needle) const;

    bool isVisible() const;
    void setVisibility(TypeSystem::Visibility v);

    // C++ 11 inline namespace, from code model
    bool isInlineNamespace() const;
    void setInlineNamespace(bool i);

    static bool isVisibleScope(const TypeEntry *e);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

     // Whether to generate "using namespace" into wrapper
    bool generateUsing() const;
    void setGenerateUsing(bool generateUsing);

protected:
    explicit NamespaceTypeEntry(NamespaceTypeEntryPrivate *d);
};

class ValueTypeEntry : public ComplexTypeEntry
{
public:
    explicit ValueTypeEntry(const QString &entryName, const QVersionNumber &vr,
                            const TypeEntry *parent);

    bool isValue() const override;

    TypeEntry *clone() const override;

protected:
    explicit ValueTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                            const TypeEntry *parent);
    explicit ValueTypeEntry(ComplexTypeEntryPrivate *d);
};

class FunctionTypeEntry : public TypeEntry
{
public:
    explicit FunctionTypeEntry(const QString& name, const QString& signature,
                               const QVersionNumber &vr,
                               const TypeEntry *parent);

    const QStringList &signatures() const;
    bool hasSignature(const QString& signature) const;
    void addSignature(const QString& signature);

    TypeEntry *clone() const override;

protected:
    explicit FunctionTypeEntry(FunctionTypeEntryPrivate *d);
};

class ObjectTypeEntry : public ComplexTypeEntry
{
public:
    explicit ObjectTypeEntry(const QString &entryName, const QVersionNumber &vr,
                             const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    explicit ObjectTypeEntry(ComplexTypeEntryPrivate *d);
};

class CustomConversion
{
public:
    CustomConversion(TypeEntry* ownerType);
    ~CustomConversion();

    const TypeEntry* ownerType() const;
    QString nativeToTargetConversion() const;
    void setNativeToTargetConversion(const QString& nativeToTargetConversion);

    class TargetToNativeConversion
    {
    public:
        TargetToNativeConversion(const QString& sourceTypeName,
                                 const QString& sourceTypeCheck,
                                 const QString& conversion = QString());
        ~TargetToNativeConversion();

        const TypeEntry* sourceType() const;
        void setSourceType(const TypeEntry* sourceType);
        bool isCustomType() const;
        QString sourceTypeName() const;
        QString sourceTypeCheck() const;
        QString conversion() const;
        void setConversion(const QString& conversion);
    private:
        struct TargetToNativeConversionPrivate;
        TargetToNativeConversionPrivate* m_d;
    };

    /**
     *  Returns true if the target to C++ custom conversions should
     *  replace the original existing ones, and false if the custom
     *  conversions should be added to the original.
     */
    bool replaceOriginalTargetToNativeConversions() const;
    void setReplaceOriginalTargetToNativeConversions(bool replaceOriginalTargetToNativeConversions);

    using TargetToNativeConversions = QVector<TargetToNativeConversion *>;
    bool hasTargetToNativeConversions() const;
    TargetToNativeConversions& targetToNativeConversions();
    const TargetToNativeConversions& targetToNativeConversions() const;
    void addTargetToNativeConversion(const QString& sourceTypeName,
                                     const QString& sourceTypeCheck,
                                     const QString& conversion = QString());
private:
    struct CustomConversionPrivate;
    CustomConversionPrivate* m_d;
};

#endif // TYPESYSTEM_H
