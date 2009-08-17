#!/usr/bin/python

'''Test cases for libsample bindings module'''

import sys
import unittest

import sample

class ModuleTest(unittest.TestCase):
    '''Test case for module and global functions'''

    def testModuleMembers(self):
        '''Test availability of classes, global functions and other members on binding'''
        expected_members = set(['Abstract', 'Derived', 'ListUser', 'PairUser',
                                'Point', 'gimmeComplexList', 'gimmeDouble',
                                'gimmeInt', 'makeCString', 'multiplyPair',
                                'returnCString', 'transmuteComplexIntoPoint',
                                'transmutePointIntoComplex', 'sumComplexPair',
                                'SampleNamespace', 'GlobalEnum', 'NoThing',
                                'FirstThing', 'SecondThing', 'ThirdThing'])
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

