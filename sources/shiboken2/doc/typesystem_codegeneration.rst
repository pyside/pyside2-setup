.. _codegenerationterminology:

***************************
Code Generation Terminology
***************************

Types of generated code
=======================


**Python Wrapper**
  The code that exports the C++ wrapped class to Python. **Python wrapper**
  refers to all the code needed to export a C++ class to Python, and
  **Python method/function wrapper** means the specific function that calls
  the C++ method/function on behalf of Python. This code is invoked from
  the Python side.

**C++ Wrapper**
  This term refers to a generated C++ class that extends a class from the
  wrapped library. It is generated only when a wrapped C++ class is
  polymorphic, i.e. it has or inherits any virtual methods.
  The **C++ Wrapper** overrides the virtual methods of the wrapped C++ class
  with code that allows for overriding the method with a Python implementation.
  It checks whether a corresponding method in the Python instance exists and
  calls it. This code is invoked from the C++ side.


Specifying a target for modifications
=====================================

In the typesystem files, the ``class`` attribute is used to which class a
modification is applied (see :ref:`codeinjectionsemantics`,
:ref:`objectownership`).
The value **Target** means the modification is applied to the
**Python Wrapper**. The value **Native** means the modification is applied to
the **C++ Wrapper**.

