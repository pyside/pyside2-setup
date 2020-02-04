# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'bookwindow.ui'
##
## Created by: Qt User Interface Compiler version 5.14.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QBrush, QColor, QConicalGradient, QFont,
    QFontDatabase, QIcon, QLinearGradient, QPalette, QPainter, QPixmap,
    QRadialGradient)
from PySide2.QtWidgets import *

class Ui_BookWindow(object):
    def setupUi(self, BookWindow):
        if BookWindow.objectName():
            BookWindow.setObjectName(u"BookWindow")
        BookWindow.resize(601, 420)
        self.centralWidget = QWidget(BookWindow)
        self.centralWidget.setObjectName(u"centralWidget")
        self.vboxLayout = QVBoxLayout(self.centralWidget)
        self.vboxLayout.setSpacing(6)
        self.vboxLayout.setObjectName(u"vboxLayout")
        self.vboxLayout.setContentsMargins(9, 9, 9, 9)
        self.groupBox = QGroupBox(self.centralWidget)
        self.groupBox.setObjectName(u"groupBox")
        self.vboxLayout1 = QVBoxLayout(self.groupBox)
        self.vboxLayout1.setSpacing(6)
        self.vboxLayout1.setObjectName(u"vboxLayout1")
        self.vboxLayout1.setContentsMargins(9, 9, 9, 9)
        self.bookTable = QTableView(self.groupBox)
        self.bookTable.setObjectName(u"bookTable")
        self.bookTable.setSelectionBehavior(QAbstractItemView.SelectRows)

        self.vboxLayout1.addWidget(self.bookTable)

        self.groupBox_2 = QGroupBox(self.groupBox)
        self.groupBox_2.setObjectName(u"groupBox_2")
        self.formLayout = QFormLayout(self.groupBox_2)
        self.formLayout.setObjectName(u"formLayout")
        self.label_5 = QLabel(self.groupBox_2)
        self.label_5.setObjectName(u"label_5")

        self.formLayout.setWidget(0, QFormLayout.LabelRole, self.label_5)

        self.titleEdit = QLineEdit(self.groupBox_2)
        self.titleEdit.setObjectName(u"titleEdit")
        self.titleEdit.setEnabled(True)

        self.formLayout.setWidget(0, QFormLayout.FieldRole, self.titleEdit)

        self.label_2 = QLabel(self.groupBox_2)
        self.label_2.setObjectName(u"label_2")

        self.formLayout.setWidget(1, QFormLayout.LabelRole, self.label_2)

        self.authorEdit = QComboBox(self.groupBox_2)
        self.authorEdit.setObjectName(u"authorEdit")
        self.authorEdit.setEnabled(True)

        self.formLayout.setWidget(1, QFormLayout.FieldRole, self.authorEdit)

        self.label_3 = QLabel(self.groupBox_2)
        self.label_3.setObjectName(u"label_3")

        self.formLayout.setWidget(2, QFormLayout.LabelRole, self.label_3)

        self.genreEdit = QComboBox(self.groupBox_2)
        self.genreEdit.setObjectName(u"genreEdit")
        self.genreEdit.setEnabled(True)

        self.formLayout.setWidget(2, QFormLayout.FieldRole, self.genreEdit)

        self.label_4 = QLabel(self.groupBox_2)
        self.label_4.setObjectName(u"label_4")

        self.formLayout.setWidget(3, QFormLayout.LabelRole, self.label_4)

        self.yearEdit = QSpinBox(self.groupBox_2)
        self.yearEdit.setObjectName(u"yearEdit")
        self.yearEdit.setEnabled(True)
        self.yearEdit.setMinimum(-1000)
        self.yearEdit.setMaximum(2100)

        self.formLayout.setWidget(3, QFormLayout.FieldRole, self.yearEdit)

        self.label = QLabel(self.groupBox_2)
        self.label.setObjectName(u"label")

        self.formLayout.setWidget(4, QFormLayout.LabelRole, self.label)

        self.ratingEdit = QSpinBox(self.groupBox_2)
        self.ratingEdit.setObjectName(u"ratingEdit")
        self.ratingEdit.setMaximum(5)

        self.formLayout.setWidget(4, QFormLayout.FieldRole, self.ratingEdit)


        self.vboxLayout1.addWidget(self.groupBox_2)


        self.vboxLayout.addWidget(self.groupBox)

        BookWindow.setCentralWidget(self.centralWidget)
        QWidget.setTabOrder(self.bookTable, self.titleEdit)
        QWidget.setTabOrder(self.titleEdit, self.authorEdit)
        QWidget.setTabOrder(self.authorEdit, self.genreEdit)
        QWidget.setTabOrder(self.genreEdit, self.yearEdit)

        self.retranslateUi(BookWindow)

        QMetaObject.connectSlotsByName(BookWindow)
    # setupUi

    def retranslateUi(self, BookWindow):
        BookWindow.setWindowTitle(QCoreApplication.translate("BookWindow", u"Books", None))
        self.groupBox.setTitle("")
        self.groupBox_2.setTitle(QCoreApplication.translate("BookWindow", u"Details", None))
        self.label_5.setText(QCoreApplication.translate("BookWindow", u"<b>Title:</b>", None))
        self.label_2.setText(QCoreApplication.translate("BookWindow", u"<b>Author: </b>", None))
        self.label_3.setText(QCoreApplication.translate("BookWindow", u"<b>Genre:</b>", None))
        self.label_4.setText(QCoreApplication.translate("BookWindow", u"<b>Year:</b>", None))
        self.yearEdit.setPrefix("")
        self.label.setText(QCoreApplication.translate("BookWindow", u"<b>Rating:</b>", None))
    # retranslateUi
