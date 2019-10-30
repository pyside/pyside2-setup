

#include <QtWidgets/QApplication>
#include <QtQuick/QQuickView>
#include <QtCore/QDir>
#include <QtQml/QQmlEngine>
import sys
import os
from PySide2.QtQuick import QQuickView 
from PySide2.QtCore import Qt, QUrl
from PySide2.QtWidgets import QApplication, QMainWindow

        
if __name__ == '__main__':
    app = QApplication(sys.argv)
    viewer = QQuickView()
  
    viewer.engine().addImportPath(os.path.dirname(__file__))
    #viewerquit.connect(viewer.close())

    viewer.setTitle = "QML Polar Chart"
    qmlFile = os.path.join(os.path.dirname(__file__), 'main.qml')
    viewer.setSource(QUrl.fromLocalFile(os.path.abspath(qmlFile)))
    viewer.setResizeMode(QQuickView.SizeRootObjectToView)
    viewer.show()

    sys.exit(app.exec_())