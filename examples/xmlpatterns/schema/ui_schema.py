# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'schema.ui'
##
## Created by: Qt User Interface Compiler version 5.14.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QColor, QFont, QIcon, QPixmap)
from PySide2.QtWidgets import *

class Ui_SchemaMainWindow(object):
    def setupUi(self, SchemaMainWindow):
        if SchemaMainWindow.objectName():
            SchemaMainWindow.setObjectName(u"SchemaMainWindow")
        SchemaMainWindow.resize(417, 594)
        self.centralwidget = QWidget(SchemaMainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.gridLayout = QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName(u"gridLayout")
        self.schemaLabel = QLabel(self.centralwidget)
        self.schemaLabel.setObjectName(u"schemaLabel")

        self.gridLayout.addWidget(self.schemaLabel, 0, 0, 1, 2)

        self.schemaSelection = QComboBox(self.centralwidget)
        self.schemaSelection.setObjectName(u"schemaSelection")

        self.gridLayout.addWidget(self.schemaSelection, 0, 2, 1, 2)

        self.schemaView = QTextBrowser(self.centralwidget)
        self.schemaView.setObjectName(u"schemaView")

        self.gridLayout.addWidget(self.schemaView, 1, 0, 1, 4)

        self.instanceLabel = QLabel(self.centralwidget)
        self.instanceLabel.setObjectName(u"instanceLabel")

        self.gridLayout.addWidget(self.instanceLabel, 2, 0, 1, 2)

        self.instanceSelection = QComboBox(self.centralwidget)
        self.instanceSelection.setObjectName(u"instanceSelection")

        self.gridLayout.addWidget(self.instanceSelection, 2, 2, 1, 2)

        self.instanceEdit = QTextEdit(self.centralwidget)
        self.instanceEdit.setObjectName(u"instanceEdit")

        self.gridLayout.addWidget(self.instanceEdit, 3, 0, 1, 4)

        self.label = QLabel(self.centralwidget)
        self.label.setObjectName(u"label")

        self.gridLayout.addWidget(self.label, 4, 0, 1, 1)

        self.validationStatus = QLabel(self.centralwidget)
        self.validationStatus.setObjectName(u"validationStatus")

        self.gridLayout.addWidget(self.validationStatus, 4, 1, 1, 2)

        self.validateButton = QPushButton(self.centralwidget)
        self.validateButton.setObjectName(u"validateButton")

        self.gridLayout.addWidget(self.validateButton, 4, 3, 1, 1)

        SchemaMainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QStatusBar(SchemaMainWindow)
        self.statusbar.setObjectName(u"statusbar")
        SchemaMainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(SchemaMainWindow)

        QMetaObject.connectSlotsByName(SchemaMainWindow)
    # setupUi

    def retranslateUi(self, SchemaMainWindow):
        SchemaMainWindow.setWindowTitle(QCoreApplication.translate("SchemaMainWindow", u"XML Schema Validation", None))
        self.schemaLabel.setText(QCoreApplication.translate("SchemaMainWindow", u"XML Schema Document:", None))
        self.instanceLabel.setText(QCoreApplication.translate("SchemaMainWindow", u"XML Instance Document:", None))
        self.label.setText(QCoreApplication.translate("SchemaMainWindow", u"Status:", None))
        self.validationStatus.setText(QCoreApplication.translate("SchemaMainWindow", u"not validated", None))
        self.validateButton.setText(QCoreApplication.translate("SchemaMainWindow", u"Validate", None))
    # retranslateUi

