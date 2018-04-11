#!/usr/bin/python

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

import unittest
from PySide2 import __version_info__, __version__, QtCore

class CheckForVariablesTest(unittest.TestCase):
    def testVesions(self):
        version_tuple = (__version_info__[0], __version_info__[1], __version_info__[2])
        self.assertTrue(version_tuple >= (1, 0, 0))

        self.assertTrue(version_tuple < (99, 99, 99))
        self.assertTrue(__version__)

        self.assertTrue(QtCore.__version_info__ >= (4, 5, 0))
        self.assertTrue(QtCore.__version__)

if __name__ == '__main__':
    unittest.main()

