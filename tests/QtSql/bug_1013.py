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

from PySide2.QtCore import *
from PySide2.QtSql import *
import unittest

class TestBug1013 (unittest.TestCase):

    def someSlot(self, row, record):
        record.setValue(0, 2)
        self._wasCalled = True

    def testIt(self):
        app = QCoreApplication([])
        db = QSqlDatabase.addDatabase('QSQLITE')
        db.setDatabaseName(':memory:')
        db.open()
        query = QSqlQuery()
        query.exec_('CREATE TABLE "foo" (id INT);')
        model = QSqlTableModel()
        model.setTable('foo')

        self._wasCalled = False
        model.primeInsert.connect(self.someSlot)
        model.select()
        QTimer.singleShot(0,lambda: model.insertRow(0) and app.quit())
        app.exec_()
        self.assertTrue(self._wasCalled)
        self.assertEqual(model.data(model.index(0, 0)), 2)

if __name__ == "__main__":
    unittest.main()
