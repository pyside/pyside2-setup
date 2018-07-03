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
#include "dependency.h"

#include "clangparser/compilersupport.h"

QT_FORWARD_DECLARE_CLASS(QIODevice)

class AbstractMetaBuilderPrivate;
class AbstractMetaClass;
class AbstractMetaType;
class AbstractMetaEnumValue;
class TypeInfo;

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
        NoReason
    };

    AbstractMetaBuilder();
    virtual ~AbstractMetaBuilder();

    AbstractMetaClassList classes() const;
    AbstractMetaClassList templates() const;
    AbstractMetaClassList smartPointers() const;
    AbstractMetaFunctionList globalFunctions() const;
    AbstractMetaEnumList globalEnums() const;

    /**
    *   Sorts a list of classes topologically, if an AbstractMetaClass object
    *   is passed the list of classes will be its inner classes, otherwise
    *   the list will be the module global classes.
    *   \return a list of classes sorted topologically
    */
    AbstractMetaClassList classesTopologicalSorted(const AbstractMetaClass *cppClass = Q_NULLPTR,
                                                   const Dependencies &additionalDependencies = Dependencies()) const;

    bool build(const QByteArrayList &arguments,
               LanguageLevel level = LanguageLevel::Default,
               unsigned clangFlags = 0);
    void setLogDirectory(const QString& logDir);

    /**
    *   AbstractMetaBuilder should know what's the global header being used,
    *   so any class declared under this header wont have the include file
    *   filled.
    */
    void setGlobalHeader(const QString& globalHeader);

    static AbstractMetaType *translateType(const TypeInfo &_typei,
                                           AbstractMetaClass *currentClass = nullptr,
                                           bool resolveType = true,
                                           QString *errorMessage = nullptr);
    static AbstractMetaType *translateType(const QString &t,
                                           AbstractMetaClass *currentClass = nullptr,
                                           bool resolveType = true,
                                           QString *errorMessage = nullptr);


#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    friend class AbstractMetaBuilderPrivate;
    AbstractMetaBuilderPrivate *d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaBuilder &ab);
#endif

#endif // ABSTRACTMETBUILDER_H
