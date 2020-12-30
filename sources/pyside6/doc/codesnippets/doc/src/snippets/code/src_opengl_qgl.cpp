/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of Qt for Python.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
fmt = QGLFormat()
fmt.setAlpha(True)
fmt.setStereo(True)
QGLFormat.setDefaultFormat(fmt)
//! [0]


//! [1]
fmt = QGLFormat()
fmt.setDoubleBuffer(False)                 # single buffer
fmt.setDirectRendering(False)              # software rendering
myWidget = MyGLWidget(fmt, ...)
//! [1]


//! [2]
fmt = QGLFormat()
fmt.setOverlay(True)
fmt.setStereo(True)
myWidget = MyGLWidget(fmt, ...)
if !myWidget.format().stereo():
    # ok, goggles off
    if !myWidget.format().hasOverlay():
        print("Cool hardware required")
//! [2]


//! [3]
# The rendering in MyGLWidget depends on using
# stencil buffer and alpha channel

class MyGLWidget(QGLWidget):
    def __init__(self, parent):
        QGLWidget.__init__(self, QGLFormat(QGL.StencilBuffer | QGL.AlphaChannel), parent)

        if !format().stencil():
            print("Could not get stencil buffer results will be suboptimal")
        if !format().alpha():
            print("Could not get alpha channel results will be suboptimal")
    ...
//! [3]


//! [4]
a = QApplication([])
f = QGLFormat()
f.setDoubleBuffer(False)
QGLFormat.setDefaultFormat(f)
//! [4]


//! [5]
f = QGLFormat.defaultOverlayFormat()
f.setDoubleBuffer(True)
QGLFormat.setDefaultOverlayFormat(f)
//! [5]


//! [6]
# ...continued from above
myWidget = MyGLWidget(QGLFormat(QGL.HasOverlay), ...)
if myWidget.format().hasOverlay():
    # Yes, we got an overlay, let's check _its_ format:
    olContext = myWidget.overlayContext()
    if olContext.format().doubleBuffer():
         # yes, we got a double buffered overlay
    else:
         # no, only single buffered overlays are available
//! [6]


//! [7]
cx = QGLContext()
#  ...
f = QGLFormat()
f.setStereo(True)
cx.setFormat(f)
if !cx.create():
    exit() # no OpenGL support, or cannot render on the specified paintdevice
if !cx.format().stereo():
    exit() # could not create stereo context
//! [7]


//! [8]
class MyGLDrawer(QGLWidget):

    def __init__(self, parent):
        QGLWidget.__init__(self, parent)
        pass

    def initializeGL(self):
        # Set up the rendering context, define display lists etc.:
        ...
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glEnable(GL_DEPTH_TEST)
        ...

    def resizeGL(self, w, h):
        # setup viewport, projection etc.:
        glViewport(0, 0, w, h)
        ...
        glFrustum(...)
        ...

    def paintGL(self):
        # draw the scene:
        ...
        glRotatef(...)
        glMaterialfv(...)
        glBegin(GL_QUADS)
        glVertex3f(...)
        glVertex3f(...)
        ...
        glEnd()
        ...
//! [8]
