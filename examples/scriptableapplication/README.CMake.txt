For general information read README.txt instead.

To build this example you will need:
* A recent version of CMake (3.1+)
* Make sure that a --standalone PySide2 package (bundled with Qt libraries) is installed into the
  current active Python environment (system or virtualenv)
* qmake to be in your PATH (so that CMake find_package(Qt5) works; used for include headers)
* use the same Qt version for building the example application, as was used for building
* PySide2, this is to ensure binary compatibility between the newly generated bindings libraries,
  the PySide2 libraries and the Qt libraries.

For Windows you will also need:
* Visual studio environment to be active in your terminal
* Correct visual studio architecture chosen (32 vs 64 bit)
* Make sure that your Qt + Python + PySide + CMake app build configuration is the same (either or
  all Release (which is more likely) or all Debug).

You can build this example by executing the following commands (slightly adapted to your file
system) in a terminal:

cd ~/pyside-setup/examples/scriptableapplication
(or cd C:\pyside-setup\examples\scriptableapplication)
mkdir build
cd build
cmake -H.. -B. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
(or cmake -H.. -B. -G "NMake Makefiles JOM" -DCMAKE_BUILD_TYPE=Release)
make (or nmake / jom)
./scriptableapplication (or scriptableapplication.exe)
