
.. _boost-python-generator:

***********************
Boost::Python Generator
***********************

Overview
=========================================

The **Boost::Python Generator** (A.K.A. :program:`boostpythongenerator`) is
the program that creates the bindings source files from Qt headers and
auxiliary files  (typesystems, ``global.h`` and glue files). It makes
heavy use of the :ref:`api-extractor` library.


Getting the sources
===================

* Download URL: http://www.pyside.org/downloads/

Build requirements
==================

+ CMake >= 2.6.0
+ Qt4.5 libraries and development headers >= 4.5.0
+ :ref:`api-extractor` + development headers

Building and installing
=======================

To build and install just follow the generic cmake instructions in
section :ref:`cmake-primer`.

Debian packaging
================

In order to compile this package in a debian environment, make sure the
following packages are installed:

* debhelper (>= 5)
* cdbs
* cmake (>= 2.6.0)
* libqt4-dev (>= 4.5)
* libapiextractor-dev (>= 0.1)

And then you can build the package using::

  $ dpkg-buildpackage -rfakeroot
