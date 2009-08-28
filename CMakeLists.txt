project(shiboken)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor REQUIRED)

add_definitions(${QT_DEFINITIONS})

set(CMAKE_CXX_FLAGS_RELEASE "-Wall -DNDEBUG -O2 -Wl,-O1 -Wl,--hash-style=gnu")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

set(shiboken_VERSION 0.1)
set(CMAKE_BUILD_TYPE Debug)

set(shiboken_SRC
shibokengenerator.cpp
headergenerator.cpp
cppgenerator.cpp
polymorphicdata.cpp
shiboken.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}/..
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(shiboken_generator SHARED ${shiboken_SRC})

target_link_libraries(shiboken_generator
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTXML_LIBRARY}
                      -lgenrunner) # FIXME libgenrunner needs a cmake FindPkg script!

add_executable(shiboken main.cpp)
target_link_libraries(shiboken ${QT_QTCORE_LIBRARY})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

enable_testing()

add_subdirectory(libshiboken)
add_subdirectory(tests)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${shiboken_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_dependencies(shiboken shiboken_generator)

install(TARGETS shiboken_generator DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS shiboken DESTINATION bin)

