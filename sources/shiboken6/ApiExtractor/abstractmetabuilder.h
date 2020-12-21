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

#ifndef ABSTRACTMETABUILDER_H
#define ABSTRACTMETABUILDER_H

#include "abstractmetalang_typedefs.h"
#include "header_paths.h"
#include "typesystem_enums.h"

#include "clangparser/compilersupport.h"

#include <QFileInfoList>

#include <optional>

QT_FORWARD_DECLARE_CLASS(QIODevice)

class AbstractMetaBuilderPrivate;
class AbstractMetaClass;
class AbstractMetaType;
class AbstractMetaEnumValue;
class TypeInfo;
class TypeEntry;

class AbstractMetaBuilder
{
public:
    enum RejectReason {
        NotInTypeSystem,
        GenerationDisabled,
        RedefinedToNotClass,
        UnmatchedArgumentType,
        UnmatchedReturnType,
        ApiIncompatible,
        Deprecated,
        NoReason
    };

    AbstractMetaBuilder();
    virtual ~AbstractMetaBuilder();

    const AbstractMetaClassList &classes() const;
    const AbstractMetaClassList &templates() const;
    const AbstractMetaClassList &smartPointers() const;
    const AbstractMetaFunctionCList &globalFunctions() const;
    const AbstractMetaEnumList &globalEnums() const;
    const QHash<const TypeEntry *, AbstractMetaEnum> &typeEntryToEnumsHash() const;

    bool build(const QByteArrayList &arguments,
               LanguageLevel level = LanguageLevel::Default,
               unsigned clangFlags = 0);
    void setLogDirectory(const QString& logDir);

    /**
    *   AbstractMetaBuilder should know what's the global header being used,
    *   so any class declared under this header wont have the include file
    *   filled.
    */
    void setGlobalHeaders(const QFileInfoList& globalHeaders);
    void setHeaderPaths(const HeaderPaths &h);

    void setSkipDeprecated(bool value);

    enum TranslateTypeFlag {
        DontResolveType = 0x1
    };
    Q_DECLARE_FLAGS(TranslateTypeFlags, TranslateTypeFlag);

    static std::optional<AbstractMetaType>
        translateType(const TypeInfo &_typei, AbstractMetaClass *currentClass = nullptr,
                      TranslateTypeFlags flags = {}, QString *errorMessage = nullptr);
    static std::optional<AbstractMetaType>
        translateType(const QString &t, AbstractMetaClass *currentClass = nullptr,
                      TranslateTypeFlags flags = {}, QString *errorMessage = nullptr);

    static QString getSnakeCaseName(const QString &name);
    // Names under which an item will be registered to Python depending on snakeCase
    static QStringList definitionNames(const QString &name,
                                       TypeSystem::SnakeCase snakeCase);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    friend class AbstractMetaBuilderPrivate;
    AbstractMetaBuilderPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractMetaBuilder::TranslateTypeFlags);

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaBuilder &ab);
#endif

#endif // ABSTRACTMETBUILDER_H
