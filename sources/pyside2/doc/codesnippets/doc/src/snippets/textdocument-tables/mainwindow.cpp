############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the examples of the Qt for Python project.
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

//! [0] //! [1]
    cursor = QTextCursor(editor.textCursor())
//! [0]
    cursor.movePosition(QTextCursor.Start)
//! [1]

//! [2]
    tableFormat = QTextTableFormat()
    tableFormat.setBackground(QColor("#e0e0e0"))
    QVector<QTextLength> constraints
    constraints << QTextLength(QTextLength.PercentageLength, 16)
    constraints << QTextLength(QTextLength.PercentageLength, 28)
    constraints << QTextLength(QTextLength.PercentageLength, 28)
    constraints << QTextLength(QTextLength.PercentageLength, 28)
    tableFormat.setColumnWidthConstraints(constraints)
//! [3]
    table = cursor.insertTable(rows, columns, tableFormat)
//! [2] //! [3]

//! [4]
    cell = table.cellAt(0, 0)
    cellCursor = cell.firstCursorPosition()
    cellCursor.insertText(tr("Week"), charFormat)
//! [4]

//! [5]
    for column  in range(columns):
        cell = table.cellAt(0, column)
        cellCursor = cell.firstCursorPosition()
        cellCursor.insertText(tr("Team %1").arg(column), charFormat)
    

    for row in range(rows):
        cell = table.cellAt(row, 0)
        cellCursor = cell.firstCursorPosition()
        cellCursor.insertText(tr("%1").arg(row), charFormat)

        for column in range(columns)
            if (row-1) % 3 == column-1:
//! [5] //! [6]
                cell = table.cellAt(row, column)
                cellCursor = cell.firstCursorPosition()
                cellCursor.insertText(tr("On duty"), charFormat)

//! [6] //! [7]

//! [7] //! [8]

//! [8]

//! [9]
    for row in range(table.rows()):
        for column in range(table.columns()):
            tableCell = table.cellAt(row, column)
//! [9]
            QTextFrame.iterator it
            QString text
            for (it = tableCell.begin() !(it.atEnd()); ++it):
                QTextBlock childBlock = it.currentBlock()
                if (childBlock.isValid())
                    text += childBlock.text()

            Item = QTableWidgetItem(text)
            tableWidget.setItem(row, column, Item)
            
//! [10]
            processTableCell(tableCell)
//! [10]
            
//! [11]

//! [11] //! [12]

//! [12]

