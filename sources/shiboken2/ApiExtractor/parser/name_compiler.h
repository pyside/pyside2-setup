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


#ifndef NAME_COMPILER_H
#define NAME_COMPILER_H

#include "default_visitor.h"
#include <QtCore/QStringList>

class TokenStream;
class Binder;

class NameCompiler: protected DefaultVisitor
{
public:
    NameCompiler(Binder *binder);

    void run(NameAST *node) {
        internal_run(node);
    }
    void run(UnqualifiedNameAST *node) {
        internal_run(node);
    }

    QString name() const {
        return _M_name.join(QLatin1String("::"));
    }
    QStringList qualifiedName() const {
        return _M_name;
    }

protected:
    virtual void visitUnqualifiedName(UnqualifiedNameAST *node);
    virtual void visitTemplateArgument(TemplateArgumentAST *node);

    QString internal_run(AST *node);
    QString decode_operator(std::size_t index) const;

private:
    Binder *_M_binder;
    TokenStream *_M_token_stream;
    QStringList _M_name;
};

#endif // NAME_COMPILER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
