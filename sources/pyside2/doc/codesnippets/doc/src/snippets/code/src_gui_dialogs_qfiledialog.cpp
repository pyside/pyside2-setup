/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt for Python project.
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
fileName = QFileDialog.getOpenFileName(self,
    tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"))
//! [0]


//! [1]
"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
//! [1]


//! [2]
dialog = QFileDialog(self)
dialog.setFileMode(QFileDialog.AnyFile)
//! [2]


//! [3]
dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"))
//! [3]


//! [4]
dialog.setViewMode(QFileDialog.Detail)
//! [4]


//! [5]
if dialog.exec_():
    fileNames = dialog.selectedFiles()
//! [5]


//! [6]
dialog.setNameFilter("All C++ files (*.cpp *.cc *.C *.cxx *.c++)")
dialog.setNameFilter("*.cpp *.cc *.C *.cxx *.c++")
//! [6]


//! [7]
filters = QStringList()
filters << "Image files (*.png *.xpm *.jpg)"
        << "Text files (*.txt)"
        << "Any files (*)"

dialog = QFileDialog(this)
dialog.setNameFilters(filters)
dialog.exec_()
//! [7]


//! [8]
fileName = QFileDialog.getOpenFileName(self, tr("Open File"),
                                       "/home",
                                       tr("Images (*.png *.xpm *.jpg)"))
//! [8]


//! [9]
files = QFileDialog.getOpenFileNames(self,
                                     "Select one or more files to open",
                                     "/home",
                                     "Images (*.png *.xpm *.jpg)")
//! [9]


//! [10]
for it in list:
    myProcessing(it)
    it++
//! [10]


//! [11]
fileName = QFileDialog.getSaveFileName(self, tr("Save F:xile"),
                                       "/home/jana/untitled.png",
                                       tr("Images (*.png *.xpm *.jpg)"))
//! [11]


//! [12]
dir = QFileDialog.getExistingDirectory(self, tr("Open Directory"),
                                       "/home",
                                       QFileDialog.ShowDirsOnly
                                       | QFileDialog.DontResolveSymlinks)
//! [12]
