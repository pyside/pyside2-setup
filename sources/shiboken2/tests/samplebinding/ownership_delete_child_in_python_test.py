#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

'''Tests for deleting a child object in python'''

import unittest
import random
import string

from sample import ObjectType
from py3kcompat import IS_PY3K

if IS_PY3K:
    string.letters = string.ascii_letters


class DeleteChildInPython(unittest.TestCase):
    '''Test case for deleting (unref) a child in python'''

    def testDeleteChild(self):
        '''Delete child in python should not invalidate child'''
        parent = ObjectType()
        child = ObjectType(parent)
        name = ''.join(random.sample(string.letters, 5))
        child.setObjectName(name)

        del child
        new_child = parent.children()[0]
        self.assertEqual(new_child.objectName(), name)

if __name__ == '__main__':
    unittest.main()
