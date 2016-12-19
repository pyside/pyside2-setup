#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

'''Test cases for connecting signals between threads'''

import unittest

from PySide2.QtCore import QThread, QObject, SIGNAL, QCoreApplication

thread_run = False

class Source(QObject):
    def __init__(self, *args):
        QObject.__init__(self, *args)

    def emit_sig(self):
        self.emit(SIGNAL('source()'))

class Target(QObject):
    def __init__(self, *args):
        QObject.__init__(self, *args)
        self.called = False

    def myslot(self):
        self.called = True

class ThreadJustConnects(QThread):
    def __init__(self, source, *args):
        QThread.__init__(self, *args)
        self.source = source
        self.target = Target()

    def run(self):
        global thread_run
        thread_run = True
        QObject.connect(self.source, SIGNAL('source()'), self.target.myslot)

        while not self.target.called:
            pass



class BasicConnection(unittest.TestCase):

    def testEmitOutsideThread(self):
        global thread_run

        app = QCoreApplication([])
        source = Source()
        thread = ThreadJustConnects(source)

        QObject.connect(thread, SIGNAL('finished()'), lambda: app.exit(0))
        thread.start()

        while not thread_run:
            pass

        source.emit_sig()

        app.exec_()
        thread.wait()

        self.assertTrue(thread.target.called)

if __name__ == '__main__':
    unittest.main()
