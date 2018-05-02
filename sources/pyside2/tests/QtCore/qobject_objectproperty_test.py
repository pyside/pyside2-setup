#!/usr/bin/python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

'''Test case for the bug #378
http://bugs.openbossa.org/show_bug.cgi?id=378
'''

import unittest
from PySide2.QtCore import QObject

class ExtQObject(QObject):
    def __init__(self):
        # "foobar" will become a object attribute that will not be
        # listed on the among the type attributes. Thus for bug
        # condition be correctly triggered the "foobar" attribute
        # must not previously exist in the parent class.
        self.foobar = None
        # The parent __init__ method must be called after the
        # definition of "self.foobar".
        QObject.__init__(self)

class TestBug378(unittest.TestCase):
    '''Test case for the bug #378'''

    def testBug378(self):
        obj = ExtQObject()

if __name__ == '__main__':
    unittest.main()

