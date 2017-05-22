/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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

#include "codemodel_finder.h"
#include "codemodel.h"
#include "binder.h"

CodeModelFinder::CodeModelFinder(CodeModel *model, Binder *binder)
        : _M_model(model),
        _M_binder(binder),
        _M_token_stream(binder->tokenStream()),
        name_cc(_M_binder),
        _M_resolve_policy(ResolveItem)
{
}

CodeModelFinder::~CodeModelFinder()
{
}

ScopeModelItem CodeModelFinder::resolveScope(NameAST *name, ScopeModelItem scope)
{
    Q_ASSERT(scope);

    ResolvePolicy saved_resolve_policy = _M_resolve_policy;
    _M_resolve_policy = ResolveScope;

    ScopeModelItem old = changeCurrentScope(scope);

    visit(name);
    ScopeModelItem result = _M_current_scope;

    changeCurrentScope(old); // restore

    _M_resolve_policy = saved_resolve_policy;

    return result;
}

ScopeModelItem CodeModelFinder::changeCurrentScope(ScopeModelItem scope)
{
    ScopeModelItem old = _M_current_scope;
    _M_current_scope = scope;
    return old;
}

void CodeModelFinder::visitName(NameAST *node)
{
    visitNodes(this, node->qualified_names);

    if (_M_resolve_policy == ResolveItem)
        visit(node->unqualified_name);
}

void CodeModelFinder::visitUnqualifiedName(UnqualifiedNameAST *node)
{
    if (!_M_current_scope) {
        // nothing to do
        return;
    }

    name_cc.run(node);
    QString id = name_cc.name();

    if (ClassModelItem klass = _M_current_scope->findClass(id)) {
        _M_current_scope = klass;
    } else if (NamespaceModelItem parentNamespace = qSharedPointerDynamicCast<_NamespaceModelItem>(_M_current_scope)) {
        NamespaceModelItem ns = parentNamespace->findNamespace(id);
        _M_current_scope = ns;
    } else if (FileModelItem file = qSharedPointerDynamicCast<_FileModelItem>(_M_current_scope)) {
        NamespaceModelItem ns = file->findNamespace(id);
        _M_current_scope = ns;
    }
}

// kate: space-indent on; indent-width 2; replace-tabs on;

