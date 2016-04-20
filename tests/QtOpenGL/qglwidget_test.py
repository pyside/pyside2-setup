import unittest
from PySide2.QtWidgets import *
from PySide2.QtOpenGL import *

class TestQGLWidget (unittest.TestCase):
    def testIt(self):
        """Just test if the bindTexture(*, GLenum, GLint) methods overloads exists"""
        app = QApplication([])
        img = QImage()
        w = QGLWidget()
        a = w.bindTexture(img, 0, 0) # ok if it throws nothing.. :-)





if __name__ == "__main__":
    unittest.main()
