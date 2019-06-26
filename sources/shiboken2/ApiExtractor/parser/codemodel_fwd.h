/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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


#ifndef CODEMODEL_FWD_H
#define CODEMODEL_FWD_H

#include <QtCore/QVector>
#include <QtCore/QSharedPointer>

// forward declarations
class CodeModel;
class _ArgumentModelItem;
class _ClassModelItem;
class _CodeModelItem;
class _EnumModelItem;
class _EnumeratorModelItem;
class _FileModelItem;
class _FunctionModelItem;
class _NamespaceModelItem;
class _ScopeModelItem;
class _TemplateParameterModelItem;
class _TypeDefModelItem;
class _VariableModelItem;
class _MemberModelItem;
class TypeInfo;

using ArgumentModelItem = QSharedPointer<_ArgumentModelItem>;
using ClassModelItem = QSharedPointer<_ClassModelItem>;
using CodeModelItem = QSharedPointer<_CodeModelItem>;
using EnumModelItem = QSharedPointer<_EnumModelItem>;
using EnumeratorModelItem = QSharedPointer<_EnumeratorModelItem>;
using FileModelItem = QSharedPointer<_FileModelItem>;
using FunctionModelItem = QSharedPointer<_FunctionModelItem>;
using NamespaceModelItem = QSharedPointer<_NamespaceModelItem>;
using ScopeModelItem = QSharedPointer<_ScopeModelItem>;
using TemplateParameterModelItem = QSharedPointer<_TemplateParameterModelItem>;
using TypeDefModelItem = QSharedPointer<_TypeDefModelItem>;
using VariableModelItem = QSharedPointer<_VariableModelItem>;
using MemberModelItem = QSharedPointer<_MemberModelItem>;

using ArgumentList = QVector<ArgumentModelItem>;
using ClassList = QVector<ClassModelItem>;
using ItemList = QVector<CodeModelItem>;
using EnumList = QVector<EnumModelItem>;
using EnumeratorList = QVector<EnumeratorModelItem>;
using FileList = QVector<FileModelItem>;
using FunctionList = QVector<FunctionModelItem>;
using NamespaceList = QVector<NamespaceModelItem>;
using ScopeList = QVector<ScopeModelItem>;
using TemplateParameterList = QVector<TemplateParameterModelItem>;
using TypeDefList = QVector<TypeDefModelItem>;
using VariableList = QVector<VariableModelItem>;
using MemberList = QVector<MemberModelItem>;

#endif // CODEMODEL_FWD_H
