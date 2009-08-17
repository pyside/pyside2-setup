.. _gentut-buildingthebinding:

Building The Binding
====================

As mentioned before the build system used must perform the following tasks
in the correct order:

    + Gather data about locations of headers and external needed typesystems.
    + Run the generator with the correct parameters.
    + Compile and link the binding.

The first and last are the usual, being the second the only novelty in the
process.

Running the Generator
---------------------

The generator is called with the following parameters and options:

::

    $ boostgenerator global_headers.h \
                    --include-paths=$(PATHS_TO_HEADERS)) \
                    --typesystem-paths=$(PATHS_TO_TYPESYSTEMS) \
                    --output-directory=. \
                    typesystem.xml

Notice that the variables for include and typesystem paths could be determined
at build time with the pkg-config tool.

Collecting information with pkg-config
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Qt4 bindings include compile and build information through the pkg-config
mechanism. The pkg-config name for Qt4 Python bindings is **qt4python** and a
simple ``pkg-config qt4python --cflags --libs`` will retrieve the information
needed to build the new binding.

The Qt4 bindings file ``qt4python.pc`` for the use of pkg-config requires
the ``.pc`` files from Qt4 to be installed. If the library is in an unusual
location, e.g. ``/opt/qt45``, remember to export it to the ``PKG_CONFIG_PATH``
environment variable.
For example: ``export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/opt/qt45/lib/pkgconfig``

There is a vital information also available through pkg-config:
the **typesystemdir** variable. It is used like this:
``pkg-config qt4python --variable=typesystemdir`` This provides information
where to find the typesystem files used to create the Qt4 bindings, and as said
before the binding being created needs this to complement its own binding
information for the generation proccess.

Makefile
--------

Below is a plain Makefile for the binding project.

**foobinding/foo/Makefile**
::

    LIBTEST_DIR       = ../../libfoo
    LIBS              = -lboost_python-gcc43-1_38-py25 -lpython2.5 \
                        `pkg-config qt4python --libs` \
                        -lfoo -L$(LIBTEST_DIR) \
                        -lpthread -ldl -lutil
    CFLAGS            = -I/usr/share/qt4/mkspecs/linux-g++ -I. \
                        -I$(LIBTEST_DIR) \
                        `pkg-config qt4python --cflags` \
                        -I/usr/include/python2.5\
                        -I/usr/include/boost/python
    QT4TYPESYSTEM_DIR = `pkg-config --variable=typesystemdir qt4python`
    QT4HEADER_DIRS    = `pkg-config --variable=includedir QtCore`:`pkg-config --variable=includedir QtCore`/..

    SOURCES           = math_wrapper.cpp foo_module_wrapper.cpp foo_global_functions_wrapper.cpp
    OBJECTS           = math_wrapper.o foo_module_wrapper.o foo_global_functions_wrapper.o

    all: generate compile link

    generate:
        boostgenerator ../data/global.h \
        --include-paths=$(LIBTEST_DIR):$(QT4HEADER_DIRS):/usr/include \
        --typesystem-paths=../data:$(QT4TYPESYSTEM_DIR) \
        --output-directory=.. \
        ../data/typesystem_foo.xml

    compile: $(SOURCES)
        g++ -fPIC -DPIC $(CFLAGS) math_wrapper.cpp -c
        g++ -fPIC -DPIC $(CFLAGS) foo_global_functions_wrapper.cpp -c
        g++ -fPIC -DPIC $(CFLAGS) foo_module_wrapper.cpp -c

    link:
        g++ -shared -Wl,-soname,foo.so -o foo.so $(LIBS) $(OBJECTS)

    test:
        LD_LIBRARY_PATH=$(LIBTEST_DIR) python -c \
        "import foo; print dir(foo); m = foo.Math(); print m.squared(5)"

    clean:
        rm -rf *.o *.so *.?pp *.log


Keepe in mind that the Makefile above expects the ``libfoo`` and
``foobinding`` directories to be in the same level in the directory
hierarchy, remember to change any path references accordingly if
you choose to change things.

**Warning:**
  The order in which the link flags are passed matters.
  **libboost_python** must come first, otherwise weeping
  and gnashing of teeth will follow.

Testing the Binding
-------------------
Now compile the binding with ``make``:

::

    $ cd foobinding/foo
    $ make

To test if the new binding is working (it can pass the build phase but still
blow up at runtime) start up a Python terminal and import it by the name.

::

    $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/libfoo/shared/object/dir
    $ export PYTHONPATH=$PYTHONPATH:/path/to/foo/python/module/file/dir
    $ python
    >> import foo
    >> print dir(foo)
    >> m = foo.Math()
    >> print m.squared(5)

