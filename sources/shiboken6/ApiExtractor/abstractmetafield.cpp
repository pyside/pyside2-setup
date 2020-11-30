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

#include "abstractmetafield.h"
#include "abstractmetabuilder.h"
#include "abstractmetalang.h"
#include "abstractmetatype.h"
#include "documentation.h"
#include "modifications.h"
#include "typesystem.h"

#include <QtCore/QDebug>

class AbstractMetaFieldData : public QSharedData
{
public:
    QString m_originalName;
    QString m_name;
    AbstractMetaType m_type;
    Documentation m_doc;
    bool m_setterEnabled = true; // Modifications
    bool m_getterEnabled = true; // Modifications
};

AbstractMetaField::AbstractMetaField() : d(new AbstractMetaFieldData)
{
}

AbstractMetaField::AbstractMetaField(const AbstractMetaField &) = default;
AbstractMetaField &AbstractMetaField::operator=(const AbstractMetaField &) = default;
AbstractMetaField::AbstractMetaField(AbstractMetaField &&) = default;
AbstractMetaField &AbstractMetaField::operator=(AbstractMetaField &&) = default;
AbstractMetaField::~AbstractMetaField() = default;

//    returned->setEnclosingClass(nullptr);

std::optional<AbstractMetaField>
    AbstractMetaField::find(const AbstractMetaFieldList &haystack,
                            const QString &needle)
{
    for (const auto &f : haystack) {
        if (f.name() == needle)
            return f;
    }
    return {};
}

/*******************************************************************************
 * Indicates that this field has a modification that removes it
 */
bool AbstractMetaField::isModifiedRemoved(int types) const
{
    const FieldModificationList &mods = modifications();
    for (const FieldModification &mod : mods) {
        if (mod.isRemoved())
            return true;
    }

    return false;
}

const AbstractMetaType &AbstractMetaField::type() const
{
    return d->m_type;
}

void AbstractMetaField::setType(const AbstractMetaType &type)
{
    if (d->m_type != type)
        d->m_type = type;
}

QString AbstractMetaField::name() const
{
    return d->m_name;
}

void AbstractMetaField::setName(const QString &name)
{
    if (d->m_name != name)
        d->m_name = name;
}

QString AbstractMetaField::qualifiedCppName() const
{
    return enclosingClass()->qualifiedCppName() + QLatin1String("::")
           + originalName();
}

QStringList AbstractMetaField::definitionNames() const
{
    return AbstractMetaBuilder::definitionNames(d->m_name, snakeCase());
}

QString AbstractMetaField::originalName() const
{
    return d->m_originalName.isEmpty() ? d->m_name : d->m_originalName;
}

void AbstractMetaField::setOriginalName(const QString &name)
{
    if (d->m_originalName != name)
        d->m_originalName = name;
}

const Documentation &AbstractMetaField::documentation() const
{
    return d->m_doc;
}

void AbstractMetaField::setDocumentation(const Documentation &doc)
{
    if (d->m_doc != doc)
        d->m_doc = doc;
}

bool AbstractMetaField::isGetterEnabled() const
{
    return d->m_getterEnabled;
}

void AbstractMetaField::setGetterEnabled(bool e)
{
    if (d->m_getterEnabled != e)
        d->m_getterEnabled = e;
}

bool AbstractMetaField::isSetterEnabled() const
{
    return d->m_setterEnabled;
}

void AbstractMetaField::setSetterEnabled(bool e)
{
    if (e != d->m_setterEnabled)
        d->m_setterEnabled = e;
}

bool AbstractMetaField::canGenerateGetter() const
{
    return d->m_getterEnabled && !isStatic();
}

bool AbstractMetaField::canGenerateSetter() const
{
    return d->m_setterEnabled && !isStatic()
        && (!d->m_type.isConstant() || d->m_type.isPointerToConst());
}

TypeSystem::SnakeCase AbstractMetaField::snakeCase() const
{
    // Renamed?
    if (!d->m_originalName.isEmpty() && d->m_originalName != d->m_name)
        return TypeSystem::SnakeCase::Disabled;

    for (const auto &mod : modifications()) {
        if (mod.snakeCase() != TypeSystem::SnakeCase::Unspecified)
            return mod.snakeCase();
    }

    auto typeEntry = enclosingClass()->typeEntry();
    const auto snakeCase = typeEntry->snakeCase();
    return snakeCase != TypeSystem::SnakeCase::Unspecified
        ? snakeCase : typeEntry->typeSystemTypeEntry()->snakeCase();
}

FieldModificationList AbstractMetaField::modifications() const
{
    const FieldModificationList &mods = enclosingClass()->typeEntry()->fieldModifications();
    FieldModificationList returned;

    for (const FieldModification &mod : mods) {
        if (mod.name() == name())
            returned += mod;
    }

    return returned;
}

#ifndef QT_NO_DEBUG_STREAM
void AbstractMetaField::formatDebug(QDebug &d) const
{
    AbstractMetaAttributes::formatMetaAttributes(d, attributes());
    d << ' ' << type().name() << " \"" << name() << '"';
}

QDebug operator<<(QDebug d, const AbstractMetaField *af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaField(";
    if (af)
        af->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaField &af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaField(";
    af.formatDebug(d);
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
