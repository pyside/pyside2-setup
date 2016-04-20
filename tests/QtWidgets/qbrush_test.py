
'''Test cases for QBrush'''

import unittest

from PySide2.QtCore import Qt
from PySide2.QtGui import QColor, QBrush
from PySide2.QtWidgets import QApplication

from helper import UsesQApplication

class Constructor(UsesQApplication):
    '''Test case for constructor of QBrush'''

    def testQColor(self):
        #QBrush(QColor) constructor
        color = QColor('black')
        obj = QBrush(color)
        self.assertEqual(obj.color(), color)

        obj = QBrush(Qt.blue)
        self.assertEqual(obj.color(), Qt.blue)

if __name__ == '__main__':
    unittest.main()
