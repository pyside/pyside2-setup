Creating a Simple PySide6 Dialog Application
*********************************************

This tutorial shows how to build a simple dialog with some
basic widgets. The idea is to let users provide their name
in a `QLineEdit`, and the dialog greets them on click of a
`QPushButton`.

Let us just start with a simple stub that creates and shows
a dialog. This stub is updated during the course of this
tutorial, but you can use this stub as is if you need to:
::

    import sys
    from PySide6.QtWidgets import QApplication, QDialog, QLineEdit, QPushButton

    class Form(QDialog):

        def __init__(self, parent=None):
            super(Form, self).__init__(parent)
            self.setWindowTitle("My Form")


    if __name__ == '__main__':
        # Create the Qt Application
        app = QApplication(sys.argv)
        # Create and show the form
        form = Form()
        form.show()
        # Run the main Qt loop
        sys.exit(app.exec_())

The imports aren't new to you, the same for the creation of the
`QApplication` and the execution of the Qt main loop.
The only novelty here is the **class definition**.

You can create any class that subclasses PySide6 widgets.
In this case, we are subclassing `QDialog` to define a custom
dialog, which we name as **Form**. We have also implemented the
`init()` method that calls the `QDialog`'s init method with the
parent widget, if any. Also, the new `setWindowTitle()` method
just sets the title of the dialog window. In `main()`, you can see
that we are creating a *Form object* and showing it to the world.

Create the Widgets
===================

We are going to create two widgets: a `QLineEdit` where users can
enter their name, and a `QPushButton` that prints the contents of
the `QLineEdit`.
So, let's add the following code to the `init()` method of our Form:
::

    # Create widgets
    self.edit = QLineEdit("Write my name here..")
    self.button = QPushButton("Show Greetings")

It's obvious from the code that both widgets will show the corresponding
texts.

Create a layout to organize the Widgets
========================================

Qt comes with layout-support that helps you organize the widgets
in your application. In this case, let's use `QVBoxLayout` to lay out
the widgets vertically. Add the following code to the `init()` method,
after creating the widgets:
::

    # Create layout and add widgets
    layout = QVBoxLayout()
    layout.addWidget(self.edit)
    layout.addWidget(self.button)
    # Set dialog layout
    self.setLayout(layout)

So, we create the layout, add the widgets with `addWidget()`,
and finally we say that our **Form** will have our `QVBoxLayout`
as its layout.

Create the function to greet and connect the Button
====================================================

Finally, we just have to add a function to our custom **Form**
and *connect* our button to it. Our function will be a part of
the Form, so you have to add it after the `init()` function:
::

    # Greets the user
    def greetings(self):
        print ("Hello {}".format(self.edit.text()))

Our function just prints the contents of the `QLineEdit` to the
python console. We have access to the text by means of the
`QLineEdit.text()` method.

Now that we have everything, we just need to *connect* the
`QPushButton` to the `Form.greetings()` method. To do so, add the
following line to the `init()` method:
::

    # Add button signal to greetings slot
    self.button.clicked.connect(self.greetings)

Once executed, you can enter your name in the `QLineEdit` and watch
the console for greetings.

Complete code
=============

Here is the complete code for this tutorial:
::

    import sys
    from PySide6.QtWidgets import (QLineEdit, QPushButton, QApplication,
        QVBoxLayout, QDialog)

    class Form(QDialog):

        def __init__(self, parent=None):
            super(Form, self).__init__(parent)
            # Create widgets
            self.edit = QLineEdit("Write my name here")
            self.button = QPushButton("Show Greetings")
            # Create layout and add widgets
            layout = QVBoxLayout()
            layout.addWidget(self.edit)
            layout.addWidget(self.button)
            # Set dialog layout
            self.setLayout(layout)
            # Add button signal to greetings slot
            self.button.clicked.connect(self.greetings)

        # Greets the user
        def greetings(self):
            print ("Hello %s" % self.edit.text())

    if __name__ == '__main__':
        # Create the Qt Application
        app = QApplication(sys.argv)
        # Create and show the form
        form = Form()
        form.show()
        # Run the main Qt loop
        sys.exit(app.exec_())
