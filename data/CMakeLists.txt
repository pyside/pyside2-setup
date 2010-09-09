# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/apiextractor.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/apiextractor${apiextractor_SUFFIX}.pc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ApiExtractorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/ApiExtractorConfig.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ApiExtractorConfigVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/ApiExtractorConfigVersion.cmake" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/apiextractor${apiextractor_SUFFIX}.pc" DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ApiExtractorConfig.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/ApiExtractor-${apiextractor_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ApiExtractorConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/ApiExtractor-${apiextractor_VERSION}")
