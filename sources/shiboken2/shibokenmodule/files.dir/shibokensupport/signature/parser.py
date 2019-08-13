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

import sys
import re
import warnings
import types
import keyword
import functools
from shibokensupport.signature.mapping import (type_map, update_mapping,
    namespace, typing, _NotCalled)
from shibokensupport.signature.lib.tool import (SimpleNamespace,
    build_brace_pattern)

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


_cache = {}

def _parse_arglist(argstr):
    # The following is a split re. The string is broken into pieces which are
    # between the recognized strings. Because the re has groups, both the
    # strings and the separators are returned, where the strings are not
    # interesting at all: They are just the commata.
    key = "_parse_arglist"
    if key not in _cache:
        regex = build_brace_pattern(level=3, separators=",")
        _cache[key] = re.compile(regex, flags=re.VERBOSE)
    split = _cache[key].split
    # Note: this list is interspersed with "," and surrounded by ""
    return [x.strip() for x in split(argstr) if x.strip() not in ("", ",")]


def _parse_line(line):
    line_re = r"""
        ((?P<multi> ([0-9]+)) : )?    # the optional multi-index
        (?P<funcname> \w+(\.\w+)*)    # the function name
        \( (?P<arglist> .*?) \)       # the argument list
        ( -> (?P<returntype> .*) )?   # the optional return type
        $
        """
    ret = SimpleNamespace(**re.match(line_re, line, re.VERBOSE).groupdict())
    argstr = ret.arglist
    arglist = _parse_arglist(argstr)
    args = []
    for arg in arglist:
        name, ann = arg.split(":")
        if name in keyword.kwlist:
            if LIST_KEYWORDS:
                print("KEYWORD", ret)
            name = name + "_"
        if "=" in ann:
            ann, default = ann.split("=", 1)
            tup = name, ann, default
        else:
            tup = name, ann
        args.append(tup)
    ret.arglist = args
    multi = ret.multi
    if multi is not None:
        ret.multi = int(multi)
    funcname = ret.funcname
    parts = funcname.split(".")
    if parts[-1] in keyword.kwlist:
        ret.funcname = funcname + "_"
    return vars(ret)


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
        if valtype.startswith("PySide2.") or valtype.startswith("typing."):
            return None
        mapped = type_map[valtype]
        # typing.Any: '_SpecialForm' object has no attribute '__name__'
        name = mapped.__name__ if hasattr(mapped, "__name__") else str(mapped)
        thing = "zero({})".format(name)
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


def _resolve_arraytype(thing, line):
    search = re.search(r"\[(\d*)\]$", thing)
    thing = thing[:search.start()]
    if thing.endswith("]"):
        thing = _resolve_arraytype(thing, line)
    if search.group(1):
        # concrete array, use a tuple
        nelem = int(search.group(1))
        thing = ", ".join([thing] * nelem)
        thing = "Tuple[" + thing + "]"
    else:
        thing = "QList[" + thing + "]"
    return thing


def to_string(thing):
    if isinstance(thing, str):
        return thing
    if hasattr(thing, "__name__"):
        dot = "." in str(thing)
        return thing.__module__ + "." + thing.__name__ if dot else thing.__name__
    # Note: This captures things from the typing module:
    return str(thing)


matrix_pattern = "PySide2.QtGui.QGenericMatrix"

# The matrix patch is borrowed from the future (extracted).
# It will work when the parser recognizes matrices.
def handle_matrix(arg):
    n, m, typstr = tuple(map(lambda x:x.strip(), arg.split(",")))
    assert typstr == "float"
    result = "PySide2.QtGui.QMatrix{n}x{m}".format(**locals())
    return eval(result, namespace)


debugging_aid = """
from inspect import currentframe

def lno(level):
    lineno = currentframe().f_back.f_lineno
    spaces = level * "  "
    return "{lineno}{spaces}".format(**locals())
"""


def _resolve_type(thing, line, level):
    # Capture total replacements, first. Happens in
    # "PySide2.QtCore.QCborStreamReader.StringResult[PySide2.QtCore.QByteArray]"
    if thing in type_map:
        return type_map[thing]
    # Now the nested structures are handled.
    if "[" in thing:
        # handle primitive arrays
        if re.search(r"\[\d*\]$", thing):
            thing = _resolve_arraytype(thing, line)
        # Handle a container return type. (see PYSIDE-921 in cppgenerator.cpp)
        contr, thing = re.match(r"(.*?)\[(.*?)\]$", thing).groups()
        # Special case: Handle the generic matrices.
        if contr == matrix_pattern:
            return handle_matrix(thing)
        contr = _resolve_type(contr, line, level+1)
        if isinstance(contr, _NotCalled):
            raise SystemError("Container types must exist:", repr(contr))
        contr = to_string(contr)
        pieces = []
        for part in _parse_arglist(thing):
            part = _resolve_type(part, line, level+1)
            if isinstance(part, _NotCalled):
                # fix the tag (i.e. "Missing") by repr
                part = repr(part)
            pieces.append(to_string(part))
        thing = ", ".join(pieces)
        result = "{contr}[{thing}]".format(**locals())
        return eval(result, namespace)
    return _resolve_value(thing, None, line)


def calculate_props(line):
    parsed = SimpleNamespace(**_parse_line(line.strip()))
    arglist = parsed.arglist
    annotations = {}
    _defaults = []
    for idx, tup in enumerate(arglist):
        name, ann = tup[:2]
        if ann == "...":
            name = "*args" if name.startswith("arg_") else "*" + name
            # copy the pathed fields back
            ann = 'nullptr'     # maps to None
            tup = name, ann
            arglist[idx] = tup
        annotations[name] = _resolve_type(ann, line, 0)
        if len(tup) == 3:
            default = _resolve_value(tup[2], ann, line)
            _defaults.append(default)
    defaults = tuple(_defaults)
    returntype = parsed.returntype
    if returntype is not None:
        annotations["return"] = _resolve_type(returntype, line, 0)
    props = SimpleNamespace()
    props.defaults = defaults
    props.kwdefaults = {}
    props.annotations = annotations
    props.varnames = varnames = tuple(tup[0] for tup in arglist)
    funcname = parsed.funcname
    props.fullname = funcname
    shortname = funcname[funcname.rindex(".")+1:]
    props.name = shortname
    props.multi = parsed.multi
    return vars(props)


def fixup_multilines(lines):
    """
    Multilines can collapse when certain distinctions between C++ types
    vanish after mapping to Python.
    This function fixes this by re-computing multiline-ness.
    """
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


def pyside_type_init(type_key, sig_strings):
    dprint()
    dprint("Initialization of type key '{}'".format(type_key))
    update_mapping()
    lines = fixup_multilines(sig_strings)
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
