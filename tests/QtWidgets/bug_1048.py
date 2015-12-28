from PySide2 import QtWidgets

a = QtWidgets.QApplication([])

w = QtWidgets.QWidget()
l = QtWidgets.QGridLayout(w)

l.itemAtPosition(0, 0)
