#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

import unittest
from PySide2 import QtCore, QtWebKit

from helper import UsesQApplication

class QWebPageHeadless(QtWebKit.QWebPage):
    # FIXME: This is not working, the slot is not overriden!
    # http://doc.qt.nokia.com/4.7-snapshot/qwebpage.html#shouldInterruptJavaScript
    @QtCore.Slot()
    def shouldInterruptJavaScript(self):
        self._interrupted = True
        QtCore.QTimer.singleShot(300, self._app.quit)
        return True

class TestSlotOverride(UsesQApplication):
    def testFunctionCall(self):
        page = QWebPageHeadless()
        page._interrupted = False
        page._app = self.app
        page.mainFrame().setHtml('<script>while(1);</script>')
        self.app.exec_()
        self.assertTrue(page._interrupted)

if __name__ == '__main__':
    unittest.main()
