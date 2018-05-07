.. _pysideapi2:

|pymodname| API
***************

One of the goals of |pymodname| is to be API compatible with PyQt5,
with certain exceptions. For example, |pymodname| will not export C++ components
that are marked as deprecated by Qt.

__hash__() function return value
================================

The hash value returned for the classes :class:`PySide2.QtCore.QDate`, :class:`PySide2.QtCore.QDateTime`, :class:`PySide2.QtCore.QTime`, :class:`PySide2.QtCore.QUrl` will be
based on their string representations, thus objects with the same value will
produce the same hash.


QString
=======

Methods that change QString arguments
-------------------------------------

Methods and functions that change the contents of a QString argument were
modified to receive an immutable Python unicode (or str) and return another
Python unicode/str as the modified string.

The following methods had their return types modified this way:

**Classes:** QAbstractSpinBox, QDateTimeEdit, QDoubleSpinBox, QSpinBox, QValidator

- *fixup(string)*: string
- *validate(string, int)*: [QValidator.State, string, int]


**Classes:** QDoubleValidator, QIntValidator, QRegExpValidator

- *validate(string, int)*: [QValidator.State, string, int]

**Class:** QClipboard

- *text(string, QClipboard.Mode mode=QClipboard.Clipboard)*: [string, string]


**Class:** QFileDialog

Instead of *getOpenFileNameAndFilter()*, *getOpenFileNamesAndFilter()* and *getSaveFileNameAndFilter()* like PyQt4 does,
PySide has modified the original methods to return a tuple.

- *getOpenFileName(QWidget parent=None, str caption=None, str dir=None, str filter=None, QFileDialog.Options options=0)*: [string, filter]
- *getOpenFileNames(QWidget parent=None, str caption=None, str dir=None, str filter=None, QFileDialog.Options options=0)*: [list(string), filter]
- *getSaveFileName(QWidget parent=None, str caption=None, str dir=None, str filter=None, QFileDialog.Options options=0)*: [string, filter]

**Class:** QWebPage

- *javaScriptPrompt(QWebFrame, string, string)*: [bool, string]

Other QString related changes
-----------------------------

**Classes:** QFontMetrics and QFontMetricsF

They had two new methods added. Both take a string of one character and convert to a QChar (to call the C++ counterpart):

- widthChar(string)
- boundingRectChar(string)


QTextStream
===========

Inside this class some renames were applied to avoid clashes with native Python functions. They are: *bin_()*, *hex_()* and *oct_()*.
The only modification was the addition of '_' character.


QVariant
========

As QVariant was removed, any function expecting it can receive any Python object (None is an invalid QVariant). The same rule is valid when returning something: the returned QVariant will be converted to the its original Python object type.

When a method expects a *QVariant::Type* the programmer can use a string (the type name) or the type itself.
