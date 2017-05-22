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
def __init__(self, parent = None):
    QWidget.__init__(self, parent)

    grid = QGridLayout()
    grid.addWidget(createFirstExclusiveGroup(), 0, 0)
    grid.addWidget(createSecondExclusiveGroup(), 1, 0)
    grid.addWidget(createNonExclusiveGroup(), 0, 1)
    grid.addWidget(createPushButtonGroup(), 1, 1)
    setLayout(grid)

    setWindowTitle("Group Boxes")
    resize(480, 320)

//! [0]

//! [1]
def createFirstExclusiveGroup(self):
//! [2]
    groupBox = QGroupBox("Exclusive Radio Buttons")

    radio1 = QRadioButton("&Radio button 1")
    radio2 = QRadioButton("R&adio button 2")
    radio3 = QRadioButton("Ra&dio button 3")

    radio1.setChecked(True)
//! [1] //! [3]

    vbox = QVBoxLayout()
    vbox.addWidget(radio1)
    vbox.addWidget(radio2)
    vbox.addWidget(radio3)
    vbox.addStretch(1)
    groupBox.setLayout(vbox)
//! [2]
    return groupBox
//! [3]

//! [4]
def createSecondExclusiveGroup(self):
    groupBox = QGroupBox("E&xclusive Radio Buttons")
    groupBox.setCheckable(True)
    groupBox.setChecked(False)
//! [4]

//! [5]
    radio1 = QRadioButton("Rad&io button 1")
    radio2 = QRadioButton("Radi&o button 2")
    radio3 = QRadioButton("Radio &button 3")
    radio1.setChecked(True)
    checkBox = QCheckBox("Ind&ependent checkbox")
    checkBox.setChecked(True)
//! [5]

//! [6]
    vbox = QVBoxLayout()
    vbox.addWidget(radio1)
    vbox.addWidget(radio2)
    vbox.addWidget(radio3)
    vbox.addWidget(checkBox)
    vbox.addStretch(1)
    groupBox.setLayout(vbox)

    return groupBox
//! [6]

//! [7]
def createNonExclusiveGroup(self):
    groupBox = QGroupBox("Non-Exclusive Checkboxes")
    groupBox.setFlat(True)
//! [7]

//! [8]
    checkBox1 = QCheckBox("&Checkbox 1")
    checkBox2 = QCheckBox("C&heckbox 2")
    checkBox2.setChecked(True)
    tristateBox = QCheckBox("Tri-&state button")
    tristateBox.setTristate(True)
//! [8]
    tristateBox.setCheckState(Qt.PartiallyChecked)

//! [9]
    vbox = QVBoxLayout()
    vbox.addWidget(checkBox1)
    vbox.addWidget(checkBox2)
    vbox.addWidget(tristateBox)
    vbox.addStretch(1)
    groupBox.setLayout(vbox)

    return groupBox
//! [9]

//! [10]
def createPushButtonGroup(self):
    groupBox = QGroupBox("&Push Buttons")
    groupBox.setCheckable(True)
    groupBox.setChecked(True)
//! [10]

//! [11]
    pushButton = QPushButton("&Normal Button")
    toggleButton = QPushButton("&Toggle Button")
    toggleButton.setCheckable(True)
    toggleButton.setChecked(True)
    flatButton = QPushButton("&Flat Button")
    flatButton.setFlat(True)
//! [11]

//! [12]
    popupButton = QPushButton("Pop&up Button")
    menu = QMenu(self)
    menu.addAction("&First Item")
    menu.addAction("&Second Item")
    menu.addAction("&Third Item")
    menu.addAction("F&ourth Item")
    popupButton.setMenu(menu)
//! [12]

    newAction = menu.addAction("Submenu")
    QMenu *subMenu = QMenu("Popup Submenu")
    subMenu.addAction("Item 1")
    subMenu.addAction("Item 2")
    subMenu.addAction("Item 3")
    newAction.setMenu(subMenu)

//! [13]
    vbox = QVBoxLayout()
    vbox.addWidget(pushButton)
    vbox.addWidget(toggleButton)
    vbox.addWidget(flatButton)
    vbox.addWidget(popupButton)
    vbox.addStretch(1)
    groupBox.setLayout(vbox)

    return groupBox
}
//! [13]
