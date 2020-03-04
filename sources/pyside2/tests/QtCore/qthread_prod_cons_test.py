#!/usr/bin/python

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

'''Producer-Consumer test/example with QThread'''

import logging
import os
from random import random
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

logging.basicConfig(level=logging.WARNING)

from PySide2.QtCore import QThread, QCoreApplication, QObject, SIGNAL

class Bucket(QObject):
    '''Dummy class to hold the produced values'''
    def __init__(self, max_size=10, *args):
        #Constructor which receives the max number of produced items
        super(Bucket, self).__init__(*args)
        self.data = []
        self.max_size = 10

    def pop(self):
        #Retrieves an item
        return self.data.pop(0)

    def push(self, data):
        #Pushes an item
        self.data.append(data)

class Producer(QThread):
    '''Producer thread'''

    def __init__(self, bucket, *args):
        #Constructor. Receives the bucket
        super(Producer, self).__init__(*args)
        self.runs = 0
        self.bucket = bucket
        self.production_list = []

    def run(self):
        #Produces at most bucket.max_size items
        while self.runs < self.bucket.max_size:
            value = int(random()*10) % 10
            self.bucket.push(value)
            self.production_list.append(value)
            logging.debug('PRODUCER - pushed %d' % value)
            self.runs += 1
            self.msleep(5)



class Consumer(QThread):
    '''Consumer thread'''
    def __init__(self, bucket, *args):
        #Constructor. Receives the bucket
        super(Consumer, self).__init__(*args)
        self.runs = 0
        self.bucket = bucket
        self.consumption_list = []

    def run(self):
        #Consumes at most bucket.max_size items
        while self.runs < self.bucket.max_size:
            try:
                value = self.bucket.pop()
                self.consumption_list.append(value)
                logging.debug('CONSUMER - got %d' % value)
                self.runs += 1
            except IndexError:
                logging.debug('CONSUMER - empty bucket')
            self.msleep(5)

class ProducerConsumer(unittest.TestCase):
    '''Basic test case for producer-consumer QThread'''

    def setUp(self):
        #Create fixtures
        self.app = QCoreApplication([])

    def tearDown(self):
        #Destroy fixtures
        del self.app

    def finishCb(self):
        #Quits the application
        self.app.exit(0)

    def testProdCon(self):
        #QThread producer-consumer example
        bucket = Bucket()
        prod = Producer(bucket)
        cons = Consumer(bucket)

        prod.start()
        cons.start()

        QObject.connect(prod, SIGNAL('finished()'), self.finishCb)
        QObject.connect(cons, SIGNAL('finished()'), self.finishCb)

        self.app.exec_()

        prod.wait(50)
        cons.wait(50)

        self.assertEqual(prod.production_list, cons.consumption_list)


if __name__ == '__main__':
    unittest.main()
