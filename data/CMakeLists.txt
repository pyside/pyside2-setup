# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generatorrunner.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner${generator_SUFFIX}.pc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GeneratorRunnerConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfig.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GeneratorRunnerConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfigVersion.cmake" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner${generator_SUFFIX}.pc" DESTINATION
        "${LIB_INSTALL_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfig.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/GeneratorRunner-${generator_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GeneratorRunnerConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/GeneratorRunner-${generator_VERSION}")

#FIXME: manpages should be installed only on Linux targets
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/docgenerator.1"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/docgenerator${generator_SUFFIX}.1")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/generatorrunner.1"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner${generator_SUFFIX}.1")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generatorrunner.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generatorrunner${generator_SUFFIX}.pc" @ONLY)

file(GLOB manpages "${CMAKE_CURRENT_BINARY_DIR}/*${generator_SUFFIX}.1")
install(FILES ${manpages} DESTINATION share/man/man1)
