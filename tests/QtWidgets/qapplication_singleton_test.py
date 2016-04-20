
import unittest

from PySide2.QtWidgets import QApplication

class TestSingleton(unittest.TestCase):
    def testBasic(self):
        a = QApplication([])
        self.assertRaises(RuntimeError, QApplication, [])

if __name__ == '__main__':
    unittest.main()
