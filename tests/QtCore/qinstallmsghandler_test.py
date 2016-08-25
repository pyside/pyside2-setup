
'''Test cases for qInstallMsgHandler'''

import unittest
import sys

from PySide2.QtCore import *

param = []

def handler(msgt, ctx, msg):
    global param
    param = [msgt, ctx, msg.strip()]

def handleruseless(msgt, ctx, msg):
    pass

class QInstallMsgHandlerTest(unittest.TestCase):

    def tearDown(self):
        # Ensure that next test will have a clear environment
        qInstallMessageHandler(None)

    def testNone(self):
        ret = qInstallMessageHandler(None)
        self.assertEqual(ret, None)

    def testRet(self):
        ret = qInstallMessageHandler(None)
        self.assertEqual(ret, None)
        refcount = sys.getrefcount(handleruseless)
        retNone = qInstallMessageHandler(handleruseless)
        self.assertEqual(sys.getrefcount(handleruseless), refcount + 1)
        rethandler = qInstallMessageHandler(None)
        self.assertEqual(rethandler, handleruseless)
        del rethandler
        self.assertEqual(sys.getrefcount(handleruseless), refcount)

    def testHandler(self):
        rethandler = qInstallMessageHandler(handler)
        qDebug("Test Debug")
        self.assertEqual(param[0], QtDebugMsg)
        self.assertEqual(param[2], "Test Debug")
        qWarning("Test Warning")
        self.assertEqual(param[0], QtWarningMsg)
        self.assertEqual(param[2], "Test Warning")
        qCritical("Test Critical")
        self.assertEqual(param[0], QtCriticalMsg)
        self.assertEqual(param[2], "Test Critical")

if __name__ == '__main__':
    unittest.main()

