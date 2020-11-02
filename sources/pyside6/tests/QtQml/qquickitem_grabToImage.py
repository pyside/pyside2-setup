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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from helper.helper import adjust_filename
from helper.timedqapplication import TimedQApplication

from PySide6 import QtCore, QtGui, QtQuick

class TestGrabToSharedPointerImage(TimedQApplication):
    def setUp(self):
        TimedQApplication.setUp(self, 1000)

    def testQQuickItemGrabToImageSharedPointer(self):
        view = QtQuick.QQuickView()
        view.setSource(QtCore.QUrl.fromLocalFile(
                                   adjust_filename('qquickitem_grabToImage.qml', __file__)))
        view.show()

        # Get the QQuickItem objects for the blue Rectangle and the Image item.
        root = view.rootObject()
        blueRectangle = root.findChild(QtQuick.QQuickItem, "blueRectangle")
        imageContainer = root.findChild(QtQuick.QQuickItem, "imageContainer")

        # Start the image grabbing.
        grabResultSharedPtr = blueRectangle.grabToImage()

        # Implicit call of operator bool() of the smart pointer, to check that it holds
        # a valid pointer.
        self.assertTrue(grabResultSharedPtr)

        self.grabbedColor = None
        def onGrabReady():
            # Signal early exit.
            QtCore.QTimer.singleShot(0, self.app.quit)

            # Show the grabbed image in the QML Image item.
            imageContainer.setProperty("source", grabResultSharedPtr.url())

        # Wait for signal when grabbing is complete.
        grabResultSharedPtr.ready.connect(onGrabReady)
        self.app.exec_()

        # Get the first pixel color of the grabbed image.
        self.image = grabResultSharedPtr.image()
        self.assertTrue(self.image)
        self.grabbedColor = self.image.pixelColor(0,0)
        self.assertTrue(self.grabbedColor.isValid())

        # Compare the grabbed color with the one we set in the rectangle.
        blueColor = QtGui.QColor("blue")
        self.assertEqual(self.grabbedColor, blueColor)


if __name__ == '__main__':
    unittest.main()
