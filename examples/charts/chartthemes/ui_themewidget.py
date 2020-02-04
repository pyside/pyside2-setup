# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'themewidget.ui'
##
## Created by: Qt User Interface Compiler version 5.14.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import (QCoreApplication, QMetaObject, QObject, QPoint,
    QRect, QSize, QUrl, Qt)
from PySide2.QtGui import (QColor, QFont, QIcon, QPixmap)
from PySide2.QtWidgets import *

class Ui_ThemeWidgetForm(object):
    def setupUi(self, ThemeWidgetForm):
        if ThemeWidgetForm.objectName():
            ThemeWidgetForm.setObjectName(u"ThemeWidgetForm")
        ThemeWidgetForm.resize(900, 600)
        self.gridLayout = QGridLayout(ThemeWidgetForm)
        self.gridLayout.setObjectName(u"gridLayout")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.themeLabel = QLabel(ThemeWidgetForm)
        self.themeLabel.setObjectName(u"themeLabel")

        self.horizontalLayout.addWidget(self.themeLabel)

        self.themeComboBox = QComboBox(ThemeWidgetForm)
        self.themeComboBox.setObjectName(u"themeComboBox")

        self.horizontalLayout.addWidget(self.themeComboBox)

        self.animatedLabel = QLabel(ThemeWidgetForm)
        self.animatedLabel.setObjectName(u"animatedLabel")

        self.horizontalLayout.addWidget(self.animatedLabel)

        self.animatedComboBox = QComboBox(ThemeWidgetForm)
        self.animatedComboBox.setObjectName(u"animatedComboBox")

        self.horizontalLayout.addWidget(self.animatedComboBox)

        self.legendLabel = QLabel(ThemeWidgetForm)
        self.legendLabel.setObjectName(u"legendLabel")

        self.horizontalLayout.addWidget(self.legendLabel)

        self.legendComboBox = QComboBox(ThemeWidgetForm)
        self.legendComboBox.setObjectName(u"legendComboBox")

        self.horizontalLayout.addWidget(self.legendComboBox)

        self.antialiasCheckBox = QCheckBox(ThemeWidgetForm)
        self.antialiasCheckBox.setObjectName(u"antialiasCheckBox")
        self.antialiasCheckBox.setChecked(False)

        self.horizontalLayout.addWidget(self.antialiasCheckBox)

        self.horizontalSpacer = QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout.addItem(self.horizontalSpacer)


        self.gridLayout.addLayout(self.horizontalLayout, 0, 0, 1, 3)


        self.retranslateUi(ThemeWidgetForm)
        self.themeComboBox.currentIndexChanged.connect(ThemeWidgetForm.updateUI)
        self.antialiasCheckBox.toggled.connect(ThemeWidgetForm.updateUI)
        self.legendComboBox.currentIndexChanged.connect(ThemeWidgetForm.updateUI)
        self.animatedComboBox.currentIndexChanged.connect(ThemeWidgetForm.updateUI)

        QMetaObject.connectSlotsByName(ThemeWidgetForm)
    # setupUi

    def retranslateUi(self, ThemeWidgetForm):
        self.themeLabel.setText(QCoreApplication.translate("ThemeWidgetForm", u"Theme:", None))
        self.animatedLabel.setText(QCoreApplication.translate("ThemeWidgetForm", u"Animation:", None))
        self.legendLabel.setText(QCoreApplication.translate("ThemeWidgetForm", u"Legend:", None))
        self.antialiasCheckBox.setText(QCoreApplication.translate("ThemeWidgetForm", u"Anti-aliasing", None))
    # retranslateUi

