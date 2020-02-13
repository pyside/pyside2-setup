# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'dialog.ui'
##
## Created by: Qt User Interface Compiler version 5.14.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QBrush, QColor, QConicalGradient, QCursor, QFont,
    QFontDatabase, QIcon, QLinearGradient, QPalette, QPainter, QPixmap,
    QRadialGradient)
from PySide2.QtWidgets import *


class Ui_Dialog(object):
    def setupUi(self, Dialog):
        if not Dialog.objectName():
            Dialog.setObjectName(u"Dialog")
        Dialog.resize(400, 300)
        self.gridLayout = QGridLayout(Dialog)
        self.gridLayout.setObjectName(u"gridLayout")
        self.input = QLineEdit(Dialog)
        self.input.setObjectName(u"input")

        self.gridLayout.addWidget(self.input, 1, 0, 1, 1)

        self.send = QPushButton(Dialog)
        self.send.setObjectName(u"send")

        self.gridLayout.addWidget(self.send, 1, 1, 1, 1)

        self.output = QPlainTextEdit(Dialog)
        self.output.setObjectName(u"output")
        self.output.setReadOnly(True)
        self.output.setPlainText(u"Initializing WebChannel...")
        self.output.setBackgroundVisible(False)

        self.gridLayout.addWidget(self.output, 0, 0, 1, 2)


        self.retranslateUi(Dialog)

        QMetaObject.connectSlotsByName(Dialog)
    # setupUi

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QCoreApplication.translate("Dialog", u"Dialog", None))
        self.input.setPlaceholderText(QCoreApplication.translate("Dialog", u"Message Contents", None))
        self.send.setText(QCoreApplication.translate("Dialog", u"Send", None))
    # retranslateUi

