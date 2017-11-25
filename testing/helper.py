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

from __future__ import print_function

import os
import sys
from collections import namedtuple

PY3 = sys.version_info[0] == 3  # from the six module
from subprocess import PIPE
if PY3:
    from subprocess import TimeoutExpired
    from io import StringIO
else:
    class SubprocessError(Exception): pass
    # this is a fake, just to keep the source compatible.
    # timeout support is in python 3.3 and above.
    class TimeoutExpired(SubprocessError): pass
    from StringIO import StringIO


script_dir = os.path.dirname(os.path.dirname(__file__))

def decorate(mod_name):
    """
    Write the combination of "modulename_funcname"
    in the Qt-like form "modulename::funcname"
    """
    if "_" not in mod_name:
        return mod_name
    if "::" in mod_name:
        return mod_name
    name, rest = mod_name.split("_", 1)
    return name + "::" + rest

