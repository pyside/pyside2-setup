#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

''' Test case for a class that holds a unknown handle object.
    Test case for BUG #1105.
'''

import unittest

from sample import HandleHolder

class HandleHolderTest(unittest.TestCase):
    def testCreation(self):
        holder = HandleHolder(HandleHolder.createHandle())
        holder2 = HandleHolder(HandleHolder.createHandle())
        self.assertEqual(holder.compare(holder2), False)

    def testTransfer(self):
        holder = HandleHolder()
        holder2 = HandleHolder(holder.handle())
        self.assertTrue(holder.compare(holder2))

    def testUseDefinedType(self):
        holder = HandleHolder(8)
        holder2 = HandleHolder(holder.handle2())
        self.assertTrue(holder.compare2(holder2))

if __name__ == '__main__':
    unittest.main()
