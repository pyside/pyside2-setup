# WigglyWidget

The original Qt/C++ example can be found here:
https://doc.qt.io/qt-5/qtwidgets-widgets-wiggly-example.html

This example shows how to interact with a custom widget from two
different ways:

 * A full Python translation from a C++ example,
 * A Python binding generated from the C++ file.


The original example contained three different files:
 * `main.cpp/h`, which was translated to `main.py`,
 * `dialog.cpp/h`, which was translated to `dialog.py`,
 * `wigglywidget.cpp/h`, which was translated to `wigglywidget.py`,
   but also remains as is, to enable the binding generation through
   Shiboken.

In the `dialog.py` file you will find two imports that will be related
to each of the two approaches described before::


    # Python translated file
    from wigglywidget import WigglyWidget

    # Binding module create with Shiboken
    from wiggly import WigglyWidget


## Steps to build the bindings

The most important files are:
 * `bindings.xml`, to specify the class that we want to expose from C++
   to Python,
 * `bindings.h` to include the header of the classes we want to expose
 * `CMakeList.txt`, with all the instructions to build the shared libraries
   (DLL, or dylib)
 * `pyside2_config.py` which is located in the utils directory, one level
   up, to get the path for Shiboken and PySide.

Now create a `build/` directory, and from inside run `cmake ..` to use
the provided `CMakeLists.txt`.
To build, just run `make`, and `make install` to copy the generated files
to the main example directory to be able to run the final example:
`python main.py`.
You should be able to see two identical custom widgets, one being the
Python translation, and the other one being the C++ one.

### Windows

For windows it's recommended to use either `nmake`, `jom` or `ninja`,
when running cmake.

```bash
cmake -H.. -B. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release  # for nmake
cmake -H.. -B. -G "NMake Makefiles JOM" -DCMAKE_BUILD_TYPE=Release  # for jom
cmake -H.. -B. -G Ninja -DCMAKE_BUILD_TYPE=Release  # for ninja
```

### Linux, macOS

Generally using `make` will be enough, but as in the Windows case, you can use
ninja to build the project.

```bash
cmake -H.. -B. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

## Final words

Since this example originated by mixing the concepts of the `scriptableapplication`
and `samplebinding` examples, you can complement this README with the ones in
those directories.
