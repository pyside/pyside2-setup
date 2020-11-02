/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of Qt for Python.
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
def paintEvent(self, qpaintevent):
    option = QStyleOptionButton()
    option.initFrom(self)
    if isDown():
        option.state = QStyle.State_Sunken
    else:
        option.state = QStyle.State_Raised

    if self.isDefault():
        option.features = option.features or QStyleOptionButton.DefaultButton
    option.text = self.text()
    option.icon = self.icon()

    painter = QPainter(self)
    self.style().drawControl(QStyle.CE_PushButton, option, painter, self)
//! [0]
//! [1]
    option = QStyleOptionFrame()

    if isinstance(option, QStyleOptionFrameV2):
        frameOptionV2 = QStyleOptionFrameV2(option)

        # draw the frame using frameOptionV2

//! [1]

//! [2]
    if isinstance(option, QStyleOptionProgressBarV2):
        progressBarV2 = QStyleOptionProgressBarV2(option)

        # draw the progress bar using progressBarV2

//! [2]

//! [3]
    if isinstance(option, QStyleOptionTabV2):
        tabV2 = QStyleOptionTabV2(option)

        # draw the tab using tabV2

//! [3]


//! [4]
def drawPrimitive(self, element, option, painter, widget):
    if element == self.PE_FrameFocusRect:
        focusRectOption =  QStyleOptionFocusRect(option)
        if focusRectOption:
            # ...


    # ...

//! [4]
