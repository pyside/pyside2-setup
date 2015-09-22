
import unittest

from PySide2.QtUiTools import QUiLoader

from helper import UsesQApplication

class QUiLoaderCreation(UsesQApplication):

    def testConstructor(self):
        loader = QUiLoader()

if __name__ == '__main__':
    unittest.main()
