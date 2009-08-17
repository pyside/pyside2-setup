.. highlight:: xml

.. _gentut-bindinglibfoo:

Binding libfoo with the Generator
=================================

In order to create bindings for a library based on Qt4 a number of components
must be available on the system.

  + Qt4 library (with headers and pkg-config .pc files for development -- the
    ``-dev`` packages in a Debian distribution).
  + Qt4 Python bindings made with :program:`boostpythongenerator`.
  + Typesystems for the Qt4 Python bindings.
  + Headers for the library to be bound.

With the items listed above the developer must write the components from
where the generator will gather information to create the binding source code.

  + Typesystem file describing the way the binding must be done.
  + **global.h** including all the **libfoo** headers and defining required macros.
  + A build system to direct the process of generating, compiling and linking the
    binding.

The directory structure for the binding project could be something like the tree
shown below:

::

  foobinding/
  |-- data/
  `-- module_dir/
      `-- glue/


The **data** directory should contain the **global.h** and the typesystem
file. This typesystem need to refer to the ones used to create the Qt4 bindings,
commonly located on **/usr/share/PySide/typesystem**, the exact location
can be checked with pkg-config:

::

    $ pkg-config pyside --variable=typesystemdir


The **module_dir** directory is the place where the sources generated should
be placed. It starts empty except for the build instructions file (Makefile,
Makefile.am, CMakeLists.txt, etc). The realname of this directory must be the
same written in the typesystem file:

::

    <typesystem package="module_dir">


If there is any need for handwritten source code longer than a couple of lines,
making them unconfortable to be put on the typesystem xml file, the sources
could be orderly placed in a **glue** directory, also referred in the
new binding typesystem.

When writing the typesystem file (more on this later) there is no need to refer
to the other required typesystem files with absolute paths, the locations where
they can be found could be passed to the generator through a command line
option (``--typesystem-paths=PATH1:PATH2:[...]``) or the environment variable
**TYPESYSTEMPATH**.

For **libfoo** no glue code will be needed so this directory is not used,
the other directories are created with proper names.

::

  foobinding/
  |-- data/global.h
  |   `--  typesystem_foo.xml
  `-- foo/
      `-- Makefile

