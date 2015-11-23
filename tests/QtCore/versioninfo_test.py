import unittest
import PySide2

class TestVersionInfo(unittest.TestCase):
    def testIt(self):

        v = PySide2.__version_info__
        self.assertEqual(type(v), tuple)
        self.assertEqual(len(v), 5)
        self.assertEqual(type(v[0]), int)
        self.assertEqual(type(v[1]), int)
        self.assertEqual(type(v[2]), int)
        self.assertEqual(type(v[3]), str)
        self.assertEqual(type(v[4]), int)

        self.assertEqual(type(PySide2.__version__), str)


if __name__ == '__main__':
    unittest.main()
