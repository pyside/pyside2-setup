# Scriptable Application

This example demonstrates how to make a Qt C++ application scriptable.

It has a class **MainWindow** (`mainwindow.{cpp,h}`)
that inherits from *QMainWindow*, for which bindings are generated
using Shiboken.

The header `wrappedclasses.h` is passed to Shiboken which generates
class wrappers and headers in a sub directory called **AppLib/**
which are linked to the application.

The files `pythonutils.{cpp,h}` contains some code which binds the
instance of **MainWindow** to a variable called **'mainWindow'** in
the global Python namespace (`__main___`).
It is then possible to run Python script snippets like:

```python
mainWindow.testFunction1()
```
which trigger the underlying C++ function.

## Building the project

This example can be built using *CMake* or *QMake*,
but there are common requirements that you need to take into
consideration:

* Make sure that a --standalone PySide2 package (bundled with Qt libraries)
  is installed into the current active Python environment
  (system or virtualenv)
* qmake to be in your PATH:
  * so that CMake find_package(Qt5) works (used for include headers),
  * and also for using the proper qmake version when building with qmake
* use the same Qt version for building the example application, as was used
  for building PySide2, this is to ensure binary compatibility between the
  newly generated bindings libraries, the PySide2 libraries and the
  Qt libraries.

For Windows you will also need:
* Visual studio environment to be active in your terminal
* Correct visual studio architecture chosen (32 vs 64 bit)
* Make sure that your Qt + Python + PySide + app build configuration
  is the same (either or all Release, which is more likely, or all Debug).

Both build options will use the `pyside2.pri` file to configure the project
using the current PySide2/Shiboken2 installation (via `pyside2_config.py`).

Keep in mind that Clang libraries must be on your path.


### Using CMake

To build this example with CMake you will need a recent version of CMake (3.1+).

You can build this example by executing the following commands
(slightly adapted to your file system) in a terminal:

On macOS/Linux:
```bash
cd ~/pyside-setup/examples/scriptableapplication
mkdir build
cd build
cmake -H.. -B. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
./scriptableapplication
```

On Windows:
```bash
cd C:\pyside-setup\examples\scriptableapplication
mkdir build
cd build
cmake -H.. -B. -G "NMake Makefiles JOM" -DCMAKE_BUILD_TYPE=Release
nmake # or jom
scriptableapplication.exe
```

### Using QMake

The file `scriptableapplication.pro` is the project file associated
to the example when using qmake.

You can build this example by executing:
```bash
mkdir build
cd build
qmake ..
make # or nmake for Windows
```

#### Windows troubleshooting

Using **qmake** should work out of the box, there was a known issue
with directories and white spaces that is solved by using the
"~1" character, so the path will change from:
c:\Program Files\Python34\libs
to
c:\Progra~1\Python34\libs
this will avoid the issues when the Makefiles are generated.

It is possible when using **cmake** to pick up the wrong compiler
for a different architecture, but it can be addressed explicitly
using the -G option:

```bash
cmake -H.. -B. -G "Visual Studio 14 Win64" -DCMAKE_BUILD_TYPE=Release
```

If the `-G "Visual Studio 14 Win64"` option is used, a `sln` file
will be generated, and can be used with `MSBuild`
instead of `nmake/jom`

```bash
MSBuild scriptableapplication.sln "/p:DebugType=None"
```

## Virtualenv Support

If the application is started from a terminal with an activated python
virtual environment, that environment's packages will be used for the
python module import process.
In this case, make sure that the application was built while the
`virtualenv` was active, so that the build system picks up the correct
python shared library.

## Shared Libraries Notes

For this example's purpose, we are using the absolute path of the
shared libraries (`libshiboken` and `libpyside`) because the
installation of the modules is being made via wheels, and there is
no clean solution to include symbolic links into the package.

## Windows Notes

The build config of the application (Debug or Release) should match
the PySide2 build config, otherwise the application will not properly
work.

In practice this means the only supported configurations are:

1. release config build of the application +
   PySide2 `setup.py` without `--debug` flag + `python.exe` for the
   PySide2 build process + `python36.dll` for the linked in shared
   library + release build of Qt.
2. debug config build of the application +
   PySide2 `setup.py` **with** `--debug` flag + `python_d.exe` for the
   PySide2 build process + `python36_d.dll` for the linked in shared
   library + debug build of Qt.

This is necessary because all the shared libraries in question have to
link to the same C++ runtime library (`msvcrt.dll` or `msvcrtd.dll`).
To make the example as self-contained as possible, the shared libraries
in use (`pyside2.dll`, `shiboken2.dll`) are hard-linked into the build
folder of the application.
