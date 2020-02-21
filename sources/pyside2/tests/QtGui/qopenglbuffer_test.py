#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

'''Unit tests for QOpenGLBuffer'''

import ctypes
import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.usesqapplication import UsesQApplication
from PySide2.QtGui import QOpenGLBuffer
from PySide2 import QtGui

def createSurface(surfaceClass):
    if surfaceClass == QtGui.QSurface.Window:
        window = QtGui.QWindow()
        window.setSurfaceType(QtGui.QWindow.OpenGLSurface)
        window.setGeometry(0, 0, 10, 10)
        window.create()
        return window
    elif surfaceClass == QtGui.QSurface.Offscreen:
        # Create a window and get the format from that.  For example, if an EGL
        # implementation provides 565 and 888 configs for PBUFFER_BIT but only
        # 888 for WINDOW_BIT, we may end up with a pbuffer surface that is
        # incompatible with the context since it could choose the 565 while the
        # window and the context uses a config with 888.
        format = QtGui.QSurfaceFormat
        if format.redBufferSize() == -1:
            window = QtGui.QWindow()
            window.setSurfaceType(QtGui.QWindow.OpenGLSurface)
            window.setGeometry(0, 0, 10, 10)
            window.create()
            format = window.format()
        offscreenSurface = QtGui.QOffscreenSurface()
        offscreenSurface.setFormat(format)
        offscreenSurface.create()
        return offscreenSurface
    return 0

class QOpenGLBuffer(UsesQApplication):
    def testBufferCreate(self):
        surface = createSurface(QtGui.QSurface.Window)
        ctx = QtGui.QOpenGLContext()
        ctx.create()
        ctx.makeCurrent(surface)

        buf = QtGui.QOpenGLBuffer()

        self.assertTrue(not buf.isCreated())

        self.assertTrue(buf.create())
        self.assertTrue(buf.isCreated())

        self.assertEqual(buf.type(), QtGui.QOpenGLBuffer.VertexBuffer)

        buf.bind()
        buf.allocate(128)
        self.assertEqual(buf.size(), 128)

        buf.release()

        buf.destroy()
        self.assertTrue(not buf.isCreated())

        ctx.doneCurrent()

if __name__ == '__main__':
    unittest.main()
