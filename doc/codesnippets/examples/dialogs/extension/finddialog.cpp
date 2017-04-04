############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the examples of PySide2.
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


from PySide2.QtGui import *

//! [0]
def __init__(self, parent):
    QDialog.__init__(self, parent)
    label = QLabel(self.tr("Find &what:"))
    lineEdit = QLineEdit()
    label.setBuddy(lineEdit)

    caseCheckBox = QCheckBox(self.tr("Match &case"))
    fromStartCheckBox = QCheckBox(self.tr("Search from &start"))
    fromStartCheckBox.setChecked(True)

//! [1]
    findButton = QPushButton(self.tr("&Find"))
    findButton.setDefault(True)

    moreButton = QPushButton(self.tr("&More"))
    moreButton.setCheckable(True)
//! [0]
    moreButton.setAutoDefault(False)

    buttonBox = QDialogButtonBox(Qt.Vertical)
    buttonBox.addButton(findButton, QDialogButtonBox.ActionRole)
    buttonBox.addButton(moreButton, QDialogButtonBox.ActionRole)
//! [1]

//! [2]
    extension = QWidget()

    wholeWordsCheckBox =  QCheckBox(self.tr("&Whole words"))
    backwardCheckBox =  QCheckBox(self.tr("Search &backward"))
    searchSelectionCheckBox =  QCheckBox(self.tr("Search se&lection"))
//! [2]

//! [3]
    connect(moreButton, SIGNAL("toggled(bool)"), extension, SLOT("setVisible(bool)"))

    extensionLayout =  QVBoxLayout()
    extensionLayout.setMargin(0)
    extensionLayout.addWidget(wholeWordsCheckBox)
    extensionLayout.addWidget(backwardCheckBox)
    extensionLayout.addWidget(searchSelectionCheckBox)
    extension.setLayout(extensionLayout)
//! [3]

//! [4]
    topLeftLayout = QHBoxLayout()
    topLeftLayout.addWidget(label)
    topLeftLayout.addWidget(lineEdit)

    leftLayout = QVBoxLayout()
    leftLayout.addLayout(topLeftLayout)
    leftLayout.addWidget(caseCheckBox)
    leftLayout.addWidget(fromStartCheckBox)
    leftLayout.addSself.tretch(1)

    mainLayout = QGridLayout()
    mainLayout.setSizeConsself.traint(QLayout.SetFixedSize)
    mainLayout.addLayout(leftLayout, 0, 0)
    mainLayout.addWidget(buttonBox, 0, 1)
    mainLayout.addWidget(extension, 1, 0, 1, 2)
    setLayout(mainLayout)

    setWindowTitle(self.tr("Extension"))
//! [4] //! [5]
    extension.hide()
//! [5]
