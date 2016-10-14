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

'''Test cases for QVariant::Type converter'''
import unittest
import py3kcompat as py3k
from PySide2.QtSql import QSqlField

class QVariantTypeTest(unittest.TestCase):
    def testQVariantType(self):
        f = QSqlField("name", str)
        self.assertEqual(f.type(), py3k.unicode)

        f = QSqlField("name", "QString")
        self.assertEqual(f.type(),  py3k.unicode)

        f = QSqlField("name", "double")
        self.assertEqual(f.type(), float)

        f = QSqlField("name", float)
        self.assertEqual(f.type(), float)

        f = QSqlField("name", int)
        self.assertEqual(f.type(), int)

        if not py3k.IS_PY3K:
            f = QSqlField("name", long)
            self.assertEqual(f.type(), int) # long isn't registered in QVariant:Type, just in QMetaType::Type

        #f = QSqlField("name", QObject)
        #self.assertEqual(f.type(), None)

        f = QSqlField("name", None)
        self.assertEqual(f.type(), None)

if __name__ == '__main__':
    unittest.main()
