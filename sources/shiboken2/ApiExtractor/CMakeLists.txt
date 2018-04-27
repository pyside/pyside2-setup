project(apiextractor)

cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
find_package(LibXml2 2.6.32)
find_package(LibXslt 1.1.19)

option(DISABLE_DOCSTRINGS "Disable documentation extraction." FALSE)

set (USE_LIBXSLT 0)
if (NOT DISABLE_DOCSTRINGS)
    if (LIBXSLT_FOUND AND LIBXML2_FOUND)
        add_definitions(-DHAVE_LIBXSLT)
        set (USE_LIBXSLT 1)
    else()
        message(WARNING "libxslt and/or libxml not found, falling back to QtXmlPatterns (QTBUG-66925)")
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(BUILD_TESTS)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)
endif ()

set(QT_USE_QTCORE 1)
set(QT_USE_QTXML 1)
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DRXX_ALLOCATOR_INIT_0)

# Pass the path to the clang includes dir, so it headers like stdarg.h.
if(UNIX AND NOT APPLE)
    add_definitions(-DCLANG_BUILTIN_INCLUDES_DIR="${CLANG_BUILTIN_INCLUDES_DIR}")
else()
    add_definitions(-DCLANG_BUILTIN_INCLUDES_DIR="")
endif()

set(apiextractor_SRC
apiextractor.cpp
abstractmetabuilder.cpp
abstractmetalang.cpp
fileout.cpp
graph.cpp
reporthandler.cpp
typeparser.cpp
typesystem.cpp
include.cpp
typedatabase.cpp
# Clang
clangparser/compilersupport.cpp
clangparser/clangparser.cpp
clangparser/clangbuilder.cpp
clangparser/clangdebugutils.cpp
clangparser/clangutils.cpp
# Old parser
parser/codemodel.cpp
parser/enumvalue.cpp
)

set(APIEXTRACTOR_EXTRA_INCLUDES ${CLANG_EXTRA_INCLUDES})
set(APIEXTRACTOR_EXTRA_LIBRARIES ${CLANG_EXTRA_LIBRARIES})

add_subdirectory(doc)
if (NOT DISABLE_DOCSTRINGS)
    set(apiextractor_SRC
        ${apiextractor_SRC}
        docparser.cpp
        doxygenparser.cpp
        qtdocparser.cpp
    )
    set(APIEXTRACTOR_EXTRA_INCLUDES ${APIEXTRACTOR_EXTRA_INCLUDES})
    set(APIEXTRACTOR_EXTRA_LIBRARIES ${APIEXTRACTOR_EXTRA_LIBRARIES})
    if (USE_LIBXSLT)
        list(APPEND APIEXTRACTOR_EXTRA_INCLUDES ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
        list(APPEND APIEXTRACTOR_EXTRA_LIBRARIES ${LIBXSLT_LIBRARIES} ${LIBXML2_LIBRARIES})
    endif()
endif()

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

set(CMAKE_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/rpp
                    ${APIEXTRACTOR_EXTRA_INCLUDES}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Xml_INCLUDE_DIRS}
                    )

add_library(apiextractor STATIC ${apiextractor_SRC} ${apiextractor_RCCS_SRC})
target_link_libraries(apiextractor
                      ${Qt5Xml_LIBRARIES}
                      ${Qt5XmlPatterns_LIBRARIES}
                      ${APIEXTRACTOR_EXTRA_LIBRARIES}
                      )

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
