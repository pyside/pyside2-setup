Shiboken
********

Shiboken is a fundamental piece on the `Qt for Python`_ project that serves two purposes:

* Generator_: Extract information from C or C++ headers and generate CPython_ code that allow
  to bring C or C++ projects to Python. This process uses a library called ApiExtractor_ which
  internally uses Clang_.
* Module_: An utility Python module that exposed new Python types, functions to handle pointers,
  among other things, that is written in CPython_ and can use independently of the generator.

.. _`Qt for Python`: ../index.html
.. _Generator: shibokengenerator.html
.. _Module: shibokenmodule.html
.. _CPython: https://github.com/python/cpython
.. _Clang: https://clang.llvm.org/
.. _ApiExtractor: typesystem.html

Documentation
=============

.. raw:: html

    <table class="special">
        <colgroup>
            <col style="width: 33%" />
            <col style="width: 33%" />
            <col style="width: 33%" />
        </colgroup>
        <tr>
          <td><a href="gettingstarted.html"><p><strong>Getting Started</strong><br/>Install and build from source.</p></a></td>
          <td><a href="shibokengenerator.html"><p><strong>Shiboken Generator</strong><br/>Binding generator executable.</p></a></td>
          <td><a href="shibokenmodule.html"><p><strong>Shiboken Module</strong><br/>Python utility module.</p></a></td>
        </tr>
        <tr>
          <td><a href="typesystem.html"><p><strong>Type System</strong><br/>Reference and functionallities.</p></a></td>
          <td><a href="examples/index.html"><p><strong>Examples</strong><br/>Using Shiboken.</p></a></td>
          <td><a href="considerations.html"><p><strong>Considerations</strong><br/>Known issues and FAQ.</p></a></td>
        </tr>

    </table>

.. toctree::
   :hidden:
   :glob:

   gettingstarted.rst
   shibokengenerator.rst
   shibokenmodule.rst
   typesystem.rst
   examples/index.rst
   considerations.rst
