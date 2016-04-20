import unittest

from PySide2.QtCore import QCoreApplication, QTimer, QEvent, Qt
from PySide2.QtWidgets import QApplication, QSpinBox
from PySide2.QtGui import QKeyEvent

class MySpinBox(QSpinBox):

    def validate(self,text,pos):
        return QSpinBox.validate(self,text,pos)

class TestBug879 (unittest.TestCase):

    def testIt(self):
        app = QApplication([])
        self.box = MySpinBox()
        self.box.show()

        QTimer.singleShot(0, self.sendKbdEvent)
        QTimer.singleShot(100, app.quit)
        app.exec_()

        self.assertEqual(self.box.text(), '0')

    def sendKbdEvent(self):
        ev = QKeyEvent(QEvent.KeyPress, Qt.Key_A, Qt.NoModifier, 'a')
        QCoreApplication.sendEvent(self.box, ev)

if __name__ == '__main__':
    unittest.main()
