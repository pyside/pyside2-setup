/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of PySide2.
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
class ZipEngineHandler(QAbstractFileEngineHandler):
    def create(self, fileName):
        # ZipEngineHandler returns a ZipEngine for all .zip files
        if fileName.toLower().endsWith(".zip"):
            return ZipEngine(fileName)
        else
            return None

def main():
    app = QApplication(sys.argv)

    engine = ZipEngineHandler()

    window = MainWindow()
    window.show()

    return app.exec()
//! [0]


//! [1]
def create(fileName):
    # ZipEngineHandler returns a ZipEngine for all .zip files
    if fileName.lower().endswith(".zip"):
        return ZipEngine(fileName)
    else
        return None
//! [1]


//! [2]
# @arg filters QDir.Filters
# @arg filterNames [str, ...]
# @return QAbstractFileEngineIterator
def beginEntryList(filters, filterNames):
    return CustomFileEngineIterator(filters, filterNames)
//! [2]


//! [3]
class CustomIterator(QAbstractFileEngineIterator):
    def __init__(self, nameFilters, filters):
        QAbstractFileEngineIterator.__init__(self, nameFilters, filters)

        self.index = 0
        # In a real iterator, these entries are fetched from the
        # file system based on the value of path().
        self.entries << "entry1" << "entry2" << "entry3"

    def hasNext(self):
        return self.index < self.entries.size() - 1

    def next(self):
       if not self.hasNext():
           return None
       index += 1
       return currentFilePath()

    def currentFileName(self):
       return self.entries.at(index)
//! [3]
