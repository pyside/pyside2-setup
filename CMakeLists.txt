project(apiextractor)

cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS graph REQUIRED)
find_package(Qt4 4.5.0 REQUIRED)
find_package(PkgConfig)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0>=2.6.32)
pkg_check_modules(LIBXSLT REQUIRED libxslt>=1.1.19)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DAPIEXTRACTOR_ENABLE_DUPLICATE_ENUM_VALUES")

set(apiextractor_MAJOR_VERSION 0)
set(apiextractor_MINOR_VERSION 3)
set(apiextractor_MICRO_VERSION 1)
set(apiextractor_VERSION "${apiextractor_MAJOR_VERSION}.${apiextractor_MINOR_VERSION}.${apiextractor_MICRO_VERSION}")
configure_file(apiextractorversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/apiextractorversion.h @ONLY)
set(QT_USE_QTCORE 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DRXX_ALLOCATOR_INIT_0)

set(CMAKE_BUILD_TYPE Debug)

set(apiextractor_SRC
apiextractor.cpp
abstractmetabuilder.cpp
abstractmetalang.cpp
asttoxml.cpp
docparser.cpp
doxygenparser.cpp
qtdocparser.cpp
fileout.cpp
reporthandler.cpp
typeparser.cpp
typesystem.cpp
parser/ast.cpp
parser/binder.cpp
parser/class_compiler.cpp
parser/codemodel.cpp
parser/codemodel_finder.cpp
parser/compiler_utils.cpp
parser/control.cpp
parser/declarator_compiler.cpp
parser/default_visitor.cpp
parser/dumptree.cpp
parser/lexer.cpp
parser/list.cpp
parser/name_compiler.cpp
parser/parser.cpp
parser/smallobject.cpp
parser/tokens.cpp
parser/type_compiler.cpp
parser/visitor.cpp
parser/rpp/builtin-macros.cpp
parser/rpp/pp-main.cpp
parser/rpp/preprocessor.cpp
)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

qt4_add_resources(apiextractor_RCCS_SRC generator.qrc)
qt4_automoc(apiextractor_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/rpp
                    ${QT_INCLUDE_DIR} ${Boost_INCLUDE_DIR}
                    ${LIBXSLT_INCLUDE_DIRS}
                    ${LIBXML2_INCLUDE_DIRS}
                    )

add_library(apiextractor SHARED ${apiextractor_SRC} ${apiextractor_RCCS_SRC})
target_link_libraries(apiextractor ${Boost_GRAPH_LIBRARY} ${LIBXSLT_LIBRARIES} ${LIBXML2_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTXMLPATTERNS_LIBRARY} ${QT_QTXML_LIBRARY})
set_target_properties(apiextractor PROPERTIES VERSION ${apiextractor_VERSION} SOVERSION ${apiextractor_MAJOR_VERSION})

# create pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/apiextractor.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/apiextractor.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindApiExtractor.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FindApiExtractor.cmake @ONLY)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${apiextractor_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(root_HEADERS
abstractmetalang.h
apiextractor.h
reporthandler.h
typesystem.h
fileout.h
docparser.h
qtdocparser.h
)

enable_testing()
add_subdirectory(tests)

install(FILES ${root_HEADERS} DESTINATION include/apiextractor)
install(TARGETS apiextractor LIBRARY DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/apiextractor.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FindApiExtractor.cmake
        DESTINATION share/cmake-2.6/Modules)

