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

''' unit test for BUG #1063 '''

import unittest
import tempfile
from PySide2 import QtCore
import os
import py3kcompat as py3k

class QTextStreamTestCase(unittest.TestCase):
    def setUp(self):
        self.temp_file = tempfile.NamedTemporaryFile(delete=False)
        self.temp_file.close()
        self.f = QtCore.QFile(self.temp_file.name)
        self.f.open(QtCore.QIODevice.WriteOnly)
        self.strings = (py3k.unicode_('foo'), py3k.unicode_('bar'))
        self.stream = QtCore.QTextStream(self.f)

    def testIt(self):
        for s in self.strings:
            self.stream << s

        self.f.close()

        # make sure we didn't get an empty file
        self.assertNotEqual(QtCore.QFile(self.temp_file.name).size(), 0)

        os.unlink(self.temp_file.name)

if __name__ == "__main__":
    unittest.main()
