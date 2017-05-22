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


#include "control.h"
#include "lexer.h"

Control::Control()
        : current_context(0),
        _M_skipFunctionBody(false),
        _M_lexer(0),
        _M_parser(0)
{
    pushContext();

    declareTypedef(findOrInsertName("__builtin_va_list",
                                    strlen("__builtin_va_list")), 0);
}

Control::~Control()
{
    popContext();

    Q_ASSERT(current_context == 0);
}

Lexer *Control::changeLexer(Lexer *lexer)
{
    Lexer *old = _M_lexer;
    _M_lexer = lexer;
    return old;
}

Parser *Control::changeParser(Parser *parser)
{
    Parser *old = _M_parser;
    _M_parser = parser;
    return old;
}

Type *Control::lookupType(const NameSymbol *name) const
{
    Q_ASSERT(current_context != 0);

    return current_context->resolve(name);
}

void Control::declare(const NameSymbol *name, Type *type)
{
    //printf("*** Declare:");
    //printSymbol(name);
    //putchar('\n');
    Q_ASSERT(current_context != 0);

    current_context->bind(name, type);
}

void Control::pushContext()
{
    // printf("+Context\n");
    Context *new_context = new Context;
    new_context->parent = current_context;
    current_context = new_context;
}

void Control::popContext()
{
    // printf("-Context\n");
    Q_ASSERT(current_context != 0);

    Context *old_context = current_context;
    current_context = current_context->parent;

    delete old_context;
}

void Control::declareTypedef(const NameSymbol *name, Declarator *d)
{
    //  printf("declared typedef:");
    //  printSymbol(name);
    //  printf("\n");
    stl_typedef_table.insert(name, d);
}

bool Control::isTypedef(const NameSymbol *name) const
{
    //  printf("is typedef:");
    //  printSymbol(name);
    // printf("= %d\n", (stl_typedef_table.find(name) != stl_typedef_table.end()));

    return stl_typedef_table.contains(name);
}

QList<Control::ErrorMessage> Control::errorMessages() const
{
    return _M_error_messages;
}

void Control::clearErrorMessages()
{
    _M_error_messages.clear();
}

void Control::reportError(const ErrorMessage &errmsg)
{
    _M_error_messages.append(errmsg);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
