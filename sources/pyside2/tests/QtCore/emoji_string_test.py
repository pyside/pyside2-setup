# This Python file uses the following encoding: utf-8

#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""
emoji-string-test.py

This is the original code from the bug report
https://bugreports.qt.io/browse/PYSIDE-336

The only changes are the emoji constant creation which avoids unicode in the
source itself, utf8 encoding in line 1 and a short plausibility test to make
it safely fail.
"""

import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths()

from PySide2 import QtCore

emoji_str = u'\U0001f632' + u' '  # "😲 "

class TestStuff(QtCore.QObject):
    testsig = QtCore.Signal(str)

    def a_nop(self, sendMeAnEmoji):
        print(sendMeAnEmoji)
        return

    def __init__(self):
        super(TestStuff, self).__init__()
        self.testsig.connect(self.a_nop)
        self.testsig.emit(emoji_str)

    def plausi(self):
        # Python 2 may be built with UCS-2 or UCS-4 support.
        # UCS-2 creates 2 surrogate code points. See
        # https://stackoverflow.com/questions/30775689/python-length-of-unicode-string-confusion
        assert len(emoji_str) == 2 if sys.maxunicode > 0xffff else 3

if __name__ == "__main__":
    mything = TestStuff()
    mything.plausi()
