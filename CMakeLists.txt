Include(icecc.cmake)
project(shiboken2)

cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/
                      ${CMAKE_MODULE_PATH})

find_package(Qt5 REQUIRED COMPONENTS Core Xml XmlPatterns)

add_definitions(${Qt5Core_DEFINITIONS})

set(shiboken_MAJOR_VERSION "2")
set(shiboken_MINOR_VERSION "0")
set(shiboken_MICRO_VERSION "0")
set(shiboken2_VERSION "${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}.${shiboken_MICRO_VERSION}")

option(BUILD_TESTS "Build tests." TRUE)
option(USE_PYTHON_VERSION "Use specific python version to build shiboken2." "")

if (USE_PYTHON_VERSION)
    find_package(PythonLibs ${USE_PYTHON_VERSION} REQUIRED)
    find_package(PythonInterp ${USE_PYTHON_VERSION} REQUIRED)
else()
    find_package(PythonLibs 2.6)
    find_package(PythonInterp 2.6)
endif()

## For debugging the PYTHON* variables
message("PYTHONLIBS_FOUND: " ${PYTHONLIBS_FOUND})
message("PYTHON_LIBRARIES: " ${PYTHON_LIBRARIES})
message("PYTHON_INCLUDE_DIRS: " ${PYTHON_INCLUDE_DIRS})
message("PYTHON_DEBUG_LIBRARIES: " ${PYTHON_DEBUG_LIBRARIES})
message("PYTHONINTERP_FOUND: " ${PYTHONINTERP_FOUND})
message("PYTHON_EXECUTABLE: " ${PYTHON_EXECUTABLE})
message("PYTHON_VERSION: " ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH})

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

# Note the quirk that UNIX includes Apple!
if (UNIX AND NOT APPLE)
  if (NOT PYTHON_EXTENSION_SUFFIX)
    get_python_multi_arch_suffix()
    # The suffix added to .so libraries should be differenet between Python 2 and 3.
    # The position of the multiarch suffix is different, and the way the debug flag is set
    # computed differently.
    # In Python 2 there is no standard way to query if the python interpeter was built in debug or
    # release build (sysconfig.get_config_var('Py_Debug') can have a different value than you would
    # expect if you do a custom Python build). The solution is to query for the import
    # suffixes and check if _d is present there. It is present on Linux distribution
    # packages of Python, but not in custom built Python builds, because the distros apply their
    # custom patches too append the '_d's.
    # In Python 3 (starting with 3.2) there is a standard way to check if '_d' needs to be added,
    # as well as any other letters, by querying the abiflags sysconfig variable.
    if (PYTHON_VERSION_MAJOR EQUAL 2)
        get_python2_release_suffix()
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
  message(STATUS "PYTHON_EXTENSION_SUFFIX: ${PYTHON_EXTENSION_SUFFIX}")
endif ()

if (NOT PYTHON_SITE_PACKAGES)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "if True:
            from distutils import sysconfig
            from os.path import sep
            print(sysconfig.get_python_lib(1, 0, prefix='${CMAKE_INSTALL_PREFIX}').replace(sep, '/'))
            "
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT PYTHON_SITE_PACKAGES)
        message(FATAL_ERROR "Could not detect Python module installation directory.")
    elseif (APPLE)
        message(STATUS "!!! The generated bindings will be installed on ${PYTHON_SITE_PACKAGES}, is it right!?")
    endif()
endif()

if(MSVC)
    # Qt5: this flag has changed from /Zc:wchar_t- in Qt4.X
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t /GR /EHsc /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS")
else()
    if(CMAKE_HOST_UNIX AND NOT CYGWIN)
        add_definitions(-fPIC)
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
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D QT_NO_CAST_FROM_ASCII -D QT_NO_CAST_TO_ASCII")

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

if (WIN32)
    set(PATH_SEP "\;")
else()
    set(PATH_SEP ":")
endif()

if(CMAKE_HOST_APPLE)
    set(OSX_USE_LIBCPP "OFF" CACHE BOOL "Explicitly link the libc++ standard library (useful for osx deployment targets lower than 10.9.")
    if(OSX_USE_LIBCPP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

add_subdirectory(ApiExtractor)

set(generator_plugin_DIR ${LIB_INSTALL_DIR}/generatorrunner${generator_SUFFIX})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

set(SHIBOKEN_BUILD_TYPE "Release")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT PYTHON_DEBUG_LIBRARIES)
        message(WARNING "Python debug shared library not found; assuming python was built with shared library support disabled.")
    endif()
    if(NOT PYTHON_WITH_DEBUG)
        message(WARNING "Compiling shiboken2 with debug enabled, but the python executable was not compiled with debug support.")
    else()
        add_definitions("-DPy_DEBUG")
        set(SBK_ADD_PY_DEBUG_DEFINITION "add_definitions(\"-DPy_DEBUG\")")
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION " -DPy_DEBUG")
    endif()
    set(SBK_PYTHON_LIBRARIES ${PYTHON_DEBUG_LIBRARIES})
    set(SHIBOKEN_BUILD_TYPE "Debug")
else()
    set(SBK_PYTHON_LIBRARIES ${PYTHON_LIBRARIES})
    add_definitions("-DNDEBUG")
endif()
if(APPLE)
    set(SBK_PYTHON_LIBRARIES "-undefined dynamic_lookup")
endif()

if(CMAKE_VERSION VERSION_LESS 2.8)
    set(SBK_PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_PATH})
else()
    set(SBK_PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
endif()

add_subdirectory(libshiboken)
add_subdirectory(doc)

# deps found, compile the generator.
if (Qt5Core_FOUND AND PYTHONINTERP_FOUND)
    add_subdirectory(generator)
    add_subdirectory(shibokenmodule)

    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
else()
    message(WARNING "Some dependencies were not found, shiboken2 generator compilation disabled!")
endif()

add_subdirectory(data)

# dist target
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${shiboken2_VERSION})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


