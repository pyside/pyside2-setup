project(generatorrunner)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor REQUIRED)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /w /EHsc- /GS- /GR- /DGENRUNNER_BUILD -D_SCL_SECURE_NO_WARNINGS")
else (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden")
endif(MSVC)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)


# lib generator version
set(generator_MAJOR_VERSION "0")
set(generator_MINOR_VERSION "3")
set(generator_MICRO_VERSION "4")
set(generator_VERSION "${generator_MAJOR_VERSION}.${generator_MINOR_VERSION}.${generator_MICRO_VERSION}")

set(generator_plugin_DIR ${LIB_INSTALL_DIR}/generatorrunner)

add_definitions(${QT_DEFINITIONS})

configure_file(generatorrunnerconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/generatorrunnerconfig.h" @ONLY)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(genrunner SHARED generator.cpp)
set_target_properties(genrunner PROPERTIES VERSION ${generator_VERSION})
target_link_libraries(genrunner ${QT_QTCORE_LIBRARY} ${APIEXTRACTOR_LIBRARY})

add_executable(generatorrunner main.cpp)
target_link_libraries(generatorrunner
                      genrunner
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY})

# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generatorrunner.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner.pc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/FindGeneratorRunner.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/FindGeneratorRunner.cmake" @ONLY)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")



set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${generator_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

install(TARGETS genrunner LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                          ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                          RUNTIME DESTINATION bin)
install(TARGETS generatorrunner DESTINATION bin)
install(FILES generator.h DESTINATION include/generatorrunner)
install(FILES generatorrunnermacros.h DESTINATION include/generatorrunner)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner.pc" DESTINATION
        "${LIB_INSTALL_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/FindGeneratorRunner.cmake"
        DESTINATION "share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules")

#FIXME: manpages should be installed only on Linux targets
file(GLOB manpages "${CMAKE_CURRENT_SOURCE_DIR}/*.1")
install(FILES ${manpages} DESTINATION share/man/man1)

enable_testing()

add_subdirectory(generators)
add_subdirectory(tests)

