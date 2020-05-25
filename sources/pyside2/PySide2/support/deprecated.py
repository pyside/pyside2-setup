# This Python file uses the following encoding: utf-8
#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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
deprecated.py

This module contains deprecated things that are removed from the interface.
They are implemented in Python again, together with a deprecation warning.

Functions that are to be called for
    PySide2.<module> must be named
    fix_for_<module> .

Note that this fixing code is run after all initializations, but before the
import is finished. But that is no problem since the module is passed in.
"""

import warnings
from textwrap import dedent


class PySideDeprecationWarningRemovedInQt6(Warning):
    pass


def constData(self):
    cls = self.__class__
    name = cls.__qualname__
    warnings.warn(dedent("""
        {name}.constData is unpythonic and will be removed in Qt For Python 6.0 .
        Please use {name}.data instead."""
        .format(**locals())), PySideDeprecationWarningRemovedInQt6, stacklevel=2)
    return cls.data(self)


def fix_for_QtGui(QtGui):
    for name, cls in QtGui.__dict__.items():
        if name.startswith("QMatrix") and "data" in cls.__dict__:
            cls.constData = constData

# eof
