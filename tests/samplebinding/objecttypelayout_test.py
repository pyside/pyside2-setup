#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the Shiboken Python Bindings Generator project.
#
# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
#
# Contact: PySide team <contact@pyside.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation. Please
# review the following information to ensure the GNU Lesser General
# Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
# #
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

'''Tests ObjectTypeLayout class'''

import sys
import unittest

from sample import *


class ObjectTypeLayoutTest(unittest.TestCase):
    '''Test cases  ObjectTypeLayout class'''

    def testObjectTypeLayout(self):

        '''ObjectType.setLayout.'''
        p1 = ObjectType.create()
        c1 = ObjectType.create()
        c2 = ObjectType.create()
        c3 = ObjectType.create()
        layout = ObjectTypeLayout()
        layout.addObject(c1)
        layout.addObject(c2)
        layout.addObject(c3)
        self.assertEqual(c1.parent(), None)
        self.assertEqual(c2.parent(), None)
        self.assertEqual(c3.parent(), None)

        p1.setObjectLayout(layout)
        del p1 # This must kill c1, c2 and c3
        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, layout.objectName)

if __name__ == '__main__':
    unittest.main()

