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

#include "propertyspec.h"
#include "abstractmetalang.h"
#include "abstractmetabuilder_p.h"
#include "codemodel.h"
#include "messages.h"
#include "typesystem.h"

#include <QtCore/QHash>

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QDebug>
#endif

#include <algorithm>

QPropertySpec::QPropertySpec(const TypeSystemProperty &ts,
                             const AbstractMetaType *type) :
    m_name(ts.name),
    m_read(ts.read),
    m_write(ts.write),
    m_designable(ts.designable),
    m_reset(ts.reset),
    m_type(type),
    m_generateGetSetDef(ts.generateGetSetDef)
{
}

QPropertySpec::~QPropertySpec()
{
    delete m_type;
}

bool QPropertySpec::isValid() const
{
    return m_type != nullptr && !m_name.isEmpty() && !m_read.isEmpty();
}

const TypeEntry *QPropertySpec::typeEntry() const
{
    return m_type->typeEntry();
}

// Parse a Q_PROPERTY macro
// Q_PROPERTY(QString objectName READ objectName WRITE setObjectName NOTIFY objectNameChanged)
// into a TypeSystemProperty.
TypeSystemProperty QPropertySpec::typeSystemPropertyFromQ_Property(const QString &declarationIn,
                                                                   QString *errorMessage)
{
    enum class PropertyToken { None, Read, Write, Designable, Reset };

    static const QHash<QString, PropertyToken> tokenLookup = {
        {QStringLiteral("READ"), PropertyToken::Read},
        {QStringLiteral("WRITE"), PropertyToken::Write},
        {QStringLiteral("DESIGNABLE"), PropertyToken::Designable},
        {QStringLiteral("RESET"), PropertyToken::Reset}
    };

    errorMessage->clear();

    TypeSystemProperty result;

    // Q_PROPERTY(QString objectName READ objectName WRITE setObjectName NOTIFY objectNameChanged)

    const QString declaration = declarationIn.simplified();
    auto propertyTokens = declaration.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    // To properly parse complicated type declarations like
    // "Q_PROPERTY(const QList<QString > *objectName READ objectName ..."
    // we first search the first "READ" token, parse the subsequent tokens and
    // extract type and name from the tokens before "READ".
    const auto it = std::find_if(propertyTokens.cbegin(), propertyTokens.cend(),
                                 [](const QString &t) { return tokenLookup.contains(t); });
    if (it == propertyTokens.cend()) {
        *errorMessage = QLatin1String("Invalid property specification, READ missing");
        return result;
    }

    const int firstToken = int(it - propertyTokens.cbegin());
    if (firstToken < 2) {
        *errorMessage = QLatin1String("Insufficient number of tokens in property specification");
        return result;
    }

    for (int pos = firstToken; pos + 1 < propertyTokens.size(); pos += 2) {
        switch (tokenLookup.value(propertyTokens.at(pos))) {
        case PropertyToken::Read:
            result.read = propertyTokens.at(pos + 1);
            break;
        case PropertyToken::Write:
            result.write = propertyTokens.at(pos + 1);
            break;
        case PropertyToken::Reset:
            result.reset = propertyTokens.at(pos + 1);
            break;
        case PropertyToken::Designable:
            result.designable = propertyTokens.at(pos + 1);
            break;
        case PropertyToken::None:
            break;
        }
    }

    const int namePos = firstToken - 1;
    result.name = propertyTokens.at(namePos);

    result.type = propertyTokens.constFirst();
    for (int pos = 1; pos < namePos; ++pos)
        result.type += QLatin1Char(' ') + propertyTokens.at(pos);

    // Fix errors like "Q_PROPERTY(QXYSeries *series .." to be of type "QXYSeries*"
    while (!result.name.isEmpty() && !result.name.at(0).isLetter()) {
        result.type += result.name.at(0);
        result.name.remove(0, 1);
    }
    if (!result.isValid())
        *errorMessage = QLatin1String("Incomplete property specification");
    return result;
}

// Create a QPropertySpec from a TypeSystemProperty, determining
// the AbstractMetaType from the type string.
QPropertySpec *QPropertySpec::fromTypeSystemProperty(AbstractMetaBuilderPrivate *b,
                                                     AbstractMetaClass *metaClass,
                                                     const TypeSystemProperty &ts,
                                                     const QStringList &scopes,
                                                     QString *errorMessage)
 {
     Q_ASSERT(ts.isValid());
     QString typeError;
     TypeInfo info = TypeParser::parse(ts.type, &typeError);
     if (info.qualifiedName().isEmpty()) {
         *errorMessage = msgPropertyTypeParsingFailed(ts.name, ts.type, typeError);
         return nullptr;
     }

     AbstractMetaType *type = b->translateType(info, metaClass, {}, &typeError);
     if (!type) {
         const QStringList qualifiedName = info.qualifiedName();
         for (int j = scopes.size(); j >= 0 && type == nullptr; --j) {
             info.setQualifiedName(scopes.mid(0, j) + qualifiedName);
             type = b->translateType(info, metaClass, {}, &typeError);
         }
     }

     if (!type) {
         *errorMessage = msgPropertyTypeParsingFailed(ts.name, ts.type, typeError);
         return nullptr;
     }
     return new QPropertySpec(ts, type);
 }

// Convenience to create a QPropertySpec from a Q_PROPERTY macro
// via TypeSystemProperty.
QPropertySpec *QPropertySpec::parseQ_Property(AbstractMetaBuilderPrivate *b,
                                              AbstractMetaClass *metaClass,
                                              const QString &declarationIn,
                                              const QStringList &scopes,
                                              QString *errorMessage)
{
    const TypeSystemProperty ts =
        typeSystemPropertyFromQ_Property(declarationIn, errorMessage);
    return ts.isValid()
        ? fromTypeSystemProperty(b, metaClass, ts, scopes, errorMessage)
        : nullptr;
}

#ifndef QT_NO_DEBUG_STREAM
void QPropertySpec::formatDebug(QDebug &d) const
{
    d << '#' << m_index << " \"" << m_name << "\" (" << m_type->cppSignature();
    d << "), read=" << m_read;
    if (!m_write.isEmpty())
        d << ", write=" << m_write;
    if (!m_reset.isEmpty())
          d << ", reset=" << m_reset;
    if (!m_designable.isEmpty())
          d << ", designable=" << m_designable;
}

QDebug operator<<(QDebug d, const QPropertySpec &p)
{
    QDebugStateSaver s(d);
    d.noquote();
    d.nospace();
    d << "QPropertySpec(";
    p.formatDebug(d);
    d << ')';
    return d;
}
#endif // QT_NO_DEBUG_STREAM
