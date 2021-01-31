#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""
debug_renamer.py
================

This script renames object addresses in debug protocols to useful names.
Comparing output will produce minimal deltas.


Problem:
--------

In the debugging output of PYSIDE-79, we want to study different output
before and after applying some change to the implementation.

We have support from the modified Python interpreter that creates full
traces of every object creation and increment/decrement of refcounts.

The comparison between "before" and "after" gets complicated because
the addresses of objects do not compare well.


Input format:
-------------
The Python output lines can be freely formatted.

Any line which contains "0x.." followed by some name will be changed,
all others are left alone.
We name these fields `object_id` and `typename`.


Operation
---------

The script reads from <stdin> until EOF. It produces output where the
`object_id` field is removed and some text is combined with `typename`
to produce a unique object name.


Example
-------

You can create reference debugging output by using the modified interpreter at

    https://github.com/ctismer/cpython/tree/3.9-refdebug

and pipe the error output through this script.
This is work in flux that might change quite often.


To Do List
----------

Names of objects which are already deleted should be monitored and
not by chance be re-used.
"""

import re
import sys
from collections import OrderedDict


def make_name(typename, name_pos):
    """
    Build a name by using uppercase letters and numbers
    """
    if name_pos < 26:
        name = chr(ord("A") + name_pos)
        return f"{typename}_{name}"
    return f"{typename}_{str(name_pos)}"


known_types = {}
pattern = r"0x\w+\s+\S+"    # hex word followed by non-WS
rex = re.compile(pattern, re.IGNORECASE)

while True:
    line = sys.stdin.readline()
    if not line:
        break
    if not (res := rex.search(line)):
        print(line.rstrip())
        continue
    start_pos, end_pos = res.start(), res.end()
    beg, mid, end = line[:start_pos], line[start_pos : end_pos], line[end_pos:].rstrip()
    object_id, typename = mid.split()
    if typename not in known_types:
        known_types[typename] = OrderedDict()
    obj_store = known_types[typename]
    if object_id not in obj_store:
        obj_store[object_id] = make_name(typename, len(obj_store))
    print(f"{beg}{obj_store[object_id]}{end}")
