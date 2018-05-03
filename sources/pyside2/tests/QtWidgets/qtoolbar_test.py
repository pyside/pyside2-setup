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

'''Test cases for QToolbar'''

import unittest

from PySide2.QtWidgets import QToolBar, QMainWindow, QAction

from helper import UsesQApplication

class AddActionText(UsesQApplication):
    '''Test case for calling QToolbar.addAction passing a text'''

    def setUp(self):
        #Acquire resources
        super(AddActionText, self).setUp()
        self.window = QMainWindow()
        self.toolbar = QToolBar()
        self.window.addToolBar(self.toolbar)

    def tearDown(self):
        #Release resources
        super(AddActionText, self).tearDown()
        del self.toolbar
        del self.window

    def testText(self):
        #QToolBar.addAction(text) - add a QToolButton
        self.toolbar.addAction('aaaa')
        self.assertEqual(len(self.toolbar.actions()), 1)
        action = self.toolbar.actions()[0]
        self.assertTrue(isinstance(action, QAction))
        self.assertEqual(action.text(), 'aaaa')

if __name__ == '__main__':
    unittest.main()
