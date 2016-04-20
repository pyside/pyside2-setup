''' Test bug 389: http://bugs.openbossa.org/show_bug.cgi?id=389'''

import sys
import unittest
from helper import UsesQApplication
from PySide2 import QtCore, QtGui, QtWidgets

class BugTest(UsesQApplication):
    def testCase(self):
        s = QtWidgets.QWidget().style()
        i = s.standardIcon(QtWidgets.QStyle.SP_TitleBarMinButton)
        self.assertEqual(type(i), QtGui.QIcon)

if __name__ == '__main__':
    unittest.main()
