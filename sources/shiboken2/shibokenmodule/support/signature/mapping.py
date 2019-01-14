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

from __future__ import print_function, absolute_import

"""
mapping.py

This module has the mapping from the pyside C-modules view of signatures
to the Python representation.

The PySide modules are not loaded in advance, but only after they appear
in sys.modules. This minimizes the loading overhead.
"""

import sys
import struct
import os
import pkgutil

from signature_loader import typing

class ellipsis(object):
    def __repr__(self):
        return "..."
ellipsis = ellipsis()
Char = typing.Union[str, int]     # how do I model the limitation to 1 char?
StringList = typing.List[str]
IntList = typing.List[int]
Point = typing.Tuple[float, float]
PointList = typing.List[Point]
IntMatrix = typing.List[IntList]
Variant = typing.Any
ModelIndexList = typing.List[int]
QImageCleanupFunction = typing.Callable
FloatList = typing.List[float]
FloatMatrix = typing.List[FloatList]
# Pair could be more specific, but we loose the info in the generator.
Pair = typing.Tuple[typing.Any, typing.Any]
MultiMap = typing.DefaultDict[str, typing.List[str]]

# ulong_max is only 32 bit on windows.
ulong_max = 2*sys.maxsize+1 if len(struct.pack("L", 1)) != 4 else 0xffffffff
ushort_max = 0xffff

GL_COLOR_BUFFER_BIT = 0x00004000
GL_NEAREST = 0x2600

WId = int

# from 5.9
GL_TEXTURE_2D = 0x0DE1
GL_RGBA = 0x1908


class _NotCalled(str):
    """
    Wrap some text with semantics

    This class is wrapped around text in order to avoid calling it.
    There are three reasons for this:

      - some instances cannot be created since they are abstract,
      - some can only be created after qApp was created,
      - some have an ugly __repr__ with angle brackets in it.

    By using derived classes, good looking instances can be created
    which can be used to generate source code or .pyi files. When the
    real object is needed, the wrapper can simply be called.
    """
    def __repr__(self):
        suppress = "support.signature.typing27."
        text = self[len(suppress):] if self.startswith(suppress) else self
        return "{}({})".format(type(self).__name__, text)

    def __call__(self):
        from signature_loader.mapping import __dict__ as namespace
        text = self if self.endswith(")") else self + "()"
        return eval(text, namespace)

USE_PEP563 = sys.version_info[:2] >= (3, 7)


# Some types are abstract. They just show their name.
class Virtual(_NotCalled):
    pass

# Other types I simply could not find.
class Missing(_NotCalled):
    if not USE_PEP563:
        # The string must be quoted, because the object does not exist.
        def __repr__(self):
            return '{}("{}")'.format(type(self).__name__, self)


class Invalid(_NotCalled):
    pass

# Helper types
class Default(_NotCalled):
    pass


class Instance(_NotCalled):
    pass


class Reloader(object):
    """
    Reloder class

    This is a singleton class which provides the update function for the
    shiboken and PySide classes.
    """
    ## Note: We needed to rename shiboken2 in order to avoid a name clash.
    _uninitialized = "Shiboken minimal sample other smart".split()
    _prefixes = [""]

    def __init__(self):
        self.sys_module_count = 0
        self.uninitialized = self._uninitialized

    def update(self, g=None):
        """
        update is responsible to import all modules from shiboken and PySide
        which are already in sys.modules.
        The purpose is to follow all user imports without introducing new
        ones.
        This function is called by pyside_type_init to adapt imports
        when the number of imported modules has changed.
        """
        if self.sys_module_count == len(sys.modules):
            return
        self.sys_module_count = len(sys.modules)
        if g is None:
            g = globals()
        for mod_name in self.uninitialized[:]:
            for prefix in self._prefixes:
                import_name = prefix + mod_name
                if import_name in sys.modules:
                    # check if this is a real module
                    check_module(sys.modules[import_name])
                    # module is real
                    self.uninitialized.remove(mod_name)
                    proc_name = "init_" + mod_name
                    if proc_name in g:
                        # Do the 'import {import_name}' first.
                        # 'top' is PySide2 when we do 'import PySide.QtCore'
                        # or Shiboken if we do 'import Shiboken'.
                        # Convince yourself that these two lines below have the same
                        # global effect as "import Shiboken" or "import PySide2.QtCore".
                        top = __import__(import_name)
                        g[top.__name__] = top
                        # Modules are in place, we can update the type_map.
                        g.update(g[proc_name]())

def check_module(mod):
    # During a build, there exist the modules already as directories,
    # although the '*.so' was not yet created. This causes a problem
    # in Python 3, because it accepts folders as namespace modules
    # without enforcing an '__init__.py'.
    if not getattr(mod, "__file__", None) or os.path.isdir(mod.__file__):
        mod_name = mod.__name__
        raise ImportError("Module '{mod_name}' is at most a namespace!"
                          .format(**locals()))


update_mapping = Reloader().update
type_map = {}
namespace = globals()  # our module's __dict__


def init_Shiboken():
    type_map.update({
        "shiboken2.bool": bool,
        "size_t": int,
        "PyType": type,
    })
    return locals()


def init_minimal():
    type_map.update({
        "MinBool": bool,
    })
    return locals()


def init_sample():
    import datetime
    type_map.update({
        "sample.int": int,
        "Complex": complex,
        "sample.OddBool": bool,
        "sample.bool": bool,
        "sample.PStr": str,
        "double[]": FloatList,
        "OddBool": bool,
        "PStr": str,
        "sample.char": Char,
        "double[][]": FloatMatrix,
        "int[]": IntList,
        "int[][]": IntMatrix,
        "sample.Point": Point,
        "sample.ObjectType": object,
        "std.string": str,
        "HANDLE": int,
        "Foo.HANDLE": int,
        "sample.Photon.TemplateBase": Missing("sample.Photon.TemplateBase"),
        "ObjectType.Identifier": Missing("sample.ObjectType.Identifier"),
        "zero(HANDLE)": 0,
        "Null": None,
        "zero(sample.ObjectType)": None,
        "std.size_t": int,
        'Str("<unknown>")': "<unknown>",
        'Str("<unk")': "<unk",
        'Str("nown>")': "nown>",
        "zero(sample.ObjectModel)": None,
        "sample.unsigned char": Char,
        "sample.double": float,
        "zero(sample.bool)": False,
        "PyDate": datetime.date,
        "ZeroIn": 0,
        "Point[]": PointList,
    })
    return locals()


def init_other():
    import numbers
    type_map.update({
        "other.Number": numbers.Number,
        "other.ExtendsNoImplicitConversion": Missing("other.ExtendsNoImplicitConversion"),
    })
    return locals()


def init_smart():
    type_map.update({
        "smart.SharedPtr": Missing("smart.SharedPtr"),  # bad object "SharedPtr<Obj >"
    })
    return locals()

# end of file
