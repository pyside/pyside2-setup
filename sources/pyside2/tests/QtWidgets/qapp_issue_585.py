#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

"""
The bug was caused by this commit:
"Support the qApp macro correctly, final version incl. debug"
e30e0c161b2b4d50484314bf006e9e5e8ff6b380
2017-10-27

The bug was first solved by this commit:
"Fix qApp macro refcount"
b811c874dedd14fd8b072bc73761d39255216073
2018-03-21

This test triggers the refcounting bug of qApp, issue PYSIDE-585.
Finally, the real patch included more changes, because another error
was in the ordering of shutdown calls. It was found using the following
Python configuration:

    In Python 3.6 create a directory 'debug' and cd into it.

    ../configure --with-pydebug --prefix=$HOME/pydebug/ --enable-shared

Then a lot more refcounting errors show up, which are due to a bug in
the code position of the shutdown procedure.
The reason for the initial refcount bug was that the shutdown code is once
more often called than the creation of the qApp wrapper.
Finally, it was easiest and more intuitive to simply make the refcount of
qApp_content equal to that of Py_None, which is also not supposed to be
garbage-collected.

For some reason, the test does not work as a unittest because it creates
no crash. We leave it this way.
"""

from PySide2.QtCore import QTimer
from PySide2 import QtWidgets

app_instance = QtWidgets.QApplication([])
# If the following line is commented, application doesn't crash on exit anymore.
app_instance2 = app_instance
QTimer.singleShot(0, qApp.quit)
app_instance.exec_()
