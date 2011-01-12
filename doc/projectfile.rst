.. _project-file:

********************
Binding Project File
********************

Instead of directing the Generator behaviour via command line, the binding developer
can write a XML project file describing the same information, and avoid the hassle
of a long stream of command line arguments.

.. _project-file-structure:

The project file structure
==========================

Here follows a comprehensive example of a generator project file.

    .. code-block:: xml

         <?xml version="1.0"?>
         <generator-project>
             <generator-set generator="path/to/generator/CHOICE_GENERATOR" />
             <header-file location="DIR/global.h" />
             <typesystem-file location="DIR/typesystem_for_your_binding.xml" />
             <output-directory location="OUTPUTDIR" />
             <include-paths>
                 <path location="path/to/library/being/wrapped/headers/1" />
                 <path location="path/to/library/being/wrapped/headers/2" />
             </include-paths>
             <typesystem-paths>
                 <path location="path/to/directory/containing/type/system/files/1" />
                 <path location="path/to/directory/containing/type/system/files/2" />
             </typesystem-paths>
             <enable-parent-ctor-heuristic />
         </generator-project>


Project file tags
=================

The generator project file tags are in direct relation to the
:ref:`command line arguments <command-line>`. All of the current command line
options provided by |project| were already seen on the :ref:`<project-file-structure>`,
for new command line options provided by additional generator modules (e.g.: qtdoc,
Shiboken) could also be used in the generator project file following simple conversion rules.

For tags without options, just write as an empty tag without any attributes. Example:

    .. code-block:: bash

         --BOOLEAN-ARGUMENT

becomes

    .. code-block:: xml

         <BOOLEAN-ARGUMENT />

and

    .. code-block:: bash

         --VALUE-ARGUMENT=VALUE

becomes

    .. code-block:: xml

         <VALUE-ARGUMENT value="VALUE" />


