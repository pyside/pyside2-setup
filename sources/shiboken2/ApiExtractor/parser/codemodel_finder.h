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


#ifndef CODEMODEL_FINDER_H
#define CODEMODEL_FINDER_H

#include <default_visitor.h>
#include <codemodel_fwd.h>
#include <name_compiler.h>

class TokenStream;
class Binder;

class CodeModelFinder: protected DefaultVisitor
{
    enum ResolvePolicy {
        ResolveScope,
        ResolveItem
    };

public:
    CodeModelFinder(CodeModel *model, Binder *binder);
    virtual ~CodeModelFinder();

    ScopeModelItem resolveScope(NameAST *name, ScopeModelItem scope);

    inline CodeModel *model() const
    {
        return _M_model;
    }

protected:
    virtual void visitName(NameAST *node);
    virtual void visitUnqualifiedName(UnqualifiedNameAST *node);

    ScopeModelItem changeCurrentScope(ScopeModelItem scope);

private:
    CodeModel *_M_model;
    Binder *_M_binder;
    TokenStream *_M_token_stream;
    NameCompiler name_cc;

    ScopeModelItem _M_current_scope;
    ResolvePolicy _M_resolve_policy;
};

#endif // CODEMODEL_FINDER_H
