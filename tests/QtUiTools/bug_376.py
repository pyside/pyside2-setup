import unittest
import os
from helper import UsesQApplication

from PySide2 import QtCore, QtWidgets
from PySide2.QtUiTools import QUiLoader

class BugTest(UsesQApplication):
    def testCase(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()

        filePath = os.path.join(os.path.dirname(__file__), 'test.ui')
        result = loader.load(filePath, w)
        self.assert_(isinstance(result.child_object, QtWidgets.QFrame))

if __name__ == '__main__':
    unittest.main()

