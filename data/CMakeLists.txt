if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()

add_custom_target(data)
add_dependencies(data shiboken2)
get_target_property(SHIBOKEN_GENERATOR shiboken2 OUTPUT_NAME)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2Config-spec.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config${PYTHON_EXTENSION_SUFFIX}.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shiboken2ConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2ConfigVersion.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shiboken2.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shiboken2${shiboken2_SUFFIX}.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2Config${PYTHON_EXTENSION_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Shiboken2ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/shiboken2${shiboken2_SUFFIX}.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

