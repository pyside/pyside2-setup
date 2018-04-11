#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

'''Unit tests for QTextStream'''

import unittest
import py3kcompat as py3k

from PySide2.QtCore import QByteArray, QTextStream, QIODevice, QTextCodec, QFile

class QTextStreamShiftTest(unittest.TestCase):

    def setUp(self):
        self.ba = QByteArray()
        self.read = QTextStream(self.ba, QIODevice.ReadOnly)
        self.write = QTextStream(self.ba, QIODevice.WriteOnly)

    def testNumber(self):
        '''QTextStream << number'''

        self.write << '4'
        self.write.flush()
        res = self.read.readLine()
        self.assertTrue(isinstance(res, py3k.unicode))
        self.assertEqual(res, '4')

class QTextStreamGetSet(unittest.TestCase):

    def setUp(self):
        self.obj = QTextStream()


    def testCodec(self):
        '''QTextStream set/get Codec'''

        codec = QTextCodec.codecForName('ISO8859-1')
        self.obj.setCodec(codec)
        self.assertEqual(codec, self.obj.codec())

    def testDevice(self):
        '''QTextStream get/set Device'''
        device = QFile()
        self.obj.setDevice(device)
        self.assertEqual(device, self.obj.device())
        self.obj.setDevice(None)
        self.assertEqual(None, self.obj.device())

class QTextStreamInitialization(unittest.TestCase):

    def testConstruction(self):
        '''QTextStream construction'''
        obj = QTextStream()

        self.assertEqual(obj.codec(), QTextCodec.codecForLocale())
        self.assertEqual(obj.device(), None)
        self.assertEqual(obj.string(), None)

        self.assertTrue(obj.atEnd())
        self.assertEqual(obj.readAll(), '')

class QTextStreamReadLinesFromDevice(unittest.TestCase):

    def _check_data(self, data_set):

        for data, lines in data_set:
            stream = QTextStream(data)

            res = []
            while not stream.atEnd():
                res.append(stream.readLine())

            self.assertEqual(res, lines)

    def testLatin1(self):
        '''QTextStream readLine for simple Latin1 strings'''

        data = []

        data.append((QByteArray(), []))
        data.append((QByteArray('\n'), ['']))
        data.append((QByteArray('\r\n'), ['']))
        data.append((QByteArray('ole'), ['ole']))
        data.append((QByteArray('ole\n'), ['ole']))
        data.append((QByteArray('ole\r\n'), ['ole']))
        data.append((QByteArray('ole\r\ndole\r\ndoffen'), ['ole', 'dole', 'doffen']))

        self._check_data(data)

if __name__ == '__main__':
    unittest.main()
