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

from argparse import ArgumentParser, RawTextHelpFormatter
import logging
import os
import sys
from qtpy2cpp_lib.visitor import ConvertVisitor


DESCRIPTION = "Tool to convert Python to C++"


def create_arg_parser(desc):
    parser = ArgumentParser(description=desc,
                            formatter_class=RawTextHelpFormatter)
    parser.add_argument('--debug', '-d', action='store_true',
                        help='Debug')
    parser.add_argument('--stdout', '-s', action='store_true',
                        help='Write to stdout')
    parser.add_argument('--force', '-f', action='store_true',
                        help='Force overwrite of existing files')
    parser.add_argument('file', type=str, help='Python source file')
    return parser


if __name__ == '__main__':
    if sys.version_info < (3, 6, 0):
        raise Exception("This script requires Python 3.6")
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger(__name__)
    arg_parser = create_arg_parser(DESCRIPTION)
    args = arg_parser.parse_args()
    ConvertVisitor.debug = args.debug

    input_file = args.file
    if not os.path.isfile(input_file):
        logger.error(f'{input_file} does not exist or is not a file.')
        sys.exit(-1)
    file_root, ext = os.path.splitext(input_file)
    if ext != '.py':
        logger.error(f'{input_file} does not appear to be a Python file.')
        sys.exit(-1)

    ast_tree = ConvertVisitor.create_ast(input_file)
    if args.stdout:
        sys.stdout.write(f'// Converted from {input_file}\n')
        ConvertVisitor(sys.stdout).visit(ast_tree)
        sys.exit(0)

    target_file = file_root + '.cpp'
    if os.path.exists(target_file):
        if not os.path.isfile(target_file):
            logger.error(f'{target_file} exists and is not a file.')
            sys.exit(-1)
        if not args.force:
            logger.error(f'{target_file} exists. Use -f to overwrite.')
            sys.exit(-1)

    with open(target_file, "w") as file:
        file.write(f'// Converted from {input_file}\n')
        ConvertVisitor(file).visit(ast_tree)
        logger.info(f"Wrote {target_file} ...")
