
'''(Very) Simple test case for missing names from QtGui and QtWidgets'''

import unittest
from PySide2 import QtGui
from PySide2 import QtWidgets

class MissingClasses(unittest.TestCase):
    def testQDrag(self): # Bug 222
        getattr(QtGui, 'QDrag')

    def testQDropEvent(self): # Bug 255
        getattr(QtGui, 'QDropEvent')

class MissingMembers(unittest.TestCase):

    def testQFontMetricsSize(self): # Bug 223
        QtGui.QFontMetrics.size

    def testQLayoutSetSpacing(self): # Bug 231
        QtWidgets.QLayout.setSpacing

    def testQImageLoad(self): # Bug 257
        QtGui.QImage.load

    def testQStandardItemModelinsertRow(self): # Bug 227
        QtGui.QStandardItemModel.insertRow

if __name__ == '__main__':
    unittest.main()
