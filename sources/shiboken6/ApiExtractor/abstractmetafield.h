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

#ifndef ABSTRACTMETAFIELD_H
#define ABSTRACTMETAFIELD_H

#include "abstractmetalang_typedefs.h"
#include "documentation.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "abstractmetaattributes.h"
#include "enclosingclassmixin.h"
#include "abstractmetatype.h"
#include "documentation.h"

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMetaVariable
{
    Q_DISABLE_COPY(AbstractMetaVariable)
public:
    AbstractMetaVariable();

    virtual ~AbstractMetaVariable();

    const AbstractMetaType &type() const { return m_type; }
    void setType(const AbstractMetaType &type) { m_type = type; }

    QString name() const { return m_name; }
    void setName(const QString &name, bool realName = true)
    {
        m_name = name;
        m_hasName = realName;
    }
    bool hasName() const { return m_hasName; }

    QString originalName() const { return m_originalName; }
    void setOriginalName(const QString& name) { m_originalName = name; }

    Documentation documentation() const { return m_doc; }
    void setDocumentation(const Documentation& doc) { m_doc = doc; }

protected:
    void assignMetaVariable(const AbstractMetaVariable &other);

private:
    QString m_originalName;
    QString m_name;
    AbstractMetaType m_type;
    bool m_hasName = false;

    Documentation m_doc;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaVariable *av);
#endif

class AbstractMetaField : public AbstractMetaVariable, public AbstractMetaAttributes, public EnclosingClassMixin
{
public:
    AbstractMetaField();

    FieldModificationList modifications() const;

    bool isModifiedRemoved(int types = TypeSystem::All) const;

    AbstractMetaField *copy() const;

    static AbstractMetaField *
        find(const AbstractMetaFieldList &haystack, const QString &needle);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaField *af);
#endif

#endif // ABSTRACTMETAFIELD_H
