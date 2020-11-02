#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

from __future__ import print_function

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import *
for modname, mod in sys.modules.items():
    # Python 2 leaves "None" in the dict.
    if modname.startswith("PySide6.") and mod is not None:
        print("importing", modname)
        exec("import " + modname)

# This test tests the existence and callability of the newly existing functions,
# after the inheritance was made complete in the course of PYSIDE-331.

new_functions = """
    PySide6.QtCore.QAbstractItemModel().parent()
    PySide6.QtCore.QAbstractListModel().parent()
    PySide6.QtCore.QAbstractTableModel().parent()
    PySide6.QtCore.QFile().resize(qint64)
    m = PySide6.QtCore.QMutex(); m.tryLock(); m.unlock() # prevent a message "QMutex: destroying locked mutex"
    PySide6.QtCore.QSortFilterProxyModel().parent()
    PySide6.QtCore.QTemporaryFile(tfarg).open(openMode)
"""

new_functions += """
    PySide6.QtGui.QStandardItemModel().insertColumn(int,qModelIndex)
    PySide6.QtGui.QStandardItemModel().parent()
    # PySide6.QtGui.QTextList(qTextDocument).setFormat(qTextFormat) # Segmentation fault: 11
    # PySide6.QtGui.QTextTable(qTextDocument).setFormat(qTextFormat) # Segmentation fault: 11
""" if "PySide6.QtGui" in sys.modules else ""

new_functions += """
    PySide6.QtWidgets.QAbstractItemView().update()
    PySide6.QtWidgets.QApplication.palette()
    PySide6.QtWidgets.QApplication.setFont(qFont)
    PySide6.QtWidgets.QApplication.setPalette(qPalette)
    PySide6.QtWidgets.QBoxLayout(direction).addWidget(qWidget)
    PySide6.QtWidgets.QColorDialog().open()
    PySide6.QtWidgets.QFileDialog().open()
    PySide6.QtWidgets.QFileSystemModel().index(int,int,qModelIndex)
    PySide6.QtWidgets.QFileSystemModel().parent()
    PySide6.QtWidgets.QFontDialog().open()
    PySide6.QtWidgets.QGestureEvent([]).accept()
    PySide6.QtWidgets.QGestureEvent([]).ignore()
    PySide6.QtWidgets.QGestureEvent([]).isAccepted()
    PySide6.QtWidgets.QGestureEvent([]).setAccepted(bool)
    # PySide6.QtWidgets.QGraphicsView().render(qPaintDevice,qPoint,qRegion,renderFlags) # QPaintDevice: NotImplementedError
    PySide6.QtWidgets.QGridLayout().addWidget(qWidget)
    PySide6.QtWidgets.QHeaderView(orientation).initStyleOption(qStyleOptionFrame)
    PySide6.QtWidgets.QInputDialog().open()
    PySide6.QtWidgets.QLineEdit().addAction(qAction)
    PySide6.QtWidgets.QListWidget().closePersistentEditor(qModelIndex)
    PySide6.QtWidgets.QListWidget().openPersistentEditor(qModelIndex)
    PySide6.QtWidgets.QMessageBox().open()
    PySide6.QtWidgets.QPlainTextEdit().find(findStr)
    PySide6.QtWidgets.QProgressDialog().open()
    PySide6.QtWidgets.QStackedLayout().widget()
    # PySide6.QtWidgets.QStylePainter().begin(qPaintDevice) # QPaintDevice: NotImplementedError
    PySide6.QtWidgets.QTableWidget().closePersistentEditor(qModelIndex)
    PySide6.QtWidgets.QTableWidget().openPersistentEditor(qModelIndex)
    PySide6.QtWidgets.QTextEdit().find(findStr)
    PySide6.QtWidgets.QTreeWidget().closePersistentEditor(qModelIndex)
    PySide6.QtWidgets.QTreeWidget().openPersistentEditor(qModelIndex)
    PySide6.QtWidgets.QWidget.find(quintptr)
""" if "PySide6.QtWidgets" in sys.modules else ""

new_functions += """
    # PySide6.QtPrintSupport.QPageSetupDialog().open() # Segmentation fault: 11
    # PySide6.QtPrintSupport.QPrintDialog().open() # opens the dialog, but works
    PySide6.QtPrintSupport.QPrintDialog().printer()
    PySide6.QtPrintSupport.QPrintPreviewDialog().open() # note: this prints something, but really shouldn't ;-)
""" if "PySide6.QtPrintSupport" in sys.modules else ""

new_functions += """
    PySide6.QtHelp.QHelpContentModel().parent()
    # PySide6.QtHelp.QHelpIndexModel().createIndex(int,int,quintptr) # returned NULL without setting an error
    # PySide6.QtHelp.QHelpIndexModel().createIndex(int,int,object()) # returned NULL without setting an error
""" if "PySide6.QtHelp" in sys.modules else ""

new_functions += """
    PySide6.QtQuick.QQuickPaintedItem().update()
""" if "PySide6.QtQuick" in sys.modules else ""


class MainTest(unittest.TestCase):

    def testNewInheriedFunctionsExist(self):
        """
        Run all new method signarures
        """
        for app in "QtWidgets.QApplication", "QtGui.QGuiApplication", "QtCore.QCoreApplication":
            try:
                exec("qApp = PySide6.{0}([]) or PySide6.{0}.instance()".format(app))
                break
            except AttributeError:
                continue
        bool = True
        int = 42
        qint64 = 42
        tfarg = os.path.join(PySide6.QtCore.QDir.tempPath(), "XXXXXX.tmp")
        findStr = 'bla'
        orientation = PySide6.QtCore.Qt.Orientation()
        openMode = PySide6.QtCore.QIODevice.OpenMode(PySide6.QtCore.QIODevice.ReadOnly)
        qModelIndex = PySide6.QtCore.QModelIndex()
        transformationMode = PySide6.QtCore.Qt.TransformationMode()
        qObject = PySide6.QtCore.QObject()
        qPoint = PySide6.QtCore.QPoint()
        try:
            PySide6.QtGui
            #qPaintDevice = PySide6.QtGui.QPaintDevice()  # NotImplementedError
            qTextDocument = PySide6.QtGui.QTextDocument()
            qTextFormat = PySide6.QtGui.QTextFormat()
            quintptr = 42
            qFont = PySide6.QtGui.QFont()
            qPalette = PySide6.QtGui.QPalette()
        except AttributeError:
            pass
        try:
            PySide6.QtWidgets
            direction = PySide6.QtWidgets.QBoxLayout.Direction()
            qWidget = PySide6.QtWidgets.QWidget()
            qStyleOptionFrame = PySide6.QtWidgets.QStyleOptionFrame()
            qAction = PySide6.QtGui.QAction(qObject)
            renderFlags = PySide6.QtWidgets.QWidget.RenderFlags
        except AttributeError:
            pass

        for func in new_functions.splitlines():
            func = func.strip()
            if func.startswith("#"):
                # this is a crashing or otherwise untestable function
                print(func)
                continue
            try:
                exec(func)
            except NotImplementedError:
                print(func, "# raises NotImplementedError")
            else:
                print(func)

    def testQAppSignatures(self):
        """
        Verify that qApp.palette owns three signatures, especially
        palette() without argument.
        """
        try:
            qApp = (PySide6.QtWidgets.QApplication.instance() or
                    PySide6.QtWidgets.QApplication([]))
        except AttributeError:
            unittest.TestCase().skipTest("this test makes only sense if QtWidgets is available.")
        try:
            PySide6.QtWidgets.QApplication.palette(42) # raises
        except TypeError as e:
            lines = e.args[0].splitlines()
        heading_pos = lines.index("Supported signatures:")
        lines = lines[heading_pos+1:]
        self.assertEqual(len(lines), 3)
        txt = '\n'.join(lines)
        print("Signatures found:")
        print(txt)
        self.assertTrue("palette()" in txt)

if __name__ == '__main__':
    unittest.main()
