
import unittest

from PySide2.QtCore import QAbstractTableModel
from PySide2.QtWidgets import QTableWidget
from helper import UsesQApplication

class QPenTest(UsesQApplication):

    def testItemModel(self):
        tv = QTableWidget()

        self.assertEqual(type(tv.model()), QAbstractTableModel)

if __name__ == '__main__':
    unittest.main()

