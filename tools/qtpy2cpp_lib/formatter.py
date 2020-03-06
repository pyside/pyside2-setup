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

"""C++ formatting helper functions and formatter class"""


import ast
import sys


CLOSING = {"{": "}", "(": ")", "[": "]"}  # Closing parenthesis for C++


def to_string(node):
    """Helper to retrieve a string from the (Lists of)Name/Attribute
       aggregated into some nodes"""
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Attribute):
        return node.attr
    return ''


def format_inheritance(class_def_node):
    """Returns inheritance specification of a class"""
    result = ''
    for base in class_def_node.bases:
        name = to_string(base)
        if name != 'object':
            result += ', public ' if result else ' : public '
            result += name
    return result


def format_for_target(target_node):
    if isinstance(target_node, ast.Tuple):  # for i,e in enumerate()
        result = ''
        for i, el in enumerate(target_node):
            if i > 0:
                result += ', '
            result += format_reference(el)
        return result
    return format_reference(target_node)


def format_for_loop(f_node):
    """Format a for loop
       This applies some heuristics to detect:
       1) "for a in [1,2])" -> "for (f: {1, 2}) {"
       2) "for i in range(5)" -> "for (i = 0; i < 5; ++i) {"
       3) "for i in range(2,5)" -> "for (i = 2; i < 5; ++i) {"

       TODO: Detect other cases, maybe including enumerate().
    """
    loop_vars = format_for_target(f_node.target)
    result = 'for (' + loop_vars
    if isinstance(f_node.iter, ast.Call):
        f = format_reference(f_node.iter.func)
        if f == 'range':
            start = 0
            end = -1
            if len(f_node.iter.args) == 2:
                start = format_literal(f_node.iter.args[0])
                end = format_literal(f_node.iter.args[1])
            elif len(f_node.iter.args) == 1:
                end = format_literal(f_node.iter.args[0])
            result += f' = {start}; {loop_vars} < {end}; ++{loop_vars}'
    elif isinstance(f_node.iter, ast.List):
        # Range based for over list
        result += ': ' + format_literal_list(f_node.iter)
    result += ') {'
    return result


def format_literal(node):
    """Returns the value of number/string literals"""
    if isinstance(node, ast.Num):
        return str(node.n)
    if isinstance(node, ast.Str):
        # Fixme: escaping
        return f'"{node.s}"'
    return ''


def format_literal_list(l_node, enclosing='{'):
    """Formats a list/tuple of number/string literals as C++ initializer list"""
    result = enclosing
    for i, el in enumerate(l_node.elts):
        if i > 0:
            result += ', '
        result += format_literal(el)
    result += CLOSING[enclosing]
    return result


def format_member(attrib_node, qualifier='auto'):
    """Member access foo->member() is expressed as an attribute with
       further nested Attributes/Names as value"""
    n = attrib_node
    result = ''
    # Black magic: Guess '::' if name appears to be a class name
    if qualifier == 'auto':
        qualifier = '::' if n.attr[0:1].isupper() else '->'
    while isinstance(n, ast.Attribute):
        result = n.attr if not result else n.attr + qualifier + result
        n = n.value
    if isinstance(n, ast.Name) and n.id != 'self':
        result = n.id + qualifier + result
    return result


def format_reference(node, qualifier='auto'):
    """Format member reference or free item"""
    return node.id if isinstance(node, ast.Name) else format_member(node, qualifier)


def format_function_def_arguments(function_def_node):
    """Formats arguments of a function definition"""
    # Default values is a list of the last default values, expand
    # so that indexes match
    argument_count = len(function_def_node.args.args)
    default_values = function_def_node.args.defaults
    while len(default_values) < argument_count:
        default_values.insert(0, None)
    result = ''
    for i, a in enumerate(function_def_node.args.args):
        if result:
            result += ', '
        if a.arg != 'self':
            result += a.arg
            if default_values[i]:
                result += ' = '
                result += format_literal(default_values[i])
    return result


def format_start_function_call(call_node):
    """Format a call of a free or member function"""
    return format_reference(call_node.func) + '('


def write_import(file, i_node):
    """Print an import of a Qt class as #include"""
    for alias in i_node.names:
        if alias.name.startswith('Q'):
            file.write(f'#include <{alias.name}>\n')


def write_import_from(file, i_node):
    """Print an import from Qt classes as #include sequence"""
    # "from PySide2.QtGui import QGuiApplication" or
    # "from PySide2 import QtGui"
    mod = i_node.module
    if not mod.startswith('PySide') and not mod.startswith('PyQt'):
        return
    dot = mod.find('.')
    qt_module = mod[dot + 1:] + '/' if dot >= 0 else ''
    for i in i_node.names:
        if i.name.startswith('Q'):
            file.write(f'#include <{qt_module}{i.name}>\n')


class Indenter:
    """Helper for Indentation"""

    def __init__(self, output_file):
        self._indent_level = 0
        self._indentation = ''
        self._output_file = output_file

    def indent_string(self, string):
        """Start a new line by a string"""
        self._output_file.write(self._indentation)
        self._output_file.write(string)

    def indent_line(self, line):
        """Write an indented line"""
        self._output_file.write(self._indentation)
        self._output_file.write(line)
        self._output_file.write('\n')

    def INDENT(self):
        """Write indentation"""
        self._output_file.write(self._indentation)

    def indent(self):
        """Increase indentation level"""
        self._indent_level = self._indent_level + 1
        self._indentation = '    ' * self._indent_level

    def dedent(self):
        """Decrease indentation level"""
        self._indent_level = self._indent_level - 1
        self._indentation = '    ' * self._indent_level


class CppFormatter(Indenter):
    """Provides helpers for formatting multi-line C++ constructs"""

    def __init__(self, output_file):
        Indenter.__init__(self, output_file)

    def write_class_def(self, class_node):
        """Print a class definition with inheritance"""
        self._output_file.write('\n')
        inherits = format_inheritance(class_node)
        self.indent_line(f'class {class_node.name}{inherits}')
        self.indent_line('{')
        self.indent_line('public:')

    def write_function_def(self, f_node, class_context):
        """Print a function definition with arguments"""
        self._output_file.write('\n')
        arguments = format_function_def_arguments(f_node)
        warn = True
        if f_node.name == '__init__' and class_context:  # Constructor
            name = class_context
            warn = len(arguments) > 0
        elif f_node.name == '__del__' and class_context:  # Destructor
            name = '~' + class_context
            warn = False
        else:
            name = 'void ' + f_node.name
        self.indent_string(f'{name}({arguments})')
        if warn:
            self._output_file.write(' /* FIXME: types */')
        self._output_file.write('\n')
        self.indent_line('{')
