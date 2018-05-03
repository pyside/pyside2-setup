/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of Qt for Python.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
# WRONG
db = QSqlDatabase.database("sales")
query = QSqlQuery("SELECT NAME, DOB FROM EMPLOYEES", db)
QSqlDatabase.removeDatabase("sales") # will output a warning

# "db" is now a dangling invalid database connection,
# "query" contains an invalid result set
//! [0]


//! [1]
db = QSqlDatabase.database("sales")
query = QSqlQuery("SELECT NAME, DOB FROM EMPLOYEES", db)
# Both "db" and "query" are destroyed because they are out of scope
QSqlDatabase.removeDatabase("sales") # correct
//! [1]


//! [2]
class MyDatabaseDriverCreatorBase(QtSql.QSqlDriverCreatorBase):
    ...
    def createObject(self):
        return MyDatabaseDriver()

mydriver = MyDatabaseDriverCreatorBase()
QtSql.QSqlDatabase.registerSqlDriver("MYDRIVER", mydriver)
db = QtSql.QSqlDatabase.addDatabase("MYDRIVER")
//! [2]


//! [3]
...
db = QSqlDatabase.addDatabase("QODBC")
db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=myaccessfile.mdb")
if db.open():
    # success!
    pass
...
//! [3]


//! [4]
...
# MySQL connection
db.setConnectOptions("CLIENT_SSL=1;CLIENT_IGNORE_SPACE=1") # use an SSL connection to the server
if not db.open():
    db.setConnectOptions() # clears the connect option string
    ...
...
# PostgreSQL connection
db.setConnectOptions("requiressl=1")    # enable PostgreSQL SSL connections
if not db.open():
    db.setConnectOptions() # clear options
    ...
...
# ODBC connection
# set ODBC options
db.setConnectOptions("SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_ONLY;SQL_ATTR_TRACE=SQL_OPT_TRACE_ON")
if not db.open():
    db.setConnectOptions()              # don't try to set this option
    ...
//! [4]


//! [5]
#include "qtdir/src/sql/drivers/psql/qsql_psql.cpp"
//! [5]


//! [6]
con = PQconnectdb("host=server user=bart password=simpson dbname=springfield")
drv = QPSQLDriver(con)
db = QSqlDatabase.addDatabase(drv) # becomes the new default connection
query = QSqlQuery()
query.exec_("SELECT NAME, ID FROM STAFF")
...
//! [6]


//! [7]
unix:LIBS += -lpq
win32:LIBS += libpqdll.lib
//! [7]


//! [8]
db = QSqlDatabase()
print(db.isValid())         # Returns False

db = QSqlDatabase.database("sales")
print(db.isValid())         # Returns True if "sales" connection exists

QSqlDatabase.removeDatabase("sales")
print(db.isValid())         # Returns False
//! [8]
