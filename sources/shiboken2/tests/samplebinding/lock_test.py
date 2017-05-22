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

'''Simple test with a blocking C++ method that should allow python
   threads to run.'''

import unittest
import threading

from sample import Bucket


class Unlocker(threading.Thread):

    def __init__(self, bucket):
        threading.Thread.__init__(self)
        self.bucket = bucket

    def run(self):
        while not self.bucket.locked():
            pass

        self.bucket.unlock()


class MyBucket(Bucket):

    def __init__(self):
        Bucket.__init__(self)

    def virtualBlockerMethod(self):
        self.lock()
        return True


class TestLockUnlock(unittest.TestCase):

    def testBasic(self):
        '''Locking in C++ and releasing in a python thread'''
        bucket = Bucket()
        unlocker = Unlocker(bucket)

        unlocker.start()
        bucket.lock()
        unlocker.join()

    def testVirtualBlocker(self):
        '''Same as the basic case but blocker method is a C++ virtual called from C++.'''
        bucket = Bucket()
        unlocker = Unlocker(bucket)

        unlocker.start()
        result = bucket.callVirtualBlockerMethodButYouDontKnowThis()
        unlocker.join()
        self.assertTrue(result)

    def testReimplementedVirtualBlocker(self):
        '''Same as the basic case but blocker method is a C++ virtual reimplemented in Python and called from C++.'''
        mybucket = MyBucket()
        unlocker = Unlocker(mybucket)

        unlocker.start()
        result = mybucket.callVirtualBlockerMethodButYouDontKnowThis()
        unlocker.join()
        self.assertTrue(result)

if __name__ == '__main__':
    unittest.main()
