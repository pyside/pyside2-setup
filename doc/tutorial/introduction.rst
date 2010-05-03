Binding Generation Tutorial
***************************

This tutorial intends to describe the process of binding creation with
Shiboken and using a very simple Qt4 based library will be used as an
example.

The image below shows the inputs needed to generate the binding source code.

.. image:: images/generatorworkings.png

Putting in words, the user provides the headers for the library along with a
`typesystem <http://www.pyside.org/docs/apiextractor/typesystem.html>`_ file
describing how the classes will be exposed in the target language, as well as
any needed custom source code to be merged with the generated source code.

This tutorial will go through the steps needed to have the binding
ready to be imported and used from a Python program. The tutorial
source code is available as a tar ball `here <../_static/bindingexample.tar.gz>`_.

**NOTE:** the binding generator is intended to be used with Qt4 based libraries
only, at least for the time being.

.. toctree::
    :maxdepth: 3

    libfoo
    bindinglibfoo
    typesystemcreation
    globalheader
    buildingthebinding

