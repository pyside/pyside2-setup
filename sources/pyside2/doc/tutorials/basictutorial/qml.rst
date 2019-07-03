Your First Application Using PySide2 and QtQuick/QML
*****************************************************

QML is a declarative language that lets you develop applications
faster than with traditional languages. It is ideal for designing the
UI of your application because of its declarative nature. In QML, a
user interface is specified as a tree of objects with properties. In
this tutorial, we will show how to make a simple "Hello World"
application with PySide2 and QML.

A PySide2/QML application consists, at least, of two different files -
a file with the QML description of the user interface, and a python file
that loads the QML file. To make things easier, let's save both files in
the same directory.

Here is a simple QML file called `view.qml`:
::
    import QtQuick 2.0

    Rectangle {
        width: 200
        height: 200
        color: "green"

        Text {
            text: "Hello World"
            anchors.centerIn: parent
        }
    }

We start by importing `QtQuick 2.0`, which is a QML module.

The rest of the QML code is pretty straightforward for those who
have previously used HTML or XML files. Basically, we are creating
a green rectangle with the size `200*200`, and adding a Text element
that reads "Hello World". The code `anchors.centerIn: parent` makes
the text appear centered in relation to its immediate parent, which
is the Rectangle in this case.

Now, let's see how the code looks on the PySide2.
Let's call it `main.py`:
::
    from PySide2.QtWidgets import QApplication
    from PySide2.QtQuick import QQuickView
    from PySide2.QtCore import QUrl

    app = QApplication([])
    view = QQuickView()
    url = QUrl("view.qml")

    view.setSource(url)
    view.show()
    app.exec_()

If you are already familiar with PySide2 and have followed our
tutorials, you have already seen much of this code.
The only novelties are that you must `import QtQuick` and set the
source of the `QQuickView` object to the URL of your QML file.
Then, as any Qt widget, you call `QQuickView.show()`.

.. note:: If you are programming for desktop, you should consider
    adding `view.setResizeMode(QQuickView.SizeRootObjectToView)`
    before showing the view.
