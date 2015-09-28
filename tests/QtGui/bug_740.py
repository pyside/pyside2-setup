import unittest
import py3kcompat as py3k
from helper import UsesQApplication
from PySide2.QtCore import QSize
from PySide2.QtGui import QBitmap, QImage

class TestQBitmap(UsesQApplication):
    def testFromDataMethod(self):
        dataBits = py3k.b('\x38\x28\x38\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\xfe\xfe\x7c\x7c\x38\x38\x10\x10')
        bim = QBitmap.fromData(QSize(8, 48), dataBits, QImage.Format_Mono) # missing function

if __name__ == '__main__':
    unittest.main()
