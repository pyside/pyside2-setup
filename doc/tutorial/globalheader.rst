.. highlight:: cpp

.. _gentut-globalheader:

The Global Header
=================

Besides the information provided by the typesystem, the generator needs to
gather more data from the library headers containing the classes to be exposed
in Python. If there is a header that include all the others (or just one, as is
the case of **libfoo**) this could be passed directly to the generator.

If such a file is not available, or only a subset of the library is desired, or
if some flags must be defined before parsing the library headers, then a
``global.h`` file must be provided.

The use of a ``global.h`` file is preferred if some macros must be defined
before the parser gather data from the headers. For example, if ``NULL`` is not
defined and it is used as a default paramater for some constructor or method,
the parser will not recognize it.

The solve this create a ``global.h`` including all the desired headers and the
defined (and undefined) flags as follows:

**foobinding/data/global.h**
::

    #undef QT_NO_STL
    #undef QT_NO_STL_WCHAR

    #ifndef NULL
    #define NULL    0
    #endif

    #include <foo.h>

