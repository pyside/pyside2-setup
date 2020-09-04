project(apiextractor)

cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

set(CMAKE_AUTOMOC ON)

set(apiextractor_SRC
apiextractor.cpp
abstractmetabuilder.cpp
abstractmetalang.cpp
fileout.cpp
graph.cpp
messages.cpp
reporthandler.cpp
sourcelocation.cpp
typeparser.cpp
typesystem.cpp
typesystemparser.cpp
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
xmlutils.cpp
)

find_package(Qt${QT_MAJOR_VERSION}Xml 5.12)
find_package(LibXml2 2.6.32)
find_package(LibXslt 1.1.19)

set(HAS_LIBXSLT 0)
if (LIBXSLT_FOUND AND LIBXML2_FOUND)
    set(HAS_LIBXSLT 1)
endif()

if(NOT HAS_LIBXSLT)
    set(DISABLE_DOCSTRINGS TRUE)
    message(WARNING
            "Documentation will not be built due to missing dependency (no Qt5XmlPatterns found).")
endif()

# Export to parent scope so that generator/CMakeLists.txt gets it
set(DISABLE_DOCSTRINGS ${DISABLE_DOCSTRINGS} PARENT_SCOPE)

add_library(apiextractor STATIC ${apiextractor_SRC})
target_include_directories(apiextractor PRIVATE ${CLANG_INCLUDE_DIRS}
                                                ${CMAKE_CURRENT_SOURCE_DIR}
                                                ${CMAKE_CURRENT_BINARY_DIR}
                                                ${CMAKE_CURRENT_SOURCE_DIR}/parser)
target_link_libraries(apiextractor PUBLIC Qt${QT_MAJOR_VERSION}::Core)
target_link_libraries(apiextractor PRIVATE libclang)

if (HAS_LIBXSLT)
    target_compile_definitions(apiextractor PUBLIC HAVE_LIBXSLT)
    target_sources(apiextractor PRIVATE xmlutils_libxslt.cpp)
    target_include_directories(apiextractor
                               PRIVATE ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
    target_link_libraries(apiextractor
                          PRIVATE ${LIBXSLT_LIBRARIES} ${LIBXML2_LIBRARIES})
endif()

if (NOT DISABLE_DOCSTRINGS)
    target_sources(apiextractor PRIVATE docparser.cpp
                                        doxygenparser.cpp
                                        qtdocparser.cpp)
endif()

target_compile_definitions(apiextractor PRIVATE CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")

set(LIB_INSTALL_DIR "lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

if (BUILD_TESTS)
    find_package(Qt${QT_MAJOR_VERSION}Test 5.12 REQUIRED)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
