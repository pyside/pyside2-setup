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


//! [0]
def find(self)

    if !self.findDialog:
        self.findDialog = FindDialog(self)
        connect(findDialog, SIGNAL("findNext()"), self, SLOT("findNext()"))
    

    self.findDialog.show()
    self.findDialog.raise()
    self.findDialog.activateWindow()

//! [0]

//! [1]
def countWords(self):
    dialog = WordCountDialog(self)
    dialog.setWordCount(document().wordCount())
    dialog.exec_()

//! [1]

//! [2]
        mb = QMessageBox("Application Name",
                        "Hardware failure.\n\nDisk error detected\nDo you want to stop?",
                       QMessageBox.Question,
                       QMessageBox.Yes | QMessageBox.Default,
                       QMessageBox.No | QMessageBox.Escape,
                       QMessageBox.NoButton)
        if mb.exec() == QMessageBox.No:
            # try again
//! [2]

//! [3]
    progress = QProgressDialog("Copying files...", "Abort Copy", 0, numFiles, self)
    progress.setWindowModality(Qt.WindowModal)

    for i in rang(numFiles):
        progress.setValue(i)

        if progress.wasCanceled():
            break
        #... copy one file
    
    progress.setValue(numFiles)
//! [3]

//! [4]
# Operation constructor
def __init__(self, parent):
    QObject.__init__(self, parent)

    pd =  QProgressDialog("Operation in progress.", "Cancel", 0, 100)
    connect(pd, SIGNAL("canceled()"), self, SLOT("cancel()"))
    t =  QTimer(self)
    connect(t, SIGNAL("timeout()"), self, SLOT("perform()"))
    t.start(0)

//! [4] //! [5]

def perform(self):

    pd.setValue(steps)
    #... perform one percent of the operation
    steps++
    if steps > pd.maximum():
        t.stop()

//! [5] //! [6]

def cancel(self):

    t.stop()
    #... cleanup

//! [6]


