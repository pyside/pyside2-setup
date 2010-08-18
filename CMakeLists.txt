project(generatorrunner)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 REQUIRED)
find_package(ApiExtractor 0.7 REQUIRED)

option(BUILD_TESTS "Build tests." TRUE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t- /EHsc /DWIN32 /D_WINDOWS /DGENRUNNER_BUILD /D_SCL_SECURE_NO_WARNINGS")
elseif(CMAKE_HOST_UNIX)
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimize library size and performance. Only available on Release Mode" 0)
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

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)


# lib generator version
set(generator_MAJOR_VERSION "0")
set(generator_MINOR_VERSION "6")
set(generator_MICRO_VERSION "1")
set(generator_VERSION "${generator_MAJOR_VERSION}.${generator_MINOR_VERSION}.${generator_MICRO_VERSION}")
set(generator_SOVERSION "${generator_MAJOR_VERSION}.${generator_MINOR_VERSION}")

set(generator_plugin_DIR ${LIB_INSTALL_DIR}/generatorrunner)

add_definitions(${QT_DEFINITIONS})

configure_file(generatorrunnerconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/generatorrunnerconfig.h" @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(genrunner SHARED generator.cpp)
set_target_properties(genrunner PROPERTIES VERSION ${generator_VERSION})
target_link_libraries(genrunner ${QT_QTCORE_LIBRARY} ${APIEXTRACTOR_LIBRARY})
set_target_properties(genrunner PROPERTIES VERSION ${generator_VERSION} SOVERSION ${generator_SOVERSION})

add_executable(generatorrunner main.cpp)
target_link_libraries(generatorrunner
                      genrunner
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY})

# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generatorrunner.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner.pc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GeneratorRunnerConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfig.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GeneratorRunnerConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfigVersion.cmake" @ONLY)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")



set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${generator_VERSION})
add_custom_target(dist
    COMMAND mkdir -p "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}" &&
            git log > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ChangeLog" &&
            git archive --prefix=${ARCHIVE_NAME}/ HEAD --format=tar --output="${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            tar -C "${CMAKE_BINARY_DIR}" --owner=root --group=root -r "${ARCHIVE_NAME}/ChangeLog" -f "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            bzip2 -f9 "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar" &&
            echo "Source package created at ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2.\n"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(TARGETS genrunner LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                          ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                          RUNTIME DESTINATION bin)
install(TARGETS generatorrunner DESTINATION bin)
install(FILES generator.h DESTINATION include/generatorrunner)
install(FILES generatorrunnermacros.h DESTINATION include/generatorrunner)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner.pc" DESTINATION
        "${LIB_INSTALL_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfig.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/GeneratorRunner-${generator_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/GeneratorRunner-${generator_VERSION}")

#FIXME: manpages should be installed only on Linux targets
file(GLOB manpages "${CMAKE_CURRENT_SOURCE_DIR}/*.1")
install(FILES ${manpages} DESTINATION share/man/man1)

if (BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(generators)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

