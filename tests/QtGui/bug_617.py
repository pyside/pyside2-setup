from PySide2.QtCore import QEvent
from PySide2.QtGui import QColor
import unittest

class MyEvent(QEvent):
    def __init__(self):
        QEvent.__init__(self, QEvent.Type(999))


class Bug617(unittest.TestCase):
    def testRepr(self):
        c = QColor.fromRgb(1, 2, 3, 4)
        s = c.spec()
        self.assertEqual(repr(s), repr(QColor.Rgb))

    def testOutOfBounds(self):
        e = MyEvent()
        self.assertEqual(repr(e.type()), 'PySide2.QtCore.QEvent.Type(999)')

if __name__ == "__main__":
   unittest.main()
