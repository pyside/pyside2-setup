project(boostpythongenerator)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor REQUIRED)

add_definitions(${QT_DEFINITIONS})

set(boostpythongenerator_VERSION 0.2)
configure_file(boostpythongeneratorversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/boostpythongeneratorversion.h @ONLY)
set(CMAKE_BUILD_TYPE Debug)

set(boostpythongenerator_SRC
boostpythongenerator.cpp
convertergenerator.cpp
docgenerator.cpp
hppgenerator.cpp
cppgenerator.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}/..
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(libboostpythongenerator STATIC ${boostpythongenerator_SRC})
target_link_libraries(libboostpythongenerator
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTXML_LIBRARY})
add_executable(boostpythongenerator main.cpp)
target_link_libraries(boostpythongenerator libboostpythongenerator)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# "make dist", in fact "make package_source"
set(CPACK_SOURCE_PACKAGE_FILE_NAME "boostpythongenerator-${boostpythongenerator_VERSION}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES "~$" ".svn" "debian/" "build/" ".swp$" "*.kdev4")
include(CPack)

install(TARGETS boostpythongenerator DESTINATION bin)

enable_testing()
add_subdirectory(libbindgen)
add_subdirectory(tests)

