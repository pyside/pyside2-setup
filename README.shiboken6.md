# Shiboken6 module

The purpose of the [shiboken6 Python module](https://wiki.qt.io/Qt_for_Python/Shiboken)
is to access information related to the binding generation that could be used to integrate
C++ programs to Python, or even to get useful information to debug
an application.

Mostly the idea is to interact with Shiboken objects,
where one can check if it is valid, or if the generated Python wrapper
is invalid after the underlying C++ object has been destroyed.

More information on the available functions can be found
in our [official documentation](https://doc.qt.io/qtforpython/shiboken6/shibokenmodule.html)
