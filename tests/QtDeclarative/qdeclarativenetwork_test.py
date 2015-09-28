'''Test cases for QDeclarativeNetwork'''

import unittest

from PySide2.QtCore import QUrl
from PySide2.QtDeclarative import QDeclarativeView, QDeclarativeNetworkAccessManagerFactory
from PySide2.QtNetwork import QNetworkAccessManager

from helper import adjust_filename, TimedQApplication

class TestQDeclarativeNetworkFactory(TimedQApplication):

    def setUp(self):
        TimedQApplication.setUp(self, timeout=1000)

    def testQDeclarativeNetworkFactory(self):
        view = QDeclarativeView()

        url = QUrl.fromLocalFile(adjust_filename('hw.qml', __file__))

        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QDeclarativeView.Ready)

        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
