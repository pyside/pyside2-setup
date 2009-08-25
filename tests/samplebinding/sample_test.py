#!/usr/bin/python

'''Test cases for libsample bindings module'''

import sys
import unittest

import sample

class ModuleTest(unittest.TestCase):
    '''Test case for module and global functions'''

    def testModuleMembers(self):
        '''Test availability of classes, global functions and other members on binding'''
        expected_members = set(['Abstract', 'Derived', 'Point',
                                'ListUser', 'PairUser', 'MapUser',
                                'gimmeComplexList', 'gimmeDouble', 'gimmeInt',
                                'makeCString', 'multiplyPair', 'returnCString',
                                'SampleNamespace', 'transmuteComplexIntoPoint',
                                'transmutePointIntoComplex', 'sumComplexPair',
                                'FirstThing', 'SecondThing', 'ThirdThing',
                                'GlobalEnum', 'NoThing'])
        self.assert_(expected_members.issubset(dir(sample)))

    def testAbstractPrintFormatEnum(self):
        '''Test availability of PrintFormat enum from Abstract class'''
        enum_members = set(['PrintFormat', 'Short', 'Verbose',
                            'OnlyId', 'ClassNameAndId'])
        self.assert_(enum_members.issubset(dir(sample.Abstract)))

    def testSampleNamespaceOptionEnum(self):
        '''Test availability of Option enum from SampleNamespace namespace'''
        enum_members = set(['Option', 'None', 'RandomNumber', 'UnixTime'])
        self.assert_(enum_members.issubset(dir(sample.SampleNamespace)))

if __name__ == '__main__':
    unittest.main()

