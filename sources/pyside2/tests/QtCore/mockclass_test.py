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

# Test case for PySide bug 634
# http://bugs.pyside.org/show_bug.cgi?id=634
# Marcus Lindblom <macke@yar.nu>; 2011-02-16

import unittest
from PySide2.QtCore import QCoreApplication

class Mock(object):
    def __init__(self):
        self.called = False
        self.return_value = None

    def __call__(self, *args, **kwargs):
        self.called = True
        return self.return_value


class MockClassTest(unittest.TestCase):
    def testMockQCoreApplication(self):
        mock = Mock()
        setattr(QCoreApplication, 'instance', mock)
        QCoreApplication.instance()
        self.assertTrue(mock.called)

if __name__ == '__main__':
    unittest.main()

