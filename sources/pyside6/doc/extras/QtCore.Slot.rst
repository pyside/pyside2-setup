.. currentmodule:: PySide6.QtCore
.. _Slot:

Slot
****

Detailed Description
--------------------

    PySide6 adopt PyQt5's new signal and slot syntax as-is. The PySide6
    implementation is functionally compatible with the PyQt5 one, with the
    exceptions listed below.

    PyQt5's new signal and slot style utilizes method and decorator names
    specific to their implementation. These will be generalized according to
    the table below:

    =======  =======================  =============
    Module   PyQt5 factory function   PySide6 class
    =======  =======================  =============
    QtCore   pyqtSignal               Signal
    QtCore   pyqtSlot                 Slot
    =======  =======================  =============

Q_INVOKABLE
-----------

    There is no equivalent of the Q_INVOKABLE macro of Qt
    since PySide6 slots can actually have return values.
    If you need to create a invokable method that returns some value,
    declare it as a slot, e.g.:

    ::

        class Foo(QObject):

            @Slot(float, result=int)
            def getFloatReturnInt(self, f):
                return int(f)
