import unittest

from PySide2.QtCore import *
from PySide2.QtScript import *

class QScriptValueTest(unittest.TestCase):
    def testQScriptValue(self): 
        app = QCoreApplication([])
        engine = QScriptEngine()
        repr(engine.evaluate('1 + 1'))

if __name__ == '__main__':
    unittest.main()
