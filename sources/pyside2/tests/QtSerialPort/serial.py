#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

'''Test cases for QSerialPort'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide2.QtCore import QIODevice

class QSerialPortTest(unittest.TestCase):
    def testDefaultConstructedPort(self):
        serialPort = QSerialPort()

        self.assertEqual(serialPort.error(), QSerialPort.NoError)
        self.assertTrue(not serialPort.errorString() == "")

        # properties
        defaultBaudRate = QSerialPort.Baud9600
        self.assertEqual(serialPort.baudRate(), defaultBaudRate)
        self.assertEqual(serialPort.baudRate(QSerialPort.Input), defaultBaudRate)
        self.assertEqual(serialPort.baudRate(QSerialPort.Output), defaultBaudRate)
        self.assertEqual(serialPort.dataBits(), QSerialPort.Data8)
        self.assertEqual(serialPort.parity(), QSerialPort.NoParity)
        self.assertEqual(serialPort.stopBits(), QSerialPort.OneStop)
        self.assertEqual(serialPort.flowControl(), QSerialPort.NoFlowControl)

        self.assertEqual(serialPort.pinoutSignals(), QSerialPort.NoSignal)
        self.assertEqual(serialPort.isRequestToSend(), False)
        self.assertEqual(serialPort.isDataTerminalReady(), False)

        # QIODevice
        self.assertEqual(serialPort.openMode(), QIODevice.NotOpen)
        self.assertTrue(not serialPort.isOpen())
        self.assertTrue(not serialPort.isReadable())
        self.assertTrue(not serialPort.isWritable())
        self.assertTrue(serialPort.isSequential())
        self.assertEqual(serialPort.canReadLine(), False)
        self.assertEqual(serialPort.pos(), 0)
        self.assertEqual(serialPort.size(), 0)
        self.assertTrue(serialPort.atEnd())
        self.assertEqual(serialPort.bytesAvailable(), 0)
        self.assertEqual(serialPort.bytesToWrite(), 0)

    def testOpenExisting(self):
        allportinfos = QSerialPortInfo.availablePorts()
        for portinfo in allportinfos:
            serialPort = QSerialPort(portinfo)
            self.assertEqual(serialPort.portName(), portinfo.portName())


class QSerialPortInfoTest(unittest.TestCase):
    def test_available_ports(self):
        allportinfos = QSerialPortInfo.availablePorts()
        for portinfo in allportinfos:
            portinfo.description()
            portinfo.hasProductIdentifier()
            portinfo.hasVendorIdentifier()
            portinfo.isNull()

if __name__ == '__main__':
    unittest.main()
