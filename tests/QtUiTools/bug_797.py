from PySide2 import QtUiTools
from PySide2 import QtCore
from PySide2 import QtGui
from helper import adjust_filename

app = QtGui.QApplication([])
loader = QtUiTools.QUiLoader()
file = QtCore.QFile(adjust_filename('bug_552.ui', __file__))
w = QtGui.QWidget()
# An exception can't be thrown
mainWindow = loader.load(file, w)
