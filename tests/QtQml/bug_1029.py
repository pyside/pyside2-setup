import sys
import gc

from PySide2.QtCore import QUrl
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import qmlRegisterType
from PySide2.QtQuick import QQuickView, QQuickItem

def register_qml_types():
    class TestClass(QQuickItem):
        def __init__(self, parent = None):
            QQuickItem.__init__(self, parent)

    qmlRegisterType(TestClass, "UserTypes", 1, 0, "TestClass")

def main():
    app = QGuiApplication([])

    # reg qml types here
    register_qml_types()

    # force gc to run
    gc.collect()

    view = QQuickView()
    url = QUrl(__file__.replace(".py", ".qml"))
    view.setSource(url)

if __name__ == "__main__":
    main()
