.. currentmodule:: PySide6.QtCore
.. _Property:
Property
********

Detailed Description
--------------------

The Property function lets you declare properties that
behave both as Qt and Python properties, and have their
setters and getters defined as Python functions.

Here is an example that illustrates how to use this
function:

.. code-block::
   :linenos:

    from PySide6.QtCore import QObject, Property

    class MyObject(QObject):
        def __init__(self,startval=42):
            QObject.__init__(self)
            self.ppval = startval

        def readPP(self):
            return self.ppval

        def setPP(self,val):
            self.ppval = val

        pp = Property(int, readPP, setPP)

    obj = MyObject()
    obj.pp = 47
    print(obj.pp)

Properties in QML expressions
-----------------------------

If you are using properties of your objects in QML expressions,
QML requires that the property changes are notified. Here is an
example illustrating how to do this:

.. code-block::
   :linenos:

    from PySide6.QtCore import QObject, Signal, Property

    class Person(QObject):
        def __init__(self, name):
            QObject.__init__(self)
            self._person_name = name

        def _name(self):
            return self._person_name

        @Signal
        def name_changed(self):
            pass

        name = Property(str, _name, notify=name_changed)
