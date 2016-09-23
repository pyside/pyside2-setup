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


#include "dumptree.h"

#include <QtCore/QString>
#include <QtCore/qdebug.h>

static char const * const names[] = {
    0,
    "AccessSpecifier",
    "AsmDefinition",
    "BaseClause",
    "BaseSpecifier",
    "BinaryExpression",
    "CastExpression",
    "ClassMemberAccess",
    "ClassSpecifier",
    "CompoundStatement",
    "Condition",
    "ConditionalExpression",
    "CppCastExpression",
    "CtorInitializer",
    "DeclarationStatement",
    "Declarator",
    "DeleteExpression",
    "DoStatement",
    "ElaboratedTypeSpecifier",
    "EnumSpecifier",
    "Enumerator",
    "ExceptionSpecification",
    "ExpressionOrDeclarationStatement",
    "ExpressionStatement",
    "ForStatement",
    "FunctionCall",
    "FunctionDefinition",
    "IfStatement",
    "IncrDecrExpression",
    "InitDeclarator",
    "Initializer",
    "InitializerClause",
    "LabeledStatement",
    "LinkageBody",
    "LinkageSpecification",
    "MemInitializer",
    "Name",
    "Namespace",
    "NamespaceAliasDefinition",
    "NewDeclarator",
    "NewExpression",
    "NewInitializer",
    "NewTypeId",
    "Operator",
    "OperatorFunctionId",
    "ParameterDeclaration",
    "ParameterDeclarationClause",
    "PostfixExpression",
    "PrimaryExpression",
    "PtrOperator",
    "PtrToMember",
    "ReturnStatement",
    "SimpleDeclaration",
    "SimpleTypeSpecifier",
    "SizeofExpression",
    "StringLiteral",
    "SubscriptExpression",
    "SwitchStatement",
    "TemplateArgument",
    "TemplateDeclaration",
    "TemplateParameter",
    "ThrowExpression",
    "TranslationUnit",
    "TryBlockStatement",
    "TypeId",
    "TypeIdentification",
    "TypeParameter",
    "Typedef",
    "UnaryExpression",
    "UnqualifiedName",
    "Using",
    "UsingDirective",
    "WhileStatement",
    "WinDeclSpec"
};

DumpTree::DumpTree()
{
}

void DumpTree::visit(AST *node)
{
    static int indent = 0;


    if (node)
        qDebug() << QByteArray(indent * 2, ' ').constData() << names[node->kind]
        << '[' << node->start_token << ", " << node->end_token << ']';

    ++indent;
    DefaultVisitor::visit(node);
    --indent;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
