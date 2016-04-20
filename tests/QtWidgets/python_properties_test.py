import unittest

from PySide2 import QtCore, QtWidgets

class Properties(unittest.TestCase):

    def testStaticProperty(self):
        self.assertEqual(QtWidgets.QGraphicsItem.UserType, 65536)

    def testInstanceProperty(self):
        p = QtWidgets.QStyleOptionViewItem()
        self.assert_(isinstance(p.locale, QtCore.QLocale))


if __name__ == '__main__':
    unittest.main()
