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
loader.py

The loader has to lazy-load the signature module and also provides a few
Python modules to support Python 2.7 .

This file was originally directly embedded into the C source.
After it grew more and more, I now prefer to have it as Python file.
The remaining stub loader in the C source is now only a short string.

This version does no longer use an embedded .zip file but is a package.
The old code without a package but with zip compression can still be found
at https://codereview.qt-project.org/#/c/203533/ for reference.
"""

import sys
import os

# Make sure that we always have the PySide containing package first.
# This is crucial for the mapping during reload in the tests.
package_dir = __file__
for _ in "four":
    package_dir = os.path.dirname(package_dir)
sys.path.insert(0, package_dir)
if sys.version_info >= (3,):
    import inspect
else:
    import inspect
    namespace = inspect.__dict__
    from PySide2.support.signature import backport_inspect as inspect
    _doc = inspect.__doc__
    inspect.__dict__.update(namespace)
    inspect.__doc__ += _doc
    # force inspect to find all attributes. See "heuristic" in pydoc.py!
    inspect.__all__ = list(x for x in dir(inspect) if not x.startswith("_"))

# name used in signature.cpp
from PySide2.support.signature.parser import pyside_type_init
sys.path.pop(0)
# Note also that during the tests we have a different encoding that would
# break the Python license decorated files without an encoding line.
from PySide2.support.signature import layout

# name used in signature.cpp
def create_signature(props, key):
    return layout.create_signature(props, key)

# end of file
