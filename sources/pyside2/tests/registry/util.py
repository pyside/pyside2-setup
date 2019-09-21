#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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
Supporting isolation of warnings

Warnings in unittests are not isolated.
We sometimes use warnings to conveniently accumulate error messages
and eventually report them afterwards as error.
"""

import sys
import warnings
import re
from contextlib import contextmanager

warn_name = "__warningregistry__"
ignore_re = 'Not importing directory .*'

@contextmanager
def isolate_warnings():
    save_warnings = {}
    for name, mod in sys.modules.items():
        if mod and hasattr(mod, warn_name):
            save_warnings[name] = mod.__dict__[warn_name]
            delattr(mod, warn_name)
        else:
            save_warnings[name] = None
    yield
    for name, warn in save_warnings.items():
        mod = sys.modules[name]
        if mod:
            setattr(mod, warn_name, warn)
            if warn is None:
                delattr(mod, warn_name)

@contextmanager
def suppress_warnings():
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        yield

def check_warnings():
    for name, mod in sys.modules.items():
        if mod:
            reg = getattr(mod, warn_name, None)
            if reg:
                # XXX We need to filter warnings for Python 2.
                # This should be avoided by renaming the duplicate folders.
                for k in reg:
                    if type(k) is tuple and re.match(ignore_re, k[0]):
                        continue
                    return True
    return False

def warn(message, category=None, stacklevel=2):
    """Issue a warning with the default 'RuntimeWarning'"""
    if category is None:
        category = UserWarning
    warnings.warn(message, category, stacklevel)

# eof
