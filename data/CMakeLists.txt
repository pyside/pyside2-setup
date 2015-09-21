if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()

add_custom_target(data)
add_dependencies(data shiboken2)
get_target_property(SHIBOKEN_GENERATOR shiboken2 OUTPUT_NAME)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ShibokenConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfig.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ShibokenConfig-spec.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfig${PYTHON_SUFFIX}.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ShibokenConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfigVersion.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shiboken.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shiboken${shiboken2_SUFFIX}.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfig.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfig${PYTHON_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ShibokenConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/shiboken${shiboken2_SUFFIX}.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

