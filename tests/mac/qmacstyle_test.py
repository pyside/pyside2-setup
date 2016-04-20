# Qt5: this is gone: from PySide2.QtGui import QMacStyle
from PySide2.QtWidgets import QApplication, QLabel, QStyleFactory
from PySide2.QtCore import QObject

import unittest

from helper import UsesQApplication

QMacStyle = type(QStyleFactory.create('Macintosh'))

class QMacStyleTest(UsesQApplication):
    def testWidgetStyle(self):
        w = QLabel('Hello')
        self.assertTrue(isinstance(w.style(), QMacStyle))

if __name__ == '__main__':
    unittest.main()
