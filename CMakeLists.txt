include(icecc.cmake)
project(shiboken)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor REQUIRED)
find_package(GeneratorRunner REQUIRED)
find_package(Boost COMPONENTS graph REQUIRED)

add_definitions(${QT_DEFINITIONS})

option(AVOID_PROTECTED_HACK "Avoid protected hack on generated bindings." FALSE)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /w /EHsc- /GS- /GR- /DLIBSHIBOKEN_BUILD -D_SCL_SECURE_NO_WARNINGS")
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "-Wall -Wno-strict-aliasing -DNDEBUG -O2 -Wl,-O1 -Wl,--hash-style=gnu -fvisibility=hidden")
    else()
        set(CMAKE_CXX_FLAGS "-g -Wall -Wno-strict-aliasing -fvisibility=hidden")
    endif()
endif()
if (WIN32 OR AVOID_PROTECTED_HACK)
    message(STATUS "Avoiding protected hack!")
    add_definitions("-DAVOID_PROTECTED_HACK")
else()
    message(STATUS "Using protected hack!")
endif()

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

set(shiboken_VERSION 0.1)
set(CMAKE_BUILD_TYPE Debug)

set(shiboken_SRC
cppgenerator.cpp
headergenerator.cpp
overloaddata.cpp
shiboken.cpp
shibokengenerator.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}/..
                    ${GENERATORRUNNER_INCLUDE_DIR}
                    ${GENERATORRUNNER_INCLUDE_DIR}/..
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
		    ${Boost_INCLUDE_DIR})

add_library(shiboken_generator SHARED ${shiboken_SRC})

target_link_libraries(shiboken_generator
                      ${APIEXTRACTOR_LIBRARY}
                      ${GENERATORRUNNER_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTXML_LIBRARY})

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

install(TARGETS shiboken_generator DESTINATION "${GENERATORRUNNER_PLUGIN_DIR}")
install(TARGETS shiboken DESTINATION bin)

