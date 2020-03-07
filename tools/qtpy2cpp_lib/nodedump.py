#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""Helper to dump AST nodes for debugging"""


import ast


def to_string(node):
    """Helper to retrieve a string from the (Lists of )Name/Attribute
       aggregated into some nodes"""
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Attribute):
        return node.attr
    return ''


def debug_format_node(node):
    """Format AST node for debugging"""
    if isinstance(node, ast.alias):
        return f'alias("{node.name}")'
    if isinstance(node, ast.arg):
        return f'arg({node.arg})'
    if isinstance(node, ast.Attribute):
        if isinstance(node.value, ast.Name):
            nested_name = debug_format_node(node.value)
            return f'Attribute("{node.attr}", {nested_name})'
        return f'Attribute("{node.attr}")'
    if isinstance(node, ast.Call):
        return 'Call({}({}))'.format(to_string(node.func), len(node.args))
    if isinstance(node, ast.ClassDef):
        base_names = [to_string(base) for base in node.bases]
        bases = ': ' + ','.join(base_names) if base_names else ''
        return f'ClassDef({node.name}{bases})'
    if isinstance(node, ast.ImportFrom):
        return f'ImportFrom("{node.module}")'
    if isinstance(node, ast.FunctionDef):
        arg_names = [a.arg for a in node.args.args]
        return 'FunctionDef({}({}))'.format(node.name, ', '.join(arg_names))
    if isinstance(node, ast.Name):
        return 'Name("{}", Ctx={})'.format(node.id, type(node.ctx).__name__)
    if isinstance(node, ast.NameConstant):
        return f'NameConstant({node.value})'
    if isinstance(node, ast.Num):
        return f'Num({node.n})'
    if isinstance(node, ast.Str):
        return f'Str("{node.s}")'
    return type(node).__name__
