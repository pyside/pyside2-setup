include(cmake/Macros/icecc.cmake) # this must be the first line!

cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(pysidebindings)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Macros/
                      ${CMAKE_MODULE_PATH})

option(USE_PYTHON_VERSION "Use specific python version to build pyside2." "")

if (USE_PYTHON_VERSION)
    find_package(PythonLibs ${USE_PYTHON_VERSION} REQUIRED)
    find_package(PythonInterp ${USE_PYTHON_VERSION} REQUIRED)
else()
    find_package(PythonLibs 2.6)
    find_package(PythonInterp 2.6)
endif()

if (UNIX AND NOT APPLE)
    # TODO: This part needs more testing first to be available on OSX and WIN
    # Also note the quirk that UNIX includes Apple!
  if (NOT PYTHON_MULTIARCH_SUFFIX)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "if True:
            import sysconfig
            print(sysconfig.get_config_var('MULTIARCH'))
            "
        OUTPUT_VARIABLE PYTHON_MULTIARCH_SUFFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if (NOT PYTHON_EXTENSION_SUFFIX)
    if (PYTHON_VERSION_MAJOR EQUAL 2)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(PYTHON_RELEASE_SUFFIX "-dbg")
            set(PYTHON_MODULE_RELEASE_SUFFIX "_d")
        else()
            set(PYTHON_RELEASE_SUFFIX "")
            set(PYTHON_MODULE_RELEASE_SUFFIX "")
        endif()
        set(PYTHON_EXTENSION_SUFFIX "-python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}${PYTHON_RELEASE_SUFFIX}")
        if(PYTHON_MULTIARCH_SUFFIX)
            set(PYTHON_MODULE_SUFFIX ".${PYTHON_MULTIARCH_SUFFIX}${PYTHON_MODULE_RELEASE_SUFFIX}")
            set(PYTHON_EXTENSION_SUFFIX "${PYTHON_EXTENSION_SUFFIX}-${PYTHON_MULTIARCH_SUFFIX}")
        endif()
    elseif (PYTHON_VERSION_MAJOR EQUAL 3)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(PYTHON_RELEASE_SUFFIX "dm")
        else()
            set(PYTHON_RELEASE_SUFFIX "m")
        endif()
        set(PYTHON_EXTENSION_SUFFIX ".cpython-${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}${PYTHON_RELEASE_SUFFIX}")
        if(PYTHON_MULTIARCH_SUFFIX)
            set(PYTHON_EXTENSION_SUFFIX "${PYTHON_EXTENSION_SUFFIX}-${PYTHON_MULTIARCH_SUFFIX}")
            set(PYTHON_MODULE_SUFFIX "${PYTHON_EXTENSION_SUFFIX}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}!")
    endif()
  endif()
  message("PYTHON_EXTENSION_SUFFIX: " ${PYTHON_EXTENSION_SUFFIX})
endif ()

find_package(Shiboken2 2.0.0 REQUIRED)
find_package(Qt5 5.3.0 REQUIRED COMPONENTS Core) 
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

find_package(Qt5Core)
find_package(Qt5Concurrent) # new in Qt5, from QtCore
find_package(Qt5Gui)        # reduced, small version without widgets
find_package(Qt5Widgets)    # widgets in their separate module
find_package(Qt5PrintSupport)
find_package(Qt5Xml)
find_package(Qt5XmlPatterns)
find_package(Qt5Svg)
find_package(Qt5Sql)
find_package(Qt5Designer)
find_package(Qt5UiTools)
find_package(Qt5Test)
find_package(Qt5Network)
find_package(Qt5WebKit)
find_package(Qt5WebKitWidgets)
find_package(Qt5OpenGL)
find_package(Qt5Script)
find_package(Qt5ScriptTools)
find_package(Qt5Help)
find_package(Qt5Multimedia)
find_package(Qt5Quick)
find_package(Qt5Qml)
find_package(Qt5QuickWidgets)
find_package(Qt5WebChannel)
find_package(Qt5WebEngine)
find_package(Qt5WebEngineWidgets)
find_package(Qt5WebSockets)

if(UNIX AND NOT APPLE)
  find_package(Qt5X11Extras) # new in Qt5, from QtGui
endif()

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT PYTHON_MODULE_SUFFIX)
    if (SHIBOKEN_PYTHON_VERSION_MAJOR EQUAL 2)
        set(PYTHON_DEBUG_SUFFIX "_d")
    endif()
endif()


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

set(GENERATOR_EXTRA_FLAGS --generator-set=shiboken --enable-parent-ctor-heuristic --enable-pyside-extensions --enable-return-value-heuristic --use-isnull-as-nb_nonzero)
if(WIN32 OR DEFINED AVOID_PROTECTED_HACK)
    message(STATUS "PySide2 will be generated avoiding the protected hack!")
    set(GENERATOR_EXTRA_FLAGS ${GENERATOR_EXTRA_FLAGS} --avoid-protected-hack)
    add_definitions(-DAVOID_PROTECTED_HACK)
else()
    message(STATUS "PySide will be generated using the protected hack!")
endif()

add_subdirectory(libpyside)
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
