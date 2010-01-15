#!/usr/bin/env python

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


class TestLockUnlock(unittest.TestCase):

    def testBasic(self):
        '''Locking in C++ and releasing in a python thread'''

        bucket = Bucket()
        unlocker = Unlocker(bucket)

        unlocker.start()
        bucket.lock()
        unlocker.join()


if __name__ == '__main__':
    unittest.main()
