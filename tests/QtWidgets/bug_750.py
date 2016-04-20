import unittest

from helper import UsesQApplication

from PySide2.QtCore import QTimer
from PySide2.QtGui import QPainter, QFont, QFontInfo
from PySide2.QtWidgets import QWidget, qApp

class MyWidget(QWidget):
    def paintEvent(self, e):
        p = QPainter(self)
        self._info = p.fontInfo()
        self._app.quit()


class TestQPainter(UsesQApplication):
    def testFontInfo(self):
        w = MyWidget()
        w._app = self.app
        w._info = None
        QTimer.singleShot(300, w.show)
        self.app.exec_()
        self.assert_(w._info)

if __name__ == '__main__':
    unittest.main()
