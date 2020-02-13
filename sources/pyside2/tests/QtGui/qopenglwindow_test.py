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

'''Unit test for QOpenGLContext, QOpenGLTexture, QOpenGLWindow and related classes'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper import UsesQApplication

from PySide2.QtCore import QSize, QTimer, Qt
from PySide2.QtGui import (QColor, QGuiApplication, QImage, QOpenGLContext,
    QOpenGLTexture, QSurfaceFormat, QOpenGLWindow)

try:
    from OpenGL import GL
except ImportError:
    print("Skipping test due to missing OpenGL module")
    sys.exit(0)

class OpenGLWindow(QOpenGLWindow):
    def __init__(self):
        super(OpenGLWindow, self).__init__()

        self.m_functions = None
        self.m_texture = None
        self.visibleChanged.connect(self.slotVisibleChanged)

    def slotVisibleChanged(self, visible):
        if not visible and self.m_texture is not None and self.context().makeCurrent(self):
            self.m_texture = None
            self.context().doneCurrent()

    def initializeGL(self):
        self.m_functions = self.context().functions()
        self.m_functions.initializeOpenGLFunctions()
        image = QImage(QSize(200, 200), QImage.Format_RGBA8888)
        image.fill(QColor(Qt.red))
        self.m_texture = QOpenGLTexture(image)

    def paintGL(self):
        GL.glMatrixMode(GL.GL_MODELVIEW);
        GL.glLoadIdentity();

        GL.glMatrixMode(GL.GL_PROJECTION);
        GL.glLoadIdentity();
        GL.glOrtho(0, 1, 1, 0, -1, 1);

        self.m_functions.glClear(GL.GL_COLOR_BUFFER_BIT)
        self.m_functions.glEnable(GL.GL_TEXTURE_2D);
        self.m_texture.bind()

        d = 0.5
        GL.glBegin(GL.GL_QUADS)
        GL.glTexCoord2f(0, 0)
        GL.glVertex2f(0, 0)
        GL.glTexCoord2f(d, 0)
        GL.glVertex2f(d, 0)
        GL.glTexCoord2f(d, d)
        GL.glVertex2f(d, d)
        GL.glTexCoord2f(0, d)
        GL.glVertex2f(0, d)
        GL.glEnd()
        self.m_texture.release()

    def resizeGL(self, w, h):
        self.m_functions.glViewport(0, 0, self.width(), self.height())

class QOpenGLWindowTest(UsesQApplication):
    # On macOS, glClear(), glViewport() are rejected due to GLbitfield/GLint not being resolved properly
    def test(self):
        openGlWindow = OpenGLWindow()
        openGlWindow.resize(640, 480)
        openGlWindow.show()
        QTimer.singleShot(100, openGlWindow.close)
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
