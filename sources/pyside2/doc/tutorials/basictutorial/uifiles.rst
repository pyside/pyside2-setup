Using UI Files
***************

This page describes the use of Qt Creator to create graphical
interfaces for your Qt for Python project.
You will need **Qt Creator** to design and modify your interface (UI file).

If you don't know how to use Qt Creator, refer to the
`Using Qt Designer <http://doc.qt.io/qtcreator/creator-using-qt-designer.html>`_
documentation page.

At Qt Creator, create a new Qt Design Form, choose "Main Window" for template.
And save as `mainwindow.ui`.
Add a `QPushButton` to the center of the centralwidget.

Your file (mainwindow.ui) should look something like this:
::
    <?xml version="1.0" encoding="UTF-8"?>
    <ui version="4.0">
     <class>MainWindow</class>
     <widget class="QMainWindow" name="MainWindow">
      <property name="geometry">
       <rect>
        <x>0</x>
        <y>0</y>
        <width>400</width>
        <height>300</height>
       </rect>
      </property>
      <property name="windowTitle">
       <string>MainWindow</string>
      </property>
      <widget class="QWidget" name="centralWidget">
       <widget class="QPushButton" name="pushButton">
        <property name="geometry">
         <rect>
          <x>110</x>
          <y>80</y>
          <width>201</width>
          <height>81</height>
         </rect>
        </property>
        <property name="text">
         <string>PushButton</string>
        </property>
       </widget>
      </widget>
      <widget class="QMenuBar" name="menuBar">
       <property name="geometry">
        <rect>
         <x>0</x>
         <y>0</y>
         <width>400</width>
         <height>20</height>
        </rect>
       </property>
      </widget>
      <widget class="QToolBar" name="mainToolBar">
       <attribute name="toolBarArea">
        <enum>TopToolBarArea</enum>
       </attribute>
       <attribute name="toolBarBreak">
        <bool>false</bool>
       </attribute>
      </widget>
      <widget class="QStatusBar" name="statusBar"/>
     </widget>
     <layoutdefault spacing="6" margin="11"/>
     <resources/>
     <connections/>
    </ui>

Now we are ready to decide how to use the **UI file** from Python.

Generating a Python class
=========================

Another option to interact with a **UI file** is to generate a Python
class from it. This is possible thanks to the `pyside2-uic` tool.
To use this tool, you need to run the following command on a console:
::
    pyside2-uic mainwindow.ui > ui_mainwindow.py

We redirect all the output of the command to a file called `ui_mainwindow.py`,
which will be imported directly:
::
    from ui_mainwindow import Ui_MainWindow

Now to use it, we should create a personalized class for our widget
to **setup** this generated design.

To understand the idea, let's take a look at the whole code:
::
    import sys
    from PySide2.QtWidgets import QApplication, QMainWindow
    from PySide2.QtCore import QFile
    from ui_mainwindow import Ui_MainWindow

    class MainWindow(QMainWindow):
        def __init__(self):
            super(MainWindow, self).__init__()
            self.ui = Ui_MainWindow()
            self.ui.setupUi(self)

    if __name__ == "__main__":
        app = QApplication(sys.argv)

        window = MainWindow()
        window.show()

        sys.exit(app.exec_())

What is inside the *if* statement is already known from the previous
examples, and our new basic class contains only two new lines
that are in charge of loading the generated python class from the UI
file:
::
    self.ui = Ui_MainWindow()
    self.ui.setupUi(self)

.. note:: You must run `pyside2-uic` again every time you make changes
to the **UI file**.

Loading it directly
====================

To load the UI file directly, we will need a class from the **QtUiTools**
module:
::
    from PySide2.QtUiTools import QUiLoader

The `QUiLoader` lets us load the **ui file** dynamically
and use it right away:
::
    ui_file = QFile("mainwindow.ui")
    ui_file.open(QFile.ReadOnly)

    loader = QUiLoader()
    window = loader.load(ui_file)
    window.show()

The complete code of this example looks like this:
::
    # File: main.py
    import sys
    from PySide2.QtUiTools import QUiLoader
    from PySide2.QtWidgets import QApplication
    from PySide2.QtCore import QFile

    if __name__ == "__main__":
        app = QApplication(sys.argv)

        ui_file = QFile("mainwindow.ui")
        ui_file.open(QFile.ReadOnly)

        loader = QUiLoader()
        window = loader.load(ui_file)
        ui_file.close()
        window.show()

        sys.exit(app.exec_())

Then to execute it we just need to run the following on a
command prompt:
::
    python main.py
