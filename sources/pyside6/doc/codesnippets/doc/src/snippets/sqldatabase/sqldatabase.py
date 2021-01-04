############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the examples of Qt for Python.
##
## $QT_BEGIN_LICENSE:BSD$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## BSD License Usage
## Alternatively, you may use this file under the terms of the BSD license
## as follows:
##
## "Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##   * Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in
##     the documentation and/or other materials provided with the
##     distribution.
##   * Neither the name of The Qt Company Ltd nor the names of its
##     contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
##
## $QT_END_LICENSE$
##
############################################################################

from PySide6.QtGui import *
from PySide6.QtSql import *


def tr(text):
    return QApplication.translate(text, text)

def QSqlDatabase_snippets():
//! [0]
    db = QSqlDatabase.addDatabase("QPSQL")
    db.setHostName("acidalia")
    db.setDatabaseName("customdb")
    db.setUserName("mojito")
    db.setPassword("J0a1m8")
    ok = db.open()
//! [0]

//! [1]
    db = QSqlDatabase.database()
//! [1]

def QSqlField_snippets():
//! [2]
    field = QSqlField("age", QVariant.Int)
    field.setValue(QPixmap())  # WRONG
//! [2]

//! [3]
    field = QSqlField("age", QVariant.Int)
    field.setValue(str(123))  # casts str to int
//! [3]

//! [4]
    query = QSqlQuery()
//! [4] //! [5]
    record = query.record()
//! [5] //! [6]
    field = record.field("country")
//! [6]

def doSomething(str):
    pass

def QSqlQuery_snippets():
    # typical loop
//! [7]
    query = QSqlQuery("SELECT country FROM artist")
    while query.next():
        country = query.value(0)
        doSomething(country)
//! [7]


    # field index lookup
//! [8]
    query = QSqlQuery("SELECT * FROM artist")
    fieldNo = query.record().indexOf("country")
    while query.next():
        country = query.value(fieldNo)
        doSomething(country)
//! [8]

    # named with named
//! [9]
    query = QSqlQuery()
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (:id, :forename, :surname)")
    query.bindValue(":id", 1001)
    query.bindValue(":forename", "Bart")
    query.bindValue(":surname", "Simpson")
    query.exec_()
//! [9]

    # positional with named
//! [10]
    query = QSqlQuery()
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (:id, :forename, :surname)")
    query.bindValue(0, 1001)
    query.bindValue(1, "Bart")
    query.bindValue(2, "Simpson")
    query.exec_()
//! [10]

    # positional 1
//! [11]
    query = QSqlQuery()
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (?, ?, ?)")
    query.bindValue(0, 1001)
    query.bindValue(1, "Bart")
    query.bindValue(2, "Simpson")
    query.exec_()
//! [11]

    # positional 2
//! [12]
    query = QSqlQuery()
    query.prepare("INSERT INTO person (id, forename, surname) "
                  "VALUES (?, ?, ?)")
    query.addBindValue(1001)
    query.addBindValue("Bart")
    query.addBindValue("Simpson")
    query.exec_()
//! [12]

    # stored
//! [13]
    query = QSqlQuery()
    query.prepare("CALL AsciiToInt(?, ?)")
    query.bindValue(0, "A")
    query.bindValue(1, 0, QSql.Out)
    query.exec_()
    i = query.boundValue(1) # i is 65
//! [13]

    query = QSqlQuery()

    # examine with named binding
//! [14]
    i = query.boundValues()
    while i.hasNext():
        i.next()
        print(f"{i.key()}:{i.value()}")
//! [14]

    # examine with positional binding
//! [15]
    list_ = query.boundValues().values()
    for item in list:
        print(item)
//! [15]

def QSqlQueryModel_snippets():

//! [16]
    model =  QSqlQueryModel()
    model.setQuery("SELECT name, salary FROM employee")
    model.setHeaderData(0, Qt.Horizontal, tr("Name"))
    model.setHeaderData(1, Qt.Horizontal, tr("Salary"))

//! [17]
    view =  QTableView()
//! [17] //! [18]
    view.setModel(model)
//! [18] //! [19]
    view.show()
//! [16] //! [19] //! [20]
    view.setEditTriggers(QAbstractItemView.NoEditTriggers)
//! [20]

//! [21]
    model = QSqlQueryModel()
    model.setQuery("SELECT * FROM employee")
    salary = model.record(4).value("salary")
//! [21]

//! [22]
    salary = model.data(model.index(4, 2))
//! [22]

    for row in range(model.rowCount()):
        for (col in range(model.columnCount())):
            print(model.data(model.index(row, col)))


class MyModel(QSqlQueryModel)
    m_specialColumnNo = 0
    def data(item, role):
//! [23]
        if item.column() == self.m_specialColumnNo:
            # handle column separately
            pass

        return QSqlQueryModel.data(item, role)

//! [23]


def QSqlTableModel_snippets():

//! [24]
    model =  QSqlTableModel()
    model.setTable("employee")
    model.setEditStrategy(QSqlTableModel.OnManualSubmit)
    model.select()
    model.removeColumn(0) # don't show the ID
    model.setHeaderData(0, Qt.Horizontal, tr("Name"))
    model.setHeaderData(1, Qt.Horizontal, tr("Salary"))

    view =  QTableView()
    view.setModel(model)
    view.show()
//! [24]


//! [25]
    model = QSqlTableModel()
    model.setTable("employee")
    name = model.record(4).value("name")
//! [25]

def sql_intro_snippets():

//! [26]
    db = QSqlDatabase.addDatabase("QMYSQL")
    db.setHostName("bigblue")
    db.setDatabaseName("flightdb")
    db.setUserName("acarlson")
    db.setPassword("1uTbSbAs")
    ok = db.open()
//! [26]

//! [27]
    firstDB = QSqlDatabase.addDatabase("QMYSQL", "first")
    secondDB = QSqlDatabase.addDatabase("QMYSQL", "second")
//! [27]

//! [28]
    defaultDB = QSqlDatabase.database()
//! [28] //! [29]
    firstDB = QSqlDatabase.database("first")
//! [29] //! [30]
    secondDB = QSqlDatabase.database("second")
//! [30]

    # SELECT1
//! [31]
    query = QSqlQuery()
    query.exec_("SELECT name, salary FROM employee WHERE salary > 50000")
//! [31]

//! [32]
    while query.next():
        name = query.value(0)
        salary = query.value(1)
        print(name, salary)
//! [32]

    # FEATURE
//! [33]
    query = QSqlQuery()
    query.exec_("SELECT name, salary FROM employee WHERE salary > 50000")

    defaultDB = QSqlDatabase.database()
    if defaultDB.driver().hasFeature(QSqlDriver.QuerySize):
        numRows = query.size()
    else:
        # self can be very slow
        query.last()
        numRows = query.at() + 1
//! [33]

    # INSERT1
//! [34]
    query = QSqlQuery()
    query.exec_("INSERT INTO employee (id, name, salary) "
                "VALUES (1001, 'Thad Beaumont', 65000)")
//! [34]

    # NAMED BINDING
//! [35]
    query = QSqlQuery()
    query.prepare("INSERT INTO employee (id, name, salary) "
                  "VALUES (:id, :name, :salary)")
    query.bindValue(":id", 1001)
    query.bindValue(":name", "Thad Beaumont")
    query.bindValue(":salary", 65000)
    query.exec_()
//! [35]

    # POSITIONAL BINDING
//! [36]
    query = QSqlQuery()
    query.prepare("INSERT INTO employee (id, name, salary) "
                  "VALUES (?, ?, ?)")
    query.addBindValue(1001)
    query.addBindValue("Thad Beaumont")
    query.addBindValue(65000)
    query.exec_()
//! [36]

    # UPDATE1
//! [37]
    query = QSqlQuery()
    query.exec_("UPDATE employee SET salary = 70000 WHERE id = 1003")
//! [37]

    # DELETE1
//! [38]
    query = QSqlQuery()
    query.exec_("DELETE FROM employee WHERE id = 1007")
//! [38]

    # TRANSACTION
//! [39]
    QSqlDatabase.database().transaction()
    query = QSqlQuery()
    query.exec_("SELECT id FROM employee WHERE name = 'Torild Halvorsen'")
    if query.next():
        employeeId = query.value(0)
        query.exec_("INSERT INTO project (id, name, ownerid) "
                    "VALUES (201, 'Manhattan Project', "
                    + str(employeeId) + ')')

    QSqlDatabase.database().commit()
//! [39]

    # SQLQUERYMODEL1
//! [40]
    model = QSqlQueryModel()
    model.setQuery("SELECT * FROM employee")

    for i in range(model.rowCount()):
        _id = model.record(i).value("id")
        name = model.record(i).value("name")
        print(_id, name)

//! [40]
    }

    {
    # SQLTABLEMODEL1
//! [41]
    model = QSqlTableModel()
    model.setTable("employee")
    model.setFilter("salary > 50000")
    model.setSort(2, Qt.DescendingOrder)
    model.select()

    for i in range(model.rowCount()):
        name = model.record(i).value("name")
        salary = model.record(i).value("salary")
        print(f"{name}: {salary}")

//! [41]

    # SQLTABLEMODEL2
    model = QSqlTableModel()
    model.setTable("employee")

//! [42]
    for i in range(model.rowCount()):
        record = model.record(i)
        salary = record.value("salary")
        salary *= 1.1
        record.setValue("salary", salary)
        model.setRecord(i, record)

    model.submitAll()
//! [42]

    # SQLTABLEMODEL3
    row = 1
    column = 2
//! [43]
    model.setData(model.index(row, column), 75000)
    model.submitAll()
//! [43]

    # SQLTABLEMODEL4
//! [44]
    model.insertRows(row, 1)
    model.setData(model.index(row, 0), 1013)
    model.setData(model.index(row, 1), "Peter Gordon")
    model.setData(model.index(row, 2), 68500)
    model.submitAll()
//! [44]

//! [45]
    model.removeRows(row, 5)
//! [45]

//! [46]
    model.submitAll()
//! [46]

//! [47]
class XyzResult(QSqlResult):
    def __init__(driver):
        QSqlResult.__init__(self, driver)
        pass

    def data(self, index):
        return QVariant()

    def isNull(self, index):
        return False

    def reset(self, query):
        return False

    def fetch(self, index):
        return False

    def fetchFirst(self):
        return False

    def fetchLast(self):
        return False

    def size(self):
        return 0

    def numRowsAffected(self):
        return 0

    def record(self):
        return QSqlRecord()

//! [47]

//! [48]
class XyzDriver(QSqlDriver)
    def hasFeature(self, feature):
        return False

    def open(self, db, user, password, host, port, options):
        return False

    def close(self):
        pass

    def createResult(self):
        return XyzResult(self)

//! [48]

def main():
    app = QApplication([])

    QSqlDatabase_snippets()
    QSqlField_snippets()
    QSqlQuery_snippets()
    QSqlQueryModel_snippets()
    QSqlTableModel_snippets()

    driver = XyzDriver()
    result = XyzResult(driver)
