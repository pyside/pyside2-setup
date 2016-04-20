import unittest
from PySide2.QtWidgets import QToolBar, QApplication, QAction, QToolButton

try:
    xrange
except NameError:
    xrange = range # py3k

class TestLabelPixmap(unittest.TestCase):
    def testReference(self):
        toolbar = QToolBar()

        for i in xrange(20):
            toolbar.addAction(QAction("Action %d" % i, None))

        buttons = toolbar.findChildren(QToolButton, "")
        toolbar.clear()

        for b in buttons:
            self.assertRaises(RuntimeError, b.objectName)

if __name__ == '__main__':
    app = QApplication([])
    unittest.main()

