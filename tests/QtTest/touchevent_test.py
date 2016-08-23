from PySide2.QtWidgets import QWidget
from PySide2.QtCore import QPoint, QTimer, Qt, QEvent
from PySide2.QtGui import QTouchDevice
from PySide2.QtTest import QTest

import unittest

from helper import UsesQApplication

class MyWidget(QWidget):
    def __init__(self, parent = None):
        QWidget.__init__(self, parent)
        self._sequence = []
        # Fixme (Qt 5): The device needs to be registered (using
        # QWindowSystemInterface::registerTouchDevice()) for the test to work
        self._device = QTouchDevice()
        self.setAttribute(Qt.WA_AcceptTouchEvents)
        QTimer.singleShot(200, self.generateEvent)

    def event(self, e):
        self._sequence.append(e.type())
        return QWidget.event(self, e)

    def generateEvent(self):
        o = QTest.touchEvent(self, self._device)
        o.press(0, QPoint(10, 10))
        o.commit()
        del o

        QTest.touchEvent(self, self._device).press(0, QPoint(10, 10))
        QTest.touchEvent(self, self._device).stationary(0).press(1, QPoint(40, 10))
        QTest.touchEvent(self, self._device).move(0, QPoint(12, 12)).move(1, QPoint(45, 5))
        QTest.touchEvent(self, self._device).release(0, QPoint(12, 12)).release(1, QPoint(45, 5))
        QTimer.singleShot(200, self.deleteLater)


class TouchEventTest(UsesQApplication):
    def testCreateEvent(self):
        w = MyWidget()
        w.show()
        self.app.exec_()
        # same values as C++
        self.assertEqual(w._sequence.count(QEvent.Type.TouchBegin), 2)
        self.assertEqual(w._sequence.count(QEvent.Type.TouchUpdate), 2)
        self.assertEqual(w._sequence.count(QEvent.Type.TouchEnd), 1)


if __name__ == '__main__':
    unittest.main()
