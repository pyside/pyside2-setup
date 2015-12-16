import unittest

from PySide2 import QtCore, QtWidgets
from helper import UsesQApplication, TimedQApplication

class TestInputDialog(TimedQApplication):

    def testGetDouble(self):
        self.assertEquals(QtWidgets.QInputDialog.getDouble(None, "title", "label"), (0.0, False))

    def testGetInt(self):
        self.assertEquals(QtWidgets.QInputDialog.getInt(None, "title", "label"), (0, False))

    def testGetItem(self):
        (item, bool) = QtWidgets.QInputDialog.getItem(None, "title", "label", ["1", "2", "3"])
        self.assertEquals(str(item), "1")

    def testGetText(self):
        (text, bool) = QtWidgets.QInputDialog.getText(None, "title", "label")
        self.assertEquals(str(text),"")

if __name__ == '__main__':
    unittest.main()

