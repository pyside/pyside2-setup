
'''Test cases for virtual methods called through generated bindings'''

import unittest
try:
    from foo import Foo, Bar
except:
    import sys
    print 'You need to set correct paths for libfoo and foo bindings'
    import os
    sys.exit(1)


class DerivedFoo(Foo):

    def __init__(self):
        Foo.__init__(self)

    def pureVirtual(self):
        print 'DerivedFoo.pureVirtual'


class VirtualMethods(unittest.TestCase):
    '''Test case for virtual methods'''

    def setUp(self):
        self.foo = Foo()
        self.bar = Bar()
        self.derivedfoo = DerivedFoo()

    def tearDown(self):
        self.foo = None
        self.bar = None
        self.derivedfoo = None

    def testDerivedClassVirtualMethod(self):
        '''Test reinplemented virtual methods from derived class'''
        called = True
        try:
            self.bar.unpureVirtual()
            self.bar.pureVirtual()
        except:
            called = False
        self.assertTrue(called)

    def testBaseClassVirtualMethod(self):
        '''Test virtual method from base class'''
        called = True
        try:
            self.foo.unpureVirtual()
        except:
            called = False
        self.assertTrue(called)


    def testBaseClassPureVirtualMethod(self):
        '''Test pure virtual method from base class'''
        called = False
        try:
            self.foo.pureVirtual()
        except:
            called = False
        self.assertFalse(called)

    def testBaseClassIndirectCallToUnpureVirtualMethod(self):
        '''Test call to unpure virtual method from C++ to Python'''
        called = True
        try:
            self.foo.unpureVirtual()
        except:
            called = False
        self.assertTrue(called)

    def testDerivedClassIndirectCallToUnpureVirtualMethod(self):
        '''Test call to unpure virtual method from C++ to Python'''
        called = True
        try:
            self.bar.unpureVirtual()
        except:
            called = False
        self.assertTrue(called)

    def testCppDerivedClassIndirectCallToPureVirtualMethod(self):
        '''Test call to pure virtual method from C++ to Python'''
        called = False
        try:
            self.bar.callPureVirtual()
        except:
            called = False
        self.assertFalse(called)


    def testDerivedClassIndirectCallToPureVirtualMethod(self):
        '''Test call to pure virtual method from C++ to Python'''
        called = False
        try:
            self.derivedfoo.callPureVirtual()
        except:
            called = False
        self.assertFalse(called)


if __name__ == '__main__':
    unittest.main()

