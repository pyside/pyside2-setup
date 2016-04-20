# trimmed down diagramscene.py to demonstrate crash in sizeHint()

import sys
from PySide2 import QtCore, QtWidgets
import unittest

class CustomWidget(QtWidgets.QGraphicsProxyWidget):
   def itemChange(self, eventType, value):
      QtWidgets.QGraphicsProxyWidget.itemChange(self, eventType, value)

class Bug589(unittest.TestCase):
   def testCase(self):
      widget = QtWidgets.QGraphicsProxyWidget()
      custom = CustomWidget()
      custom.setParentItem(widget)

if __name__ == "__main__":
   app = QtWidgets.QApplication(sys.argv)
   unittest.main()
