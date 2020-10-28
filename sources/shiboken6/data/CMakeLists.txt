if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()

add_custom_target(data)
add_dependencies(data shiboken6)
get_target_property(SHIBOKEN_GENERATOR shiboken6 OUTPUT_NAME)

include(CMakePackageConfigHelpers)

# Build-tree / super project package config file.
set(SHIBOKEN_PYTHON_MODULE_DIR "${shiboken6_BINARY_DIR}/shibokenmodule")
set(SHIBOKEN_SHARED_LIBRARY_DIR "${shiboken6_BINARY_DIR}/libshiboken")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Shiboken6Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Shiboken6Config${PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
     PATH_VARS SHIBOKEN_PYTHON_MODULE_DIR SHIBOKEN_SHARED_LIBRARY_DIR
     INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
)

# Install-tree / relocatable package config file.
set(SHIBOKEN_PYTHON_MODULE_DIR "${PYTHON_SITE_PACKAGES}/shiboken6")
if (WIN32)
    set(SHIBOKEN_SHARED_LIBRARY_DIR "${BIN_INSTALL_DIR}")
else()
    set(SHIBOKEN_SHARED_LIBRARY_DIR "${LIB_INSTALL_DIR}")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Shiboken6Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install/Shiboken6Config${PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken6-${shiboken6_VERSION}"
     PATH_VARS SHIBOKEN_PYTHON_MODULE_DIR SHIBOKEN_SHARED_LIBRARY_DIR
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken6Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken6Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken6ConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken6ConfigVersion.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shiboken6.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shiboken6${shiboken6_SUFFIX}.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken6Config.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken6-${shiboken6_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/install/Shiboken6Config${PYTHON_CONFIG_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken6-${shiboken6_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken6ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken6-${shiboken6_VERSION}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/shiboken_helpers.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken6-${shiboken6_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/shiboken6${shiboken6_SUFFIX}.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

