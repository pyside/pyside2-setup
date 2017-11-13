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

all_modules = list("PySide2." + x for x in PySide2.__all__)

from PySide2.support.signature import inspect
from PySide2.QtCore import __version__

version_id = __version__.replace(".", "_")
is_ci = os.environ.get("QTEST_ENVIRONMENT", "") == "ci"
outname = "exists_{}_{}{}.py".format(sys.platform, version_id,
                                     "_ci" if is_ci else "")
outfile = None

def xprint(*args, **kw):
    if outfile:
        print(*args, file=outfile, **kw)

def simplify(signature):
    if isinstance(signature, list):
        ret = list(simplify(sig) for sig in signature)
        # remove duplicates which still sometimes occour:
        things = set(ret)
        if len(things) != len(ret):
            ret = list(things)
            if len(ret) == 1:
                ret = ret[0]
        return sorted(ret)
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

def begin_module(mod_name):
    xprint("")
    xprint("# Module", mod_name)
    xprint('if "{}" in sys.modules:'.format(mod_name))
    xprint("    dict.update({")

def end_module(mod_name):
    xprint("    })")

def begin_class(mod_name, class_name):
    xprint()
    xprint("    # class {}.{}:".format(mod_name, class_name))

def end_class(mod_name, class_name):
    pass

def show_signature(key, signature):
    if key.endswith("lY"):
        # make the robot shut up:
        key = key[:-1] + '"+"Y'
    xprint('        "{}": {},'.format(key, signature))

def enum_module(mod_name):
    __import__(mod_name)
    begin_module(mod_name)
    module = sys.modules[mod_name]
    members = inspect.getmembers(module, inspect.isclass)
    ret = {}
    for class_name, klass in members:
        begin_class(mod_name, class_name)
        signature = getattr(klass, '__signature__', None)
        # class_members = inspect.getmembers(klass)
        # gives us also the inherited things.
        if signature is not None:
            signature = simplify(signature)
            key = "{}.{}".format(class_name, "__init__")
            ret[key] = signature
            show_signature(key, signature)
        class_members = list(klass.__dict__.items())
        for func_name, func in class_members:
            signature = getattr(func, '__signature__', None)
            if signature is not None:
                signature = simplify(signature)
                key = "{}.{}".format(class_name, func_name)
                ret[key] = signature
                show_signature(key, signature)
        end_class(mod_name, class_name)
    end_module(mod_name)
    return ret

def generate_all():
    fname = os.path.join(os.path.dirname(__file__), outname)
    global outfile
    with open(fname, "w") as outfile:
        with open(__file__) as f:
            lines = f.readlines()
        license_line = next((lno for lno, line in enumerate(lines)
                             if "$QT_END_LICENSE$" in line))
        xprint("".join(lines[:license_line + 3]))
        xprint("import sys")
        xprint("")
        xprint("dict = {}")
        for mod_name in all_modules:
            enum_module(mod_name)
        xprint("# eof")
    return fname

def enum_all():
    global outfile
    outfile = None
    ret = {}
    for mod_name in all_modules:
        ret.update(enum_module(mod_name))
    return ret

def __main__():
    print("+++ generating {}. You should check this file in.".format(outname))
    generate_all()

if __name__ == "__main__":
    __main__()
