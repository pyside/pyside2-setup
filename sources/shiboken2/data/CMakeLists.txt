if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()

add_custom_target(data)
add_dependencies(data shiboken2)
get_target_property(SHIBOKEN_GENERATOR shiboken2 OUTPUT_NAME)

include(CMakePackageConfigHelpers)

# Build-tree / super project package config file.
set(SHIBOKEN_PYTHON_MODULE_DIR "${shiboken2_BINARY_DIR}/shibokenmodule")
set(SHIBOKEN_SHARED_LIBRARY_DIR "${shiboken2_BINARY_DIR}/libshiboken")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config${PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
     PATH_VARS SHIBOKEN_PYTHON_MODULE_DIR SHIBOKEN_SHARED_LIBRARY_DIR
     INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
)

# Install-tree / relocatable package config file.
set(SHIBOKEN_PYTHON_MODULE_DIR "${PYTHON_SITE_PACKAGES}/shiboken2")
if (WIN32)
    set(SHIBOKEN_SHARED_LIBRARY_DIR "${BIN_INSTALL_DIR}")
else()
    set(SHIBOKEN_SHARED_LIBRARY_DIR "${LIB_INSTALL_DIR}")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install/Shiboken2Config${PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}"
     PATH_VARS SHIBOKEN_PYTHON_MODULE_DIR SHIBOKEN_SHARED_LIBRARY_DIR
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2ConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2ConfigVersion.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shiboken2.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shiboken2${shiboken2_SUFFIX}.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/install/Shiboken2Config${PYTHON_CONFIG_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/shiboken_helpers.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/shiboken2${shiboken2_SUFFIX}.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

