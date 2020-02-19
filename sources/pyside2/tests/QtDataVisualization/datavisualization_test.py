#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

'''Test cases for QtDataVisualization'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.usesqguiapplication import UsesQGuiApplication
from PySide2.QtCore import QTimer
from PySide2.QtDataVisualization import QtDataVisualization

def dataToBarDataRow(data):
    result = []
    for d in data:
        result.append(QtDataVisualization.QBarDataItem(d))
    return result

def dataToBarDataArray(data):
    result = []
    for row in data:
        result.append(dataToBarDataRow(row))
    return result

class QtDataVisualizationTestCase(UsesQGuiApplication):
    '''Tests related to QtDataVisualization'''

    def testBars(self):
        self.bars = QtDataVisualization.Q3DBars()
        self.columnAxis = QtDataVisualization.QCategory3DAxis()
        self.columnAxis.setTitle('Columns')
        self.columnAxis.setTitleVisible(True)
        self.columnAxis.setLabels(['Column1', 'Column2'])

        self.rowAxis = QtDataVisualization.QCategory3DAxis()
        self.rowAxis.setTitle('Rows')
        self.rowAxis.setTitleVisible(True)
        self.rowAxis.setLabels(['Row1', 'Row2'])

        self.valueAxis = QtDataVisualization.QValue3DAxis()
        self.valueAxis.setTitle('Values')
        self.valueAxis.setTitleVisible(True)
        self.valueAxis.setRange(0, 5);

        self.bars.setRowAxis(self.rowAxis)
        self.bars.setColumnAxis(self.columnAxis)
        self.bars.setValueAxis(self.valueAxis)

        self.series = QtDataVisualization.QBar3DSeries()
        self.arrayData = [[1, 2], [3, 4]]
        self.series.dataProxy().addRows(dataToBarDataArray(self.arrayData))

        self.bars.setPrimarySeries(self.series)

        self.bars.show()
        QTimer.singleShot(500, self.app.quit)
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
