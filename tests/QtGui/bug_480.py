import unittest

from PySide2 import QtWidgets

class BuggyWidget(QtWidgets.QWidget):
    def setup(self):
        self.verticalLayout = QtWidgets.QVBoxLayout(self)
        self.gridLayout = QtWidgets.QGridLayout()
        self.lbl = QtWidgets.QLabel(self)
        self.gridLayout.addWidget(self.lbl, 0, 1, 1, 1)

        # this cause a segfault during the ownership transfer
        self.verticalLayout.addLayout(self.gridLayout)

class LayoutTransferOwnerShip(unittest.TestCase):
    def testBug(self):
        app = QtWidgets.QApplication([])
        w = BuggyWidget()
        w.setup()
        w.show()
        self.assert_(True)

if __name__ == '__main__':
    unittest.main()

