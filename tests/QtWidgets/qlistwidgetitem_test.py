
import unittest

from PySide2 import QtWidgets

from helper import UsesQApplication

class QListWidgetItemConstructor(UsesQApplication):

    def setUp(self):
        super(QListWidgetItemConstructor, self).setUp()
        self.widgetList = QtWidgets.QListWidget()

    def tearDown(self):
        del self.widgetList
        super(QListWidgetItemConstructor, self).tearDown()

    def testConstructorWithParent(self):
        # Bug 235 - QListWidgetItem constructor not saving ownership
        QtWidgets.QListWidgetItem(self.widgetList)
        item = self.widgetList.item(0)
        self.assertEqual(item.listWidget(), self.widgetList)

    def testConstructorWithNone(self):
        # Bug 452 - QListWidgetItem() not casting NoneType to null correctly.
        item = QtWidgets.QListWidgetItem(None, 123)

if __name__ == '__main__':
    unittest.main()
