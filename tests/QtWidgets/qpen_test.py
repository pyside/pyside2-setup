
import unittest
from helper import UsesQApplication

from PySide2.QtCore import Qt, QTimer
from PySide2.QtGui import QPen, QPainter
from PySide2.QtWidgets import QWidget

class Painting(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.penFromEnum = None
        self.penFromInteger = None

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(Qt.NoPen)
        self.penFromEnum = painter.pen()
        painter.setPen(int(Qt.NoPen))
        self.penFromInteger = painter.pen()


class QPenTest(UsesQApplication):

    def testCtorWithCreatedEnums(self):
        '''A simple case of QPen creation using created enums.'''
        width = 0
        style = Qt.PenStyle(0)
        cap = Qt.PenCapStyle(0)
        join = Qt.PenJoinStyle(0)
        pen = QPen(Qt.blue, width, style, cap, join)

    def testSetPenWithPenStyleEnum(self):
        '''Calls QPainter.setPen with both enum and integer. Bug #511.'''
        w = Painting()
        w.show()
        QTimer.singleShot(1000, self.app.quit)
        self.app.exec_()
        self.assertEqual(w.penFromEnum.style(), Qt.NoPen)
        self.assertEqual(w.penFromInteger.style(), Qt.SolidLine)


if __name__ == '__main__':
    unittest.main()

