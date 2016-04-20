from PySide2 import QtCore, QtWidgets

class Window(QtWidgets.QMainWindow):
    def childEvent(self, event):
        super(Window, self).childEvent(event)

app = QtWidgets.QApplication([])
window = Window()

dock1 = QtWidgets.QDockWidget()
dock2 = QtWidgets.QDockWidget()
window.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock1)
window.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock2)
window.tabifyDockWidget(dock1, dock2)

window.show()
QtCore.QTimer.singleShot(0, window.close)
app.exec_()
