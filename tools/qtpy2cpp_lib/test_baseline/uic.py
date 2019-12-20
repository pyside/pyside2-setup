# -*- coding: utf-8 -*-
#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QBrush, QColor, QConicalGradient, QFont,
    QFontDatabase, QIcon, QLinearGradient, QPalette, QPainter, QPixmap,
    QRadialGradient)
from PySide2.QtWidgets import *

class Ui_ImageDialog(object):
    def setupUi(self, dialog):
        if dialog.objectName():
            dialog.setObjectName(u"dialog")
        dialog.setObjectName(u"ImageDialog")
        dialog.resize(320, 180)
        self.vboxLayout = QVBoxLayout(dialog)
#ifndef Q_OS_MAC
        self.vboxLayout.setSpacing(6)
#endif
#ifndef Q_OS_MAC
        self.vboxLayout.setContentsMargins(9, 9, 9, 9)
#endif
        self.vboxLayout.setObjectName(u"vboxLayout")
        self.vboxLayout.setObjectName(u"")
        self.gridLayout = QGridLayout()
#ifndef Q_OS_MAC
        self.gridLayout.setSpacing(6)
#endif
        self.gridLayout.setContentsMargins(1, 1, 1, 1)
        self.gridLayout.setObjectName(u"gridLayout")
        self.gridLayout.setObjectName(u"")
        self.widthLabel = QLabel(dialog)
        self.widthLabel.setObjectName(u"widthLabel")
        self.widthLabel.setObjectName(u"widthLabel")
        self.widthLabel.setGeometry(QRect(1, 27, 67, 22))
        self.widthLabel.setFrameShape(QFrame.NoFrame)
        self.widthLabel.setFrameShadow(QFrame.Plain)
        self.widthLabel.setTextFormat(Qt.AutoText)

        self.gridLayout.addWidget(self.widthLabel, 1, 0, 1, 1)

        self.heightLabel = QLabel(dialog)
        self.heightLabel.setObjectName(u"heightLabel")
        self.heightLabel.setObjectName(u"heightLabel")
        self.heightLabel.setGeometry(QRect(1, 55, 67, 22))
        self.heightLabel.setFrameShape(QFrame.NoFrame)
        self.heightLabel.setFrameShadow(QFrame.Plain)
        self.heightLabel.setTextFormat(Qt.AutoText)

        self.gridLayout.addWidget(self.heightLabel, 2, 0, 1, 1)

        self.colorDepthCombo = QComboBox(dialog)
        self.colorDepthCombo.setObjectName(u"colorDepthCombo")
        self.colorDepthCombo.setObjectName(u"colorDepthCombo")
        self.colorDepthCombo.setGeometry(QRect(74, 83, 227, 22))
        sizePolicy = QSizePolicy(QSizePolicy.Preferred, QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.colorDepthCombo.sizePolicy().hasHeightForWidth())
        self.colorDepthCombo.setSizePolicy(sizePolicy)
        self.colorDepthCombo.setInsertPolicy(QComboBox.InsertAtBottom)

        self.gridLayout.addWidget(self.colorDepthCombo, 3, 1, 1, 1)

        self.nameLineEdit = QLineEdit(dialog)
        self.nameLineEdit.setObjectName(u"nameLineEdit")
        self.nameLineEdit.setObjectName(u"nameLineEdit")
        self.nameLineEdit.setGeometry(QRect(74, 83, 227, 22))
        sizePolicy1 = QSizePolicy(QSizePolicy.Preferred, QSizePolicy.Fixed)
        sizePolicy1.setHorizontalStretch(1)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.nameLineEdit.sizePolicy().hasHeightForWidth())
        self.nameLineEdit.setSizePolicy(sizePolicy1)
        self.nameLineEdit.setEchoMode(QLineEdit.Normal)

        self.gridLayout.addWidget(self.nameLineEdit, 0, 1, 1, 1)

        self.spinBox = QSpinBox(dialog)
        self.spinBox.setObjectName(u"spinBox")
        self.spinBox.setObjectName(u"spinBox")
        self.spinBox.setGeometry(QRect(74, 1, 227, 20))
        sizePolicy.setHeightForWidth(self.spinBox.sizePolicy().hasHeightForWidth())
        self.spinBox.setSizePolicy(sizePolicy)
        self.spinBox.setButtonSymbols(QAbstractSpinBox.UpDownArrows)
        self.spinBox.setValue(32)
        self.spinBox.setMaximum(1024)
        self.spinBox.setMinimum(1)

        self.gridLayout.addWidget(self.spinBox, 1, 1, 1, 1)

        self.spinBox_2 = QSpinBox(dialog)
        self.spinBox_2.setObjectName(u"spinBox_2")
        self.spinBox_2.setObjectName(u"spinBox_2")
        self.spinBox_2.setGeometry(QRect(74, 27, 227, 22))
        sizePolicy.setHeightForWidth(self.spinBox_2.sizePolicy().hasHeightForWidth())
        self.spinBox_2.setSizePolicy(sizePolicy)
        self.spinBox_2.setButtonSymbols(QAbstractSpinBox.UpDownArrows)
        self.spinBox_2.setValue(32)
        self.spinBox_2.setMaximum(1024)
        self.spinBox_2.setMinimum(1)

        self.gridLayout.addWidget(self.spinBox_2, 2, 1, 1, 1)

        self.nameLabel = QLabel(dialog)
        self.nameLabel.setObjectName(u"nameLabel")
        self.nameLabel.setObjectName(u"nameLabel")
        self.nameLabel.setGeometry(QRect(1, 1, 67, 20))
        self.nameLabel.setFrameShape(QFrame.NoFrame)
        self.nameLabel.setFrameShadow(QFrame.Plain)
        self.nameLabel.setTextFormat(Qt.AutoText)

        self.gridLayout.addWidget(self.nameLabel, 0, 0, 1, 1)

        self.colorDepthLabel = QLabel(dialog)
        self.colorDepthLabel.setObjectName(u"colorDepthLabel")
        self.colorDepthLabel.setObjectName(u"colorDepthLabel")
        self.colorDepthLabel.setGeometry(QRect(1, 83, 67, 22))
        self.colorDepthLabel.setFrameShape(QFrame.NoFrame)
        self.colorDepthLabel.setFrameShadow(QFrame.Plain)
        self.colorDepthLabel.setTextFormat(Qt.AutoText)

        self.gridLayout.addWidget(self.colorDepthLabel, 3, 0, 1, 1)


        self.vboxLayout.addLayout(self.gridLayout)

        self.spacerItem = QSpacerItem(0, 0, QSizePolicy.Minimum, QSizePolicy.Expanding)

        self.vboxLayout.addItem(self.spacerItem)

        self.hboxLayout = QHBoxLayout()
#ifndef Q_OS_MAC
        self.hboxLayout.setSpacing(6)
#endif
        self.hboxLayout.setContentsMargins(1, 1, 1, 1)
        self.hboxLayout.setObjectName(u"hboxLayout")
        self.hboxLayout.setObjectName(u"")
        self.spacerItem1 = QSpacerItem(0, 0, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.hboxLayout.addItem(self.spacerItem1)

        self.okButton = QPushButton(dialog)
        self.okButton.setObjectName(u"okButton")
        self.okButton.setObjectName(u"okButton")
        self.okButton.setGeometry(QRect(135, 1, 80, 24))

        self.hboxLayout.addWidget(self.okButton)

        self.cancelButton = QPushButton(dialog)
        self.cancelButton.setObjectName(u"cancelButton")
        self.cancelButton.setObjectName(u"cancelButton")
        self.cancelButton.setGeometry(QRect(221, 1, 80, 24))

        self.hboxLayout.addWidget(self.cancelButton)


        self.vboxLayout.addLayout(self.hboxLayout)

        QWidget.setTabOrder(self.nameLineEdit, self.spinBox)
        QWidget.setTabOrder(self.spinBox, self.spinBox_2)
        QWidget.setTabOrder(self.spinBox_2, self.colorDepthCombo)
        QWidget.setTabOrder(self.colorDepthCombo, self.okButton)
        QWidget.setTabOrder(self.okButton, self.cancelButton)

        self.retranslateUi(dialog)
        self.nameLineEdit.returnPressed.connect(self.okButton.animateClick)

        QMetaObject.connectSlotsByName(dialog)
    # setupUi

    def retranslateUi(self, dialog):
        dialog.setWindowTitle(QCoreApplication.translate("ImageDialog", u"Create Image", None))
        self.widthLabel.setText(QCoreApplication.translate("ImageDialog", u"Width:", None))
        self.heightLabel.setText(QCoreApplication.translate("ImageDialog", u"Height:", None))
        self.nameLineEdit.setText(QCoreApplication.translate("ImageDialog", u"Untitled image", None))
        self.nameLabel.setText(QCoreApplication.translate("ImageDialog", u"Name:", None))
        self.colorDepthLabel.setText(QCoreApplication.translate("ImageDialog", u"Color depth:", None))
        self.okButton.setText(QCoreApplication.translate("ImageDialog", u"OK", None))
        self.cancelButton.setText(QCoreApplication.translate("ImageDialog", u"Cancel", None))
    # retranslateUi

