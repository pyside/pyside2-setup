#!/usr/bin/python

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

'''Test cases for QtSql database creation, destruction and queries'''

import sys
import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import QtSql
from PySide6.QtCore import *
from PySide6.QtWidgets import *

class Foo(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.model = QtSql.QSqlTableModel()

class SqlDatabaseCreationDestructionAndQueries(unittest.TestCase):
    '''Test cases for QtSql database creation, destruction and queries'''

    def setUp(self):
        #Acquire resources
        self.assertFalse(not QtSql.QSqlDatabase.drivers(), "installed Qt has no DB drivers")
        self.assertTrue("QSQLITE" in QtSql.QSqlDatabase.drivers(), "\"QSQLITE\" driver not available in this Qt version")
        self.db = QtSql.QSqlDatabase.addDatabase("QSQLITE")
        self.db.setDatabaseName(":memory:")
        self.assertTrue(self.db.open())

    def tearDown(self):
        #Release resources
        self.db.close()
        QtSql.QSqlDatabase.removeDatabase(":memory:")
        del self.db

    def testTableCreationAndDestruction(self):
        #Test table creation and destruction
        query = QtSql.QSqlQuery()
        query.exec_("CREATE TABLE dummy(id int primary key, dummyfield varchar(20))")
        query.exec_("DROP TABLE dummy")
        query.clear()

    def testTableInsertionAndRetrieval(self):
        #Test table creation, insertion and retrieval
        query = QtSql.QSqlQuery()
        query.exec_("CREATE TABLE person(id int primary key, "
                    "firstname varchar(20), lastname varchar(20))")
        query.exec_("INSERT INTO person VALUES(101, 'George', 'Harrison')")
        query.prepare("INSERT INTO person (id, firstname, lastname) "
                      "VALUES (:id, :firstname, :lastname)")
        query.bindValue(":id", 102)
        query.bindValue(":firstname", "John")
        query.bindValue(":lastname", "Lennon")
        query.exec_()

        lastname = ''
        query.exec_("SELECT lastname FROM person where id=101")
        self.assertTrue(query.isActive())
        query.next()
        lastname = query.value(0)
        self.assertEqual(lastname, 'Harrison')

    def testTableModelDeletion(self):
        app = QApplication([])

        bar = Foo()
        model = bar.model
        del bar
        del app

if __name__ == '__main__':
    unittest.main()

