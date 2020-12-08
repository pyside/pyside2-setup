/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef ABSTRACTMETALANG_ENUMS_H
#define ABSTRACTMETALANG_ENUMS_H

#include <QtCore/QFlags>

enum class FunctionQueryOption {
    Constructors                 = 0x0000001, // Only constructors
    //Destructors                  = 0x0000002, // Only destructors. Not included in class.
    FinalInTargetLangFunctions   = 0x0000008, // Only functions that are non-virtual in TargetLang
    ClassImplements              = 0x0000020, // Only functions implemented by the current class
    StaticFunctions              = 0x0000080, // Only static functions
    Signals                      = 0x0000100, // Only signals
    NormalFunctions              = 0x0000200, // Only functions that aren't signals
    Visible                      = 0x0000400, // Only public and protected functions
    WasPublic                    = 0x0001000, // Only functions that were originally public
    NonStaticFunctions           = 0x0004000, // No static functions
    Empty                        = 0x0008000, // Empty overrides of abstract functions
    Invisible                    = 0x0010000, // Only private functions
    VirtualInCppFunctions        = 0x0020000, // Only functions that are virtual in C++
    VirtualInTargetLangFunctions = 0x0080000, // Only functions which are virtual in TargetLang
    NotRemoved                   = 0x0400000, // Only functions that have not been removed
    OperatorOverloads            = 0x2000000, // Only functions that are operator overloads
    GenerateExceptionHandling    = 0x4000000,
    GetAttroFunction             = 0x8000000,
    SetAttroFunction            = 0x10000000
};

Q_DECLARE_FLAGS(FunctionQueryOptions, FunctionQueryOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(FunctionQueryOptions)

enum class OperatorQueryOption {
    ArithmeticOp   = 0x01, // Arithmetic: +, -, *, /, %, +=, -=, *=, /=, %=, ++, --, unary+, unary-
    BitwiseOp      = 0x02, // Bitwise: <<, <<=, >>, >>=, ~, &, &=, |, |=, ^, ^=
    ComparisonOp   = 0x04, // Comparison: <, <=, >, >=, !=, ==
    LogicalOp      = 0x08, // Logical: !, &&, ||
    ConversionOp   = 0x10, // Conversion: operator [const] TYPE()
    SubscriptionOp = 0x20, // Subscription: []
    AssignmentOp   = 0x40 // Assignment: =
};

Q_DECLARE_FLAGS(OperatorQueryOptions, OperatorQueryOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(OperatorQueryOptions)

#endif // ABSTRACTMETALANG_ENUMS_H
