project(apiextractor)

cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS graph REQUIRED)
find_package(Qt4 4.5.0 REQUIRED)
find_package(PkgConfig)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0>=2.6.32)
pkg_check_modules(LIBXSLT REQUIRED libxslt>=1.1.19)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DAPIEXTRACTOR_ENABLE_DUPLICATE_ENUM_VALUES")

set(apiextractor_VERSION 0.2)
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
generator.cpp
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

qt4_add_resources(apiextractor_RCCS_SRC generator.qrc)

set(apiextractor_MOC_HEADERS
    fileout.h
    generator.h
)

qt4_wrap_cpp(apiextractor_MOC_SRC ${apiextractor_MOC_HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/rpp
                    ${QT_INCLUDE_DIR} ${Boost_INCLUDE_DIR}
                    ${LIBXSLT_INCLUDE_DIRS}
                    ${LIBXML2_INCLUDE_DIRS}
                    )

add_library(apiextractor SHARED ${apiextractor_SRC} ${apiextractor_MOC_SRC} ${apiextractor_RCCS_SRC})
target_link_libraries(apiextractor ${Boost_GRAPH_LIBRARY} ${LIBXSLT_LIBRARIES} ${LIBXML2_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTXMLPATTERNS_LIBRARY} ${QT_QTXML_LIBRARY})
set_target_properties(apiextractor PROPERTIES SOVERSION ${apiextractor_VERSION})

# create pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/apiextractor.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/apiextractor.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindApiExtractor.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FindApiExtractor.cmake @ONLY)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# "make dist", in fact "make package_source"
#set(CPACK_SOURCE_PACKAGE_FILE_NAME "libapiextractor-${apiextractor_VERSION}")
#set(CPACK_SOURCE_GENERATOR TGZ)
#set(CPACK_SOURCE_IGNORE_FILES "~$" ".svn" "debian/" "build/" ".swp$" "*.kdev4")
#include(CPack)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${apiextractor_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(root_HEADERS
abstractmetalang.h
generator.h
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
# TODO We do not need to expose these internal headers!
# install(DIRECTORY parser DESTINATION include/apiextractor
#         FILES_MATCHING PATTERN "*.h"
#         PATTERN ".svn" EXCLUDE
#         )
install(TARGETS apiextractor LIBRARY DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/apiextractor.pc DESTINATION lib/pkgconfig)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FindApiExtractor.cmake
        DESTINATION share/cmake-2.6/Modules)

