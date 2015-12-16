import sys
from PySide2.QtWidgets import *
import unittest

class Bug640(unittest.TestCase):
    def testIt(self):
        option = QStyleOptionGraphicsItem()
        a = option.state # crash!?

if __name__ == "__main__":
   unittest.main()
