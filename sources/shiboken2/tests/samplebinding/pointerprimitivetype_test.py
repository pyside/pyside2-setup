#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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
pointerprimitivetype_test.py

check that the primitive types are correctly mapped by the signature module.

Mapping
-------
IntArray2(const int*)                 --  <Signature (self, data: typing.Sequence)>
getMargins(int*,int*,int*,int*)const  --  <Signature (self) -> typing.Tuple[int, int, int, int]>

We explicitly check only against typing.Iterable in the first test,
because typing.Sequence is a subclass, but we will generalize this
to typing.Iterable in the future.
"""

import unittest
from sample import IntArray2, VirtualMethods

import shiboken2
type.__signature__   # trigger init, which does not happen in tests
from shibokensupport.signature import typing


class PointerPrimitiveTypeTest(unittest.TestCase):

    def testArraySignature(self):
        # signature="IntArray2(const int*)"
        found = False
        for sig in IntArray2.__signature__:
            if "data" in sig.parameters:
                found = True
                break
        self.assertTrue(found)
        ann = sig.parameters["data"].annotation
        self.assertEqual(ann.__args__, (int,))
        # un-specify this class (forget "int") by setting the _special
        # flag, so we can check using issubclass (undocumented feature).
        ann._special = True
        self.assertTrue(issubclass(ann, typing.Iterable))

    def testReturnVarSignature(self):
        # signature="getMargins(int*,int*,int*,int*)const">
        ann = VirtualMethods.getMargins.__signature__.return_annotation
        self.assertEqual(ann, typing.Tuple[int, int, int, int])


if __name__ == '__main__':
    unittest.main()
