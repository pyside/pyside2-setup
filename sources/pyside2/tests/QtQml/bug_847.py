#!/usr/bin/python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Copyright (C) 2011 Thomas Perl <m@thp.io>
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

# Testcase for PySide bug 847
# Released under the same terms as PySide itself
# 2011-05-04 Thomas Perl <m@thp.io>

from __future__ import print_function

import unittest

from helper import adjust_filename, UsesQApplication

from PySide2.QtCore import Slot, Signal, QUrl, QTimer, QCoreApplication
from PySide2.QtQuick import QQuickView

class View(QQuickView):
    def __init__(self):
        QQuickView.__init__(self)

    called = Signal(int, int)

    @Slot(int, int)
    def blubb(self, x, y):
        self.called.emit(x, y)

class TestQML(UsesQApplication):
    def done(self, x, y):
        self._sucess = True
        self.app.quit()
        print("done called")

    def testPythonSlot(self):
        self._sucess = False
        view = View()

        # Connect first, then set the property.
        view.called.connect(self.done)
        view.setSource(QUrl.fromLocalFile(adjust_filename('bug_847.qml', __file__)))
        view.rootObject().setProperty('pythonObject', view)

        view.show()
        # Essentially a timeout in case method invocation fails.
        QTimer.singleShot(2000, QCoreApplication.instance().quit)
        self.app.exec_()
        self.assertTrue(self._sucess)

if __name__ == '__main__':
    unittest.main()

