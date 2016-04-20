#!/usr/bin/python

import unittest

from PySide2.QtCore import *
from PySide2.QtPrintSupport import *
from helper import UsesQApplication

class NeverDiesTest(UsesQApplication):

    def testIt(self):
        QPrintDialog()

if __name__ == "__main__":
    unittest.main()
