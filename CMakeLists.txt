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

set(CLANG_DIR $ENV{CLANG_INSTALL_DIR})

if (NOT IS_DIRECTORY ${CLANG_DIR})
    message(FATAL_ERROR "CLANG_INSTALL_DIR is not set or does not point to a valid directory.")
else()
endif()

set(CLANG_LIB_NAME "clang")
if(MSVC)
    set(CLANG_LIB_NAME "libclang")
endif()

find_library(CLANG_LIBRARY ${CLANG_LIB_NAME} HINTS ${CLANG_DIR}/lib)
if (NOT EXISTS ${CLANG_LIBRARY})
    message(FATAL_ERROR "Unable to find Clang library ${CLANG_LIB_NAME} in ${CLANG_DIR}.")
endif()

message(STATUS "CLANG: ${CLANG_DIR}, ${CLANG_LIBRARY}")

set(CLANG_EXTRA_INCLUDES ${CLANG_DIR}/include)
set(CLANG_EXTRA_LIBRARIES ${CLANG_LIBRARY})

## For debugging the PYTHON* variables
#message("PYTHONLIBS_FOUND: " ${PYTHONLIBS_FOUND})
#message("PYTHON_LIBRARIES: " ${PYTHON_LIBRARIES})
#message("PYTHON_INCLUDE_DIRS: " ${PYTHON_INCLUDE_DIRS})
#message("PYTHON_DEBUG_LIBRARIES: " ${PYTHON_DEBUG_LIBRARIES})
#message("PYTHONINTERP_FOUND: " ${PYTHONINTERP_FOUND})
#message("PYTHON_EXECUTABLE: " ${PYTHON_EXECUTABLE})
#message("PYTHON_VERSION_MAJOR: " ${PYTHON_VERSION_MAJOR})
#message("PYTHON_VERSION_MINOR: " ${PYTHON_VERSION_MINOR})
#message("PYTHON_VERSION_PATCH: " ${PYTHON_VERSION_PATCH})

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
        else()
            set(PYTHON_RELEASE_SUFFIX "")
        endif()
        set(PYTHON_EXTENSION_SUFFIX "-python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}${PYTHON_RELEASE_SUFFIX}")
    elseif (PYTHON_VERSION_MAJOR EQUAL 3)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(PYTHON_RELEASE_SUFFIX "dm")
        else()
            set(PYTHON_RELEASE_SUFFIX "m")
        endif()
        set(PYTHON_EXTENSION_SUFFIX ".cpython-${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}${PYTHON_RELEASE_SUFFIX}")
    else()
        message(FATAL_ERROR "Unsupported PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}!")
    endif()
    if(PYTHON_MULTIARCH_SUFFIX)
        set(PYTHON_EXTENSION_SUFFIX "${PYTHON_EXTENSION_SUFFIX}-${PYTHON_MULTIARCH_SUFFIX}")
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


