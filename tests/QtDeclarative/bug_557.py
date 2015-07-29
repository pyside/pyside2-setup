from PySide.QtCore import *
from PySide.QtWidgets import *
from PySide.QtDeclarative import *

import sys

app = QApplication(sys.argv)

engine = QDeclarativeEngine()
component = QDeclarativeComponent(engine)

# This should segfault if the QDeclarativeComponent has not QDeclarativeEngine
component.loadUrl(QUrl.fromLocalFile('foo.qml'))

