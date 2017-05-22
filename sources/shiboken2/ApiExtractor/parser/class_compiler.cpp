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


#include "class_compiler.h"
#include "lexer.h"
#include "binder.h"

ClassCompiler::ClassCompiler(Binder *binder)
        : _M_binder(binder),
        _M_token_stream(binder->tokenStream()),
        name_cc(_M_binder),
        type_cc(_M_binder)
{
}

ClassCompiler::~ClassCompiler()
{
}

void ClassCompiler::run(ClassSpecifierAST *node)
{
    name_cc.run(node->name);
    _M_name = name_cc.name();
    _M_base_classes.clear();

    visit(node);
}

void ClassCompiler::visitClassSpecifier(ClassSpecifierAST *node)
{
    visit(node->base_clause);
}

void ClassCompiler::visitBaseSpecifier(BaseSpecifierAST *node)
{
    name_cc.run(node->name);
    QString name = name_cc.name();

    if (!name.isEmpty())
        _M_base_classes.append(name);
}


// kate: space-indent on; indent-width 2; replace-tabs on;
