import unittest

from helper import UsesQApplication
from PySide2.QtCore import Qt, QTimer
from PySide2.QtGui import QPainter
from PySide2.QtWidgets import QLabel
from PySide2.QtWidgets import QGraphicsScene, QGraphicsView, QGraphicsItem, QGraphicsProxyWidget

class QGraphicsProxyWidgetTest(UsesQApplication):
    def testQGraphicsProxyWidget(self):
        scene = QGraphicsScene()

        proxy = QGraphicsProxyWidget(None, Qt.Window)
        widget = QLabel('Widget')
        proxy.setWidget(widget)
        proxy.setCacheMode(QGraphicsItem.DeviceCoordinateCache)
        scene.addItem(proxy)
        scene.setSceneRect(scene.itemsBoundingRect())

        view = QGraphicsView(scene)
        view.setRenderHints(QPainter.Antialiasing|QPainter.SmoothPixmapTransform)
        view.setViewportUpdateMode(QGraphicsView.BoundingRectViewportUpdate)
        view.show()

        timer = QTimer.singleShot(100, self.app.quit)
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()

