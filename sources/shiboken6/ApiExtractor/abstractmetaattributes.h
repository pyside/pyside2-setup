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

#ifndef ABSTRACTMETAATTRIBUTES_H
#define ABSTRACTMETAATTRIBUTES_H

#include "documentation.h"

#include <QtCore/qobjectdefs.h>

class AbstractMetaAttributes
{
    Q_GADGET
public:
    Q_DISABLE_COPY(AbstractMetaAttributes)

    AbstractMetaAttributes();
    virtual ~AbstractMetaAttributes();

    enum Attribute {
        None                        = 0x00000000,

        Private                     = 0x00000001,
        Protected                   = 0x00000002,
        Public                      = 0x00000004,
        Friendly                    = 0x00000008,
        Visibility                  = 0x0000000f,

        Abstract                    = 0x00000020,
        Static                      = 0x00000040,

        FinalInTargetLang           = 0x00000080,

        GetterFunction              = 0x00000400,
        SetterFunction              = 0x00000800,

        PropertyReader              = 0x00004000,
        PropertyWriter              = 0x00008000,
        PropertyResetter            = 0x00010000,

        Invokable                   = 0x00040000,

        HasRejectedConstructor      = 0x00080000,
        HasRejectedDefaultConstructor = 0x00100000,

        FinalCppClass               = 0x00200000,
        VirtualCppMethod            = 0x00400000,
        OverriddenCppMethod         = 0x00800000,
        FinalCppMethod              = 0x01000000,
        // Add by meta builder (implicit constructors, inherited methods, etc)
        AddedMethod                 = 0x02000000,
        Deprecated                  = 0x04000000
    };
    Q_DECLARE_FLAGS(Attributes, Attribute)
    Q_FLAG(Attribute)

    Attributes attributes() const { return m_attributes; }

    void setAttributes(Attributes attributes) { m_attributes = attributes; }

    Attributes originalAttributes() const { return m_originalAttributes; }

    void setOriginalAttributes(Attributes attributes) { m_originalAttributes = attributes; }

    Attributes visibility() const { return m_attributes & Visibility; }

    void setVisibility(Attributes visi)
    {
        m_attributes = (m_attributes & ~Visibility) | visi;
    }

    void operator+=(Attribute attribute)
    {
        m_attributes |= attribute;
    }

    void operator-=(Attribute attribute)
    {
        m_attributes &= ~attribute;
    }

    bool isFinalInTargetLang() const
    {
        return m_attributes.testFlag(FinalInTargetLang);
    }

    bool isAbstract() const
    {
        return m_attributes.testFlag(Abstract);
    }

    bool isStatic() const
    {
        return m_attributes.testFlag(Static);
    }

    bool isInvokable() const
    {
        return m_attributes.testFlag(Invokable);
    }

    bool isPropertyReader() const
    {
        return m_attributes.testFlag(PropertyReader);
    }

    bool isPropertyWriter() const
    {
        return m_attributes.testFlag(PropertyWriter);
    }

    bool isPropertyResetter() const
    {
        return m_attributes.testFlag(PropertyResetter);
    }

    bool isPrivate() const
    {
        return m_attributes.testFlag(Private);
    }

    bool isProtected() const
    {
        return m_attributes.testFlag(Protected);
    }

    bool isPublic() const
    {
        return m_attributes.testFlag(Public);
    }

    bool isFriendly() const
    {
        return m_attributes.testFlag(Friendly);
    }

    bool wasPrivate() const
    {
        return m_originalAttributes.testFlag(Private);
    }

    bool wasPublic() const
    {
        return m_originalAttributes.testFlag(Public);
    }

    void setDocumentation(const Documentation& doc)
    {
        m_doc = doc;
    }

    Documentation documentation() const
    {
        return m_doc;
    }

#ifndef QT_NO_DEBUG_STREAM
    static void formatMetaAttributes(QDebug &d, AbstractMetaAttributes::Attributes value);
#endif

protected:
    void assignMetaAttributes(const AbstractMetaAttributes &other);

private:
    Attributes m_attributes;
    Attributes m_originalAttributes;
    Documentation m_doc;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaAttributes::Attributes)

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaAttributes *aa);
#endif

#endif // ABSTRACTMETAATTRIBUTES_H
