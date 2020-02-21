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

'''Test cases for QCharts'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.usesqapplication import UsesQApplication
from PySide2.QtCore import QRect, QSize, QTimer
from PySide2.QtGui import QGuiApplication, QScreen
from PySide2.QtCharts import QtCharts

class QChartsTestCase(UsesQApplication):
    '''Tests related to QCharts'''

    def testCharts(self):
        self.series = QtCharts.QPieSeries()
        self.series.append("Jane", 1);
        self.series.append("Joe", 2);
        self.series.append("Andy", 3);
        self.series.append("Barbara", 4);
        self.series.append("Axel", 5);
        slice = self.series.slices()[1]
        slice.setExploded();
        slice.setLabelVisible();
        self.chart = QtCharts.QChart()
        self.chart.addSeries(self.series);
        chartView = QtCharts.QChartView(self.chart)
        screenSize = QGuiApplication.primaryScreen().geometry().size()
        chartView.resize(screenSize / 2)
        chartView.show()
        QTimer.singleShot(500, self.app.quit)
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
