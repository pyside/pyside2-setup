# This Python file uses the following encoding: utf-8
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
import subprocess
import argparse
from contextlib import contextmanager
from textwrap import dedent


import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("generate_pyi")


# Make sure not to get .pyc in Python2.
sourcepath = os.path.splitext(__file__)[0] + ".py"

# Can we use forward references?
USE_PEP563 = sys.version_info[:2] >= (3, 7)

indent = " " * 4

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
    @contextmanager
    def module(self, mod_name):
        self.mod_name = mod_name
        self.print("# Module", mod_name)
        self.print("import shiboken2 as Shiboken")
        from PySide2.support.signature import typing
        self.print("from PySide2.support.signature import typing")
        self.print("from PySide2.support.signature.mapping import (")
        self.print("    Virtual, Missing, Invalid, Default, Instance)")
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
        is_meth = re.match(r"\((\w*)", str(signature)).group(1) == "self"
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


def safe_create(filename):
    pid = os.getpid()
    locname = "{filename}.{pid}".format(**locals())
    f = io.open(locname, "w")  # do not close for atomic rename on Linux
    if sys.platform == "win32":
        f.close()
    try:
        os.rename(locname, filename)
        logger.debug("{pid}:File {filename} created".format(**locals()))
        if sys.platform == "win32":
            f = io.open(filename, "w")
        return f
    except OSError:
        logger.debug("{pid}:Could not rename {locname} to {filename}"
                     .format(**locals()))
        try:
            os.remove(locname)
        except OSError as e:
            logger.warning("{pid}: unexpected os.remove error in safe_create: {e}"
                           .format(**locals()))
        return None


def generate_pyi(import_name, outpath, options):
    """
    Generates a .pyi file.

    Returns 1 If the result is valid, else 0.
    """
    pid = os.getpid()
    plainname = import_name.split(".")[-1]
    if not outpath:
        outpath = os.path.dirname(PySide2.__file__)
    outfilepath = os.path.join(outpath, plainname + ".pyi")
    if options.skip and os.path.exists(outfilepath):
        logger.debug("{pid}:Skipped existing: {outfilepath}".format(**locals()))
        return 1
    workpath = outfilepath + ".working"
    if os.path.exists(workpath):
        return 0
    realfile = safe_create(workpath)
    if not realfile:
        return 0

    try:
        top = __import__(import_name)
        obj = getattr(top, plainname)
        if not getattr(obj, "__file__", None) or os.path.isdir(obj.__file__):
            raise ImportError("We do not accept a namespace as module {plainname}"
                              .format(**locals()))
        module = sys.modules[import_name]

        outfile = io.StringIO()
        fmt = Formatter(outfile)
        enu = HintingEnumerator(fmt)
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
        enu.module(import_name)
        fmt.print()
        fmt.print("# eof")

    except ImportError as e:
        logger.debug("{pid}:Import problem with module {plainname}: {e}".format(**locals()))
        try:
            os.remove(workpath)
        except OSError as e:
            logger.warning("{pid}: unexpected os.remove error in generate_pyi: {e}"
                           .format(**locals()))
        return 0

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
    realfile.close()

    if os.path.exists(outfilepath):
        os.remove(outfilepath)
    try:
        os.rename(workpath, outfilepath)
    except OSError:
        logger.warning("{pid}: probable duplicate generated: {outfilepath}"#
                       .format(**locals()))
        return 0
    logger.info("Generated: {outfilepath}".format(**locals()))
    if sys.version_info[0] == 3:
        # Python 3: We can check the file directly if the syntax is ok.
        subprocess.check_output([sys.executable, outfilepath])
    return 1


def generate_all_pyi(outpath, options):
    ps = os.pathsep
    if options.sys_path:
        # make sure to propagate the paths from sys_path to subprocesses
        sys_path = [os.path.normpath(_) for _ in options.sys_path]
        sys.path[0:0] = sys_path
        pypath = ps.join(sys_path)
        os.environ["PYTHONPATH"] = pypath
    if options.lib_path:
        # the path changes are automatically propagated to subprocesses
        ospath_var = "PATH" if sys.platform == "win32" else "LD_LIBRARY_PATH"
        old_val = os.environ.get(ospath_var, "")
        lib_path = [os.path.normpath(_) for _ in options.lib_path]
        ospath = ps.join(lib_path + old_val.split(ps))
        os.environ[ospath_var] = ospath

    # now we can import
    global PySide2, inspect, HintingEnumerator
    import PySide2
    from PySide2.support.signature import inspect
    from PySide2.support.signature.lib.enum_sig import HintingEnumerator

    valid = 0
    for mod_name in PySide2.__all__:
        import_name = "PySide2."  + mod_name
        valid += generate_pyi(import_name, outpath, options)

    npyi = len(PySide2.__all__)
    if valid == npyi:
        logger.info("+++ All {npyi} .pyi files have been created.".format(**locals()))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")
    # create the parser for the "run" command
    parser_run = subparsers.add_parser("run",
        help="run the generation",
        description="This script generates the .pyi file for all PySide modules.")
    parser_run.add_argument("--skip", action="store_true",
        help="skip existing files")
    parser_run.add_argument("--outpath",
        help="the output directory (default = binary location)")
    parser_run.add_argument("--sys-path", nargs="+",
        help="a list of strings prepended to sys.path")
    parser_run.add_argument("--lib-path", nargs="+",
        help="a list of strings prepended to LD_LIBRARY_PATH (unix) or PATH (windows)")
    options = parser.parse_args()
    if options.command == "run":
        outpath = options.outpath
        if outpath and not os.path.exists(outpath):
            os.makedirs(outpath)
            logger.info("+++ Created path {outpath}".format(**locals()))
        generate_all_pyi(outpath, options=options)
    else:
        parser_run.print_help()
        sys.exit(1)
