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
"""

import sys

all_feature_names = [
    "_dummy_feature_01",
    "_dummy_feature_02",
    "_dummy_feature_04",
    "_dummy_feature_08",
    "_dummy_feature_10",
    "_dummy_feature_20",
    "_dummy_feature_40",
    "_dummy_feature_80",
]

__all__ = ["all_feature_names"] + all_feature_names

_dummy_feature_01 = 0x01
_dummy_feature_02 = 0x02
_dummy_feature_04 = 0x04
_dummy_feature_08 = 0x08
_dummy_feature_10 = 0x10
_dummy_feature_20 = 0x20
_dummy_feature_40 = 0x40
_dummy_feature_80 = 0x80

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
# XXX build an improved C version
def _import(name, *args, **kwargs):
    if name == "__feature__" and args[2]:
        # This is an `import from` statement that corresponds to `IMPORT_NAME`.
        # The following `IMPORT_FROM` will handle errors. (Confusing, ofc.)
        flag = 0
        for feature in args[2]:
            if feature in _really_all_feature_names:
                flag |= globals()[feature]
            else:
                raise SyntaxError("PySide feature {} is not defined".format(feature))
        importing_module = sys._getframe(1).f_globals['__name__']
        existing = pyside_feature_dict.get(importing_module, 0)
        if isinstance(existing, int):
            flag |= existing & 255
        pyside_feature_dict[importing_module] = flag
        return sys.modules["__feature__"]
    return original_import(name, *args, **kwargs)
