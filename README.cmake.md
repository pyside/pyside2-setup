# CMake super project
For development convenience, a CMake super project is included in the root of the repository.

The super project can be built using standalone CMake, or using an IDE's CMake integration
(Qt Creator for example).

Nevertheless the default build process is done via setup.py, in which case each of the
sub-projects are built and installed separately, as mentioned, the super project is just
for development convenience.

## IDE (Qt Creator) case

When using an IDE, just open the root CMakeLists.txt file as a new project, and make sure to
specify the following things:

 * LLVM_INSTALL_DIR - the environment variable should point to your libclang library location
 * Qt               - either select a Qt Kit when configuring the project, or make sure that the
                      qmake binary is present in the PATH environment variable.
 * Python           - the PATH environment variable should also point to the Python interpreter
                      which you wish to use for building the projects (can either be a system
                      interpreter, or a virtualenv one for example)

Once that is done, just re-run CMake, so that it picks up the new environment values.
If needed, all other cache variables defined by the project files can be re-adjusted
(for example FORCE_LIMITED_API).

## Command line CMake case

When building using the command line CMake binary, make sure to invoke it in a separate
build directory, and not in the root source directory.

Make sure you have the correct environment variables set up, as described in the previous section.

The invocation would then look like:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
# make or nmake or msbuild or jom
```
