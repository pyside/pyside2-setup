# -*- coding: utf-8 -*-

''' Test the QMenu.addAction() method'''

import unittest
import sys

from PySide2 import QtWidgets

from helper import UsesQApplication

class QMenuAddAction(UsesQApplication):

    def openFile(self, *args):
        self.arg = args

    def testQMenuAddAction(self):
        fileMenu = QtWidgets.QMenu("&File")

        addNewAction = fileMenu.addAction("&Open...", self.openFile)
        addNewAction.trigger()
        self.assertEquals(self.arg, ())

if __name__ == '__main__':
    unittest.main()
