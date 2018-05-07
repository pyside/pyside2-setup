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

import unittest
from sample import *
from other import *
import shiboken2 as shiboken

class TestHashFuncs (unittest.TestCase):

    def testIt(self):
        obj1 = HandleHolder()
        obj2 = HandleHolder()

        hash1 = hash(obj1)
        hash2 = hash(obj2)
        self.assertNotEqual(hash1, hash2)

        # Now invalidate the object and test its hash.  It shouldn't segfault.
        shiboken.invalidate(obj1)

        hash1_2 = hash(obj1)
        self.assertEqual(hash1_2, hash1)



if __name__ == '__main__':
    unittest.main()
