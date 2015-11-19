'''Test cases for QQuickView'''

import unittest

from helper import adjust_filename, TimedQApplication

from PySide2.QtCore import QUrl, QObject, Property, Slot
from PySide2.QtQuick import QQuickView

class MyObject(QObject):
    def __init__(self, text, parent=None):
        QObject.__init__(self, parent)
        self._text = text

    def getText(self):
        return self._text


    @Slot(str)
    def qmlText(self, text):
        self._qmlText = text

    title = Property(str, getText)


class TestQQuickView(TimedQApplication):

    def testQQuickViewList(self):
        view = QQuickView()

        dataList = ["Item 1", "Item 2", "Item 3", "Item 4"]

        ctxt = view.rootContext()
        ctxt.setContextProperty("myModel", dataList)

        url = QUrl.fromLocalFile(adjust_filename('view.qml', __file__))
        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)


    def testModelExport(self):
        view = QQuickView()
        dataList = [MyObject("Item 1"), MyObject("Item 2"), MyObject("Item 3"), MyObject("Item 4")]

        ctxt = view.rootContext()
        ctxt.setContextProperty("myModel", dataList)

        url = QUrl.fromLocalFile(adjust_filename('viewmodel.qml', __file__))
        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)


if __name__ == '__main__':
    unittest.main()
