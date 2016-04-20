import unittest
import sys

from PySide2 import QtCore
from PySide2 import QtGui
from PySide2 import QtWidgets

from helper import TimedQApplication

class TestFontDialog(TimedQApplication):

    def testGetFont(self):
        QtWidgets.QFontDialog.getFont()

    def testGetFontQDialog(self):
        QtWidgets.QFontDialog.getFont(QtGui.QFont("FreeSans",10))
    
    def testGetFontQDialogQString(self):
        QtWidgets.QFontDialog.getFont(QtGui.QFont("FreeSans",10), None, "Select font")

if __name__ == '__main__':
    unittest.main()

