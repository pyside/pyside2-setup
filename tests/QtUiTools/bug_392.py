import unittest
import os
from helper import UsesQApplication

from PySide2 import QtWidgets
from PySide2.QtUiTools import QUiLoader

class MyWidget(QtWidgets.QComboBox):
    def __init__(self, parent=None):
        QtWidgets.QComboBox.__init__(self, parent)

    def isPython(self):
        return True

class BugTest(UsesQApplication):
    def testCase(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()

        filePath = os.path.join(os.path.dirname(__file__), 'action.ui')
        result = loader.load(filePath, w)
        self.assert_(isinstance(result.actionFoo, QtWidgets.QAction))

    def testPythonCustomWidgets(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()
        loader.registerCustomWidget(MyWidget)

        filePath = os.path.join(os.path.dirname(__file__), 'pycustomwidget.ui')
        result = loader.load(filePath, w)
        self.assert_(isinstance(result.custom, MyWidget))
        self.assert_(result.custom.isPython())

    def testPythonCustomWidgetsTwice(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()
        loader.registerCustomWidget(MyWidget)

        filePath = os.path.join(os.path.dirname(__file__), 'pycustomwidget2.ui')
        result = loader.load(filePath, w)
        self.assert_(isinstance(result.custom, MyWidget))
        self.assert_(isinstance(result.custom2, MyWidget))
        self.assert_(result.custom.isPython())

if __name__ == '__main__':
    unittest.main()

