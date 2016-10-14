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

import unittest
import sys

from PySide2.QtCore import QObject, QCoreApplication, QEvent, QThread

class MyEvent(QEvent):
    def __init__(self,i):
        print("TYPE:", type(QEvent.User))
        super(MyEvent,self).__init__(QEvent.Type(QEvent.User + 100 ))
        self.i = i

class MyThread (QThread):
    def __init__(self,owner):
        super(MyThread,self).__init__()
        self.owner=owner;

    def run(self):
        for i in range(3):
            e=MyEvent(i);
            QCoreApplication.postEvent(self.owner,e)

class MyBaseObject(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.events = []
        self.t = MyThread(self)
        self.t.start()

    def customEvent(self, event):
        self.events.append(event)
        if len(self.events) == 3:
            self.t.wait()
            self.app.quit()


class CheckForEventsTypes(unittest.TestCase):
    def testTypes(self):
        o = MyBaseObject()
        o.app = QCoreApplication(sys.argv)
        o.app.exec_()
        for e in o.events:
            self.assertTrue(isinstance(e, MyEvent))
        o.app = None

if __name__ == '__main__':
    unittest.main()
