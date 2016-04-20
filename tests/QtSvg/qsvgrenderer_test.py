#!/usr/bin/python
import unittest, os
from PySide2.QtCore import *
from PySide2.QtWidgets import *
from PySide2.QtSvg import *

class QSvgRendererTest(unittest.TestCase):

    def testLoad(self):
        tigerPath = os.path.join(os.path.dirname(__file__), 'tiger.svg')
        app = QApplication([])

        fromFile = QSvgRenderer(tigerPath)
        self.assertTrue(fromFile.isValid())

        tigerFile = QFile(tigerPath)
        tigerFile.open(QFile.ReadOnly)
        tigerData = tigerFile.readAll()
        fromContents = QSvgRenderer(tigerData)
        self.assertTrue(fromContents.isValid())

if __name__ == '__main__':
    unittest.main()

