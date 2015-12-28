''' Test bug 338: http://bugs.openbossa.org/show_bug.cgi?id=338'''

import sys
import unittest

from PySide2 import QtCore, QtWidgets

class DiagramItem(QtWidgets.QGraphicsPolygonItem):
    def __init__(self, parent=None, scene=None):
        super(DiagramItem, self).__init__(parent, scene)

    def itemChange(self, change, value):
        return value


class BugTest(unittest.TestCase):
    def test(self):
        app = QtWidgets.QApplication(sys.argv)
        scene = QtWidgets.QGraphicsScene()
        item = DiagramItem()
        item2 = DiagramItem()
        #this cause segfault 
        scene.addItem(item)
        scene.addItem(item2)
  
if __name__ == "__main__":
    unittest.main()