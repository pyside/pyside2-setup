""" Unittest for bug #576 """
""" http://bugs.openbossa.org/show_bug.cgi?id=576 """

import sys
import unittest

from PySide2 import QtCore, QtWidgets

class Bug576(unittest.TestCase):
    def onButtonDestroyed(self, button):
        self._destroyed = True
        self.assertTrue(isinstance(button, QtWidgets.QPushButton))

    def testWidgetParent(self):
        self._destroyed = False
        app = QtWidgets.QApplication(sys.argv)
        w = QtWidgets.QWidget()

        b = QtWidgets.QPushButton("test")
        b.destroyed[QtCore.QObject].connect(self.onButtonDestroyed)
        self.assertEqual(sys.getrefcount(b), 2)
        b.setParent(w)
        self.assertEqual(sys.getrefcount(b), 3)
        b.parent()
        self.assertEqual(sys.getrefcount(b), 3)
        b.setParent(None)
        self.assertEqual(sys.getrefcount(b), 2)
        del b
        self.assertTrue(self._destroyed)


if __name__ == '__main__':
    unittest.main()

