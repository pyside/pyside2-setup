import unittest
from sample import *
from other import *

class TestHashFuncs (unittest.TestCase):

    def testIt(self):
        obj1 = HandleHolder()
        obj2 = HandleHolder()

        hash1 = hash(obj1)
        hash2 = hash(obj2)
        self.assertNotEqual(hash1, hash2)
        self.assertNotEqual(hash(HandleHolder()), hash(HandleHolder()))

if __name__ == '__main__':
    unittest.main()
