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

    $ generatorrunner --generatorSet=shiboken \
                    global_headers.h \
                    --include-paths=$(PATHS_TO_HEADERS)) \
                    --typesystem-paths=$(PATHS_TO_TYPESYSTEMS) \
                    --output-directory=. \
                    typesystem.xml

Notice that the variables for include and typesystem paths could be determined
at build time with the pkg-config tool.

Collecting information with pkg-config
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Qt4 bindings include compile and build information through the pkg-config
mechanism. The pkg-config name for Qt4 Python bindings is **pyside** and a
simple ``pkg-config pyside --cflags --libs`` will retrieve the information
needed to build the new binding.

The Qt4 bindings file ``pyside.pc`` for the use of pkg-config requires
the ``.pc`` files from Qt4 to be installed. If the library is in an unusual
location, e.g. ``/opt/qt46``, remember to export it to the ``PKG_CONFIG_PATH``
environment variable.
For example: ``export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/opt/qt46/lib/pkgconfig``

There is a vital information also available through pkg-config:
the **typesystemdir** variable. It is used like this:
``pkg-config pyside --variable=typesystemdir`` This provides information
where to find the typesystem files used to create the Qt4 bindings, and as said
before the binding being created needs this to complement its own binding
information for the generation proccess.

Makefile
--------

Below is a plain Makefile for the binding project.

**foobinding/foo/Makefile**
::

    LIBFOO_DIR = ../../libfoo
    LIBS = `python-config --libs` \
        -lpyside \
        -lfoo -L$(LIBFOO_DIR)
    CXXFLAGS = -I/usr/share/qt4/mkspecs/linux-g++ -I. \
            -I$(LIBFOO_DIR) \
            -I`pkg-config --variable=includedir pyside`/QtCore/ \
            -I`pkg-config --variable=includedir QtCore` \
            -I`pkg-config --variable=includedir QtGui` \
            -I`pkg-config --variable=includedir QtCore`/.. \
            -I`python-config --includes` \
            `pkg-config pyside --cflags` \
            `pkg-config shiboken --cflags`

    QT4TYPESYSTEM_DIR = "/usr/share/PySide/typesystem/"
    QT4HEADER_DIRS = `pkg-config --variable=includedir QtCore`:`pkg-config --variable=includedir QtCore`/..

    SOURCES = foo_module_wrapper.cpp  math_wrapper.cpp
    OBJECTS = foo_module_wrapper.o  math_wrapper.o

    all: generate compile link

    generate:
        generatorrunner --generatorSet=shiboken \
        ../data/global.h \
        --include-paths=$(LIBFOO_DIR):$(QT4HEADER_DIRS):/usr/include \
        --typesystem-paths=../data:$(QT4TYPESYSTEM_DIR) \
        --output-directory=.. \
        ../data/typesystem_foo.xml

    compile: $(SOURCES)
    g++ -Wall -fPIC -DPIC $(CXXFLAGS) -c foo_module_wrapper.cpp
    g++ -Wall -fPIC -DPIC $(CXXFLAGS) -c math_wrapper.cpp

    link:
        g++ -shared -Wl,-soname,foo.so -o foo.so $(LIBS) $(OBJECTS)

    test:
        LD_LIBRARY_PATH=$(LIBFOO_DIR):$LD_LIBRARY_PATH python -c \
        "import PySide.QtCore; import foo; print dir(foo); m = foo.Math(); print \"5 squared is %d\" % m.squared(5)"

    clean:
        rm -rf *.o *.so *.?pp *.log *.h

Keep in mind that the Makefile above expects the ``libfoo`` and
``foobinding`` directories to be in the same level in the directory
hierarchy, remember to change any path references accordingly if
you choose to change things.

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
