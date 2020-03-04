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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import Property, QObject
from PySide2.QtWebKit import QWebView
from helper.timedqapplication import TimedQApplication

class TestLambdaPropery(TimedQApplication):

    def testBug899(self):
        html = '''
        <html><body>
        <script type="text/javascript">
        document.write("<p>"+py_obj.list1+"</p>")
        document.write("<p>"+py_obj.list2+"</p>")
        </script>
        </body></html>
        '''

        class Obj(object):
            list1 = ['foo', 'bar', 'baz']
            list2 = ['fi', 'fo', 'fum']

        obj = Obj()

        wrapper_dict = {}
        for name in ('list1', 'list2'):
            getter = lambda arg=None, name=name: getattr(obj, name)
            wrapper_dict[name] = Property('QVariantList', getter)
        wrapper = type('PyObj', (QObject,), wrapper_dict)

        view = QWebView()
        view.page().mainFrame().addToJavaScriptWindowObject('py_obj', wrapper())
        view.setHtml(html)


if __name__ == '__main__':
    unittest.main()
