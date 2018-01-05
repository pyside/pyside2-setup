#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide2.
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

from __future__ import print_function, absolute_import

"""
Existence registry

This is a registry for all existing function signatures.
One file is generated with all signatures of a platform and version.
"""

import sys
import os
import PySide2
from contextlib import contextmanager
from textwrap import dedent

all_modules = list("PySide2." + x for x in PySide2.__all__)

from PySide2.support.signature import inspect
from PySide2.QtCore import __version__

is_py3 = sys.version_info[0] == 3
is_ci = os.environ.get("QTEST_ENVIRONMENT", "") == "ci"
# Python2 legacy: Correct 'linux2' to 'linux', recommended way.
platform = 'linux' if sys.platform.startswith('linux') else sys.platform
# Make sure not to get .pyc in Python2.
sourcepath = os.path.splitext(__file__)[0] + ".py"

def qtVersion():
    return tuple(map(int, __version__.split(".")))

# Format a registry file name for version
def _registryFileName(version):
    name = "exists_{}_{}_{}_{}{}.py".format(platform,
        version[0], version[1], version[2], "_ci" if is_ci else "")
    return os.path.join(os.path.dirname(__file__), name)

# Return the expected registry file name
def getRefPath():
    return _registryFileName(qtVersion())

# Return the registry file name, either that of the current
# version or fall back to a previous patch release
def getEffectiveRefPath():
    refpath = getRefPath()
    if os.path.exists(refpath):
        return refpath
    version = qtVersion()
    majorVersion = version[0]
    minorVersion = version[1]
    patchVersion = version[2]
    while patchVersion >= 0:
        file = _registryFileName((majorVersion, minorVersion, patchVersion))
        if os.path.exists(file):
            return file
        patchVersion = patchVersion - 1
    return refpath

class Formatter(object):
    """
    Formatter is formatting the signature listing of an enumerator.

    It is written as context managers in order to avoid many callbacks.
    The division in formatter and enumerator is done to keep the
    unrelated tasks of enumeration and formatting apart.
    """
    def __init__(self, outfile):
        self.outfile = outfile

    def print(self, *args, **kw):
        print(*args, file=self.outfile, **kw) if self.outfile else None

    @contextmanager
    def module(self, mod_name):
        self.mod_name = mod_name
        self.print("")
        self.print("# Module", mod_name)
        self.print('if "{}" in sys.modules:'.format(mod_name))
        self.print("    dict.update({")
        yield
        self.print("    })")

    @contextmanager
    def klass(self, class_name):
        self.class_name = class_name
        self.print()
        self.print("    # class {}.{}:".format(self.mod_name, class_name))
        yield

    @contextmanager
    def function(self, func_name, signature):
        key = viskey = "{}.{}".format(self.class_name, func_name)
        if key.endswith("lY"):
            # Some classes like PySide2.QtGui.QContextMenuEvent have functions
            # globalX and the same with Y. The gerrit robot thinks that this
            # is a badly written "globally". Convince it by hiding this word.
            viskey = viskey[:-1] + '""Y'
        self.print('        "{}": {},'.format(viskey, signature))
        yield key


class ExactEnumerator(object):
    """
    ExactEnumerator enumerates all signatures in a module as they are.

    This class is used for generating complete listings of all signatures.
    An appropriate formatter should be supplied, if printable output
    is desired.
    """
    def __init__(self, formatter, result_type=dict):
        self.fmt = formatter
        self.result_type = result_type

    def module(self, mod_name):
        __import__(mod_name)
        with self.fmt.module(mod_name):
            module = sys.modules[mod_name]
            members = inspect.getmembers(module, inspect.isclass)
            ret = self.result_type()
            for class_name, klass in members:
                ret.update(self.klass(class_name, klass))
            return ret

    def klass(self, class_name, klass):
        with self.fmt.klass(class_name):
            ret = self.function("__init__", klass)
            # class_members = inspect.getmembers(klass)
            # gives us also the inherited things.
            class_members = sorted(list(klass.__dict__.items()))
            for func_name, func in class_members:
                ret.update(self.function(func_name, func))
            return ret

    def function(self, func_name, func):
        ret = self.result_type()
        signature = getattr(func, '__signature__', None)
        if signature is not None:
            with self.fmt.function(func_name, signature) as key:
                ret[key] = signature
        return ret


def simplify(signature):
    if isinstance(signature, list):
        # remove duplicates which still sometimes occour:
        ret = set(simplify(sig) for sig in signature)
        return sorted(ret) if len(ret) > 1 else list(ret)[0]
    ret = []
    for pv in signature.parameters.values():
        txt = str(pv)
        if txt == "self":
            continue
        txt = txt[txt.index(":") + 1:]
        if "=" in txt:
            txt = txt[:txt.index("=")]
        quote = txt[0]
        if quote in ("'", '"') and txt[-1] == quote:
            txt = txt[1:-1]
        ret.append(txt)
    return tuple(ret)


class SimplifyingEnumerator(ExactEnumerator):
    """
    SimplifyingEnumerator enumerates all signatures in a module filtered.

    There are no default values, no variable
    names and no self parameter. Only types are present after simplification.
    The functions 'next' resp. '__next__' are removed
    to make the output identical for Python 2 and 3.
    An appropriate formatter should be supplied, if printable output
    is desired.
    """

    def function(self, func_name, func):
        ret = self.result_type()
        signature = getattr(func, '__signature__', None)
        sig = simplify(signature) if signature is not None else None
        if sig is not None and func_name not in ("next", "__next__"):
            with self.fmt.function(func_name, sig) as key:
                ret[key] = sig
        return ret


def enum_all():
    fmt = Formatter(None)
    enu = SimplifyingEnumerator(fmt)
    ret = enu.result_type()
    for mod_name in all_modules:
        ret.update(enu.module(mod_name))
    return ret

def generate_all():
    refPath = getRefPath()
    module = os.path.basename(os.path.splitext(refPath)[0])
    with open(refPath, "w") as outfile, open(sourcepath) as f:
        fmt = Formatter(outfile)
        enu = SimplifyingEnumerator(fmt)
        lines = f.readlines()
        license_line = next((lno for lno, line in enumerate(lines)
                             if "$QT_END_LICENSE$" in line))
        fmt.print("".join(lines[:license_line + 3]))
        fmt.print(dedent('''\
            """
            This file contains the simplified signatures for all functions in PySide
            for module '{}'. There are no default values, no variable
            names and no self parameter. Only types are present after simplification.
            The functions 'next' resp. '__next__' are removed
            to make the output identical for Python 2 and 3.
            """
            '''.format(module)))
        fmt.print("import sys")
        fmt.print("")
        fmt.print("dict = {}")
        for mod_name in all_modules:
            enu.module(mod_name)
        fmt.print("# eof")

def __main__():
    print("+++ generating {}. You should probably check this file in."
          .format(getRefPath()))
    generate_all()

if __name__ == "__main__":
    __main__()
