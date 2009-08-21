project(boostpythongenerator)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor REQUIRED)

# lib generator version
set(generator_VERSION "0.1")

add_definitions(${QT_DEFINITIONS})

set(boostpythongenerator_VERSION 0.2)
configure_file(boostpythongeneratorversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/boostpythongeneratorversion.h @ONLY)
set(CMAKE_BUILD_TYPE Debug)

set(boostpythongenerator_SRC
main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}/..
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(generator SHARED generator.cpp)
set_target_properties(generator PROPERTIES SOVERSION ${generator_VERSION})
target_link_libraries(generator ${QT_QTCORE_LIBRARY} ${APIEXTRACTOR_LIBRARY})

add_executable(boostpythongenerator ${boostpythongenerator_SRC})
target_link_libraries(boostpythongenerator
                      generator
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")



set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${boostpythongenerator_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(TARGETS boostpythongenerator DESTINATION bin)
install(TARGETS generator DESTINATION lib)
install(FILES generator.h DESTINATION include)

enable_testing()
#add_subdirectory(libbindgen)
# add_subdirectory(tests)
add_subdirectory(generators)
