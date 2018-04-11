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

from sample import *
import unittest

class MetaA(type):
    pass

class A(object):
    __metaclass__ = MetaA

MetaB = type(Point)
B = Point

class MetaC(MetaA, MetaB):
    pass
class C(A, B):
    __metaclass__ = MetaC

class D(C):
    pass

class TestMetaClass(unittest.TestCase):
    def testIt(self):
        w1 = C() # works
        w1.setX(1)
        w1.setY(2)

        w2 = D() # should work!
        w2.setX(3)
        w2.setY(4)

        w3 = w1 + w2
        self.assertEqual(w3.x(), 4)
        self.assertEqual(w3.y(), 6)
