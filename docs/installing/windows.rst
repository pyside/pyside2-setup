.. _installing_windows:

Installing PySide on a Windows System
=====================================

Installing prerequisites
------------------------

Install latest ``pip`` distribution: download `get-pip.py
<https://bootstrap.pypa.io/get-pip.py>`_ and run it using
the ``python`` interpreter.

Installing PySide
-----------------

To install PySide on Windows you can choose from the following options:

#. Use pip to install the ``wheel`` binary packages:

   ::

      pip install -U PySide

#. Use setuptools to install the ``egg`` binary packages (deprecated):

   ::

      easy_install -U PySide

.. note::

   Provided binaries are without any other external dependencies.
   All required Qt libraries, development tools and examples are included.
