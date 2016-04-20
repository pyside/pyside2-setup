import sys

from PySide2.QtCore import QUrl
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import QQmlEngine, QQmlComponent

app = QGuiApplication(sys.argv)

engine = QQmlEngine()
component = QQmlComponent(engine)

# This should segfault if the QDeclarativeComponent has not QQmlEngine
component.loadUrl(QUrl.fromLocalFile('foo.qml'))

