Chapter 4 - Add a QTableView
=============================

Now that you have a QMainWindow, you can include a centralWidget to your
interface. Usually, a QWidget is used to display data in most data-driven
applications. Use a table view to display your data.

The first step is to add a horizontal layout with just a QTableView. You
can create a QTableView object and place it inside a QHBoxLayout. Once the
QWidget is properly built, pass the object to the QMainWindow as its central
widget.

Remember that a QTableView needs a model to display information. In this case,
you can use a QAbstractTableModel instance.

.. note:: You could also use the default item model that comes with a
   QTableWidget instead. QTableWidget is a convenience class that reduces
   your codebase considerably as you don't need to implement a data model.
   However, it's less flexible than a QTableView, as QTableWidget cannot be
   used with just any data. For more insight about Qt's model-view framework,
   refer to the
   `Model View Programming <http://doc.qt.io/qt-5/model-view-programming.html>`
   documentation.

Implementing the model for your QTableView, allows you to:
- set the headers,
- manipulate the formats of the cell values (remember we have UTC time and float
numbers),
- set style properties like text alignment,
- and even set color properties for the cell or its content.

To subclass the QAbstractTable, you must reimplement its virtual methods,
rowCount(), columnCount(), and data(). This way, you can ensure that the data
is handled properly. In addition, reimplement the headerData() method to
provide the header information to the view.

Here is a script that implements the CustomTableModel:

.. literalinclude:: datavisualize4/table_model.py
   :language: python
   :linenos:
   :lines: 40-

Now, create a QWidget that has a QTableView, and connect it to your
CustomTableModel.

.. literalinclude:: datavisualize4/main_widget.py
   :language: python
   :linenos:
   :emphasize-lines: 56-61
   :lines: 40-

You also need these minor changes to the :code:`main_window.py` and
:code:`main.py` from chapter 3 to include the Widget inside the
MainWindow:

.. literalinclude:: datavisualize4/main_window.py
   :language: python
   :linenos:
   :diff: datavisualize3/main_window.py

.. literalinclude:: datavisualize4/main.py
   :language: python
   :linenos:
   :diff: datavisualize3/main.py

