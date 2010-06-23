include(icecc.cmake)
project(shiboken)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor 0.6.1 REQUIRED)
find_package(GeneratorRunner 0.5 REQUIRED)

add_definitions(${QT_DEFINITIONS})

option(AVOID_PROTECTED_HACK "Avoid protected hack on generated bindings." FALSE)
option(BUILD_TESTS "Build tests." TRUE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /w /EHsc- /GS- /GR- /DLIBSHIBOKEN_BUILD -D_SCL_SECURE_NO_WARNINGS")
elseif(CMAKE_HOST_UNIX)
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimization library size and performance. Only available on Release Mode" 0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    if(ENABLE_GCC_OPTIMIZATION)
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -Wno-strict-aliasing -Wl,-O1")
        if(NOT CMAKE_HOST_APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--hash-style=gnu")
        endif()
    endif()
endif()

if(WIN32 OR AVOID_PROTECTED_HACK)
    message(STATUS "Avoiding protected hack!")
    add_definitions("-DAVOID_PROTECTED_HACK")
else()
    message(STATUS "Using protected hack!")
endif()

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

set(shiboken_MAJOR_VERSION "0")
set(shiboken_MINOR_VERSION "3")
set(shiboken_MICRO_VERSION "3")
set(shiboken_VERSION "${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}.${shiboken_MICRO_VERSION}")

set(shiboken_SRC
cppgenerator.cpp
headergenerator.cpp
overloaddata.cpp
shiboken.cpp
shibokengenerator.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${GENERATORRUNNER_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(shiboken_generator SHARED ${shiboken_SRC})

target_link_libraries(shiboken_generator
                      ${APIEXTRACTOR_LIBRARY}
                      ${GENERATORRUNNER_LIBRARY}
                      ${QT_QTCORE_LIBRARY})

add_executable(shiboken main.cpp)
target_link_libraries(shiboken ${QT_QTCORE_LIBRARY})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

if (BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(libshiboken)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${shiboken_VERSION})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2.\n"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_dependencies(shiboken shiboken_generator)

install(TARGETS shiboken_generator DESTINATION "${GENERATORRUNNER_PLUGIN_DIR}")
install(TARGETS shiboken DESTINATION bin)

