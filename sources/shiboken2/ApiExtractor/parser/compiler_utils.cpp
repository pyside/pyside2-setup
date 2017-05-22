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


#include "compiler_utils.h"
#include "type_compiler.h"
#include "name_compiler.h"
#include "declarator_compiler.h"
#include "ast.h"
#include "binder.h"

TypeInfo CompilerUtils::typeDescription(TypeSpecifierAST *type_specifier, DeclaratorAST *declarator, Binder *binder)
{
    TypeCompiler type_cc(binder);
    DeclaratorCompiler decl_cc(binder);

    type_cc.run(type_specifier);
    decl_cc.run(declarator);

    TypeInfo typeInfo;
    typeInfo.setQualifiedName(type_cc.qualifiedName());
    typeInfo.setConstant(type_cc.isConstant());
    typeInfo.setVolatile(type_cc.isVolatile());
    typeInfo.setReferenceType(decl_cc.isReference() ? LValueReference : NoReference);
    typeInfo.setIndirections(decl_cc.indirection());
    typeInfo.setArrayElements(decl_cc.arrayElements());

    return typeInfo;
}
