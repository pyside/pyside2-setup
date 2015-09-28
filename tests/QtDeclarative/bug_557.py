from PySide2.QtCore import *
from PySide2.QtWidgets import *
from PySide2.QtDeclarative import *

import sys

app = QApplication(sys.argv)

engine = QDeclarativeEngine()
component = QDeclarativeComponent(engine)

# This should segfault if the QDeclarativeComponent has not QDeclarativeEngine
component.loadUrl(QUrl.fromLocalFile('foo.qml'))

