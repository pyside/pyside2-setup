'''Test cases for QQmlNetwork'''

import unittest

from PySide2.QtCore import QUrl
from PySide2.QtQuick import QQuickView
from PySide2.QtQml import QQmlNetworkAccessManagerFactory
from PySide2.QtNetwork import QNetworkAccessManager

from helper import adjust_filename, TimedQApplication

class TestQQmlNetworkFactory(TimedQApplication):

    def setUp(self):
        TimedQApplication.setUp(self, timeout=1000)

    def testQQuickNetworkFactory(self):
        view = QQuickView()

        url = QUrl.fromLocalFile(adjust_filename('hw.qml', __file__))

        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)

        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
