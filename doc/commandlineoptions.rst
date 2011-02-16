.. _command-line:

Command line options
********************

Usage
-----

::

   generator [options] header-file typesystem-file


Options
-------

.. _api-version:

``--api-version=<version>``
    Specify the supported api version used to generate the bindings.

.. _debug-level:

``--debug-level=[sparse|medium|full]``
    Set the debug level.

.. _documentation-only:

``--documentation-only``
    Do not generate any code, just the documentation.

.. _drop-type-entries:

``--drop-type-entries="<TypeEntry0>[;TypeEntry1;...]"``
    Semicolon separated list of type system entries (classes, namespaces,
    global functions and enums) to be dropped from generation.

.. _generation-set:

``--generation-set``
    Generator set to be used (e.g. qtdoc).

.. _help:

``--help``
    Display this help and exit.

.. _include-paths:

``--include-paths=<path>[:<path>:...]``
    Include paths used by the C++ parser.

.. _license-file=[license-file]:

``--license-file=[license-file]``
    File used for copyright headers of generated files.

.. _no-suppress-warnings:

``--no-suppress-warnings``
    Show all warnings.

.. _output-directory:

``--output-directory=[dir]``
    The directory where the generated files will be written.

.. _silent:

``--silent``
    Avoid printing any message.

.. _typesystem-paths:

``--typesystem-paths=<path>[:<path>:...]``
    Paths used when searching for type system files.

.. _version:

``--version``
    Output version information and exit.

