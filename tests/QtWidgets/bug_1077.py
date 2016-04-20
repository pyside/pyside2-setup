''' unit test for BUG #1077 '''

from PySide2 import QtCore, QtGui, QtWidgets
import time

class Highlighter(QtGui.QSyntaxHighlighter):
    def __init__(self, parent, mode):
        QtGui.QSyntaxHighlighter.__init__(self, parent)
        self.tstamp = time.time()

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    python = QtWidgets.QTextEdit()
    python.setWindowTitle("python")
    hl = Highlighter(python.document(), "python")
    python.show()
    text = hl.document()
