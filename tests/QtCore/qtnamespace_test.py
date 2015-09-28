#!/usr/bin/python
'''Test suite for QtCore.Qt namespace'''

import unittest

from PySide2.QtCore import Qt

class QtNamespace(unittest.TestCase):
    '''Test case for accessing attributes from Qt namespace'''

    def testBasic(self):
        #Access to Qt namespace
        getattr(Qt, 'Horizontal')
        getattr(Qt, 'WindowMaximizeButtonHint')
        self.assert_(True)


if __name__ == '__main__':
    unittest.main()
