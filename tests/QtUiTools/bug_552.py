from helper import adjust_filename
from PySide2 import QtWidgets, QtCore
from PySide2.QtUiTools import QUiLoader

class View_1(QtWidgets.QWidget):

    def __init__(self):
        QtWidgets.QWidget.__init__(self)
        loader = QUiLoader()
        widget = loader.load(adjust_filename('bug_552.ui', __file__), self)
        self.children = []
        for child in widget.findChildren(QtCore.QObject, None):
            self.children.append(child)
        self.t = widget.tabWidget
        self.t.removeTab(0)

app = QtWidgets.QApplication([])
window = View_1()
window.show()

# If it doesn't crash it works :-)
