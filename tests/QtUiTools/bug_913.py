#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import unittest
from helper import adjust_filename

from PySide2.QtCore import *
from PySide2.QtWidgets import *
from PySide2.QtUiTools import *

class TestBug913 (unittest.TestCase):

    def testIt(self):
        app = QApplication([])

        loader = QUiLoader()
        widget = loader.load(adjust_filename('bug_913.ui', __file__))
        widget.tabWidget.currentIndex() # direct child is available as member
        widget.le_first.setText('foo') # child of QTabWidget must also be available!

if __name__ == '__main__':
    unittest.main()
