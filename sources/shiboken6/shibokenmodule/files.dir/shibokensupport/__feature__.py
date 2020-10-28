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

from __future__ import print_function, absolute_import

"""
__feature__.py

This is the feature file for the Qt for Python project. There is some
similarity to Python's `__future__` file, but also some distinction.

The normal usage is like

    from __feature__ import <feature_name> [, ...]
    ...

Alternatively, there is the `set_selection` function which uses select_id's
and takes an optional `mod_name` parameter.

The select id `-1` has the spectial meaning "ignore this module".
"""

import sys

all_feature_names = [
    "snake_case",
    "true_property",
    "_feature_04",
    "_feature_08",
    "_feature_10",
    "_feature_20",
    "_feature_40",
    "_feature_80",
]

__all__ = ["all_feature_names", "set_selection", "info"] + all_feature_names

snake_case = 0x01
true_property = 0x02
_feature_04 = 0x04
_feature_08 = 0x08
_feature_10 = 0x10
_feature_20 = 0x20
_feature_40 = 0x40
_feature_80 = 0x80

# let's remove the dummies for the normal user
_really_all_feature_names = all_feature_names[:]
all_feature_names = list(_ for _ in all_feature_names if not _.startswith("_"))

# Install an import hook that controls the `__feature__` import.
"""
Note: This are two imports.
>>> import dis
>>> def test():
...     from __feature__ import snake_case
...
>>> dis.dis(test)
  2           0 LOAD_CONST               1 (0)
              2 LOAD_CONST               2 (('snake_case',))
              4 IMPORT_NAME              0 (__feature__)
              6 IMPORT_FROM              1 (snake_case)
              8 STORE_FAST               0 (snake_case)
             10 POP_TOP
             12 LOAD_CONST               0 (None)
             14 RETURN_VALUE
"""
# XXX build an improved C version? I guess not.
def _import(name, *args, **kwargs):
    # PYSIDE-1368: The `__name__` attribute does not need to exist in all modules.
    # PYSIDE-1398: sys._getframe(1) may not exist when embedding.
    calling_frame = _cf = sys._getframe().f_back
    importing_module = _cf.f_globals.get("__name__", "__main__") if _cf else "__main__"
    existing = pyside_feature_dict.get(importing_module, 0)

    if name == "__feature__" and args[2]:
        __init__()

        # This is an `import from` statement that corresponds to `IMPORT_NAME`.
        # The following `IMPORT_FROM` will handle errors. (Confusing, ofc.)
        flag = 0
        for feature in args[2]:
            if feature in _really_all_feature_names:
                flag |= globals()[feature]
            else:
                raise SyntaxError("PySide feature {} is not defined".format(feature))

        flag |= existing & 255 if isinstance(existing, int) and existing >= 0 else 0
        pyside_feature_dict[importing_module] = flag

        if importing_module == "__main__":
            # We need to add all modules here which should see __feature__.
            pyside_feature_dict["rlcompleter"] = flag

        # Initialize feature (multiple times allowed) and clear cache.
        sys.modules["PySide2.QtCore"].__init_feature__()
        return sys.modules["__feature__"]

    if name.split(".")[0] == "PySide2":
        # This is a module that imports PySide2.
        flag = existing if isinstance(existing, int) else 0
    else:
        # This is some other module. Ignore it in switching.
        flag = -1
    pyside_feature_dict[importing_module] = flag
    return original_import(name, *args, **kwargs)

_is_initialized = False

def __init__():
    global _is_initialized
    if not _is_initialized:
        # use _one_ recursive import...
        import PySide2.QtCore
        # Initialize all prior imported modules
        for name in sys.modules:
            pyside_feature_dict.setdefault(name, -1)


def set_selection(select_id, mod_name=None):
    """
    Internal use: Set the feature directly by Id.
    Id == -1: ignore this module in switching.
    """
    mod_name = mod_name or sys._getframe(1).f_globals['__name__']
    __init__()
    # Reset the features to the given id
    flag = 0
    if isinstance(select_id, int):
        flag = select_id & 255
    pyside_feature_dict[mod_name] = flag
    sys.modules["PySide2.QtCore"].__init_feature__()
    return _current_selection(flag)


def info(mod_name=None):
    """
    Internal use: Return the current selection
    """
    mod_name = mod_name or sys._getframe(1).f_globals['__name__']
    flag = pyside_feature_dict.get(mod_name, 0)
    return _current_selection(flag)


def _current_selection(flag):
    names = []
    if flag >= 0:
        for idx, name in enumerate(_really_all_feature_names):
            if (1 << idx) & flag:
                names.append(name)
    return names

#eof
