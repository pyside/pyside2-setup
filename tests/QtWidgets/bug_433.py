import sys

from PySide2 import QtCore, QtWidgets

class Test(QtWidgets.QGraphicsView):
    def __init__(self, parent=None):
        super(Test, self).__init__(parent)
        self.s = QtWidgets.QGraphicsScene()
        self.setScene(self.s)

a = QtWidgets.QApplication(sys.argv)
t = Test()
t.show()
QtCore.QTimer.singleShot(0, t.close)
sys.exit(a.exec_())
