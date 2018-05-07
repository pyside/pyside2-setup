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

import sys
import unittest

from PySide2.QtCore import QObject, QCoreApplication, ClassInfo

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
        valid_dict = { '123': '456' }

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

    def test_can_only_be_used_on_qobjects(self):
        def test_function(): pass
        self.assertRaises(TypeError, ClassInfo(), test_function)

        class NotAQObject(object): pass
        self.assertRaises(TypeError, ClassInfo(), NotAQObject)

        class QObjectSubclass(QObject): pass
        ClassInfo()(QObjectSubclass)

        class SubclassOfNativeQObjectSubclass(QCoreApplication): pass
        ClassInfo()(SubclassOfNativeQObjectSubclass)

        class SubclassOfPythonQObjectSubclass(QObjectSubclass): pass
        ClassInfo()(SubclassOfPythonQObjectSubclass)

if __name__ == '__main__':
    if sys.version_info[0] < 2:
        sys.exit(0)
    elif (sys.version_info[0] == 2) and (sys.version_info[1] <= 5):
        sys.exit(0)
    else:
        unittest.main()
