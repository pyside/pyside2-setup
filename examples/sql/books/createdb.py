#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the Qt for Python examples of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:BSD$
## You may use this file under the terms of the BSD license as follows:
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
#############################################################################

from PySide2.QtSql import QSqlDatabase, QSqlError, QSqlQuery
from datetime import datetime


def add_book(q, title, year, authorId, genreId, rating):
    q.addBindValue(title)
    q.addBindValue(year)
    q.addBindValue(authorId)
    q.addBindValue(genreId)
    q.addBindValue(rating)
    q.exec_()


def add_genre(q, name):
    q.addBindValue(name)
    q.exec_()
    return q.lastInsertId()


def add_author(q, name, birthdate):
    q.addBindValue(name)
    q.addBindValue(birthdate)
    q.exec_()
    return q.lastInsertId()


def init_db():
    db = QSqlDatabase.addDatabase("QSQLITE")
    db.setDatabaseName(":memory:")

    if not db.open():
        return db.lastError()

    tables = db.tables()
    for table in tables:
        if table == "books" and table == "authors":
            return QSqlError()

    q = QSqlQuery()
    if not q.exec_("create table books(id integer primary key, title varchar, author integer, "
            "genre integer, year integer, rating integer)"):
        return q.lastError()
    if not q.exec_("create table authors(id integer primary key, name varchar, birthdate date)"):
        return q.lastError()
    if not q.exec_("create table genres(id integer primary key, name varchar)"):
        return q.lastError()

    if not q.prepare("insert into authors(name, birthdate) values(?, ?)"):
        return q.lastError()
    asimovId = add_author(q, "Isaac Asimov", datetime(1920, 2, 1))
    greeneId = add_author(q, "Graham Greene", datetime(1904, 10, 2))
    pratchettId = add_author(q, "Terry Pratchett", datetime(1948, 4, 28))

    if not q.prepare("insert into genres(name) values(?)"):
        return q.lastError()
    sfiction = add_genre(q, "Science Fiction")
    fiction = add_genre(q, "Fiction")
    fantasy = add_genre(q, "Fantasy")

    if not q.prepare("insert into books(title, year, author, genre, rating) "
            "values(?, ?, ?, ?, ?)"):
        return q.lastError()
    add_book(q, "Foundation", 1951, asimovId, sfiction, 3)
    add_book(q, "Foundation and Empire", 1952, asimovId, sfiction, 4)
    add_book(q, "Second Foundation", 1953, asimovId, sfiction, 3)
    add_book(q, "Foundation's Edge", 1982, asimovId, sfiction, 3)
    add_book(q, "Foundation and Earth", 1986, asimovId, sfiction, 4)
    add_book(q, "Prelude to Foundation", 1988, asimovId, sfiction, 3)
    add_book(q, "Forward the Foundation", 1993, asimovId, sfiction, 3)
    add_book(q, "The Power and the Glory", 1940, greeneId, fiction, 4)
    add_book(q, "The Third Man", 1950, greeneId, fiction, 5)
    add_book(q, "Our Man in Havana", 1958, greeneId, fiction, 4)
    add_book(q, "Guards! Guards!", 1989, pratchettId, fantasy, 3)
    add_book(q, "Night Watch", 2002, pratchettId, fantasy, 3)
    add_book(q, "Going Postal", 2004, pratchettId, fantasy, 3)

    return QSqlError()
