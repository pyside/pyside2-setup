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


#include "declarator_compiler.h"
#include "name_compiler.h"
#include "type_compiler.h"
#include "compiler_utils.h"
#include "lexer.h"
#include "binder.h"
#include "tokens.h"

#include <qdebug.h>

DeclaratorCompiler::DeclaratorCompiler(Binder *binder)
        : _M_binder(binder), _M_token_stream(binder->tokenStream())
{
}

void DeclaratorCompiler::run(DeclaratorAST *node)
{
    _M_id.clear();
    _M_parameters.clear();
    _M_array.clear();
    _M_function = false;
    _M_reference = false;
    _M_variadics = false;
    _M_indirection = 0;

    if (node) {
        NameCompiler name_cc(_M_binder);

        DeclaratorAST *decl = node;
        while (decl && decl->sub_declarator)
            decl = decl->sub_declarator;

        Q_ASSERT(decl != 0);

        name_cc.run(decl->id);
        _M_id = name_cc.name();
        _M_function = (node->parameter_declaration_clause != 0);
        if (node->parameter_declaration_clause && node->parameter_declaration_clause->ellipsis)
            _M_variadics = true;

        visitNodes(this, node->ptr_ops);
        visit(node->parameter_declaration_clause);

        if (const ListNode<ExpressionAST*> *it = node->array_dimensions) {
            it->toFront();
            const ListNode<ExpressionAST*> *end = it;

            do {
                QString elt;
                if (ExpressionAST *expr = it->element) {
                    const Token &start_token = _M_token_stream->token((int) expr->start_token);
                    const Token &end_token = _M_token_stream->token((int) expr->end_token);

                    elt += QString::fromUtf8(&start_token.text[start_token.position],
                                             (int)(end_token.position - start_token.position)).trimmed();
                }

                _M_array.append(elt);

                it = it->next;
            } while (it != end);
        }
    }
}

void DeclaratorCompiler::visitPtrOperator(PtrOperatorAST *node)
{
    std::size_t op =  _M_token_stream->kind(node->op);

    switch (op) {
    case '&':
        _M_reference = true;
        break;
    case '*':
        ++_M_indirection;
        break;

    default:
        break;
    }

    if (node->mem_ptr) {
#if defined(__GNUC__)
#warning "ptr to mem -- not implemented"
#endif
    }
}

void DeclaratorCompiler::visitParameterDeclaration(ParameterDeclarationAST *node)
{
    Parameter p;
    DeclaratorCompiler decl_cc(_M_binder);

    // Find the innermost declarator, to extract the name / id of the declaration.
    DeclaratorAST *declarator = node->declarator;
    while (declarator && declarator->sub_declarator)
        declarator = declarator->sub_declarator;
    decl_cc.run(declarator);
    p.name = decl_cc.id();

    // Use the original declarator to extract the type.
    p.type = CompilerUtils::typeDescription(node->type_specifier, node->declarator, _M_binder);

    // In case if the declarator is a function pointer, extract the arguments of the declarator
    // parameter clause. This only works for top-declarator function pointers, it will fail to
    // determine nested function pointers.
    if (declarator != node->declarator
        && node->declarator->parameter_declaration_clause) {
        p.type.setFunctionPointer(true);
        decl_cc.run(node->declarator);
        foreach (const DeclaratorCompiler::Parameter &innerParam, decl_cc.parameters())
            p.type.addArgument(innerParam.type);
    }

    if (node->expression != 0) {
        const Token &start = _M_token_stream->token((int) node->expression->start_token);
        const Token &end = _M_token_stream->token((int) node->expression->end_token);
        int length = (int)(end.position - start.position);

        p.defaultValueExpression = QString();
        QString source = QString::fromUtf8(&start.text[start.position], length).trimmed();
        QStringList list = source.split(QLatin1Char('\n'));


        for (int i = 0; i < list.size(); ++i) {
            if (!list.at(i).startsWith(QLatin1Char('#')))
                p.defaultValueExpression += list.at(i).trimmed();
        }

        p.defaultValue = p.defaultValueExpression.size() > 0;

    }

    _M_parameters.append(p);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
