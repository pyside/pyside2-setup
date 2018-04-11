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

'''Test cases for QObject protected methods'''

import unittest

from PySide2.QtCore import QObject, QThread, SIGNAL

class Dummy(QObject):
    '''Dummy class'''
    pass

class QObjectReceivers(unittest.TestCase):
    '''Test case for QObject.receivers()'''

    def cb(self, *args):
        #Dummy callback
        pass

    def testQObjectReceiversExtern(self):
        #QObject.receivers() - Protected method external access

        obj = Dummy()
        self.assertEqual(obj.receivers(SIGNAL("destroyed()")), 0)

        QObject.connect(obj, SIGNAL("destroyed()"), self.cb)
        self.assertEqual(obj.receivers(SIGNAL("destroyed()")), 1)

    def testQThreadReceiversExtern(self):
        #QThread.receivers() - Inherited protected method

        obj = QThread()
        self.assertEqual(obj.receivers(SIGNAL('destroyed()')), 0)
        QObject.connect(obj, SIGNAL("destroyed()"), self.cb)
        self.assertEqual(obj.receivers(SIGNAL("destroyed()")), 1)


if __name__ == '__main__':
    unittest.main()
