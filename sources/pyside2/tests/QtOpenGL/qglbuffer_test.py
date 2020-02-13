#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

'''Unit tests for QGLBuffer'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QByteArray
from PySide2.QtOpenGL import QGLBuffer, QGLWidget
import py3kcompat as py3k
from helper import UsesQApplication

class QGLBufferTest(UsesQApplication):
    def testIt(self):
        w = QGLWidget()
        w.makeCurrent()

        b = QGLBuffer()
        b.setUsagePattern(QGLBuffer.DynamicDraw)

        self.assertTrue(b.create())
        self.assertTrue(b.bufferId() != 0)
        self.assertTrue(b.bind())

        data = QByteArray(py3k.b("12345"))
        b.allocate(data)
        self.assertEqual(b.size(), data.size())

        m = b.map(QGLBuffer.ReadOnly)
        if m:
            self.assertEqual(m, py3k.buffer(py3k.b(data.data())))
            b.unmap()

            m = b.map(QGLBuffer.ReadWrite)
            m[3] = py3k.b('A')[0]
            b.unmap()
            result, rdata = b.read(3, 1)
            self.assertTrue(result)
            self.assertEqual(py3k.b('A'), rdata.data())
        else:
            print(" memory mapping is not possible in this OpenGL implementation.")
        b.release()

if __name__ == '__main__':
    unittest.main()
