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
import sys

from helper import UsesQApplication
from PySide2.QtCore import QObject, QEvent
from PySide2.QtWidgets import QWidget

class MyFilter(QObject):
  def eventFilter(self, obj, event):
    if event.type() == QEvent.KeyPress:
      pass
    return QObject.eventFilter(self, obj, event)


class EventFilter(UsesQApplication):
    def testRefCount(self):
        o = QObject()
        filt = MyFilter()
        o.installEventFilter(filt)
        self.assertEqual(sys.getrefcount(o), 2)

        o.installEventFilter(filt)
        self.assertEqual(sys.getrefcount(o), 2)

        o.removeEventFilter(filt)
        self.assertEqual(sys.getrefcount(o), 2)

    def testObjectDestructorOrder(self):
        w = QWidget()
        filt = MyFilter()
        filt.app = self.app
        w.installEventFilter(filt)
        w.show()
        w.close()
        w = None
        self.assertTrue(True)

if __name__ == '__main__':
    unittest.main()
