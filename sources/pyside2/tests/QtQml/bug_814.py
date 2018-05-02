#!/usr/bin/python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Copyright (C) 2011 Thomas Perl <m@thp.io>
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

# Test case for PySide bug 814
# http://bugs.pyside.org/show_bug.cgi?id=814
# archive:
# https://srinikom.github.io/pyside-bz-archive/814.html
# 2011-04-08 Thomas Perl <m@thp.io>
# Released under the same terms as PySide itself

import unittest

from helper import adjust_filename, TimedQApplication

from PySide2.QtCore import QUrl, QAbstractListModel, QModelIndex, Qt
from PySide2.QtQuick import QQuickView

class ListModel(QAbstractListModel):
    def __init__(self):
        QAbstractListModel.__init__(self)

    def roleNames(self):
        return { Qt.DisplayRole: 'pysideModelData' }

    def rowCount(self, parent = QModelIndex()):
        return 3

    def data(self, index, role):
        if index.isValid() and role == Qt.DisplayRole:
            return 'blubb'
        return None

class TestBug814(TimedQApplication):
    def testAbstractItemModelTransferToQML(self):
        view = QQuickView()
        model = ListModel()
        view.rootContext().setContextProperty("pythonModel", model)
        view.setSource(QUrl.fromLocalFile(adjust_filename('bug_814.qml', __file__)))
        root = view.rootObject()
        view.show()

if __name__ == '__main__':
    unittest.main()

