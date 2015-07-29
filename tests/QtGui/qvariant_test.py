
import unittest
from PySide.QtCore import *
from PySide.QtWidgets import *

class MyDiagram(QGraphicsScene):
    pass

class MyItem(QGraphicsRectItem):
    def itemChange(self, change, value):
        return value;

class QGraphicsSceneOnQVariantTest(unittest.TestCase):
    """Test storage ot QGraphicsScene into QVariants"""
    def testIt(self):
        app = QApplication([])
        s = MyDiagram()
        i = MyItem()
        s.addItem(i)
        self.assertEqual(len(s.items()), 1)

if __name__ == '__main__':
    unittest.main()
