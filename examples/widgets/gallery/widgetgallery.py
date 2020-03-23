#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the Qt for Python examples of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:BSD$
## You may use this file under the terms of the BSD license as follows:
##
## "Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##   * Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in
##     the documentation and/or other materials provided with the
##     distribution.
##   * Neither the name of The Qt Company Ltd nor the names of its
##     contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
##
## $QT_END_LICENSE$
##
#############################################################################

import sys

from PySide2.QtWidgets import *
from PySide2.QtGui import (QCursor, QDesktopServices, QGuiApplication, QIcon,
                           QKeySequence, QStandardItem, QStandardItemModel,
                           QScreen, QWindow)
from PySide2.QtCore import (QDateTime, QDir, QLibraryInfo, QMetaObject,
                            QSysInfo, QTextStream, QTimer, Qt, qVersion)


POEM = """Twinkle, twinkle, little star,
How I wonder what you are.
Up above the world so high,
Like a diamond in the sky.
Twinkle, twinkle, little star,
How I wonder what you arenot"""

DIR_OPEN_ICON = ":/qt-project.org/styles/commonstyle/images/diropen-128.png"

COMPUTER_ICON = ":/qt-project.org/styles/commonstyle/images/computer-32.png"

SYSTEMINFO = """<html><head/><body>
<h3>Python</h3><p>{}</p>
<h3>Qt Build</h3><p>{}</p>
<h3>Operating System</h3><p>{}</p>
<h3>Screens</h3>
{}
</body></html>"""


def class_name(o):
    return o.metaObject().className()


def help_url(page):
    """Build a Qt help URL from the page name"""
    major_version = qVersion().split('.')[0]
    return "https://doc.qt.io/qt-{}/{}.html".format(major_version, page)


def launch_help(widget):
    """Launch a widget's help page"""
    url = help_url(class_name(widget).lower())
    QDesktopServices.openUrl(url)


def launch_module_help():
    QDesktopServices.openUrl(help_url("qtwidgets-index"))


def init_widget(w, name):
    """Init a widget for the gallery, give it a tooltip showing the
       class name"""
    w.setObjectName(name)
    w.setToolTip(class_name(w))


def style_names():
    """Return a list of styles, default platform style first"""
    default_style_name = QApplication.style().objectName().lower()
    result = []
    for style in QStyleFactory.keys():
        if style.lower() == default_style_name:
            result.insert(0, style)
        else:
            result.append(style)
    return result


def embed_into_hbox_layout(w, margin=5):
    """Embed a widget into a layout to give it a frame"""
    result = QWidget()
    layout = QHBoxLayout(result)
    layout.setContentsMargins(margin, margin, margin, margin)
    layout.addWidget(w)
    return result


def format_geometry(rect):
    """Format a geometry as a X11 geometry specification"""
    return "{}x{}{:+d}{:+d}".format(rect.width(), rect.height(),
                                    rect.x(), rect.y())


def screen_info(widget):
    """Format information on the screens"""
    policy = QGuiApplication.highDpiScaleFactorRoundingPolicy()
    policy_string = str(policy).split('.')[-1]
    result = "<p>High DPI scale factor rounding policy: {}</p><ol>".format(policy_string)
    for screen in QGuiApplication.screens():
        current = screen == widget.screen()
        result += "<li>"
        if current:
            result += "<i>"
        result += '"{}" {} {}DPI, DPR={}'.format(screen.name(),
                                                 format_geometry(screen.geometry()),
                                                 int(screen.logicalDotsPerInchX()),
                                                 screen.devicePixelRatio())
        if current:
            result += "</i>"
        result += "</li>"
    result += "</ol>"
    return result


class WidgetGallery(QDialog):
    """Dialog displaying a gallery of Qt Widgets"""

    def __init__(self):
        super(WidgetGallery, self).__init__()

        self._progress_bar = self.create_progress_bar()

        self._style_combobox = QComboBox()
        init_widget(self._style_combobox, "styleComboBox")
        self._style_combobox.addItems(style_names())

        style_label = QLabel("Style:")
        init_widget(style_label, "style_label")
        style_label.setBuddy(self._style_combobox)

        help_label = QLabel("Press F1 over a widget to see Documentation")
        init_widget(help_label, "help_label")

        disable_widgets_checkbox = QCheckBox("Disable widgets")
        init_widget(disable_widgets_checkbox, "disable_widgets_checkbox")

        buttons_groupbox = self.create_buttons_groupbox()
        itemview_tabwidget = self.create_itemview_tabwidget()
        simple_input_widgets_groupbox = self.create_simple_inputwidgets_groupbox()
        text_toolbox = self.create_text_toolbox()

        self._style_combobox.textActivated.connect(self.change_style)
        disable_widgets_checkbox.toggled.connect(buttons_groupbox.setDisabled)
        disable_widgets_checkbox.toggled.connect(text_toolbox.setDisabled)
        disable_widgets_checkbox.toggled.connect(itemview_tabwidget.setDisabled)
        disable_widgets_checkbox.toggled.connect(simple_input_widgets_groupbox.setDisabled)

        help_shortcut = QShortcut(self)
        help_shortcut.setKey(QKeySequence.HelpContents)
        help_shortcut.activated.connect(self.help_on_current_widget)

        top_layout = QHBoxLayout()
        top_layout.addWidget(style_label)
        top_layout.addWidget(self._style_combobox)
        top_layout.addStretch(1)
        top_layout.addWidget(help_label)
        top_layout.addStretch(1)
        top_layout.addWidget(disable_widgets_checkbox)

        dialog_buttonbox = QDialogButtonBox(QDialogButtonBox.Help |
                                            QDialogButtonBox.Close)
        init_widget(dialog_buttonbox, "dialogButtonBox")
        dialog_buttonbox.helpRequested.connect(launch_module_help)
        dialog_buttonbox.rejected.connect(self.reject)

        main_layout = QGridLayout(self)
        main_layout.addLayout(top_layout, 0, 0, 1, 2)
        main_layout.addWidget(buttons_groupbox, 1, 0)
        main_layout.addWidget(simple_input_widgets_groupbox, 1, 1)
        main_layout.addWidget(itemview_tabwidget, 2, 0)
        main_layout.addWidget(text_toolbox, 2, 1)
        main_layout.addWidget(self._progress_bar, 3, 0, 1, 2)
        main_layout.addWidget(dialog_buttonbox, 4, 0, 1, 2)

        self.setWindowTitle("Widget Gallery Qt {}".format(qVersion()))

    def setVisible(self, visible):
        super(WidgetGallery, self).setVisible(visible)
        if visible:
            self.windowHandle().screenChanged.connect(self.update_systeminfo)
            self.update_systeminfo()

    def change_style(self, style_name):
        QApplication.setStyle(QStyleFactory.create(style_name))

    def advance_progressbar(self):
        cur_val = self._progress_bar.value()
        max_val = self._progress_bar.maximum()
        self._progress_bar.setValue(cur_val + (max_val - cur_val) / 100)

    def create_buttons_groupbox(self):
        result = QGroupBox("Buttons")
        init_widget(result, "buttons_groupbox")

        default_pushbutton = QPushButton("Default Push Button")
        init_widget(default_pushbutton, "default_pushbutton")
        default_pushbutton.setDefault(True)

        toggle_pushbutton = QPushButton("Toggle Push Button")
        init_widget(toggle_pushbutton, "toggle_pushbutton")
        toggle_pushbutton.setCheckable(True)
        toggle_pushbutton.setChecked(True)

        flat_pushbutton = QPushButton("Flat Push Button")
        init_widget(flat_pushbutton, "flat_pushbutton")
        flat_pushbutton.setFlat(True)

        toolbutton = QToolButton()
        init_widget(toolbutton, "toolButton")
        toolbutton.setText("Tool Button")

        menu_toolbutton = QToolButton()
        init_widget(menu_toolbutton, "menuButton")
        menu_toolbutton.setText("Menu Button")
        tool_menu = QMenu(menu_toolbutton)
        menu_toolbutton.setPopupMode(QToolButton.InstantPopup)
        tool_menu.addAction("Option")
        tool_menu.addSeparator()
        action = tool_menu.addAction("Checkable Option")
        action.setCheckable(True)
        menu_toolbutton.setMenu(tool_menu)
        tool_layout = QHBoxLayout()
        tool_layout.addWidget(toolbutton)
        tool_layout.addWidget(menu_toolbutton)

        commandlinkbutton = QCommandLinkButton("Command Link Button")
        init_widget(commandlinkbutton, "commandLinkButton")
        commandlinkbutton.setDescription("Description")

        button_layout = QVBoxLayout()
        button_layout.addWidget(default_pushbutton)
        button_layout.addWidget(toggle_pushbutton)
        button_layout.addWidget(flat_pushbutton)
        button_layout.addLayout(tool_layout)
        button_layout.addWidget(commandlinkbutton)
        button_layout.addStretch(1)

        radiobutton_1 = QRadioButton("Radio button 1")
        init_widget(radiobutton_1, "radioButton1")
        radiobutton_2 = QRadioButton("Radio button 2")
        init_widget(radiobutton_2, "radioButton2")
        radiobutton_3 = QRadioButton("Radio button 3")
        init_widget(radiobutton_3, "radioButton3")
        radiobutton_1.setChecked(True)

        checkbox = QCheckBox("Tri-state check box")
        init_widget(checkbox, "checkBox")
        checkbox.setTristate(True)
        checkbox.setCheckState(Qt.PartiallyChecked)

        checkableLayout = QVBoxLayout()
        checkableLayout.addWidget(radiobutton_1)
        checkableLayout.addWidget(radiobutton_2)
        checkableLayout.addWidget(radiobutton_3)
        checkableLayout.addWidget(checkbox)
        checkableLayout.addStretch(1)

        main_layout = QHBoxLayout(result)
        main_layout.addLayout(button_layout)
        main_layout.addLayout(checkableLayout)
        main_layout.addStretch()
        return result

    def create_text_toolbox(self):
        result = QToolBox()
        init_widget(result, "toolBox")

        # Create centered/italic HTML rich text
        rich_text = "<html><head/><body><i>"
        for line in POEM.split('\n'):
            rich_text += "<center>" + line + "</center>"
        rich_text += "</i></body></html>"

        text_edit = QTextEdit(rich_text)
        init_widget(text_edit, "textEdit")
        plain_textedit = QPlainTextEdit(POEM)
        init_widget(plain_textedit, "plainTextEdit")

        self._systeminfo_textbrowser = QTextBrowser()
        init_widget(self._systeminfo_textbrowser, "systemInfoTextBrowser")

        result.addItem(embed_into_hbox_layout(text_edit), "Text Edit")
        result.addItem(embed_into_hbox_layout(plain_textedit),
                       "Plain Text Edit")
        result.addItem(embed_into_hbox_layout(self._systeminfo_textbrowser),
                       "Text Browser")
        return result

    def create_itemview_tabwidget(self):
        result = QTabWidget()
        init_widget(result, "bottomLeftTabWidget")
        result.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Ignored)

        tree_view = QTreeView()
        init_widget(tree_view, "treeView")
        filesystem_model = QFileSystemModel(tree_view)
        filesystem_model.setRootPath(QDir.rootPath())
        tree_view.setModel(filesystem_model)

        table_widget = QTableWidget()
        init_widget(table_widget, "tableWidget")
        table_widget.setRowCount(10)
        table_widget.setColumnCount(10)

        list_model = QStandardItemModel(0, 1, result)

        list_model.appendRow(QStandardItem(QIcon(DIR_OPEN_ICON), "Directory"))
        list_model.appendRow(QStandardItem(QIcon(COMPUTER_ICON), "Computer"))

        list_view = QListView()
        init_widget(list_view, "listView")
        list_view.setModel(list_model)

        icon_mode_listview = QListView()
        init_widget(icon_mode_listview, "iconModeListView")

        icon_mode_listview.setViewMode(QListView.IconMode)
        icon_mode_listview.setModel(list_model)

        result.addTab(embed_into_hbox_layout(tree_view), "Tree View")
        result.addTab(embed_into_hbox_layout(table_widget), "Table")
        result.addTab(embed_into_hbox_layout(list_view), "List")
        result.addTab(embed_into_hbox_layout(icon_mode_listview),
                      "Icon Mode List")
        return result

    def create_simple_inputwidgets_groupbox(self):
        result = QGroupBox("Simple Input Widgets")
        init_widget(result, "bottomRightGroupBox")
        result.setCheckable(True)
        result.setChecked(True)

        lineedit = QLineEdit("s3cRe7")
        init_widget(lineedit, "lineEdit")
        lineedit.setClearButtonEnabled(True)
        lineedit.setEchoMode(QLineEdit.Password)

        spin_box = QSpinBox()
        init_widget(spin_box, "spinBox")
        spin_box.setValue(50)

        date_timeedit = QDateTimeEdit()
        init_widget(date_timeedit, "dateTimeEdit")
        date_timeedit.setDateTime(QDateTime.currentDateTime())

        slider = QSlider()
        init_widget(slider, "slider")
        slider.setOrientation(Qt.Horizontal)
        slider.setValue(40)

        scrollbar = QScrollBar()
        init_widget(scrollbar, "scrollBar")
        scrollbar.setOrientation(Qt.Horizontal)
        scrollbar.setValue(60)

        dial = QDial()
        init_widget(dial, "dial")
        dial.setValue(30)
        dial.setNotchesVisible(True)

        layout = QGridLayout(result)
        layout.addWidget(lineedit, 0, 0, 1, 2)
        layout.addWidget(spin_box, 1, 0, 1, 2)
        layout.addWidget(date_timeedit, 2, 0, 1, 2)
        layout.addWidget(slider, 3, 0)
        layout.addWidget(scrollbar, 4, 0)
        layout.addWidget(dial, 3, 1, 2, 1)
        layout.setRowStretch(5, 1)
        return result

    def create_progress_bar(self):
        result = QProgressBar()
        init_widget(result, "progressBar")
        result.setRange(0, 10000)
        result.setValue(0)

        timer = QTimer(self)
        timer.timeout.connect(self.advance_progressbar)
        timer.start(1000)
        return result

    def update_systeminfo(self):
        """Display system information"""
        system_info = SYSTEMINFO.format(sys.version,
                                        QLibraryInfo.build(),
                                        QSysInfo.prettyProductName(),
                                        screen_info(self))
        self._systeminfo_textbrowser.setHtml(system_info)

    def help_on_current_widget(self):
        """Display help on widget under mouse"""
        w = QApplication.widgetAt(QCursor.pos(self.screen()))
        while w:  # Skip over internal widgets
            name = w.objectName()
            if name and not name.startswith("qt_"):
                launch_help(w)
                break
            w = w.parentWidget()
