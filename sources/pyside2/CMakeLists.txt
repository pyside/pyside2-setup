include(cmake/Macros/icecc.cmake) # this must be the first line!

cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

# Don't ignore targets that do not exist, inside add_dependencies calls.
cmake_policy(SET CMP0046 NEW)

project(pysidebindings)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Macros/
                      ${CMAKE_MODULE_PATH})

option(USE_PYTHON_VERSION "Use specific python version to build pyside2." "")

if (USE_PYTHON_VERSION)
    find_package(PythonInterp ${USE_PYTHON_VERSION} REQUIRED)
    find_package(PythonLibs ${USE_PYTHON_VERSION} REQUIRED)
else()
    find_package(PythonInterp 2.6)
    find_package(PythonLibs 2.6)
endif()

# Queries the python sysconfig for the abi flags which need to be inserted into extension suffixes.
# Only present starting with Python 3.2.
# Corresponding configure switches to single letter flags:
# --with-pymalloc -> m
# --with-pydebug -> d
# --with-unicode -> u (rare)
macro(get_python3_abi_flags)
  if (NOT PYTHON_ABI_FLAGS)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -c "if True:
          import sysconfig
          print(sysconfig.get_config_var('abiflags'))
          "
      OUTPUT_VARIABLE PYTHON_ABI_FLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
  message("PYTHON_ABI_FLAGS: " ${PYTHON_ABI_FLAGS})
endmacro()

macro(get_python_multi_arch_suffix)
  # TODO: This part needs testing to check if it is available on Windows.
  # It is present on macOS, but is not used yet.
  # Result is something like 'x86_64-linux-gnu'.
  if (NOT PYTHON_MULTIARCH_SUFFIX)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -c "if True:
         import sysconfig
         print(sysconfig.get_config_var('MULTIARCH'))
         "
      OUTPUT_VARIABLE PYTHON_MULTIARCH_SUFFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
  message("PYTHON_MULTIARCH_SUFFIX: " ${PYTHON_MULTIARCH_SUFFIX})
endmacro()

macro(get_python2_release_suffix)
  # Result of imp.get_suffixes() is something like:
  # [('_d.so', 'rb', 3), ('module_d.so', 'rb', 3), ('.x86_64-linux-gnu_d.so', 'rb', 3)]
  # or alternatively the same but withut the '_d' part.
  # The list comprehension is used to choose which suffix to include in library names.
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import imp
       print('_d' if any([tup[0].endswith('_d.so') for tup in imp.get_suffixes()]) else '')
       "
    OUTPUT_VARIABLE PYTHON_MODULE_RELEASE_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("PYTHON_MODULE_RELEASE_SUFFIX: " ${PYTHON_MODULE_RELEASE_SUFFIX})
endmacro()

if (UNIX AND NOT APPLE)
  if (NOT PYTHON_EXTENSION_SUFFIX)
    get_python_multi_arch_suffix()
    # The suffix added to .so libraries should be differenet between Python 2 and 3.
    # The position of the multiarch suffix is different, and the way the debug flag is set
    # computed differently.
    # In Python 2 there is no standard way to query if the python interpeter was built in debug or
    # release build (sysconfig.get_config_var('Py_Debug') can have a different value than you would
    # expect if you do a custom Python build). The solution to query for the import
    # suffixes and check if _d is present there. It is present on Linux distribution
    # packages of Python, but not in custom built Python builds, because the distros apply their
    # custom patches too append the '_d's.
    # In Python 3 (starting with 3.2) there is a standard way to check if '_d' needs to be added,
    # as well as any other letters, by querying the abiflags sysconfig variable.
    if (PYTHON_VERSION_MAJOR EQUAL 2)
        get_python2_release_suffix()
        # The suffix of Python module files (e.g. QtCore.x86_64-linux-gnu_d.so) as well as library
        # libraries (e.g. libpyside2.x86_64-linux-gnu.so).
        if(PYTHON_MULTIARCH_SUFFIX)
            set(PYTHON_EXTENSION_SUFFIX ".${PYTHON_MULTIARCH_SUFFIX}")
        endif()
        set(PYTHON_EXTENSION_SUFFIX "${PYTHON_EXTENSION_SUFFIX}${PYTHON_MODULE_RELEASE_SUFFIX}")
    elseif (PYTHON_VERSION_MAJOR EQUAL 3)
        get_python3_abi_flags()
        set(PYTHON_EXTENSION_SUFFIX ".cpython-${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}${PYTHON_ABI_FLAGS}")
        if(PYTHON_MULTIARCH_SUFFIX)
            set(PYTHON_EXTENSION_SUFFIX "${PYTHON_EXTENSION_SUFFIX}-${PYTHON_MULTIARCH_SUFFIX}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}!")
    endif()
  endif()
  message("PYTHON_EXTENSION_SUFFIX: " ${PYTHON_EXTENSION_SUFFIX})
endif ()

find_package(Shiboken2 2.0.0 REQUIRED)
find_package(Qt5 5.6.0 REQUIRED COMPONENTS Core)
add_definitions(${Qt5Core_DEFINITIONS})

find_file(GL_H "gl.h" PATH_SUFFIXES "GL")
message("result:" "${GL_H}")
include(FindQt5Extra)

set(XVFB_EXEC "")
option(USE_XVFB "Uses xvfb-run with the unit tests to avoid QtGui tests popping windows on the screen." FALSE)
if(USE_XVFB)
    find_program(XVFB_RUN NAMES xvfb-run)
    if (NOT ${XVFB_RUN} MATCHES "XVFB_RUN-NOTFOUND")
        set(XVFB_EXEC ${XVFB_RUN} -a)
        message(STATUS "Using xvfb-run to perform QtGui tests.")
    endif()
endif()

option(BUILD_TESTS "Build tests." TRUE)
option(ENABLE_VERSION_SUFFIX "Used to use current version in suffix to generated files. This is used to allow multiples versions installed simultaneous." FALSE)
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
if(CMAKE_HOST_APPLE)
    set(ALTERNATIVE_QT_INCLUDE_DIR "" CACHE PATH "The Alternative value to QT_INCLUDE_DIR. Necessary to fix bug on cmake 2.8 MACOS users")
    set(OSX_USE_LIBCPP "OFF" CACHE BOOL "Explicitly link the libc++ standard library (useful for osx deployment targets lower than 10.9.")
    if(OSX_USE_LIBCPP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

if(NOT CMAKE_HOST_APPLE)
    # Qt5: QT_INCLUDE_DIR does no longer exist.
    # On Windows and Linux, it can be computed from Qt5Core_INCLUDE_DIRS
    message("Qt5Core_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS}")
    list(GET Qt5Core_INCLUDE_DIRS 0 QT_INCLUDE_DIR)
    message(STATUS "*** computed QT_INCLUDE_DIR as ${QT_INCLUDE_DIR}")
endif()

if(MSVC)
    # Qt5: this flag has changed from /Zc:wchar_t- in Qt4.X
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t /GR /EHsc /DNOCOLOR /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS") # XXX
else()
    if(CMAKE_HOST_UNIX AND NOT CYGWIN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden -Wno-strict-aliasing")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimization library size and performance. Only available on Release Mode" 0)
    if(ENABLE_GCC_OPTIMIZATION)
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -Wl,-O1")
        if(NOT CMAKE_HOST_APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--hash-style=gnu")
        endif()
    endif()

    if(CMAKE_HOST_APPLE)
        if (NOT QT_INCLUDE_DIR)
            set(QT_INCLUDE_DIR "/Library/Frameworks")
        endif()
        if(ALTERNATIVE_QT_INCLUDE_DIR)
            set(QT_INCLUDE_DIR ${ALTERNATIVE_QT_INCLUDE_DIR})
        endif()
        string(REPLACE " " ":" QT_INCLUDE_DIR ${QT_INCLUDE_DIR})
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${SHIBOKEN_BUILD_TYPE})
endif()

set(BINDING_NAME PySide2)
set(BINDING_API_MAJOR_VERSION "2")
set(BINDING_API_MINOR_VERSION "0")
set(BINDING_API_MICRO_VERSION "0")
set(BINDING_API_RELEASE_LEVEL "alpha") # alpha, beta, rc, or final
set(BINDING_API_SERIAL 0) # leave as 0 when release level is final
set(BINDING_API_VERSION "${BINDING_API_MAJOR_VERSION}.${BINDING_API_MINOR_VERSION}.${BINDING_API_MICRO_VERSION}" CACHE STRING "PySide version" FORCE)
if (BINDING_API_RELEASE_LEVEL STREQUAL "final")
    set(BINDING_API_VERSION_FULL "${BINDING_API_MAJOR_VERSION}.${BINDING_API_MINOR_VERSION}.${BINDING_API_MICRO_VERSION}"
        CACHE STRING "PySide version [full]" FORCE)
else()
    set(BINDING_API_VERSION_FULL "${BINDING_API_MAJOR_VERSION}.${BINDING_API_MINOR_VERSION}.${BINDING_API_MICRO_VERSION}~${BINDING_API_RELEASE_LEVEL}${BINDING_API_SERIAL}"
        CACHE STRING "PySide version [full]" FORCE)
endif()

include(PySideModules)

macro(COLLECT_MODULE_IF_FOUND shortname)
    set(name "Qt5${shortname}")
    find_package(${name})
    set(_name_found "${name}_FOUND")
    if(${_name_found})
        message(STATUS "module ${name} found (${ARGN})")

        # Put the module at the end of pyside2_global.h
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/PySide2/pyside2_global.h.add"
                    "#include \"Qt${shortname}/Qt${shortname}\"\n")
        # record the shortnames for the tests
        list(APPEND all_module_shortnames ${shortname})
    else()
        if("${ARGN}" STREQUAL "opt")
            message(STATUS "optional module ${name} skipped")
        elseif("${ARGN}" STREQUAL "essential")
            message(STATUS "skipped module ${name} is essential!\n"
                           "   We do not guarantee that all tests are working.")
        else()
            message(FATAL_ERROR "module ${name} MISSING")
        endif()
    endif()
endmacro()

# note: the order of this list is relevant for dependencies.
# For instance: Qt5Printsupport must come before Qt5WebKitWidgets.
COLLECT_MODULE_IF_FOUND(Core)
COLLECT_MODULE_IF_FOUND(Gui essential)
COLLECT_MODULE_IF_FOUND(Widgets essential)
COLLECT_MODULE_IF_FOUND(PrintSupport essential)
COLLECT_MODULE_IF_FOUND(Sql essential)
COLLECT_MODULE_IF_FOUND(Network essential)
COLLECT_MODULE_IF_FOUND(Test essential)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(Concurrent essential)
else()
    set(DISABLE_QtConcurrent 1)
ENDIF()
if(UNIX AND NOT APPLE)
    COLLECT_MODULE_IF_FOUND(X11Extras essential)
endif()
if(WIN32)
    COLLECT_MODULE_IF_FOUND(WinExtras essential)
endif()
if(APPLE)
    COLLECT_MODULE_IF_FOUND(MacExtras essential)
endif()
COLLECT_MODULE_IF_FOUND(Xml)
COLLECT_MODULE_IF_FOUND(XmlPatterns opt)
COLLECT_MODULE_IF_FOUND(Help opt)
COLLECT_MODULE_IF_FOUND(Multimedia opt)
COLLECT_MODULE_IF_FOUND(MultimediaWidgets opt)
COLLECT_MODULE_IF_FOUND(OpenGL opt)
COLLECT_MODULE_IF_FOUND(Qml opt)
COLLECT_MODULE_IF_FOUND(Quick opt)
COLLECT_MODULE_IF_FOUND(QuickWidgets opt)
COLLECT_MODULE_IF_FOUND(Script opt)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(ScriptTools opt)
else()
    set(DISABLE_QtScriptTools 1)
ENDIF()
COLLECT_MODULE_IF_FOUND(Svg opt)
if(Qt5Designer_FOUND)
    COLLECT_MODULE_IF_FOUND(UiTools opt)
else()
    set(DISABLE_QtUiTools 1)
endif()
COLLECT_MODULE_IF_FOUND(WebChannel opt)
# still forgotten:
#COLLECT_MODULE_IF_FOUND(WebEngineCore opt)
#COLLECT_MODULE_IF_FOUND(WebEngine opt)
COLLECT_MODULE_IF_FOUND(WebEngineWidgets opt)
COLLECT_MODULE_IF_FOUND(WebKit opt)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(WebKitWidgets opt)
else()
    set(DISABLE_QtWebKitWidgets 1)
ENDIF()
COLLECT_MODULE_IF_FOUND(WebSockets opt)

string(REGEX MATCHALL "[0-9]+" qt_version_helper "${Qt5Core_VERSION}")

list(GET qt_version_helper 0 QT_VERSION_MAJOR)
list(GET qt_version_helper 1 QT_VERSION_MINOR)
unset(qt_version_helper)

set(PYSIDE_QT_VERSION "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}" CACHE STRING "Qt version used to compile PySide" FORCE)
if(ENABLE_VERSION_SUFFIX)
      set(pyside_SUFFIX "-${BINDING_API_MAJOR_VERSION}.${BINDING_API_MINOR_VERSION}")
endif()

# no more supported: include(${QT_USE_FILE})

# Configure OS support
set(ENABLE_X11 "0")
set(ENABLE_MAC "0")
set(ENABLE_WIN "0")
set(ENABLE_SIMULATOR "0")

# no more Maemo, no more simulator
if(CMAKE_HOST_APPLE)
    set(ENABLE_MAC "1")
    set(AUTO_OS "mac")
elseif(CMAKE_HOST_WIN32)
    set(ENABLE_WIN "1")
    set(AUTO_OS "win")
elseif(CMAKE_HOST_UNIX)
    set(ENABLE_X11 "1")
    set(AUTO_OS "x11")
else()
    message(FATAL_ERROR "OS not supported")
endif()
message(STATUS "Detected OS: ${AUTO_OS}")

if (WIN32)
    set(PATH_SEP "\;")
else()
    set(PATH_SEP ":")
endif()

# Define supported Qt Version
set(SUPPORTED_QT_VERSION "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

set(BINDING_VERSION ${BINDING_API_VERSION}.${QT_VERSION_MAJOR}.${QT_VERSION_MINOR})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


set(ARCHIVE_NAME pyside-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}+${BINDING_API_VERSION_FULL})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2.\n"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

if (NOT SITE_PACKAGE)
    execute_process(
        COMMAND ${SHIBOKEN_PYTHON_INTERPRETER} -c "if True:
            from distutils import sysconfig
            from os.path import sep
            print(sysconfig.get_python_lib(1, 0, prefix='${CMAKE_INSTALL_PREFIX}').replace(sep, '/'))
            "
        OUTPUT_VARIABLE SITE_PACKAGE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT SITE_PACKAGE)
        message(FATAL_ERROR "Could not detect Python module installation directory.")
    elseif (APPLE)
        message(STATUS "!!! The generated bindings will be installed on ${SITE_PACKAGE}, is it right!?")
    endif()
endif()

set(GENERATOR_EXTRA_FLAGS --generator-set=shiboken
                          --enable-parent-ctor-heuristic
                          --enable-pyside-extensions
                          --enable-return-value-heuristic
                          --use-isnull-as-nb_nonzero)
# 2017-04-24 The protected hack can unfortunately not be disabled, because
# Clang does produce linker errors when we disable the hack.
# But the ugly workaround in Python is replaced by a shiboken change.
if(WIN32 OR DEFINED AVOID_PROTECTED_HACK)
    message(STATUS "PySide2 will be generated avoiding the protected hack!")
    set(GENERATOR_EXTRA_FLAGS ${GENERATOR_EXTRA_FLAGS} --avoid-protected-hack)
    add_definitions(-DAVOID_PROTECTED_HACK)
else()
    message(STATUS "PySide will be generated using the protected hack!")
endif()

add_subdirectory(libpyside)
find_package(Qt5UiTools)
find_package(Qt5Designer)
if(Qt5UiTools_FOUND AND Qt5Designer_FOUND)
    add_subdirectory(plugins)
endif()

# project directories
add_subdirectory(PySide2)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

find_program(DOT_EXEC dot)
if (QT_SRC_DIR AND DOT_EXEC)
    add_subdirectory(doc)
else ()
    message(STATUS "QT_SRC_DIR variable not set or graphviz not found, apidoc generation targets disabled.")
endif()
