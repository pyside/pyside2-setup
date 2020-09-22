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
#include "typesystem.h"

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QDebug>
#endif

bool QPropertySpec::isValid() const
{
    return m_type != nullptr && !m_name.isEmpty() && !m_read.isEmpty();
}

QPropertySpec *QPropertySpec::parseQ_Property(AbstractMetaBuilderPrivate *b,
                                              AbstractMetaClass *metaClass,
                                              const QString &declarationIn,
                                              const QStringList &scopes,
                                              QString *errorMessage)
{
    errorMessage->clear();

    // Q_PROPERTY(QString objectName READ objectName WRITE setObjectName NOTIFY objectNameChanged)

    const QString declaration = declarationIn.simplified();
    auto propertyTokens = declaration.splitRef(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (propertyTokens.size()  < 4) {
        *errorMessage = QLatin1String("Insufficient number of tokens");
        return nullptr;
    }

    QString fullTypeName = propertyTokens.takeFirst().toString();
    QString name = propertyTokens.takeFirst().toString();
    // Fix errors like "Q_PROPERTY(QXYSeries *series .." to be of type "QXYSeries*"
    while (name.startsWith(QLatin1Char('*'))) {
        fullTypeName += name.at(0);
        name.remove(0, 1);
    }

    int indirections = 0;
    QString typeName = fullTypeName;
    for (; typeName.endsWith(QLatin1Char('*')); ++indirections)
        typeName.chop(1);

    QScopedPointer<AbstractMetaType> type;
    QString typeError;
    for (int j = scopes.size(); j >= 0 && type.isNull(); --j) {
        QStringList qualifiedName = scopes.mid(0, j);
        qualifiedName.append(typeName);
        TypeInfo info;
        info.setIndirections(indirections);
        info.setQualifiedName(qualifiedName);
        type.reset(b->translateType(info, metaClass, {}, &typeError));
    }

    if (!type) {
        QTextStream str(errorMessage);
        str << "Unable to decide type of property: \"" << name << "\" ("
            <<  typeName << "): " << typeError;
        return nullptr;
    }

    QScopedPointer<QPropertySpec> spec(new QPropertySpec(type->typeEntry()));
    spec->setName(name);
    spec->setIndirections(indirections);

    for (int pos = 0; pos + 1 < propertyTokens.size(); pos += 2) {
        if (propertyTokens.at(pos) == QLatin1String("READ"))
            spec->setRead(propertyTokens.at(pos + 1).toString());
        else if (propertyTokens.at(pos) == QLatin1String("WRITE"))
            spec->setWrite(propertyTokens.at(pos + 1).toString());
        else if (propertyTokens.at(pos) == QLatin1String("DESIGNABLE"))
            spec->setDesignable(propertyTokens.at(pos + 1).toString());
        else if (propertyTokens.at(pos) == QLatin1String("RESET"))
            spec->setReset(propertyTokens.at(pos + 1).toString());
    }

    if (!spec->isValid()) {
        *errorMessage = QLatin1String("Incomplete specification");
        return nullptr;
    }
    return spec.take();
}

#ifndef QT_NO_DEBUG_STREAM
void QPropertySpec::formatDebug(QDebug &d) const
{
    d << '#' << m_index << " \"" << m_name << "\" (" << m_type->qualifiedCppName();
    for (int i = 0; i < m_indirections; ++i)
        d << '*';
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
