
import unittest

from PySide.QtCore import QAbstractTableModel
from PySide.QtWidgets import QTableWidget
from helper import UsesQApplication

class QPenTest(UsesQApplication):

    def testItemModel(self):
        tv = QTableWidget()

        self.assertEqual(type(tv.model()), QAbstractTableModel)

if __name__ == '__main__':
    unittest.main()

