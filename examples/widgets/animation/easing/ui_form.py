# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'form.ui'
##
## Created by: Qt User Interface Compiler version 5.14.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QColor, QFont, QIcon, QPixmap)
from PySide2.QtWidgets import *

class Ui_Form(object):
    def setupUi(self, Form):
        if Form.objectName():
            Form.setObjectName(u"Form")
        Form.resize(545, 471)
        self.gridLayout = QGridLayout(Form);
        self.gridLayout.setObjectName(u"gridLayout")
        self.easingCurvePicker = QListWidget(Form)
        self.easingCurvePicker.setObjectName(u"easingCurvePicker")
        sizePolicy = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(self.easingCurvePicker.sizePolicy().hasHeightForWidth())
        self.easingCurvePicker.setSizePolicy(sizePolicy)
        self.easingCurvePicker.setMaximumSize(QSize(16777215, 120))
        self.easingCurvePicker.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.easingCurvePicker.setMovement(QListView.Static)
        self.easingCurvePicker.setProperty("isWrapping", False)
        self.easingCurvePicker.setViewMode(QListView.IconMode)
        self.easingCurvePicker.setSelectionRectVisible(False)

        self.gridLayout.addWidget(self.easingCurvePicker, 0, 0, 1, 2);

        self.verticalLayout = QVBoxLayout();
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.groupBox_2 = QGroupBox(Form)
        self.groupBox_2.setObjectName(u"groupBox_2")
        self.verticalLayout_2 = QVBoxLayout(self.groupBox_2);
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.lineRadio = QRadioButton(self.groupBox_2)
        self.buttonGroup = QButtonGroup(Form)
        self.buttonGroup.setObjectName(u"buttonGroup")
        self.buttonGroup.setExclusive(False)
        self.buttonGroup.addButton(self.lineRadio)
        self.lineRadio.setObjectName(u"lineRadio")
        self.lineRadio.setChecked(True)

        self.verticalLayout_2.addWidget(self.lineRadio);

        self.circleRadio = QRadioButton(self.groupBox_2)
        self.buttonGroup.addButton(self.circleRadio)
        self.circleRadio.setObjectName(u"circleRadio")

        self.verticalLayout_2.addWidget(self.circleRadio);


        self.verticalLayout.addWidget(self.groupBox_2);

        self.groupBox = QGroupBox(Form)
        self.groupBox.setObjectName(u"groupBox")
        sizePolicy1 = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(self.groupBox.sizePolicy().hasHeightForWidth())
        self.groupBox.setSizePolicy(sizePolicy1)
        self.formLayout = QFormLayout(self.groupBox);
        self.formLayout.setObjectName(u"formLayout")
        self.formLayout.setFieldGrowthPolicy(QFormLayout.AllNonFixedFieldsGrow)
        self.label = QLabel(self.groupBox)
        self.label.setObjectName(u"label")

        self.formLayout.setWidget(0, QFormLayout.LabelRole, self.label);

        self.periodSpinBox = QDoubleSpinBox(self.groupBox)
        self.periodSpinBox.setObjectName(u"periodSpinBox")
        self.periodSpinBox.setEnabled(False)
        self.periodSpinBox.setMinimum(-1.000000000000000)
        self.periodSpinBox.setSingleStep(0.100000000000000)
        self.periodSpinBox.setValue(-1.000000000000000)

        self.formLayout.setWidget(0, QFormLayout.FieldRole, self.periodSpinBox);

        self.label_2 = QLabel(self.groupBox)
        self.label_2.setObjectName(u"label_2")

        self.formLayout.setWidget(1, QFormLayout.LabelRole, self.label_2);

        self.amplitudeSpinBox = QDoubleSpinBox(self.groupBox)
        self.amplitudeSpinBox.setObjectName(u"amplitudeSpinBox")
        self.amplitudeSpinBox.setEnabled(False)
        self.amplitudeSpinBox.setMinimum(-1.000000000000000)
        self.amplitudeSpinBox.setSingleStep(0.100000000000000)
        self.amplitudeSpinBox.setValue(-1.000000000000000)

        self.formLayout.setWidget(1, QFormLayout.FieldRole, self.amplitudeSpinBox);

        self.label_3 = QLabel(self.groupBox)
        self.label_3.setObjectName(u"label_3")

        self.formLayout.setWidget(2, QFormLayout.LabelRole, self.label_3);

        self.overshootSpinBox = QDoubleSpinBox(self.groupBox)
        self.overshootSpinBox.setObjectName(u"overshootSpinBox")
        self.overshootSpinBox.setEnabled(False)
        self.overshootSpinBox.setMinimum(-1.000000000000000)
        self.overshootSpinBox.setSingleStep(0.100000000000000)
        self.overshootSpinBox.setValue(-1.000000000000000)

        self.formLayout.setWidget(2, QFormLayout.FieldRole, self.overshootSpinBox);


        self.verticalLayout.addWidget(self.groupBox);

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding);

        self.verticalLayout.addItem(self.verticalSpacer);


        self.gridLayout.addLayout(self.verticalLayout, 1, 0, 1, 1);

        self.graphicsView = QGraphicsView(Form)
        self.graphicsView.setObjectName(u"graphicsView")
        sizePolicy2 = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(self.graphicsView.sizePolicy().hasHeightForWidth())
        self.graphicsView.setSizePolicy(sizePolicy2)

        self.gridLayout.addWidget(self.graphicsView, 1, 1, 1, 1);


        self.retranslateUi(Form)

        QMetaObject.connectSlotsByName(Form)
    # setupUi

    def retranslateUi(self, Form):
        Form.setWindowTitle(QCoreApplication.translate("Form", u"Easing curves", None))
        self.groupBox_2.setTitle(QCoreApplication.translate("Form", u"Path type", None))
        self.lineRadio.setText(QCoreApplication.translate("Form", u"Line", None))
        self.circleRadio.setText(QCoreApplication.translate("Form", u"Circle", None))
        self.groupBox.setTitle(QCoreApplication.translate("Form", u"Properties", None))
        self.label.setText(QCoreApplication.translate("Form", u"Period", None))
        self.label_2.setText(QCoreApplication.translate("Form", u"Amplitude", None))
        self.label_3.setText(QCoreApplication.translate("Form", u"Overshoot", None))
    # retranslateUi

