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

"""Tool to dump a Python AST"""


from argparse import ArgumentParser, RawTextHelpFormatter
import ast
from enum import Enum
import sys
import tokenize


from nodedump import debug_format_node

DESCRIPTION = "Tool to dump a Python AST"


_source_lines = []
_opt_verbose = False


def first_non_space(s):
    for i, c in enumerate(s):
        if c != ' ':
            return i
    return 0


class NodeType(Enum):
    IGNORE = 1
    PRINT_ONE_LINE = 2  # Print as a one liner, do not visit children
    PRINT = 3  # Print with opening closing tag, visit children
    PRINT_WITH_SOURCE = 4   # Like PRINT, but print source line above


def get_node_type(node):
    if isinstance(node, (ast.Load, ast.Store, ast.Delete)):
        return NodeType.IGNORE
    if isinstance(node, (ast.Add, ast.alias, ast.arg, ast.Eq, ast.Gt, ast.Lt,
                         ast.Mult, ast.Name, ast.NotEq, ast.NameConstant, ast.Not,
                         ast.Num, ast.Str)):
        return NodeType.PRINT_ONE_LINE
    if not hasattr(node, 'lineno'):
        return NodeType.PRINT
    if isinstance(node, (ast.Attribute)):
        return NodeType.PRINT_ONE_LINE if isinstance(node.value, ast.Name) else NodeType.PRINT
    return NodeType.PRINT_WITH_SOURCE


class DumpVisitor(ast.NodeVisitor):
    def __init__(self):
        ast.NodeVisitor.__init__(self)
        self._indent = 0
        self._printed_source_lines = {-1}

    def generic_visit(self, node):
        node_type = get_node_type(node)
        if _opt_verbose and node_type in (NodeType.IGNORE, NodeType.PRINT_ONE_LINE):
            node_type = NodeType.PRINT
        if node_type == NodeType.IGNORE:
            return
        self._indent = self._indent + 1
        indent = '    ' * self._indent

        if node_type == NodeType.PRINT_WITH_SOURCE:
            line_number = node.lineno - 1
            if line_number not in self._printed_source_lines:
                self._printed_source_lines.add(line_number)
                line = _source_lines[line_number]
                non_space = first_non_space(line)
                print('{:04d} {}{}'.format(line_number, '_' * non_space,
                                           line[non_space:]))

        if node_type == NodeType.PRINT_ONE_LINE:
            print(indent, debug_format_node(node))
        else:
            print(indent, '>', debug_format_node(node))
            ast.NodeVisitor.generic_visit(self, node)
            print(indent, '<', type(node).__name__)

        self._indent = self._indent - 1


def parse_ast(filename):
    node = None
    with tokenize.open(filename) as f:
        global _source_lines
        source = f.read()
        _source_lines = source.split('\n')
        node = ast.parse(source, mode="exec")
    return node


def create_arg_parser(desc):
    parser = ArgumentParser(description=desc,
                            formatter_class=RawTextHelpFormatter)
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Verbose')
    parser.add_argument('source', type=str, help='Python source')
    return parser


if __name__ == '__main__':
    arg_parser = create_arg_parser(DESCRIPTION)
    options = arg_parser.parse_args()
    _opt_verbose = options.verbose
    title = f'AST tree for {options.source}'
    print('=' * len(title))
    print(title)
    print('=' * len(title))
    tree = parse_ast(options.source)
    DumpVisitor().visit(tree)
