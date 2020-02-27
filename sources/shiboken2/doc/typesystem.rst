Type System Reference
=====================

The typesystem is used by a binding generator or any other software using the APIExtractor library
to map a C++ library API onto a higher level language.

The typesystem specification is a handwritten XML document listing the types
that will be available in the generated target language API; types that are not
declared in the specification will be ignored along with everything depending on
them. In addition, it is possible to manipulate and modify types and functions.
It is even possible to use the typesystem specification to inject arbitrary
code into the source files, such as an extra member function.

Below there is a complete reference guide to the various nodes (XML tags) of the typesystem.
For usage examples, take a look at the typesystem files used to generate PySide2. These files
can be found in the PySide2/<QT_MODULE_NAME> directory of the PySide2 package.

Define types
------------

.. toctree::
   :maxdepth: 1

   typesystem_specifying_types.rst

Modifying types
---------------

.. toctree::
   :maxdepth: 1

   typesystem_arguments.rst
   typesystem_codeinjection.rst
   typesystem_converters.rst
   typesystem_templates.rst
   typesystem_modify_function.rst
   typesystem_manipulating_objects.rst
   typesystem_conversionrule.rst
   typesystem_documentation.rst
   typesystem_variables.rst

Object ownership
----------------

.. toctree::
   :maxdepth: 1

   typesystem_ownership.rst

Extra options and Python caveats
--------------------------------

.. toctree::
   :maxdepth: 1

   typesystem_solving_compilation.rst
   typesystem_sequenceprotocol.rst
