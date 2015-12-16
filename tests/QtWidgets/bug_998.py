import unittest
from PySide2.QtWidgets import QApplication

class TestBug998 (unittest.TestCase):
    def testNoFocusWindow(self):
        widget = QApplication.focusWidget()
        self.assertTrue(widget == None)

if __name__ == '__main__':
    unittest.main()
