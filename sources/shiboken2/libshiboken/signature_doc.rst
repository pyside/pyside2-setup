*************************
The signature C extension
*************************

This module is a C extension for CPython 3.5 and up, and CPython 2.7.
Its purpose is to provide support for the ``__signature__`` attribute
of builtin PyCFunction objects.


Short Introduction to the Topic
===============================

Beginning with CPython 3.5, Python functions began to grow a ``__signature__``
attribute for normal Python functions. This is totally optional and just
a nice-to-have feature in Python.

PySide, on the other hand, could use ``__signature__`` very much, because the
typing info for the 15000+ PySide functions is really missing, and it
would be nice to have this info directly available.


The Idea to Support Signatures
==============================

We want to have an additional ``__signature__`` attribute in all PySide
methods, without changing lots of generated code.
Therefore, we did not change any of the existing data structures,
but supported the new attribute by a global dictionary.

When the ``__signature__`` property is requested, a method is called that
does a lookup in the global dict. This is a flexible approach with little impact
to the rest of the project. It has very limited overhead compared to direct
attribute access, but for the need of a signature access from time to time,
this is an adequate compromise.


How this Code Works
-------------------

Signatures are supported for regular Python functions, only. Creating signatures
for ``PyCFunction`` objects would require quite some extra effort in Python.

Fortunately, we found this special *stealth* technique, that saves us most of the
needed effort:

The basic idea is to create a dummy Python function with **varnames**, **defaults**
and **annotations** properties, and then to use the inspect
module to create a signature object. This object is returned as the computed
result of the ``__signature__`` attribute of the real ``PyCFunction`` object.

There is one thing that really changes Python a bit:

*   I added the ``__signature__`` attribute to every function.

That is a little change to Python that does not harm, but it saves us
tons of code, that was needed in the early versions of the module.

The internal work is done in two steps:

*   All functions of a class get the *signature text* when the module is imported.
    This is only a very small overhead added to the startup time. It is a single
    string for the whole class.
*   The actual signature object is created later, when the attribute is really
    accessed. Signatures are cached and only created on first access.

Example:

The ``PyCFunction`` ``QtWidgets.QApplication.palette`` is interrogated for its
signature. That means ``pyside_sm_get___signature__()`` is called.
It calls ``GetSignature_Function`` which returns the signature if it is found.


Why this Code is Fast
---------------------

It costs a little time (maybe 4 seconds) to run througs every single signature
object, since these are more than 15000 Python objects. But all the signature
objects will be rarely accessed but in special applications.
The normal case are only a few accesses, and these work pretty fast.

The key to make this signature module fast is to avoid computation as much as
possible. When no signature objects are used, then no time is lost in initialization.
When it comes to signature usage, then late initialization is used and cached.
This technique is also known as *full laziness* in haskell.

There are actually two locations where late initialization occurs:

*   ``dict`` can be no dict but a tuple. That is the initial argument tuple that
    was saved by ``PySide_BuildSignatureArgs`` at module load time.
    If so, then ``pyside_type_init`` in parser.py will be called,
    which parses the string and creates the dict.
*   ``props`` can be empty. Then ``create_signature`` in loader.py
    is called, which uses a dummy function to produce a signature instance
    with the inspect module.

The initialization that is always done is just two dictionary writes
per class, and we have about 1000 classes.
To measure the additional overhead, we have simulated what happens
when ``from PySide2 import *`` is performed.
It turned out that the overhead is below 0.5 ms.


The Signature Package Structure
-------------------------------

The C++ code involved with the signature module is completely in the file
shiboken2/libshiboken/signature.cpp . All other functionality is implemented in
the ``signature`` Python package. It has the following structure::

    pyside2/PySide2/support/signature/__init__.py
                                      loader.py
                                      parser.py
                                      mapping.py
                                      typing27.py
                                      backport_inspect.py

Really important are the **parser**, **mapping** and **loader** modules. The rest is
needed to create Python 2 compatibility.


loader.py
~~~~~~~~~

This module assembles and imports the ``inspect`` module, and then exports the
``create_signature`` function. This function takes a fake function and some
attributes and builds a ``__signature__`` object with the inspect module.


parser.py
~~~~~~~~~

This module takes a class signatures string from C++ and parses it into the
needed properties for the ``create_signature`` function. Its entry point is the
``pyside_type_init`` function, which is called from the C module via ``loader.py``.


mapping.py
~~~~~~~~~~

The purpose of the mapping module is maintaining a list of replacement strings
that map from the *signature text* in C to the property strings that Python
needs. A lot of mappings are resolved by rather complex expressions in ``parser.py``,
but a few hundred cases are better to spell explicitly, here.


*typing27.py*
~~~~~~~~~~~~~

Python 2 has no typing module at all. This is a backport of the minimum that is needed.


*backport_inspect.py*
~~~~~~~~~~~~~~~~~~~~~

Python 2 has an inspect module, but lacks the signature functions, completely.
This module adds the missing functionality, which is merged at runtime into
the inspect module.


Multiple Arities
----------------

One aspect that was ignored so far was *multiple arities*: How to handle it when
a function has more than one signature?

I did not find any note on how multiple signatures should be treated in Python,
but this simple rules seem to work well:

*   If there is a list, then it is a multi-signature.
*   Otherwise, it is a simple signature.


Impacts of The Signature Module
===============================

The signature module has a number of impacts to other PySide modules, which were
created as a consequence of its existence, and there will be a few more in the
future:


existence_test.py
-----------------

The file ``pyside2/tests/registry/existence_test.py`` was written using the
signatures from the signatures module. The idea is that there are some 15000
functions with a certain signature.

These functions should not get lost by some bad check-in. Therefore, a list
of all existing signatures is kept as a module that assembles a
dictionary. The function existence is checked, and also the exact arity.

This module exists for every PySide release and every platform. The initial
module is generated once and saved as ``exists_{plat}_{version}.py``.

An error is normally only reported as a warning, but:


Interaction With The Coin Module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When this test program is run in COIN, then the warnings are turned into
errors. The reason is that only in COIN, we have a stable configuration
of PySide modules that can reliably be compared.

These modules have the name ``exists_{plat}_{version}_ci.py``, and as a big
exception for generated code, these files are *intentionally* checked in.


What Happens When a List is Missing?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a new version of PySide gets created, then the existence test files
initially do not exist.

When a COIN test is run, then it will complain about the error and create
the missing module on standard output.
But since COIN tests are run multiple times, the output that was generated
by the first test will still exist at the subsequent runs.
(If COIN was properly implemented, we could not take that advantage and
would need to implement that as an extra exception.)

As a result, a missing module will be reported as a test which partially
succeeded (called "FLAKY"). To avoid further flaky tests and to activate as a real test,
we can now capture the error output of COIN and check the generated module
in.


init_platform.py
~~~~~~~~~~~~~~~~

For generating the ``exists_{plat}_{version}.py`` modules, the module
``pyside2/tests/registry/init_platform.py`` was written. It can be used
standalone from the commandline, to check the compatibility of some
changes, directly.


generate_pyi.py
---------------

``pyside2/PySide2/support/generate_pyi.py`` is still under development.
This module generates so-called hinting stubs for integration of PySide
with diverse *Python IDEs*.

Although this module creates the stubs as an add-on, the
impact on the quality of the signature module is considerable:

The module must create syntactically correct ``.pyi`` files which contain
not only signatures but also constants and enums of all PySide modules.
This serves as an extra challenge that has a very positive effect on
the completeness and correctness of signatures.


Future Extension
----------------

Before the signature module was written, there already existed the concept of
signatures, but in a more C++ - centric way. From that time, there still exist
the error messages, which are created when a function gets wrong argument types.

These error messages should be replaced by text generated on demand by
the signature module, in order to be more consistent and correct.

Additionally, the ``__doc__`` attribute of PySide methods is not set, yet.
It would be easy to get a nice ``help()`` feature by creating signatures
as default content for docstrings.


Literature
==========

    `PEP 362 – Function Signature Object <https://www.python.org/dev/peps/pep-0362/>`__

    `PEP 484 – Type Hints <https://www.python.org/dev/peps/pep-0484/>`__

    `PEP 3107 – Function Annotations <https://www.python.org/dev/peps/pep-3107/>`__


*Personal Remark: This module is dedicated to our lovebird "Püppi", who died on 2017-09-15.*
