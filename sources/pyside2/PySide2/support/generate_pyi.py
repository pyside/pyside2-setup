#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
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

from __future__ import print_function, absolute_import, unicode_literals

"""
generate_pyi.py

This script generates the .pyi files for all PySide modules.
"""

import sys
import os
import io
import re
import PySide2
import subprocess
import argparse
from contextlib import contextmanager
from textwrap import dedent

from PySide2.support.signature import inspect
from PySide2.support.signature.lib.enum_sig import HintingEnumerator
from PySide2 import *  # all modules

# Make sure not to get .pyc in Python2.
sourcepath = os.path.splitext(__file__)[0] + ".py"

# Can we use forward references?
USE_PEP563 = sys.version_info[:2] >= (3, 7)

indent = " " * 4

class Writer(object):
    def __init__(self, outfile):
        self.outfile = outfile

    def print(self, *args, **kw):
        if self.outfile:
            if args == ():
                # This is a Python 2.7 glitch.
                print("", file=self.outfile, **kw)
            else:
                print(*args, file=self.outfile, **kw)


class Formatter(Writer):
    """
    Formatter is formatting the signature listing of an enumerator.

    It is written as context managers in order to avoid many callbacks.
    The separation in formatter and enumerator is done to keep the
    unrelated tasks of enumeration and formatting apart.
    """
    @contextmanager
    def module(self, mod_name):
        self.mod_name = mod_name
        self.print("# Module", mod_name)
        self.print("import shiboken2 as Shiboken")
        from PySide2.support.signature import typing
        typing_str = "from PySide2.support.signature import typing"
        self.print(typing_str)
        self.print()
        self.print("class Object(object): pass")
        self.print()
        self.print("Shiboken.Object = Object")
        self.print()
        # This line will be replaced by the missing imports.
        self.print("IMPORTS")
        yield

    @contextmanager
    def klass(self, class_name, class_str):
        self.class_name = class_name
        spaces = ""
        while "." in class_name:
            spaces += indent
            class_name = class_name.split(".", 1)[-1]
            class_str = class_str.split(".", 1)[-1]
        self.print()
        if not spaces:
            self.print()
        here = self.outfile.tell()
        self.print("{spaces}class {class_str}:".format(**locals()))
        self.print()
        pos = self.outfile.tell()
        self.spaces = spaces
        yield
        if pos == self.outfile.tell():
            # we have not written any function
            self.outfile.seek(here)
            self.outfile.truncate()
            # Note: we cannot use class_str when we have no body.
            self.print("{spaces}class {class_name}: ...".format(**locals()))
        if "<" in class_name:
            # This is happening in QtQuick for some reason:
            ## class QSharedPointer<QQuickItemGrabResult >:
            # We simply skip over this class.
            self.outfile.seek(here)
            self.outfile.truncate()

    @contextmanager
    def function(self, func_name, signature):
        key = func_name
        spaces = indent + self.spaces if self.class_name else ""
        if type(signature) == type([]):
            for sig in signature:
                self.print('{spaces}@typing.overload'.format(**locals()))
                self._function(func_name, sig, spaces)
        else:
            self._function(func_name, signature, spaces)
        yield key

    def _function(self, func_name, signature, spaces):
        # this would be nicer to get somehow together with the signature
        is_meth = re.match("\((\w*)", str(signature)).group(1) == "self"
        if self.class_name and not is_meth:
            self.print('{spaces}@staticmethod'.format(**locals()))
        self.print('{spaces}def {func_name}{signature}: ...'.format(**locals()))


def get_license_text():
    with io.open(sourcepath) as f:
        lines = f.readlines()
        license_line = next((lno for lno, line in enumerate(lines)
                             if "$QT_END_LICENSE$" in line))
    return "".join(lines[:license_line + 3])


def find_imports(text):
    return [imp for imp in PySide2.__all__ if imp + "." in text]


def generate_pyi(mod_name, outpath):
    module = sys.modules[mod_name]
    mod_fullname = module.__file__
    plainname = os.path.basename(mod_fullname).split(".")[0]
    if not outpath:
        outpath = os.path.dirname(mod_fullname)
    outfilepath = os.path.join(outpath, plainname + ".pyi")
    outfile = io.StringIO()
    fmt = Formatter(outfile)
    enu = HintingEnumerator(fmt)
    fmt.print(get_license_text())
    need_imports = not USE_PEP563
    if USE_PEP563:
        fmt.print("from __future__ import annotations")
        fmt.print()
    fmt.print(dedent('''\
        """
        This file contains the exact signatures for all functions in PySide
        for module '{mod_fullname}',
        except for defaults which are replaced by "...".
        """
        '''.format(**locals())))
    enu.module(mod_name)
    fmt.print("# eof")
    with io.open(outfilepath, "w") as realfile:
        wr = Writer(realfile)
        outfile.seek(0)
        while True:
            line = outfile.readline()
            if not line:
                break
            line = line.rstrip()
            # we remove the IMPORTS marker and insert imports if needed
            if line == "IMPORTS":
                if need_imports:
                    for imp in find_imports(outfile.getvalue()):
                        imp = "PySide2." + imp
                        if imp != mod_name:
                            wr.print("import " + imp)
                wr.print("import " + mod_name)
                wr.print()
                wr.print()
            else:
                wr.print(line)

    print(outfilepath, file=sys.stderr)
    if sys.version_info[0] == 3:
        # Python 3: We can check the file directly if the syntax is ok.
        subprocess.check_output([sys.executable, outfilepath])


def generate_all_pyi(outpath=None):
    for mod_name in PySide2.__all__:
        generate_pyi("PySide2." + mod_name, outpath)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="This script generates the .pyi file for all PySide modules.")
    subparsers = parser.add_subparsers(dest="command", metavar="", title="required argument")
    # create the parser for the "run" command
    parser_run = subparsers.add_parser("run", help="run the generation")
    parser_run.add_argument("--outpath")
    args = parser.parse_args()
    outpath = args.outpath
    if not outpath or os.path.exists(outpath):
        generate_all_pyi(args.outpath)
    else:
        print("Please create the directory {outpath} before generating.".format(**locals()))

