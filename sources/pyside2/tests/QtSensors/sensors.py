#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

'''Test cases for QSensor'''

from PySide2.QtSensors import QSensor, QSensorReading
import unittest

class QSensorTest(unittest.TestCase):
    def test(self):
        for sensorType in QSensor.sensorTypes():
            identifiers = QSensor.sensorsForType(sensorType)
            values = []
            usedIdentifier = None
            for identifier in identifiers:
                sensor = QSensor(sensorType, None);
                sensor.setIdentifier(identifier)
                if sensor.connectToBackend():
                    usedIdentifier = identifier
                    reading = sensor.reading()
                    for i in range(0, reading.valueCount()):
                        values.append(reading.value(i))
                    break
            if usedIdentifier:
                print('Sensor ', sensorType, usedIdentifier, values)

if __name__ == '__main__':
    unittest.main()
