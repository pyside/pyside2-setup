# Shiboken6-generator

Shiboken is the generator used by the Qt for Python project.  It outputs C++
code for CPython extensions, which can be compiled and transformed into
a Python module.

C++ projects based on Qt can be wrapped, but also projects which are not
related to Qt.

## How does it work?

Shiboken uses an API Extractor that does most of the job, but it requires
a typesystem (XML file) to customize how the C++ classes/methods will be
exposed to Python.

The typesystem allows you to remove arguments from signatures, modify return
types, inject code and add conversion rules from the C++ data types to Python
data types, manipulate the ownership of the objects, etc.

# Examples

An example related to wrap a C++ library not depending on Qt can be found in
our [repository](https://code.qt.io/cgit/pyside/pyside-setup.git/tree/examples/samplebinding).

Additionally, you can find a couple of tests inside the
[git repository](https://code.qt.io/cgit/pyside/pyside-setup.git/tree/sources/shiboken6/tests).

For a more advanced case regarding extending a Qt/C++ application with Python
bindings based on the idea of the PySide module, you can check the
[scriptableapplication](https://code.qt.io/cgit/pyside/pyside-setup.git/tree/examples/scriptableapplication)
example in our repository.

# Documentation

You can find more information about Shiboken in our
[official documentation page](https://doc.qt.io/qtforpython/shiboken6/).
