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

#ifndef ABSTRACTMETATYPE_H
#define ABSTRACTMETATYPE_H

#include "abstractmetalang_typedefs.h"
#include "parser/codemodel_enums.h"

#include <QtCore/qobjectdefs.h>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QVector>

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMetaTypeData;
class TypeEntry;

class AbstractMetaType
{
    Q_GADGET
public:
    using Indirections = QVector<Indirection>;

    enum TypeUsagePattern {
        PrimitivePattern,
        FlagsPattern,
        EnumPattern,
        ValuePattern,
        ObjectPattern,
        ValuePointerPattern,
        NativePointerPattern,
        NativePointerAsArrayPattern, // "int*" as "int[]"
        ContainerPattern,
        SmartPointerPattern,
        VarargsPattern,
        ArrayPattern,
        VoidPattern,            // Plain "void", no "void *" or similar.
        TemplateArgument,       // 'T' in std::array<T,2>
        NonTypeTemplateArgument // '2' in in std::array<T,2>
    };
    Q_ENUM(TypeUsagePattern)

    AbstractMetaType();
    explicit AbstractMetaType(const TypeEntry *t);
    AbstractMetaType(const AbstractMetaType &);
    AbstractMetaType &operator=(const AbstractMetaType &);
    AbstractMetaType(AbstractMetaType &&);
    AbstractMetaType &operator=(AbstractMetaType &&);
    ~AbstractMetaType();

    QString package() const;
    QString name() const;
    QString fullName() const;

    void setTypeUsagePattern(TypeUsagePattern pattern);
    TypeUsagePattern typeUsagePattern() const;

    // true when use pattern is container
    bool hasInstantiations() const;

    const AbstractMetaTypeList &instantiations() const;
    void addInstantiation(const AbstractMetaType &inst);
    void setInstantiations(const AbstractMetaTypeList  &insts);

    QString minimalSignature() const { return formatSignature(true); }

    // returns true if the typs is used as a non complex primitive, no & or *'s
    bool isPrimitive() const { return typeUsagePattern() == PrimitivePattern; }

    bool isCppPrimitive() const;

    // returns true if the type is used as an enum
    bool isEnum() const { return typeUsagePattern() == EnumPattern; }

    // returns true if the type is used as an object, e.g. Xxx *
    bool isObject() const { return typeUsagePattern() == ObjectPattern; }

    // returns true if the type is indicated an object by the TypeEntry
    bool isObjectType() const;

    // returns true if the type is used as an array, e.g. Xxx[42]
    bool isArray() const { return typeUsagePattern() == ArrayPattern; }

    // returns true if the type is used as a value type (X or const X &)
    bool isValue() const { return typeUsagePattern() == ValuePattern; }

    bool isValuePointer() const { return typeUsagePattern() == ValuePointerPattern; }

    // returns true for more complex types...
    bool isNativePointer() const { return typeUsagePattern() == NativePointerPattern; }

    // return true if the type was originally a varargs
    bool isVarargs() const { return typeUsagePattern() == VarargsPattern; }

    // returns true if the type was used as a container
    bool isContainer() const { return typeUsagePattern() == ContainerPattern; }

    // returns true if the type was used as a smart pointer
    bool isSmartPointer() const { return typeUsagePattern() == SmartPointerPattern; }

    // returns true if the type was used as a flag
    bool isFlags() const { return typeUsagePattern() == FlagsPattern; }

    bool isVoid() const { return typeUsagePattern() == VoidPattern; }

    bool isConstant() const;
    void setConstant(bool constant);

    bool isVolatile() const;
    void setVolatile(bool v);

    bool passByConstRef() const;
    bool passByValue() const;

    ReferenceType referenceType() const;
    void setReferenceType(ReferenceType ref);

    int actualIndirections() const;

    Indirections indirectionsV() const;
    void setIndirectionsV(const Indirections &i);
    void clearIndirections();

    // "Legacy"?
    int indirections() const;
    void setIndirections(int indirections);
    void addIndirection(Indirection i = Indirection::Pointer);

    void setArrayElementCount(int n);
    int arrayElementCount() const;

    const AbstractMetaType *arrayElementType() const;
    void setArrayElementType(const AbstractMetaType &t);

    AbstractMetaTypeList nestedArrayTypes() const;

    QString cppSignature() const;

    QString pythonSignature() const;

    bool applyArrayModification(QString *errorMessage);

    const TypeEntry *typeEntry() const;
    void setTypeEntry(const TypeEntry *type);

    void setOriginalTypeDescription(const QString &otd);
    QString originalTypeDescription() const;

    void setOriginalTemplateType(const AbstractMetaType &type);
    const AbstractMetaType *originalTemplateType() const;

    AbstractMetaType getSmartPointerInnerType() const;

    QString getSmartPointerInnerTypeName() const;

    /// Decides and sets the proper usage patter for the current meta type.
    void decideUsagePattern();

    bool hasTemplateChildren() const;

    bool equals(const AbstractMetaType &rhs) const;

    // View on: Type to use for function argument conversion, fex
    // std::string_view -> std::string for foo(std::string_view);
    // cf TypeEntry::viewOn()
    const AbstractMetaType *viewOn() const;
    void setViewOn(const AbstractMetaType &v);

    static AbstractMetaType createVoid();

    // Query functions for generators
    /// Check if type is a pointer.
    bool isPointer() const;
    /// Returns true if the type is a C string (const char *).
    bool isCString() const;
    /// Returns true if the type is a void pointer.
    bool isVoidPointer() const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &debug) const;
#endif

private:
    friend class AbstractMetaTypeData;
    QSharedDataPointer<AbstractMetaTypeData> d;

    TypeUsagePattern determineUsagePattern() const;
    QString formatSignature(bool minimal) const;
    QString formatPythonSignature() const;
};

inline bool operator==(const AbstractMetaType &t1, const AbstractMetaType &t2)
{ return t1.equals(t2); }
inline bool operator!=(const AbstractMetaType &t1, const AbstractMetaType &t2)
{ return !t1.equals(t2); }

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaType &at);
QDebug operator<<(QDebug d, const AbstractMetaType *at);
#endif

#endif // ABSTRACTMETALANG_H
