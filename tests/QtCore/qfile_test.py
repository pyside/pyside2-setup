
import unittest

import os
import tempfile
import sys
import py3kcompat as py3k

from PySide2.QtCore import QFile, QIODevice

class GetCharTest(unittest.TestCase):
    '''Test case for QIODevice.getChar in QFile'''

    def setUp(self):
        '''Acquire resources'''
        handle, self.filename = tempfile.mkstemp()
        os.write(handle, py3k.b('a'))
        os.close(handle)

    def tearDown(self):
        '''release resources'''
        os.remove(self.filename)

    def testBasic(self):
        '''QFile.getChar'''
        obj = QFile(self.filename)
        obj.open(QIODevice.ReadOnly)
        try:
            self.assertEqual(obj.getChar(), (True, 'a'))
            self.assertFalse(obj.getChar()[0])
        finally:
            obj.close()

    def testBug721(self):
        obj = QFile(self.filename)
        obj.open(QIODevice.ReadOnly)
        try:
            memory = obj.map(0, 1)
            self.assertEqual(len(memory), 1)
            if sys.version_info[0] >= 3:
                self.assertEqual(memory[0], ord('a'))
            else:
                self.assertEqual(memory[0], py3k.b('a'))
            # now memory points to wild bytes... :-)
            # uncommenting this must cause a segfault.
            # self.assertEqual(memory[0], 'a')
        finally:
            obj.unmap(memory)
            obj.close()

if __name__ == '__main__':
    unittest.main()
