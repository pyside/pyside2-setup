/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

Item {
    id: root
    width: 600
    height: 600

    Rectangle {
        id: blue
        objectName: "blueRectangle"
        width: 200
        height: 200
        anchors.top: root.top
        anchors.horizontalCenter: root.horizontalCenter
        color: "blue"
    }

    Text {
        text: qsTr("Original blue rectangle")
        anchors.left: blue.right
        anchors.verticalCenter: blue.verticalCenter
    }

    Image {
        id: imageContainer
        objectName: "imageContainer"
        width: 200
        height: 200
        anchors.bottom: root.bottom
        anchors.horizontalCenter: root.horizontalCenter
    }

    Text {
        text: qsTr("Image with the source URL set to the result of calling QQuickItem::grabToImage on the rectangle. If you see a second blue rectangle, that means it works.")
        anchors.left: imageContainer.right
        anchors.verticalCenter: imageContainer.verticalCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        width: 200
    }

}
