''' Test bug 363: http://bugs.openbossa.org/show_bug.cgi?id=363'''

import sys
import unittest

from helper import UsesQApplication
from PySide2 import QtCore, QtWidgets

# Check for desktop object lifetime
class BugTest(UsesQApplication):
    def mySlot(self):
        pass

    # test if it is possible to connect with a desktop object after storing that on an auxiliar variable
    def testCase1(self):
        desktop = QtWidgets.QApplication.desktop()
        desktop.resized[int].connect(self.mySlot)
        self.assertTrue(True)

    # test if it is possible to connect with a desktop object without storing that on an auxiliar variable
    def testCase2(self):
        QtWidgets.QApplication.desktop().resized[int].connect(self.mySlot)
        self.assertTrue(True)

if __name__ == '__main__':
    unittest.main()
