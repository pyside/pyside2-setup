Include(icecc.cmake)
project(shiboken)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/
                      ${CMAKE_MODULE_PATH})

find_package(Qt4 4.5.0)
find_package(ApiExtractor 0.10.4 REQUIRED)
find_package(GeneratorRunner 0.6.11 REQUIRED)
find_package(PythonLibs)
find_package(PythonInterpWithDebug)

add_definitions(${QT_DEFINITIONS})

set(shiboken_MAJOR_VERSION "1")
set(shiboken_MINOR_VERSION "0")
set(shiboken_MICRO_VERSION "5")
set(shiboken_VERSION "${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}.${shiboken_MICRO_VERSION}")

option(BUILD_TESTS "Build tests." TRUE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t- /GR /EHsc /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS")
else()
    if(CMAKE_HOST_UNIX)
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

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

if (WIN32)
    set(PATH_SEP "\;")
else()
    set(PATH_SEP ":")
endif()

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

set(SHIBOKEN_BUILD_TYPE "Release")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT PYTHON_DEBUG_LIBRARIES)
        message(FATAL_ERROR "Python debug library not found. Try compile shiboken with -DCMAKE_BUILD_TYPE=Release")
    endif()
    if(NOT PYTHON_WITH_DEBUG)
        message(WARNING "Compiling shiboken with debug enabled, but the python executable was not compiled with debug support.")
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
if (QT4_FOUND AND ApiExtractor_FOUND AND GeneratorRunner_FOUND AND PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
    add_subdirectory(generator)

    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
else()
    message(WARNING "Some dependencies were not found, shiboken generator compilation disabled!")
endif()

add_subdirectory(data)

# dist target
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${shiboken_VERSION})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2.\n"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


