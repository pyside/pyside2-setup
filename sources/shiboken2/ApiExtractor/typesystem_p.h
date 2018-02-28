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
#ifndef TYPESYSTEM_P_H
#define TYPESYSTEM_P_H

#include <QStack>
#include "typesystem.h"

QT_FORWARD_DECLARE_CLASS(QXmlStreamAttributes)
QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)

class TypeDatabase;
class StackElement
{
    public:
        enum ElementType {
            None = 0x0,

            // Type tags (0x1, ... , 0xff)
            ObjectTypeEntry             = 0x1,
            ValueTypeEntry              = 0x2,
            InterfaceTypeEntry          = 0x3,
            NamespaceTypeEntry          = 0x4,
            ComplexTypeEntryMask        = 0x7,

            // Non-complex type tags (0x8, 0x9, ... , 0xf)
            PrimitiveTypeEntry          = 0x8,
            EnumTypeEntry               = 0x9,
            ContainerTypeEntry          = 0xa,
            FunctionTypeEntry           = 0xb,
            CustomTypeEntry             = 0xc,
            SmartPointerTypeEntry       = 0xd,
            TypeEntryMask               = 0xf,

            // Documentation tags
            InjectDocumentation         = 0x10,
            ModifyDocumentation         = 0x20,
            DocumentationMask           = 0xf0,

            // Simple tags (0x100, 0x200, ... , 0xf00)
            ExtraIncludes               = 0x0100,
            Include                     = 0x0200,
            ModifyFunction              = 0x0300,
            ModifyField                 = 0x0400,
            Root                        = 0x0500,
            CustomMetaConstructor       = 0x0600,
            CustomMetaDestructor        = 0x0700,
            ArgumentMap                 = 0x0800,
            SuppressedWarning           = 0x0900,
            Rejection                   = 0x0a00,
            LoadTypesystem              = 0x0b00,
            RejectEnumValue             = 0x0c00,
            Template                    = 0x0d00,
            TemplateInstanceEnum        = 0x0e00,
            Replace                     = 0x0f00,
            AddFunction                 = 0x1000,
            NativeToTarget              = 0x1100,
            TargetToNative              = 0x1200,
            AddConversion               = 0x1300,
            SimpleMask                  = 0x3f00,

            // Code snip tags (0x1000, 0x2000, ... , 0xf000)
            InjectCode                  = 0x4000,
            InjectCodeInFunction        = 0x8000,
            CodeSnipMask                = 0xc000,

            // Function modifier tags (0x010000, 0x020000, ... , 0xf00000)
            Access                      = 0x010000,
            Removal                     = 0x020000,
            Rename                      = 0x040000,
            ModifyArgument              = 0x080000,
            Thread                      = 0x100000,
            FunctionModifiers           = 0xff0000,

            // Argument modifier tags (0x01000000 ... 0xf0000000)
            ConversionRule              = 0x01000000,
            ReplaceType                 = 0x02000000,
            ReplaceDefaultExpression    = 0x04000000,
            RemoveArgument              = 0x08000000,
            DefineOwnership             = 0x10000000,
            RemoveDefaultExpression     = 0x20000000,
            NoNullPointers              = 0x40000000,
            ReferenceCount              = 0x80000000,
            ParentOwner                 = 0x90000000,
            Array                       = 0xA0000000,
            ArgumentModifiers           = 0xff000000
        };

        StackElement(StackElement *p) : entry(0), type(None), parent(p) { }

        TypeEntry* entry;
        ElementType type;
        StackElement *parent;

        union {
            TemplateInstance* templateInstance;
            TemplateEntry* templateEntry;
            CustomFunction* customFunction;
        } value;
};

struct StackElementContext
{
    CodeSnipList codeSnips;
    AddedFunctionList addedFunctions;
    FunctionModificationList functionMods;
    FieldModificationList fieldMods;
    DocModificationList docModifications;
};

class Handler
{
public:
    Handler(TypeDatabase* database, bool generate);

    bool parse(QXmlStreamReader &reader);

private:
    bool startElement(const QStringRef& localName, const QXmlStreamAttributes& atts);
    bool handleSmartPointerEntry(StackElement *element,
                                 QHash<QString, QString> &attributes,
                                 const QString &name,
                                 double since);
    bool endElement(const QStringRef& localName);
    template <class String> // QString/QStringRef
    bool characters(const String &ch);
    void fetchAttributeValues(const QString &name, const QXmlStreamAttributes &atts,
                              QHash<QString, QString> *acceptedAttributes);

    bool importFileElement(const QXmlStreamAttributes &atts);
    void addFlags(const QString &name, QString flagName,
                  const QHash<QString, QString> &attributes, double since);

    TypeDatabase* m_database;
    StackElement* m_current;
    StackElement* m_currentDroppedEntry;
    int m_currentDroppedEntryDepth;
    int m_ignoreDepth;
    QString m_defaultPackage;
    QString m_defaultSuperclass;
    QString m_error;
    TypeEntry::CodeGeneration m_generate;

    EnumTypeEntry* m_currentEnum;
    QStack<StackElementContext*> m_contextStack;

    QHash<QString, StackElement::ElementType> tagNames;
    QString m_currentSignature;
    QString m_currentPath;
};

#endif
