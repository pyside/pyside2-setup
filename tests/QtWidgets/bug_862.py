
from PySide2.QtCore import QObject
from PySide2.QtWidgets import *
import PySide2.QtCore
import unittest

class MyQObject(QObject):
    def __init__(self):
        QObject.__init__(self)

class MyQWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)

class MyQGraphicsObject(QGraphicsObject):
    def __init__(self):
        QGraphicsObject.__init__(self)

class MyQGraphicsItem(QGraphicsItem):
    def __init__(self):
        QGraphicsItem.__init__(self)

class TestRepr (unittest.TestCase):

    def testIt(self):

        app = QApplication([])

        self.assertEqual("<__main__.MyQObject object at ", repr(MyQObject())[:30])
        self.assertEqual("<__main__.MyQWidget object at ", repr(MyQWidget())[:30])
        self.assertEqual("<__main__.MyQGraphicsObject(0x", repr(MyQGraphicsObject())[:30])
        self.assertEqual("<__main__.MyQGraphicsItem(0x", repr(MyQGraphicsItem())[:28])

        self.assertEqual("<PySide2.QtCore.QObject object at ", repr(QObject())[:34])
        self.assertEqual("<PySide2.QtCore.QObject object at ", repr(PySide2.QtCore.QObject())[:34])
        self.assertEqual("<PySide2.QtWidgets.QWidget object at ", repr(QWidget())[:37])
        self.assertEqual("<PySide2.QtWidgets.QGraphicsWidget(0x", repr(QGraphicsWidget())[:37])

if __name__ == "__main__":
    unittest.main()
