include(icecc.cmake)
project(apiextractor)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)

find_package(LibXml2 2.6.32)
find_package(LibXslt 1.1.19)

option(DISABLE_DOCSTRINGS "Disable documentation extraction." FALSE)
option(BUILD_TESTS "Build tests." TRUE)
option(INSTALL_TESTS "Install tests" FALSE)
option(ENABLE_VERSION_SUFFIX "Used to use current version in suffix to generated files. This is used to allow multiples versions installed simultaneous." FALSE)

if (NOT DISABLE_DOCSTRINGS)
    if (NOT LIBXSLT_FOUND OR NOT LIBXML2_FOUND)
        set(DISABLE_DOCSTRINGS TRUE)
        message(WARNING "libxslt and/or libxml not found, disabling support to doc strings!")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t- /GR /EHsc /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS")
elseif(CMAKE_HOST_UNIX)
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimize library size and performance. Only available on Release Mode" 0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    if(ENABLE_GCC_OPTIMIZATION)
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -Wno-strict-aliasing -Wl,-O1")
        if (NOT CMAKE_HOST_APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--hash-style=gnu")
        endif()
    endif()
    if(NOT CMAKE_HOST_APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symbols.filter")
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(BUILD_TESTS)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)
endif ()

set(apiextractor_MAJOR_VERSION 0)
set(apiextractor_MINOR_VERSION 10)
set(apiextractor_MICRO_VERSION 7)
set(apiextractor_VERSION "${apiextractor_MAJOR_VERSION}.${apiextractor_MINOR_VERSION}.${apiextractor_MICRO_VERSION}")
configure_file(apiextractorversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/apiextractorversion.h @ONLY)
set(apiextractor_SOVERSION ${apiextractor_MAJOR_VERSION}.${apiextractor_MINOR_VERSION})
set(QT_USE_QTCORE 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DRXX_ALLOCATOR_INIT_0)

if(ENABLE_VERSION_SUFFIX)
    set(apiextractor_SUFFIX "-${apiextractor_MAJOR_VERSION}.${apiextractor_MINOR_VERSION}")
else()
    set(apiextractor_SUFFIX "")
endif()

set(apiextractor_SRC
apiextractor.cpp
abstractmetabuilder.cpp
abstractmetalang.cpp
asttoxml.cpp
fileout.cpp
graph.cpp
reporthandler.cpp
typeparser.cpp
typesystem.cpp
include.cpp
typedatabase.cpp
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
parser/rpp/preprocessor.cpp
)

if (NOT DISABLE_DOCSTRINGS)
    set(apiextractor_SRC
        ${apiextractor_SRC}
        docparser.cpp
        doxygenparser.cpp
        qtdocparser.cpp
    )
    set(APIEXTRACTOR_EXTRA_INCLUDES ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
    set(APIEXTRACTOR_EXTRA_LIBRARIES ${LIBXSLT_LIBRARIES} ${LIBXML2_LIBRARIES})
else()
    set(APIEXTRACTOR_EXTRA_INCLUDES "")
    set(APIEXTRACTOR_EXTRA_LIBRARIES "")
endif()

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

qt4_add_resources(apiextractor_RCCS_SRC generator.qrc)
qt4_automoc(apiextractor_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/rpp
                    ${QT_INCLUDE_DIR}
                    ${APIEXTRACTOR_EXTRA_INCLUDES}
                    )

add_library(apiextractor SHARED ${apiextractor_SRC} ${apiextractor_RCCS_SRC})
target_link_libraries(apiextractor ${APIEXTRACTOR_EXTRA_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTXMLPATTERNS_LIBRARY} ${QT_QTXML_LIBRARY})
set_target_properties(apiextractor PROPERTIES VERSION ${apiextractor_VERSION}
                                              SOVERSION ${apiextractor_SOVERSION}
                                              OUTPUT_NAME "apiextractor${apiextractor_SUFFIX}"
                                              DEFINE_SYMBOL APIEXTRACTOR_EXPORTS)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${apiextractor_VERSION})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2.\n"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(root_HEADERS
apiextractormacros.h
abstractmetalang.h
apiextractor.h
graph.h
reporthandler.h
typesystem.h
fileout.h
docparser.h
qtdocparser.h
include.h
typedatabase.h
)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
add_subdirectory(data)
add_subdirectory(doc)

install(FILES ${root_HEADERS} DESTINATION include/apiextractor${apiextractor_SUFFIX})
install(TARGETS apiextractor EXPORT apiextractor
                             LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                             ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                             RUNTIME DESTINATION bin)
