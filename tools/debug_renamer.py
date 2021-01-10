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
The Python output lines are of this format:

mode filename:lineno funcname object_id typename object_refcount

Mode can be "INC", "DEC", "XINC", XDEC", "NEW, "NEWV".

On "NEW" or "NEWV", an object is created and the refcount is always 1.
On "DEC" or "XDEC", when refcount is 0, the object is deleted.


Operation
---------

The script reads from <stdin> until EOF. It produces output where the
object_id field is removed and some text is combined with object_typename
to produce a unique object name.


Example
-------

You can create reference debugging output by using the modified interpreter at

    https://github.com/ctismer/cpython/tree/3.9-refdebug

and pipe the error output through this script.
This is work in flux that might change quite often.


To Do List
----------

The script should be re-worked to be more flexible, without relying on
the number of coulumns but with some intelligent guessing.

Names of objects which are already deleted should be monitored and
not by chance be re-used.
"""

import sys
from collections import OrderedDict


def make_name(type_name, name_pos):
    """
    Build a name by using uppercase letters and numbers
    """
    if name_pos < 26:
        name = chr(ord("A") + name_pos)
        return f"{type_name}_{name}"
    return f"{type_name}_{str(name_pos)}"


mode_tokens = "NEW NEWV INC DEC XINC XDEC".split()
known_types = {}

while 1:
    line = sys.stdin.readline()
    if not line:
        break
    fields = line.split()
    if len(fields) != 6 or fields[0] not in mode_tokens:
        print(line.rstrip())
        continue
    mode, fname_lno, funcname, object_id, typename, refcount = fields
    if typename not in known_types:
        known_types[typename] = OrderedDict()
    obj_store = known_types[typename]
    if object_id not in obj_store:
        obj_store[object_id] = make_name(typename, len(obj_store))
    print(f"{mode} {fname_lno} {funcname} {obj_store[object_id]} {refcount}")
