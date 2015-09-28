import unittest
from PySide2.QtCore import QObject
from PySide2.QtGui import QPen, QBrush

class TestBug991 (unittest.TestCase):
    def testReprFunction(self):
        reprPen = repr(QPen())
        self.assertTrue(reprPen.startswith("<PySide2.QtGui.QPen"))
        reprBrush = repr(QBrush())
        self.assertTrue(reprBrush.startswith("<PySide2.QtGui.QBrush"))
        reprObject = repr(QObject())
        self.assertTrue(reprObject.startswith("<PySide2.QtCore.QObject"))

if __name__ == '__main__':
    unittest.main()
