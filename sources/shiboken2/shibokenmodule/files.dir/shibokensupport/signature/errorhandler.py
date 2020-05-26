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
errorhandler.py

This module handles the TypeError messages which were previously
produced by the generated C code.

This version is at least consistent with the signatures, which
are created by the same module.

Experimentally, we are trying to guess those errors which are
just the wrong number of elements in an iterator.
At the moment, it is unclear whether the information given is
enough to produce a useful ValueError.

This matter will be improved in a later version.
"""

import sys

from shibokensupport.signature import inspect
from shibokensupport.signature import get_signature
from shibokensupport.signature.mapping import update_mapping, namespace
from textwrap import dedent


def qt_isinstance(inst, the_type):
    if the_type == float:
        return isinstance(inst, int) or isinstance(int, float)
    try:
        return isinstance(inst, the_type)
    except TypeError as e:
        print("FIXME", e)
        return False


def matched_type(args, sigs):
    for sig in sigs:
        params = list(sig.parameters.values())
        if len(args) > len(params):
            continue
        if len(args) < len(params):
            k = len(args)
            if params[k].default is params[k].empty:
                # this is a necessary parameter, so it fails.
                continue
        ok = True
        for arg, param in zip(args, params):
            ann = param.annotation
            if qt_isinstance(arg, ann):
                continue
            ok = False
        if ok:
            return sig
    return None


def seterror_argument(args, func_name):
    update_mapping()
    func = eval(func_name, namespace)
    sigs = get_signature(func, "typeerror")
    if type(sigs) != list:
        sigs = [sigs]
    if type(args) != tuple:
        args = (args,)
    # temp!
    found = matched_type(args, sigs)
    if found:
        msg = dedent("""
            '{func_name}' called with wrong argument values:
              {func_name}{args}
            Found signature:
              {func_name}{found}
            """.format(**locals())).strip()
        return ValueError, msg
    type_str = ", ".join(type(arg).__name__ for arg in args)
    msg = dedent("""
        '{func_name}' called with wrong argument types:
          {func_name}({type_str})
        Supported signatures:
        """.format(**locals())).strip()
    for sig in sigs:
        msg += "\n  {func_name}{sig}".format(**locals())
    # We don't raise the error here, to avoid the loader in the traceback.
    return TypeError, msg

def check_string_type(s):
    if sys.version_info[0] == 3:
        return isinstance(s, str)
    else:
        return isinstance(s, (str, unicode))

def make_helptext(func):
    existing_doc = func.__doc__
    sigs = get_signature(func)
    if not sigs:
        return existing_doc
    if type(sigs) != list:
        sigs = [sigs]
    try:
        func_name = func.__name__
    except AttribureError:
        func_name = func.__func__.__name__
    sigtext = "\n".join(func_name + str(sig) for sig in sigs)
    msg = sigtext + "\n\n" + existing_doc if check_string_type(existing_doc) else sigtext
    return msg

# end of file
