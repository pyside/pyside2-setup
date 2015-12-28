import unittest

from helper import TimedQApplication
from PySide2.QtGui import QPainter
from PySide2.QtWidgets import QPushButton, QStyleOptionButton, QApplication, QStyle

class MyWidget(QPushButton):
    def __init__(self, parent = None):
        QPushButton.__init__(self, parent)
        self._painted = False

    def paintEvent(self, e):
        p = QPainter(self)
        style = QApplication.style()
        option = QStyleOptionButton()
        style.drawControl(QStyle.CE_PushButton, option, p)
        self._painted = True


class TestBug919(TimedQApplication):
    def testFontInfo(self):
        w = MyWidget()
        w.show()
        self.app.exec_()
        self.assert_(w._painted)

if __name__ == '__main__':
    unittest.main()
