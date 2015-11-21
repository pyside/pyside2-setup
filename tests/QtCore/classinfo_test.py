import sys
import unittest

from PySide2.QtCore import QObject, ClassInfo

class TestClassInfo(unittest.TestCase):
    def test_metadata(self):
        @ClassInfo(author='pyside', url='http://www.pyside.org')
        class MyObject(QObject):
            pass

        o = MyObject()
        mo = o.metaObject()
        self.assertEqual(mo.classInfoCount(), 2)

        ci = mo.classInfo(0) #author
        self.assertEqual(ci.name(), 'author')
        self.assertEqual(ci.value(), 'pyside')

        ci = mo.classInfo(1) #url
        self.assertEqual(ci.name(), 'url')
        self.assertEqual(ci.value(), 'http://www.pyside.org')

    def test_verify_metadata_types(self):
        valid_dict = { '123':  '456' }

        invalid_dict_1 = { '123': 456 }
        invalid_dict_2 = {  123:  456 }
        invalid_dict_3 = {  123: '456' }

        ClassInfo(**valid_dict)

        self.assertRaises(TypeError, ClassInfo, **invalid_dict_1)

        # assertRaises only allows for string keywords, so a `try` must be used here.
        try:
            ClassInfo(**invalid_dict_2)
            self.fail('ClassInfo() accepted invalid_dict_2!')
        except TypeError:
            pass

        try:
            ClassInfo(**invalid_dict_3)
            self.fail('ClassInfo() accepted invalid_dict_3!')
        except TypeError:
            pass

    def test_can_not_use_instance_twice(self):
        decorator = ClassInfo(author='pyside', url='http://www.pyside.org')

        @decorator
        class MyObject1(QObject):
            pass

        class MyObject2(QObject):
            pass

        self.assertRaises(TypeError, decorator, MyObject2)


if __name__ == '__main__':
    if sys.version_info[0] < 2:
        sys.exit(0)
    elif (sys.version_info[0] == 2) and (sys.version_info[1] <= 5):
        sys.exit(0)
    else:
        unittest.main()
