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
leak_finder.py
==============

This script finds memory leaks in Python.

Usage:
------

Place one or more lines which should be tested for leaks in a loop:

    from leak_finder import LeakFinder
    ...
    lf = LeakFinder()
    for i in range(1000):
        leaking_statement()
    lf.find_leak()


Theory
------

How to find a leak?

We repeatedly perform an action and observe if that has an unexpected
side effect. There are typically two observations:

* one object is growing its refcount (a pseudo-leak)
* we get many new objects of one type (a true leak)

A difficulty in trying to get leak info is avoiding side effects
of the measurement. Early attempts with lists of refcounts were
unsuccessful. Using array.array for counting refcounts avoids that.


Algorithm
---------
We record a snapshot of all objects in a list and a parallel array
of refcounts.

Then we do some computation and do the same snapshot again.

The structure of a list of all objects is extending at the front for
some reason. That makes the captured structures easy to compare.
We reverse that list and array and have for the objects:

    len(all2) >= len(all1)

    all1[idx] == all2[idx] for idx in range(len(all1))

When taking the second snapshot, the objects still have references from
the first snapshot.
For objects with no effect, the following relation is true:

    refs1[idx] == refs2[idx] - 1 for idx in range(len(all1))

All other objects are potential pseudo-leaks, because they waste
references but no objects in the first place.

Then we look at the newly created objects:
These objects are real leaks if their number is growing with the probe
size. For analysis, the number of new objects per type is counted.
"""

import sys
import gc
import array
import unittest

# this comes from Python, too
from test import support

try:
    sys.getobjects
    have_debug = True
except AttributeError:
    have_debug = False


class LeakFinder(object):
    def __init__(self):
        self.all, self.refs = self._make_snapshot()

    @staticmethod
    def _make_snapshot():
        gc.collect()
        # get all objects
        all = sys.getobjects(0)
        # get an array with the refcounts
        g = sys.getrefcount
        refs = array.array("l", (g(obj) for obj in all))
        # the lists have the same endind. Make comparison easier.
        all.reverse()
        refs.reverse()
        return all, refs

    @staticmethod
    def _short_repr(x, limit=76):
        s = repr(x)
        if len(s) > limit:
            s = s[:limit] + "..."
        return s

    def find_leak(self):
        all1 = self.all
        refs1 = self.refs
        del self.all, self.refs
        all2, refs2 = self._make_snapshot()
        common = len(all1)
        del all1

        srepr = self._short_repr
        # look into existing objects for increased refcounts
        first = True
        for idx in range(common):
            ref = refs2[idx] - refs1[idx] - 1
            if abs(ref) <= 10:
                continue
            obj = all2[idx]
            if first:
                print()
            first = False
            print(f"Fake Leak ref={ref} obj={srepr(obj)}")

        # look at the extra objects by type size
        types = {}
        for idx in range(common, len(all2)):
            obj = all2[idx]
            typ = type(obj)
            if typ not in types:
                types[typ] = []
            types[typ].append(obj)
        first = True
        for typ in types:
            oblis = types[typ]
            ref = len(oblis)
            if ref <= 10:
                continue
            try:
                oblis.sort()
            except TypeError:
                pass
            if first:
                print()
            first = False
            left, mid, right = oblis[0], oblis[ref // 2], oblis[-1]
            print(f"True Leak ref={ref} typ={typ} left={left} mid={mid} right={right}")


class TestDemo(unittest.TestCase):

    @unittest.skipUnless(have_debug, 'You need a debug build with "--with-trace-refs"')
    def test_demo(self):
        # create a pseudo leak and a true leak
        fake_leak_obj = []
        true_leak_obj = []
        lf = LeakFinder()
        refs_before = sys.gettotalrefcount()
        for idx in range(100):
            fake_leak_obj.append("same string")
            true_leak_obj.append(idx + 1000)    # avoiding cached low numbers
        refs_after = sys.gettotalrefcount()
        lf.find_leak()
        self.assertNotAlmostEqual(refs_after - refs_before, 0, delta=10)


if __name__ == "__main__":
    unittest.main()
