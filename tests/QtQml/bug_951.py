import unittest

from helper import adjust_filename, TimedQApplication

from PySide2.QtCore import QUrl
from PySide2.QtQml import qmlRegisterType
from PySide2.QtQuick import QQuickItem, QQuickView

class MyItem(QQuickItem):
    COMPONENT_COMPLETE_CALLED = False
    def __init__(self,parent=None):
        super(MyItem, self).__init__(parent)
        self.setObjectName("myitem")

    def componentComplete(self):
        MyItem.COMPONENT_COMPLETE_CALLED = True
        super(MyItem, self).componentComplete()

class TestRegisterQMLType(TimedQApplication):
    def setup(self):
        TimedQApplication.setup(self, 100 * 3) # 3s

    def testSignalEmission(self):
        qmlRegisterType(MyItem, "my.item", 1, 0, "MyItem")

        view = QQuickView()
        view.setSource(QUrl.fromLocalFile(adjust_filename('bug_951.qml', __file__)))

        self.app.exec_()
        self.assertTrue(MyItem.COMPONENT_COMPLETE_CALLED)

if __name__ == '__main__':
    unittest.main()
