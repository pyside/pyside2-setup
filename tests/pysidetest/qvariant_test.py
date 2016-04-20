import unittest

from testbinding import TestObject
from PySide2.QtCore import Qt
from PySide2.QtGui import QKeySequence

from helper import UsesQApplication

class QVariantTest(UsesQApplication):

    def testQKeySequenceQVariantOperator(self):
        # bug #775
        ks = QKeySequence(Qt.SHIFT, Qt.CTRL, Qt.Key_P, Qt.Key_R)
        self.assertEqual(TestObject.checkType(ks), 75)

if __name__ == '__main__':
    unittest.main()
