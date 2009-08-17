**************************
Frequently Asked Questions
**************************

This is a list of Frequently Asked Questions about |project|.  Feel free to
suggest new entries!

General
=======

What is the generator?
----------------------

Here the name generator refers actually to a program composed of a set of
*generator classes* that output different resources based on information
contained inside C++ header files.

What is the API Extractor?
--------------------------

It is a library that parses C++ header files and builds a data model around
them in order to ease the work of manipulating these data inside
*generators*.


Is there any similar tools around?
----------------------------------

The generator framework actually started as a fork of the qtscriptgenerator,
with the focus on python bindings instead of QtScript. After some time, the
python-specific code was split from the the header parsing and data model
code. The former became what we call *generator* while the latter is now
called *API Extractor*.

What's the relationship between the generator and the API Extractor?
--------------------------------------------------------------------

The generator system relies heavily in the API Extractor classes, using
them as syntatic sugar to access the data model of the classes being
wrapped.

What are the dependencies to run the generator?
-----------------------------------------------

API Extractor, QtCore and QtXml.

Creating bindings
=================

Can I wrap non-Qt libraries?
----------------------------

Although it's not yet well tested, there's a good chance that non-Qt 
libraries can be wrapped using the generator. But remember that
generator injects runtime dependency on Qt for the generated binding.

Is there any runtime dependency on the generated binding?
---------------------------------------------------------

Yes. Only libshiboken, and the obvious Python interpreter
and the C++ library that is being wrapped.

What do I have to do to create my bindings?
-------------------------------------------

.. todo: put link to typesystem documentation

Most of the work is already done by the API Extractor. The developer creates
a typesystem file with any customization wanted in the generated code, like
removing classes or changing method signatures. The generator will output
the .h and .cpp files with the CPython code that will wrap the target
library for python.

Is there any recommended build system?
--------------------------------------

Both API Extractor and generator uses and recommends the CMake build system.

Can I write closed-source bindings with the generator?
------------------------------------------------------

Yes, as long as you use a LGPL version of Qt, due to runtime requirements.

What is 'inject code'?
----------------------

That's how we call customized code that will be *injected* into the
generated at specific locations. They are specified inside the typesytem.

How can I document my project?
------------------------------

The generator also can generate the API documentation based on the
C++ headers documentation using the qdoc syntax. Optionally you can
inject documentation at specific parts. Likewise *inject code*, the
customized documentation is specified inside the typesystem.

Other
=====

Is there any current limitation within the generator/API Extractor?
-------------------------------------------------------------------

The generator currently does not automatically detects implicit C++
type conversions. Also the code snippets in function signature and
examples are still in C++ inside the generated documentation.

