#!/usr/bin/python
# -*- coding: utf-8 -*-

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

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.helper import adjust_filename

from PySide2.QtCore import QObject, QUrl, Slot, QTimer
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import QQmlIncubationController, VolatileBool
from PySide2.QtQuick import QQuickView

class CustomIncubationController(QObject, QQmlIncubationController):
    def __init__(self, test):
        QObject.__init__(self)
        QQmlIncubationController.__init__(self)
        self.test = test
        self.interrupted = False

        # Incubate every 50 milliseconds
        self.startTimer(50)
        self.incubationShouldContinue = VolatileBool(True)
        self.test.assertEqual(self.incubationShouldContinue.get(), True)

    @Slot()
    def interrupter(self):
        if not self.interrupted:
            self.interrupted = True
            self.incubationShouldContinue.set(False)
            self.test.assertEqual(self.incubationShouldContinue.get(), False)
            QTimer.singleShot(0, QGuiApplication.instance().quit)

    def timerEvent(self, ev):
        # Incubate items for 2000 milliseconds, or until the volatile bool is set to false.
        self.incubateWhile(self.incubationShouldContinue, 2000)

class TestBug(unittest.TestCase):
    def testIncubateWhileCall(self):
        app = QGuiApplication(sys.argv)
        view = QQuickView()
        controller = CustomIncubationController(self)
        view.engine().setIncubationController(controller)
        view.setResizeMode(QQuickView.SizeRootObjectToView)
        view.setSource(QUrl.fromLocalFile(adjust_filename('qqmlincubator_incubateWhile.qml',
                       __file__)))
        view.show()

        root = view.rootObject()
        # The QML code will issue an interrupt signal after half of its items are loaded.
        root.shouldInterrupt.connect(controller.interrupter)
        res = app.exec_()

        itemsToCreate = root.property("itemsToCreate")
        loadedItems = root.property("loadedItems")
        self.assertEqual(loadedItems, itemsToCreate / 2)

        # Finish incubating the remaining items.
        controller.incubateFor(1000)
        loadedItems = root.property("loadedItems")
        self.assertEqual(loadedItems, itemsToCreate)

        # Deleting the view before it goes out of scope is required to make sure all child QML
        # instances are destroyed in the correct order.
        del view
        del app

if __name__ == '__main__':
    unittest.main()
