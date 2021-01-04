#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import datetime
import logging

from PySide6.QtCore import Qt, Slot
from PySide6.QtSql import QSqlDatabase, QSqlQuery, QSqlRecord, QSqlTableModel

table_name = "Conversations"


def createTable():
    if table_name in QSqlDatabase.database().tables():
        return

    query = QSqlQuery()
    if not query.exec_(
        """
        CREATE TABLE IF NOT EXISTS 'Conversations' (
            'author' TEXT NOT NULL,
            'recipient' TEXT NOT NULL,
            'timestamp' TEXT NOT NULL,
            'message' TEXT NOT NULL,
        FOREIGN KEY('author') REFERENCES Contacts ( name ),
        FOREIGN KEY('recipient') REFERENCES Contacts ( name )
        )
        """
    ):
        logging.error("Failed to query database")

    # This adds the first message from the Bot
    # and further development is required to make it interactive.
    query.exec_(
        """
        INSERT INTO Conversations VALUES(
            'machine', 'Me', '2019-01-07T14:36:06', 'Hello!'
        )
        """
    )
    logging.info(query)


class SqlConversationModel(QSqlTableModel):
    def __init__(self, parent=None):
        super(SqlConversationModel, self).__init__(parent)

        createTable()
        self.setTable(table_name)
        self.setSort(2, Qt.DescendingOrder)
        self.setEditStrategy(QSqlTableModel.OnManualSubmit)
        self.recipient = ""

        self.select()
        logging.debug("Table was loaded successfully.")

    def setRecipient(self, recipient):
        if recipient == self.recipient:
            pass

        self.recipient = recipient

        filter_str = (f"(recipient = '{self.recipient}' AND author = 'Me') OR "
                      f"(recipient = 'Me' AND author='{self.recipient}')")
        self.setFilter(filter_str)
        self.select()

    def data(self, index, role):
        if role < Qt.UserRole:
            return QSqlTableModel.data(self, index, role)

        sql_record = QSqlRecord()
        sql_record = self.record(index.row())

        return sql_record.value(role - Qt.UserRole)

    def roleNames(self):
        """Converts dict to hash because that's the result expected
        by QSqlTableModel"""
        names = {}
        author = "author".encode()
        recipient = "recipient".encode()
        timestamp = "timestamp".encode()
        message = "message".encode()

        names[hash(Qt.UserRole)] = author
        names[hash(Qt.UserRole + 1)] = recipient
        names[hash(Qt.UserRole + 2)] = timestamp
        names[hash(Qt.UserRole + 3)] = message

        return names

    def send_message(self, recipient, message, author):
        timestamp = datetime.datetime.now()

        new_record = self.record()
        new_record.setValue("author", author)
        new_record.setValue("recipient", recipient)
        new_record.setValue("timestamp", str(timestamp))
        new_record.setValue("message", message)

        logging.debug(f'Message: "{message}" \n Received by: "{recipient}"')

        if not self.insertRecord(self.rowCount(), new_record):
            logging.error("Failed to send message: {self.lastError().text()}")
            return

        self.submitAll()
        self.select()
