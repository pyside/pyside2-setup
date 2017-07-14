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
    find_package(PythonInterp ${USE_PYTHON_VERSION} REQUIRED)
    find_package(PythonLibs ${USE_PYTHON_VERSION} REQUIRED)
else()
    find_package(PythonInterp 2.6)
    find_package(PythonLibs 2.6)
endif()

set(CLANG_DIR "")
set(CLANG_DIR_SOURCE "")

if (DEFINED ENV{LLVM_INSTALL_DIR})
    set(CLANG_DIR $ENV{LLVM_INSTALL_DIR})
    set(CLANG_DIR_SOURCE "LLVM_INSTALL_DIR")
elseif (DEFINED ENV{CLANG_INSTALL_DIR})
    set(CLANG_DIR $ENV{CLANG_INSTALL_DIR})
    set(CLANG_DIR_SOURCE "CLANG_INSTALL_DIR")
else ()
    EXEC_PROGRAM("llvm-config" ARGS "--prefix" OUTPUT_VARIABLE CLANG_DIR)
    set(CLANG_DIR_SOURCE "llvm-config")
    if (NOT "${CLANG_DIR}" STREQUAL "")
        EXEC_PROGRAM("llvm-config" ARGS "--version" OUTPUT_VARIABLE CLANG_VERSION)
        if (CLANG_VERSION VERSION_LESS 3.9)
            message(FATAL_ERROR "LLVM version 3.9 is required (llvm-config detected ${CLANG_VERSION} at ${CLANG_DIR}).")
        endif()
    endif()
endif()

if ("${CLANG_DIR}" STREQUAL "")
    message(FATAL_ERROR "Unable to detect CLANG location by checking LLVM_INSTALL_DIR, CLANG_INSTALL_DIR or running llvm-config.")
elseif (NOT IS_DIRECTORY ${CLANG_DIR})
    message(FATAL_ERROR "${CLANG_DIR} detected by ${CLANG_DIR_SOURCE} does not exist.")
endif()

set(CLANG_LIB_NAME "clang")
if(MSVC)
    set(CLANG_LIB_NAME "libclang")
endif()

find_library(CLANG_LIBRARY ${CLANG_LIB_NAME} HINTS ${CLANG_DIR}/lib)
if (NOT EXISTS ${CLANG_LIBRARY})
    message(FATAL_ERROR "Unable to find Clang library ${CLANG_LIB_NAME} in ${CLANG_DIR}.")
endif()

message(STATUS "CLANG: ${CLANG_DIR}, ${CLANG_LIBRARY} detected by ${CLANG_DIR_SOURCE}")

set(CLANG_EXTRA_INCLUDES ${CLANG_DIR}/include)
set(CLANG_EXTRA_LIBRARIES ${CLANG_LIBRARY})

## For debugging the PYTHON* variables
message("PYTHONLIBS_FOUND: " ${PYTHONLIBS_FOUND})
message("PYTHON_LIBRARIES: " ${PYTHON_LIBRARIES})
message("PYTHON_INCLUDE_DIRS: " ${PYTHON_INCLUDE_DIRS})
message("PYTHON_DEBUG_LIBRARIES: " ${PYTHON_DEBUG_LIBRARIES})
message("PYTHONINTERP_FOUND: " ${PYTHONINTERP_FOUND})
message("PYTHON_EXECUTABLE: " ${PYTHON_EXECUTABLE})
message("PYTHON_VERSION: " ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH})

macro(get_python_extension_suffix)
  # Result of imp.get_suffixes() depends on the platform, but generally looks something like:
  # [('.cpython-34m-x86_64-linux-gnu.so', 'rb', 3), ('.cpython-34m.so', 'rb', 3),
  # ('.abi3.so', 'rb', 3), ('.so', 'rb', 3), ('.py', 'r', 1), ('.pyc', 'rb', 2)]
  # We pick the first most detailed one, strip of the file extension part.

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import imp, re
       first_suffix = imp.get_suffixes()[0][0]
       res = re.search(r'^(.+)\\.', first_suffix)
       if res:
           first_suffix = res.group(1)
       else:
           first_suffix = ''
       print(first_suffix)
       "
    OUTPUT_VARIABLE PYTHON_EXTENSION_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("PYTHON_EXTENSION_SUFFIX: " ${PYTHON_EXTENSION_SUFFIX})
endmacro()


if (NOT PYTHON_EXTENSION_SUFFIX)
  get_python_extension_suffix()
endif()

if (NOT PYTHON_CONFIG_SUFFIX)
  if (PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_CONFIG_SUFFIX "-python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
      if (PYTHON_EXTENSION_SUFFIX)
          set(PYTHON_CONFIG_SUFFIX "${PYTHON_CONFIG_SUFFIX}${PYTHON_EXTENSION_SUFFIX}")
      endif()
  elseif (PYTHON_VERSION_MAJOR EQUAL 3)
      set(PYTHON_CONFIG_SUFFIX "${PYTHON_EXTENSION_SUFFIX}")
  endif()
endif()

if (NOT PYTHON_SHARED_LIBRARY_SUFFIX)
  set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_CONFIG_SUFFIX}")

  # Append a "v" to disambiguate the python version and the shiboken version in the
  # shared library file name.
  if (APPLE AND PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_SHARED_LIBRARY_SUFFIX}v")
  endif()
endif()


message(STATUS "PYTHON_EXTENSION_SUFFIX:      ${PYTHON_EXTENSION_SUFFIX}")
message(STATUS "PYTHON_CONFIG_SUFFIX:         ${PYTHON_CONFIG_SUFFIX}")
message(STATUS "PYTHON_SHARED_LIBRARY_SUFFIX: ${PYTHON_SHARED_LIBRARY_SUFFIX}")


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
set(CMAKE_CXX_STANDARD 11)

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


# Detect if the python libs were compiled in debug mode
# On Linux distros there is no standard way to check that.
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
        is_py_debug = False
        import sys
        try:
            sys_py_debug = sys.pydebug
            if sys_py_debug:
                is_py_debug = True
        except:
            pass

        try:
            from distutils import sysconfig
            config_py_debug = sysconfig.get_config_var('Py_DEBUG')
            if config_py_debug:
                is_py_debug = True
        except:
            pass

        print(bool(is_py_debug))
        "
    OUTPUT_VARIABLE PYTHON_WITH_DEBUG
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Detect if python interpeter was compiled with COUNT_ALLOCS define
# Linux distros are inconsistent in setting the sysconfig.get_config_var('COUNT_ALLOCS') value
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
        count_allocs = False
        import sys
        try:
            if sys.getcounts:
                count_allocs = True
        except:
            pass

        print(bool(count_allocs))
        "
    OUTPUT_VARIABLE PYTHON_WITH_COUNT_ALLOCS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

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

    if (PYTHON_WITH_COUNT_ALLOCS)
        add_definitions("-DCOUNT_ALLOCS")
        set(SBK_ADD_PY_DEBUG_DEFINITION "${SBK_ADD_PY_DEBUG_DEFINITION} \nadd_definitions(\"-DCOUNT_ALLOCS\")")
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION "${SBK_PKG_CONFIG_PY_DEBUG_DEFINITION} -DCOUNT_ALLOCS")
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


