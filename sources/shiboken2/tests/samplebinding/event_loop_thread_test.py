#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

import time
import threading
import unittest
from random import random

from sample import ObjectType, Event


class Producer(ObjectType):

    def __init__(self):
        ObjectType.__init__(self)
        self.data = None
        self.read = False

    def event(self, event):
        self.data = random()

        while not self.read:
            time.sleep(0.01)

        return True


class Collector(threading.Thread):

    def __init__(self, objects):
        threading.Thread.__init__(self)
        self.max_runs = len(objects)
        self.objects = objects
        self.data = []

    def run(self):
        i = 0
        while i < self.max_runs:
            if self.objects[i].data is not None:
                self.data.append(self.objects[i].data)
                self.objects[i].read = True
                i += 1
            time.sleep(0.01)


class TestEventLoopWithThread(unittest.TestCase):
    '''Communication between a python thread and an simple
    event loop in C++'''

    def testBasic(self):
        '''Allowing threads and calling virtuals from C++'''
        number = 10
        objs = [Producer() for x in range(number)]
        thread = Collector(objs)

        thread.start()

        evaluated = ObjectType.processEvent(objs,
                                        Event(Event.BASIC_EVENT))

        thread.join()

        producer_data = [x.data for x in objs]
        self.assertEqual(evaluated, number)
        self.assertEqual(producer_data, thread.data)


if __name__ == '__main__':
    unittest.main()
