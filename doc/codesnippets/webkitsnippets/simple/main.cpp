
from PySide2.QtCore import *
from PySide2.QtGui import *
from PySide2.QtWebKit import *
import sys

app = QApplication(sys.argv)
parent = None
//! [Using QWebView]
view = QWebView(parent)
view.load(QUrl("http://qt.nokia.com/"))
view.show()
//! [Using QWebView]
sys.exit(app.exec_())
