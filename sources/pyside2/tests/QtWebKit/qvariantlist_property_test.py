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

from PySide2.QtCore import Property, QObject
from PySide2.QtWebKit import QWebView
from helper import TimedQApplication

class TestLoadFinished(TimedQApplication):

    def setUp(self):
        TimedQApplication.setUp(self, timeout=1000)

    def tearDown(self):
        TimedQApplication.tearDown(self)

    def testQVariantListProperty(self):
        class Obj(object):
            list = ['foo', 'bar', 'baz']

        obj = Obj()

        wrapper_dict = {}
        for name in ['list']:
            getter = lambda arg=None, name=name: getattr(obj, name)
            wrapper_dict[name] = Property('QVariantList', getter)
        wrapper = type('PyObj', (QObject,), wrapper_dict)

        view = QWebView()
        frame = view.page().mainFrame()
        frame.addToJavaScriptWindowObject('py_obj', wrapper())

        html = '''
        <html><body>
        <script type="text/javascript">
        document.write(py_obj.list)
        </script>
        </body></html>
        '''
        view.setHtml(html)
        view.show()
        self.app.exec_()


if __name__ == '__main__':
    unittest.main()
