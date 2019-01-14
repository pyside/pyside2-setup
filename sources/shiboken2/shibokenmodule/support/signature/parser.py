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

import sys
import re
import warnings
import types
import keyword
import functools
from signature_loader.mapping import type_map, update_mapping, namespace

_DEBUG = False
LIST_KEYWORDS = False

"""
parser.py

This module parses the signature text and creates properties for the
signature objects.

PySide has a new function 'CppGenerator::writeSignatureInfo()'
that extracts the gathered information about the function arguments
and defaults as good as it can. But what PySide generates is still
very C-ish and has many constants that Python doesn't understand.

The function 'try_to_guess()' below understands a lot of PySide's
peculiar way to assume local context. If it is able to do the guess,
then the result is inserted into the dict, so the search happens
not again. For everything that is not covered by these automatic
guesses, we provide an entry in 'type_map' that resolves it.

In effect, 'type_map' maps text to real Python objects.
"""

def dprint(*args, **kw):
    if _DEBUG:
        import pprint
        for arg in args:
            pprint.pprint(arg)
            sys.stdout.flush()

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
    # The following is a split re. The string is broken into pieces which are
    # between the recognized strings. Because the re has groups, both the
    # strings and the delimiters are returned, where the strings are not
    # interesting at all: They are just the commata.
    # Note that it is necessary to put the characters with special handling in
    # the first group (comma, brace, angle bracket).
    # Then they are not recognized there, and we can handle them differently
    # in the following expressions.
    arglist = list(x.strip() for x in re.split(r"""
        (
            (?:                     # inner group is not capturing
                [^,()<>]            # no commas or braces or angle brackets
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
                |
                <                   # or one angle bracket pair
                    [^<>]*
                >
            )+                      # longest possible span
        )   # this list is interspersed with "," and surrounded by ""
        """, arglist, flags=re.VERBOSE)
        if x.strip() not in ("", ","))
    args = []
    for arg in arglist:
        name, ann = arg.split(":")
        if name in keyword.kwlist:
            if LIST_KEYWORDS:
                print("KEYWORD", ret)
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
    funcname = ret["funcname"]
    parts = funcname.split(".")
    if parts[-1] in keyword.kwlist:
        ret["funcname"] = funcname + "_"
    return ret

def make_good_value(thing, valtype):
    try:
        if thing.endswith("()"):
            thing = 'Default("{}")'.format(thing[:-2])
        else:
            ret = eval(thing, namespace)
            if valtype and repr(ret).startswith("<"):
                thing = 'Instance("{}")'.format(thing)
        return eval(thing, namespace)
    except Exception:
        pass

def try_to_guess(thing, valtype):
    if "." not in thing and "(" not in thing:
        text = "{}.{}".format(valtype, thing)
        ret = make_good_value(text, valtype)
        if ret is not None:
            return ret
    typewords = valtype.split(".")
    valwords = thing.split(".")
    braceless = valwords[0]    # Yes, not -1. Relevant is the overlapped word.
    if "(" in braceless:
        braceless = braceless[:braceless.index("(")]
    for idx, w in enumerate(typewords):
        if w == braceless:
            text = ".".join(typewords[:idx] + valwords)
            ret = make_good_value(text, valtype)
            if ret is not None:
                return ret
    return None

def _resolve_value(thing, valtype, line):
    if thing in ("0", "None") and valtype:
        thing = "zero({})".format(valtype)
    if thing in type_map:
        return type_map[thing]
    res = make_good_value(thing, valtype)
    if res is not None:
        type_map[thing] = res
        return res
    res = try_to_guess(thing, valtype) if valtype else None
    if res is not None:
        type_map[thing] = res
        return res
    warnings.warn("""pyside_type_init:

        UNRECOGNIZED:   {!r}
        OFFENDING LINE: {!r}
        """.format(thing, line), RuntimeWarning)
    return thing

def _resolve_type(thing, line):
    return _resolve_value(thing, None, line)

def calculate_props(line):
    line = line.strip()
    res = _parse_line(line)
    arglist = res["arglist"]
    annotations = {}
    _defaults = []
    for idx, tup in enumerate(arglist):
        name, ann = tup[:2]
        if ann == "...":
            name = "*args"
            # copy the fields back :()
            ann = 'NULL' # maps to None
            tup = name, ann
            arglist[idx] = tup
        annotations[name] = _resolve_type(ann, line)
        if len(tup) == 3:
            default = _resolve_value(tup[2], ann, line)
            _defaults.append(default)
    defaults = tuple(_defaults)
    returntype = res["returntype"]
    if returntype is not None:
        annotations["return"] = _resolve_type(returntype, line)
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

def fixup_multilines(sig_str):
    lines = list(line.strip() for line in sig_str.strip().splitlines())
    res = []
    multi_lines = []
    for line in lines:
        multi = re.match(r"([0-9]+):", line)
        if multi:
            idx, rest = int(multi.group(1)), line[multi.end():]
            multi_lines.append(rest)
            if idx > 0:
                continue
            # remove duplicates
            multi_lines = sorted(set(multi_lines))
            # renumber or return a single line
            nmulti = len(multi_lines)
            if nmulti > 1:
                for idx, line in enumerate(multi_lines):
                    res.append("{}:{}".format(nmulti-idx-1, line))
            else:
                res.append(multi_lines[0])
            multi_lines = []
        else:
            res.append(line)
    return res

def pyside_type_init(typemod, sig_str):
    dprint()
    if type(typemod) is types.ModuleType:
        dprint("Initialization of module '{}'".format(typemod.__name__))
    else:
        dprint("Initialization of type '{}.{}'".format(typemod.__module__,
                                                       typemod.__name__))
    update_mapping()
    lines = fixup_multilines(sig_str)
    ret = {}
    multi_props = []
    for line in lines:
        props = calculate_props(line)
        shortname = props["name"]
        multi = props["multi"]
        if multi is None:
            ret[shortname] = props
            dprint(props)
        else:
            multi_props.append(props)
            if multi > 0:
                continue
            fullname = props.pop("fullname")
            multi_props = {"multi": multi_props, "fullname": fullname}
            ret[shortname] = multi_props
            dprint(multi_props)
            multi_props = []
    return ret

# end of file
