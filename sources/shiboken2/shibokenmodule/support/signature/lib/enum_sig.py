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
enum_sig.py

Enumerate all signatures of a class.

This module separates the enumeration process from the formatting.
It is not easy to adhere to this protocol, but in the end, it paid off
by producing a lot of clarity.
"""

import sys
from signature_loader import get_signature, inspect


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
            functions = inspect.getmembers(module, inspect.isroutine)
            ret = self.result_type()
            self.fmt.class_name = None
            for func_name, func in functions:
                ret.update(self.function(func_name, func))
            for class_name, klass in members:
                ret.update(self.klass(class_name, klass))
            return ret

    def klass(self, class_name, klass):
        bases_list = []
        for base in klass.__bases__:
            name = base.__name__
            if name == "object":
                pass
            else:
                modname = base.__module__
                name = modname + "." + base.__name__
            bases_list.append(name)
        class_str = "{}({})".format(class_name, ", ".join(bases_list))
        with self.fmt.klass(class_name, class_str):
            ret = self.function("__init__", klass)
            # class_members = inspect.getmembers(klass)
            # gives us also the inherited things.
            class_members = sorted(list(klass.__dict__.items()))
            subclasses = []
            for thing_name, thing in class_members:
                if inspect.isclass(thing):
                    subclass_name = ".".join((class_name, thing_name))
                    subclasses.append((subclass_name, thing))
                else:
                    func_name = thing_name.split(".")[0]   # remove ".overload"
                    ret.update(self.function(func_name, thing))
            for subclass_name, subclass in subclasses:
                ret.update(self.klass(subclass_name, subclass))
            return ret

    def function(self, func_name, func):
        ret = self.result_type()
        signature = getattr(func, '__signature__', None)
        if signature is not None:
            with self.fmt.function(func_name, signature) as key:
                ret[key] = signature
        return ret


def stringify(signature):
    if isinstance(signature, list):
        # remove duplicates which still sometimes occour:
        ret = set(stringify(sig) for sig in signature)
        return sorted(ret) if len(ret) > 1 else list(ret)[0]
    return tuple(str(pv) for pv in signature.parameters.values())


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
        signature = get_signature(func, 'existence')
        sig = stringify(signature) if signature is not None else None
        if sig is not None and func_name not in ("next", "__next__", "__div__"):
            with self.fmt.function(func_name, sig) as key:
                ret[key] = sig
        return ret

class HintingEnumerator(ExactEnumerator):
    """
    HintingEnumerator enumerates all signatures in a module slightly changed.

    This class is used for generating complete listings of all signatures for
    hinting stubs. Only default values are replaced by "...".
    """

    def function(self, func_name, func):
        ret = self.result_type()
        signature = get_signature(func, 'hintingstub')
        if signature is not None:
            with self.fmt.function(func_name, signature) as key:
                ret[key] = signature
        return ret

