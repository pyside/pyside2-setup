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
signature.py

This module is the python part of the PySide signature initialization.
It is not for common use and should be called by shiboken's signature.cpp.
It is initially written for Python 3, only.
Meanwhile people say it works with Python 2.7, too. ;-)
"""

import sys
import re
import warnings
import types
import keyword
import functools

PY3 = sys.version_info >= (3,)
if PY3:
    try:
        from importlib import reload
    except ImportError:
        from imp import reload

_DEBUG = False
_BREAK_ON_ERROR = False

class FakeMapping(object):
    """
    We do not import the mapping module directly:

    It is not clear from where the mapping is imported. When for instance
    the mapping is imported by a test from the source directory, reload
    would now reload from the PySide directory. This is weird and
    wasteful. We fake the module instead and load it later.
    """
    def __init__(self):
        self.type_map = {}

mapping = FakeMapping()
namespace = mapping.__dict__

class _empty:
    """ marks "no value found". We cannot use None here."""

def dprint(*args, **kw):
    if _DEBUG:
        import pprint
        for arg in args:
            pprint.pprint(arg)

def _parse_line(line):
    line_re = r"""
        ((?P<multi> ([0-9]+)) : )?    # the optional multi-index
        (?P<funcname> \w+(\.\w+)*)    # the function name
        \( (?P<arglist> .*?) \)       # the argument list
        ( -> (?P<returntype> .*) )?   # the optional return type
        $
        """
    ret = re.match(line_re, line, re.VERBOSE).groupdict()
    arglist = ret["arglist"]
    arglist = list(x.strip() for x in re.split(r"""
        (
            (?:                     # inner group is not capturing
                [^,()]              # no commas or braces
                |
                \(
                    (?:
                        [^()]*      # or one brace pair
                        |
                        \(
                            [^()]*  # or doubls nested pair
                        \)
                    )*
                \)
            )+                      # longest possible span
        )   # this list is interspersed with "," and surrounded by ""
        """, arglist, flags=re.VERBOSE)
        if x.strip() not in ("", ","))
    args = []
    for arg in arglist:
        name, ann = arg.split(":")
        if name in keyword.kwlist:
            name = name + "_"
        if "=" in ann:
            ann, default = ann.split("=")
            tup = name, ann, default
        else:
            tup = name, ann
        args.append(tup)
    ret["arglist"] = args
    multi = ret["multi"]
    if multi is not None:
        ret["multi"] = int(multi)
    return ret

def _resolve_number(thing):
    try:
        return eval(thing, namespace)
    except Exception:
        return _empty

def try_to_guess(thing, valtype):
    res = _resolve_number(thing)
    if res is not _empty:
        return res
    if "." not in thing and "(" not in thing:
        text = "{}.{}".format(valtype, thing)
        try:
            return eval(text, namespace)
        except Exception as e:
            pass
    typewords = valtype.split(".")
    valwords = thing.split(".")
    braceless = valwords[0]
    if "(" in braceless:
        braceless = braceless[:braceless.index("(")]
    for idx, w in enumerate(typewords):
        if w == braceless:
            text = ".".join(typewords[:idx] + valwords)
            try:
                return eval(text, namespace)
            except Exception as e:
                pass
    return _empty

def _resolve_value_reloaded(thing, valtype, type_map, line, maybe_redo):
    if thing in type_map:
        return type_map[thing]
    try:
        res = eval(thing, namespace)
        type_map[thing] = res
        return res
    except Exception as e:
        pass
    res = try_to_guess(thing, valtype) if valtype else _empty
    if res is not _empty:
        type_map[thing] = res
        return res
    if maybe_redo:
        return _empty
    warnings.warn("""pyside_type_init:

        UNRECOGNIZED:   {!r}
        OFFENDING LINE: {!r}

        """.format(thing, line),
        RuntimeWarning)
    if _BREAK_ON_ERROR:
        raise RuntimeError
    return thing

def _resolve_value(thing, valtype, type_map, line):
    """
    Load a value after eventually reloading.

    If an error occurs, there is maybe a new module imported that we
    don't have, yet. Reload the mapping module and try again.
    """
    try:
        val = _resolve_value_reloaded(thing, valtype, type_map, line, True)
    except Exception:
        val = _empty
    if val is not _empty:
        return val
    global mapping, namespace
    if type(mapping) is not types.ModuleType:
        # lazy import
        from . import mapping
        namespace = mapping.__dict__
        type_map.update(mapping.type_map)
        return _resolve_value(thing, valtype, type_map, line)
    reload(mapping)
    dprint("Matrix reloaded")
    type_map.update(mapping.type_map)
    return _resolve_value_reloaded(thing, valtype, type_map, line, False)

def _resolve_type(thing, type_map, line):
    return _resolve_value(thing, None, type_map, line)

def calculate_props(line, type_map):
    line = line.strip()
    res = _parse_line(line)
    arglist = res["arglist"]
    annotations = {}
    _defaults = []
    for tup in arglist:
        name, ann = tup[:2]
        annotations[name] = _resolve_type(ann, type_map, line)
        if len(tup) == 3:
            default = _resolve_value(tup[2], ann, type_map, line)
            _defaults.append(default)
    defaults = tuple(_defaults)
    returntype = res["returntype"]
    if returntype is not None:
        annotations["return"] = _resolve_type(returntype, type_map, line)
    props = {}
    props["defaults"] = defaults
    props["kwdefaults"] = {}
    props["annotations"] = annotations
    props["varnames"] = varnames = tuple(tup[0] for tup in arglist)
    funcname = res["funcname"]
    props["fullname"] = funcname
    shortname = funcname[funcname.rindex(".")+1:]
    props["name"] = shortname
    props["multi"] = res["multi"]
    return props

def pyside_type_init(typemod, sig_str, type_map):
    dprint()
    if type(typemod) is types.ModuleType:
        dprint("Initialization of module '{}'".format(typemod.__name__))
    else:
        dprint("Initialization of type '{}.{}'".format(typemod.__module__,
                                                       typemod.__name__))
    ret = {}
    multi_props = []
    for line in sig_str.strip().splitlines():
        props = calculate_props(line, type_map)
        shortname = props["name"]
        multi = props["multi"]
        if multi is None:
            ret[shortname] = props
            dprint(props)
        else:
            fullname = props.pop("fullname")
            multi_props.append(props)
            if multi > 0:
                continue
            multi_props = {"multi": multi_props, "fullname": fullname}
            ret[shortname] = multi_props
            dprint(multi_props)
            multi_props = []
    return ret

pyside_type_init = functools.partial(pyside_type_init,
                                     type_map=mapping.type_map)

# end of file
