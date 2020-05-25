# This Python file uses the following encoding: utf-8
#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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
import subprocess
import argparse
from contextlib import contextmanager
from textwrap import dedent
import logging


# Make sure not to get .pyc in Python2.
sourcepath = os.path.splitext(__file__)[0] + ".py"

# Can we use forward references?
USE_PEP563 = sys.version_info[:2] >= (3, 7)

indent = " " * 4
is_py3 = sys.version_info[0] == 3
is_ci = os.environ.get("QTEST_ENVIRONMENT", "") == "ci"
is_debug = is_ci or os.environ.get("QTEST_ENVIRONMENT")

logging.basicConfig(level=logging.DEBUG if is_debug else logging.INFO)
logger = logging.getLogger("generate_pyi")


class Writer(object):
    def __init__(self, outfile):
        self.outfile = outfile
        self.history = [True, True]

    def print(self, *args, **kw):
        # controlling too much blank lines
        if self.outfile:
            if args == () or args == ("",):
                # Python 2.7 glitch: Empty tuples have wrong encoding.
                # But we use that to skip too many blank lines:
                if self.history[-2:] == [True, True]:
                    return
                print("", file=self.outfile, **kw)
                self.history.append(True)
            else:
                print(*args, file=self.outfile, **kw)
                self.history.append(False)


class Formatter(Writer):
    """
    Formatter is formatting the signature listing of an enumerator.

    It is written as context managers in order to avoid many callbacks.
    The separation in formatter and enumerator is done to keep the
    unrelated tasks of enumeration and formatting apart.
    """
    def __init__(self, *args):
        Writer.__init__(self, *args)
        # patching __repr__ to disable the __repr__ of typing.TypeVar:
        """
            def __repr__(self):
                if self.__covariant__:
                    prefix = '+'
                elif self.__contravariant__:
                    prefix = '-'
                else:
                    prefix = '~'
                return prefix + self.__name__
        """
        def _typevar__repr__(self):
            return "typing." + self.__name__
        typing.TypeVar.__repr__ = _typevar__repr__

        # Adding a pattern to substitute "Union[T, NoneType]" by "Optional[T]"
        # I tried hard to replace typing.Optional by a simple override, but
        # this became _way_ too much.
        # See also the comment in layout.py .
        brace_pat = build_brace_pattern(3)
        pattern = (r"\b Union \s* \[ \s* {brace_pat} \s*, \s* NoneType \s* \]"
                   .format(**locals()))
        replace = r"Optional[\1]"
        optional_searcher = re.compile(pattern, flags=re.VERBOSE)
        def optional_replacer(source):
            return optional_searcher.sub(replace, str(source))
        self.optional_replacer = optional_replacer
        # self.level is maintained by enum_sig.py
        # self.after_enum() is a one-shot set by enum_sig.py .

    @contextmanager
    def module(self, mod_name):
        self.mod_name = mod_name
        self.print("# Module", mod_name)
        self.print("import PySide2")
        from PySide2.support.signature import typing
        self.print("try:")
        self.print("    import typing")
        self.print("except ImportError:")
        self.print("    from PySide2.support.signature import typing")
        self.print("from PySide2.support.signature.mapping import (")
        self.print("    Virtual, Missing, Invalid, Default, Instance)")
        self.print()
        self.print("class Object(object): pass")
        self.print()
        self.print("import shiboken2 as Shiboken")
        self.print("Shiboken.Object = Object")
        self.print()
        # This line will be replaced by the missing imports postprocess.
        self.print("IMPORTS")
        yield

    @contextmanager
    def klass(self, class_name, class_str):
        spaces = indent * self.level
        while "." in class_name:
            class_name = class_name.split(".", 1)[-1]
            class_str = class_str.split(".", 1)[-1]
        self.print()
        if self.level == 0:
            self.print()
        here = self.outfile.tell()
        if self.have_body:
            self.print("{spaces}class {class_str}:".format(**locals()))
        else:
            self.print("{spaces}class {class_str}: ...".format(**locals()))
        yield

    @contextmanager
    def function(self, func_name, signature, modifier=None):
        if self.after_enum() or func_name == "__init__":
            self.print()
        key = func_name
        spaces = indent * self.level
        if type(signature) == type([]):
            for sig in signature:
                self.print('{spaces}@typing.overload'.format(**locals()))
                self._function(func_name, sig, modifier, spaces)
        else:
            self._function(func_name, signature, modifier, spaces)
        if func_name == "__init__":
            self.print()
        yield key

    def _function(self, func_name, signature, modifier, spaces):
        if modifier:
            self.print('{spaces}@{modifier}'.format(**locals()))
        signature = self.optional_replacer(signature)
        self.print('{spaces}def {func_name}{signature}: ...'.format(**locals()))

    @contextmanager
    def enum(self, class_name, enum_name, value):
        spaces = indent * self.level
        hexval = hex(value)
        self.print("{spaces}{enum_name:25}: {class_name} = ... # {hexval}".format(**locals()))
        yield


def get_license_text():
    with io.open(sourcepath) as f:
        lines = f.readlines()
        license_line = next((lno for lno, line in enumerate(lines)
                             if "$QT_END_LICENSE$" in line))
    return "".join(lines[:license_line + 3])


def find_imports(text):
    return [imp for imp in PySide2.__all__ if imp + "." in text]


def generate_pyi(import_name, outpath, options):
    """
    Generates a .pyi file.
    """
    plainname = import_name.split(".")[-1]
    outfilepath = os.path.join(outpath, plainname + ".pyi")
    top = __import__(import_name)
    obj = getattr(top, plainname)
    if not getattr(obj, "__file__", None) or os.path.isdir(obj.__file__):
        raise ModuleNotFoundError("We do not accept a namespace as module "
                                  "{plainname}".format(**locals()))
    module = sys.modules[import_name]

    outfile = io.StringIO()
    fmt = Formatter(outfile)
    fmt.print(get_license_text())  # which has encoding, already
    need_imports = not USE_PEP563
    if USE_PEP563:
        fmt.print("from __future__ import annotations")
        fmt.print()
    fmt.print(dedent('''\
        """
        This file contains the exact signatures for all functions in module
        {import_name}, except for defaults which are replaced by "...".
        """
        '''.format(**locals())))
    HintingEnumerator(fmt).module(import_name)
    fmt.print()
    fmt.print("# eof")
    # Postprocess: resolve the imports
    with open(outfilepath, "w") as realfile:
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
                    for mod_name in find_imports(outfile.getvalue()):
                        imp = "PySide2." + mod_name
                        if imp != import_name:
                            wr.print("import " + imp)
                wr.print("import " + import_name)
                wr.print()
                wr.print()
            else:
                wr.print(line)
    logger.info("Generated: {outfilepath}".format(**locals()))
    if is_py3 and (options.check or is_ci):
        # Python 3: We can check the file directly if the syntax is ok.
        subprocess.check_output([sys.executable, outfilepath])


def generate_all_pyi(outpath, options):
    ps = os.pathsep
    if options.sys_path:
        # make sure to propagate the paths from sys_path to subprocesses
        sys_path = [os.path.normpath(_) for _ in options.sys_path]
        sys.path[0:0] = sys_path
        pypath = ps.join(sys_path)
        os.environ["PYTHONPATH"] = pypath

    # now we can import
    global PySide2, inspect, typing, HintingEnumerator, build_brace_pattern
    import PySide2
    from PySide2.support.signature import inspect, typing
    from PySide2.support.signature.lib.enum_sig import HintingEnumerator
    from PySide2.support.signature.lib.tool import build_brace_pattern

    # propagate USE_PEP563 to the mapping module.
    # Perhaps this can be automated?
    PySide2.support.signature.mapping.USE_PEP563 = USE_PEP563

    outpath = outpath or os.path.dirname(PySide2.__file__)
    name_list = PySide2.__all__ if options.modules == ["all"] else options.modules
    errors = ", ".join(set(name_list) - set(PySide2.__all__))
    if errors:
        raise ImportError("The module(s) '{errors}' do not exist".format(**locals()))
    quirk1, quirk2 = "QtMultimedia", "QtMultimediaWidgets"
    if name_list == [quirk1]:
        logger.debug("Note: We must defer building of {quirk1}.pyi until {quirk2} "
                     "is available".format(**locals()))
        name_list = []
    elif name_list == [quirk2]:
        name_list = [quirk1, quirk2]
    for mod_name in name_list:
        import_name = "PySide2." + mod_name
        generate_pyi(import_name, outpath, options)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="This script generates the .pyi file for all PySide modules.")
    parser.add_argument("modules", nargs="+",
        help="'all' or the names of modules to build (QtCore QtGui etc.)")
    parser.add_argument("--quiet", action="store_true", help="Run quietly")
    parser.add_argument("--check", action="store_true", help="Test the output if on Python 3")
    parser.add_argument("--outpath",
        help="the output directory (default = binary location)")
    parser.add_argument("--sys-path", nargs="+",
        help="a list of strings prepended to sys.path")
    options = parser.parse_args()
    if options.quiet:
        logger.setLevel(logging.WARNING)
    outpath = options.outpath
    if outpath and not os.path.exists(outpath):
        os.makedirs(outpath)
        logger.info("+++ Created path {outpath}".format(**locals()))
    generate_all_pyi(outpath, options=options)
# eof
