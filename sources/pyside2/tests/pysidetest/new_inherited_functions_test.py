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

import sys
import os
import unittest

from PySide2 import *
for modname, mod in sys.modules.items():
    # Python 2 leaves "None" in the dict.
    if modname.startswith("PySide2.") and mod is not None:
        print("importing", modname)
        exec("import " + modname)

# This test tests the existence and callability of the newly existing functions,
# after the inheritance was made complete in the course of PYSIDE-331.

new_functions = """
    PySide2.QtCore.QAbstractItemModel().parent()
    PySide2.QtCore.QAbstractListModel().parent()
    PySide2.QtCore.QAbstractTableModel().parent()
    PySide2.QtCore.QFile().resize(qint64)
    m = PySide2.QtCore.QMutex(); m.tryLock(); m.unlock() # prevent a message "QMutex: destroying locked mutex"
    PySide2.QtCore.QSortFilterProxyModel().parent()
    PySide2.QtCore.QTemporaryFile(tfarg).open(openMode)
"""

new_functions += """
    PySide2.QtGui.QBitmap().transformed(qMatrix,transformationMode)
    PySide2.QtGui.QStandardItemModel().insertColumn(int,qModelIndex)
    PySide2.QtGui.QStandardItemModel().parent()
    # PySide2.QtGui.QTextList(qTextDocument).setFormat(qTextFormat) # Segmentation fault: 11
    # PySide2.QtGui.QTextTable(qTextDocument).setFormat(qTextFormat) # Segmentation fault: 11
""" if "PySide2.QtGui" in sys.modules else ""

new_functions += """
    PySide2.QtWidgets.QAbstractItemView().update()
    PySide2.QtWidgets.QApplication.palette()
    PySide2.QtWidgets.QApplication.setFont(qFont)
    PySide2.QtWidgets.QApplication.setPalette(qPalette)
    PySide2.QtWidgets.QBoxLayout(direction).addWidget(qWidget)
    PySide2.QtWidgets.QColorDialog().open()
    PySide2.QtWidgets.QDirModel().index(int,int,qModelIndex)
    PySide2.QtWidgets.QDirModel().parent()
    PySide2.QtWidgets.QFileDialog().open()
    PySide2.QtWidgets.QFileSystemModel().index(int,int,qModelIndex)
    PySide2.QtWidgets.QFileSystemModel().parent()
    PySide2.QtWidgets.QFontDialog().open()
    PySide2.QtWidgets.QGestureEvent([]).accept()
    PySide2.QtWidgets.QGestureEvent([]).ignore()
    PySide2.QtWidgets.QGestureEvent([]).isAccepted()
    PySide2.QtWidgets.QGestureEvent([]).setAccepted(bool)
    # PySide2.QtWidgets.QGraphicsView().render(qPaintDevice,qPoint,qRegion,renderFlags) # QPaintDevice: NotImplementedError
    PySide2.QtWidgets.QGridLayout().addWidget(qWidget)
    PySide2.QtWidgets.QHeaderView(orientation).initStyleOption(qStyleOptionFrame)
    PySide2.QtWidgets.QInputDialog().open()
    PySide2.QtWidgets.QLineEdit().addAction(qAction)
    PySide2.QtWidgets.QListWidget().closePersistentEditor(qModelIndex)
    PySide2.QtWidgets.QListWidget().openPersistentEditor(qModelIndex)
    PySide2.QtWidgets.QMessageBox().open()
    PySide2.QtWidgets.QPlainTextEdit.find(quintptr)
    PySide2.QtWidgets.QProgressDialog().open()
    PySide2.QtWidgets.QStackedLayout().widget()
    # PySide2.QtWidgets.QStylePainter().begin(qPaintDevice) # QPaintDevice: NotImplementedError
    PySide2.QtWidgets.QTableWidget().closePersistentEditor(qModelIndex)
    PySide2.QtWidgets.QTableWidget().openPersistentEditor(qModelIndex)
    PySide2.QtWidgets.QTextEdit.find(quintptr)
    PySide2.QtWidgets.QTreeWidget().closePersistentEditor(qModelIndex)
    PySide2.QtWidgets.QTreeWidget().openPersistentEditor(qModelIndex)
""" if "PySide2.QtWidgets" in sys.modules else ""

new_functions += """
    # PySide2.QtPrintSupport.QPageSetupDialog().open() # Segmentation fault: 11
    # PySide2.QtPrintSupport.QPrintDialog().open() # opens the dialog, but works
    PySide2.QtPrintSupport.QPrintDialog().printer()
    PySide2.QtPrintSupport.QPrintPreviewDialog().open() # note: this prints something, but really shouldn't ;-)
""" if "PySide2.QtPrintSupport" in sys.modules else ""

new_functions += """
    PySide2.QtHelp.QHelpContentModel().parent()
    # PySide2.QtHelp.QHelpIndexModel().createIndex(int,int,quintptr) # returned NULL without setting an error
    # PySide2.QtHelp.QHelpIndexModel().createIndex(int,int,object()) # returned NULL without setting an error
""" if "PySide2.QtHelp" in sys.modules else ""

new_functions += """
    PySide2.QtQuick.QQuickPaintedItem().update()
""" if "PySide2.QtQuick" in sys.modules else ""


class MainTest(unittest.TestCase):

    def testNewInheriedFunctionsExist(self):
        """
        Run all new method signarures
        """
        for app in "QtWidgets.QApplication", "QtGui.QGuiApplication", "QtCore.QCoreApplication":
            try:
                exec("qApp = PySide2.{0}([]) or PySide2.{0}.instance()".format(app))
                break
            except AttributeError:
                continue
        bool = True
        int = 42
        qint64 = 42
        tfarg = os.path.join(PySide2.QtCore.QDir.tempPath(), "XXXXXX.tmp")
        orientation = PySide2.QtCore.Qt.Orientation()
        openMode = PySide2.QtCore.QIODevice.OpenMode(PySide2.QtCore.QIODevice.ReadOnly)
        qModelIndex = PySide2.QtCore.QModelIndex()
        transformationMode = PySide2.QtCore.Qt.TransformationMode()
        qObject = PySide2.QtCore.QObject()
        qPoint = PySide2.QtCore.QPoint()
        try:
            PySide2.QtGui
            #qPaintDevice = PySide2.QtGui.QPaintDevice()  # NotImplementedError
            qMatrix = PySide2.QtGui.QMatrix()
            qTextDocument = PySide2.QtGui.QTextDocument()
            qTextFormat = PySide2.QtGui.QTextFormat()
            quintptr = long(42) if sys.version_info[0] < 3 else 42
            qFont = PySide2.QtGui.QFont()
            qPalette = PySide2.QtGui.QPalette()
        except AttributeError:
            pass
        try:
            PySide2.QtWidgets
            direction = PySide2.QtWidgets.QBoxLayout.Direction()
            qWidget = PySide2.QtWidgets.QWidget()
            qStyleOptionFrame = PySide2.QtWidgets.QStyleOptionFrame()
            qAction = PySide2.QtWidgets.QAction(qObject)
            renderFlags = PySide2.QtWidgets.QWidget.RenderFlags
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
            qApp = (PySide2.QtWidgets.QApplication.instance() or
                    PySide2.QtWidgets.QApplication([]))
        except AttributeError:
            unittest.TestCase().skipTest("this test makes only sense if QtWidgets is available.")
        try:
            PySide2.QtWidgets.QApplication.palette(42) # raises
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
