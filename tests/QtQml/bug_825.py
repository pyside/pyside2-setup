import unittest

from helper import adjust_filename

from PySide2.QtCore import Qt, QUrl, QTimer
from PySide2.QtGui import QGuiApplication, QPen
from PySide2.QtWidgets import QGraphicsItem
from PySide2.QtQml import qmlRegisterType
from PySide2.QtQuick import QQuickView, QQuickItem

paintCalled = False

class MetaA(type):
    pass

class A(object):
    __metaclass__ = MetaA

MetaB = type(QQuickItem)
B = QQuickItem

class MetaC(MetaA, MetaB):
    pass

class C(A, B):
    __metaclass__ = MetaC

class Bug825 (C):

    def __init__(self, parent = None):
        QQuickItem.__init__(self, parent)
        # need to disable this flag to draw inside a QQuickItem
        self.setFlag(QGraphicsItem.ItemHasNoContents, False)

    def paint(self, painter, options, widget):
        global paintCalled
        pen = QPen(Qt.black, 2)
        painter.setPen(pen);
        painter.drawPie(self.boundingRect(), 0, 128);
        paintCalled = True

class TestBug825 (unittest.TestCase):
    def testIt(self):
        global paintCalled
        app = QGuiApplication([])
        qmlRegisterType(Bug825, 'bugs', 1, 0, 'Bug825')
        self.assertRaises(TypeError, qmlRegisterType, A, 'bugs', 1, 0, 'A')

        view = QQuickView()
        view.setSource(QUrl.fromLocalFile(adjust_filename('bug_825.qml', __file__)))
        view.show()
        QTimer.singleShot(250, view.close)
        app.exec_()
        self.assertTrue(paintCalled)


if __name__ == '__main__':
    unittest.main()
