Shiboken the Binding Generator
*******************************

Shiboken is the CPython-based binding code generator for C or C++ libraries.
It uses an ApiExtractor library to parse the C or C++ headers and get the
type information, using Clang. You can also use this library to parse non-Qt
projects. The following diagram summarizes Shiboken's role in the Qt for Python
project.

.. image:: images/qtforpython-underthehood.png

An XML typesystem file is used to specify the types to be exposed to Python
and to apply modifications to properly represent and manipulate the types in
the Python World. For example, you can remove and add methods to certain types,
and also modify the arguments of each method. These actions are inevitable to
properly handle the data structures or types.

The final outcome of this process is a set of wrappers written in CPython,
which can be used as a module in your Python code.

Table of Contents
*****************

.. toctree::
   :maxdepth: 1

   overview.rst
   samplebinding.rst
   commandlineoptions.rst
   projectfile.rst
   typesystemvariables.rst
   typeconverters.rst
   codeinjectionsemantics.rst
   sequenceprotocol.rst
   ownership.rst
   wordsofadvice.rst
   shibokenmodule.rst
   faq.rst
   typesystem.rst
