/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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


#include "expression.h"
#include <sstream>

Expression::Expression() : m_value(0), m_operation(None), m_operand1(nullptr), m_operand2(nullptr)
{
}

Expression::Expression(int number) : m_value(number), m_operation(None), m_operand1(nullptr), m_operand2(nullptr)
{
}

Expression::Expression(const Expression& other)
{
    m_operand1 = other.m_operand1 ? new Expression(*other.m_operand1) : nullptr;
    m_operand2 = other.m_operand2 ? new Expression(*other.m_operand2) : nullptr;
    m_value = other.m_value;
    m_operation = other.m_operation;
}

Expression& Expression::operator=(const Expression& other)
{
    delete m_operand1;
    delete m_operand2;
    m_operand1 = other.m_operand1 ? new Expression(*other.m_operand1) : nullptr;
    m_operand2 = other.m_operand2 ? new Expression(*other.m_operand2) : nullptr;
    m_operation = other.m_operation;
    m_value = other.m_value;
    return *this;
}

Expression::~Expression()
{
    delete m_operand1;
    delete m_operand2;
}

Expression Expression::operator+(const Expression& other)
{
    Expression expr;
    expr.m_operation = Add;
    expr.m_operand1 = new Expression(*this);
    expr.m_operand2 = new Expression(other);
    return expr;
}

Expression Expression::operator-(const Expression& other)
{
    Expression expr;
    expr.m_operation = Add;
    expr.m_operand1 = new Expression(*this);
    expr.m_operand2 = new Expression(other);
    return expr;
}

Expression Expression::operator<(const Expression& other)
{
    Expression expr;
    expr.m_operation = LessThan;
    expr.m_operand1 = new Expression(*this);
    expr.m_operand2 = new Expression(other);
    return expr;
}

Expression Expression::operator>(const Expression& other)
{
    Expression expr;
    expr.m_operation = GreaterThan;
    expr.m_operand1 = new Expression(*this);
    expr.m_operand2 = new Expression(other);
    return expr;
}

std::string Expression::toString() const
{
    if (m_operation == None) {
        std::ostringstream s;
        s << m_value;
        return s.str();
    }

    std::string result;
    result += '(';
    result += m_operand1->toString();
    char op;
    switch (m_operation) {
        case Add:
            op = '+';
            break;
        case Sub:
            op = '-';
            break;
        case LessThan:
            op = '<';
            break;
        case GreaterThan:
            op = '<';
            break;
        case None: // just to avoid the compiler warning
        default:
            op = '?';
            break;
    }
    result += op;
    result += m_operand2->toString();
    result += ')';
    return result;
}

